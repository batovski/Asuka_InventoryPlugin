// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/Inv_ItemComponent.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"

UInv_ItemComponent::UInv_ItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickupMessage = FString("E - Pick Up");
	SetIsReplicatedByDefault(true);
	DynamicFragments.Owner = this;
}

void UInv_ItemComponent::BeginPlay()
{
	Super::BeginPlay();
	if (StaticItemManifestID.IsValid() && StaticItemManifest.GetItemCategory() == EInv_ItemCategory::None)
	{
		StaticItemManifest = UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestID);
	}
}

void UInv_ItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, StaticItemManifestID);
	DOREPLIFETIME(ThisClass, DynamicFragments);
	DOREPLIFETIME(ThisClass, ReplicatedSkeletalMesh);
}

void UInv_ItemComponent::PickedUp()
{
	OnPickedUp();
	GetOwner()->Destroy();
}

void UInv_ItemComponent::InitItemManifest(const FPrimaryAssetId& NewItemManifestID)
{
	StaticItemManifestID = NewItemManifestID;
}

void UInv_ItemComponent::InitDynamicData(const TArray<FInstancedStruct>& NewDynamicFragments)
{
	DynamicFragments.Items.Empty();
	DynamicFragments.Owner = this;
	for (const FInstancedStruct& Fragment : NewDynamicFragments)
	{
		DynamicFragments.Items.Add(FInv_ItemFragmentArrayItem(Fragment));
	}
	DynamicFragments.MarkArrayDirty();
	ApplyDynamicFragmentsToManifest();
}

TArray<FInstancedStruct> UInv_ItemComponent::GetDynamicFragments() const
{
	TArray<FInstancedStruct> Result;
	for (const FInv_ItemFragmentArrayItem& Item : DynamicFragments.Items)
	{
		Result.Add(Item.Fragment);
	}
	return Result;
}
UInv_ItemComponent* UInv_ItemComponent::SpawnPickUpActor(const TSubclassOf<AActor>& ActorToSpawn, const UObject* WorldContextObject, const FVector& SpawnLocation,
	const FRotator& SpawnRotation)
{
	if (!ActorToSpawn || !WorldContextObject) return nullptr;

	AActor* SpawnedACtor = WorldContextObject->GetWorld()->SpawnActor<AActor>(ActorToSpawn, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedACtor)) return nullptr;

	// Set the item manifest, on the spawned actor

	UInv_ItemComponent* ItemComponent = SpawnedACtor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComponent);

	return ItemComponent;
}

FString& UInv_ItemComponent::GetPickupMessage()
{
	return PickupMessage;
}

const FPrimaryAssetId& UInv_ItemComponent::GetStaticItemManifestID() const
{
	return StaticItemManifestID;
}

void UInv_ItemComponent::OnDynamicFragmentUpdated()
{
	ApplyDynamicFragmentsToManifest();
}

void UInv_ItemComponent::UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, const FInv_ItemFragmentArray& NewDynamicFragments)
{
	// Process static fragments first
	for (FInstancedStruct& StaticItemFragment : StaticFragments)
	{
		if (const FInv_ItemFragment* FragmentBase = StaticItemFragment.GetPtr<FInv_ItemFragment>())
		{
			const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
			if (!FragmentsMap.Contains(FragmentTag))
			{
				FragmentsMap.Add(FragmentTag, &StaticItemFragment);
			}
		}
	}

	// Process dynamic fragments - they override static fragments
	for (const FInv_ItemFragmentArrayItem& DynamicFragmentItem : NewDynamicFragments.Items)
	{
		if (const FInv_ItemFragment* FragmentBase = DynamicFragmentItem.Fragment.GetPtr<FInv_ItemFragment>())
		{
			const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
			// Dynamic fragments always override static ones
			FragmentsMap.FindOrAdd(FragmentTag) = const_cast<FInstancedStruct*>(&DynamicFragmentItem.Fragment);
		}
	}
}

void UInv_ItemComponent::ApplyDynamicFragmentsToManifest()
{
	if (StaticItemManifest.GetItemCategory() == EInv_ItemCategory::None)
	{
		StaticItemManifest = UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestID);
	}
	UpdateManifestData(StaticItemManifest.GetFragmentsMutable(), DynamicFragments);
}

void UInv_ItemComponent::SetSkeletalMeshAsset_Implementation(USkeletalMesh* MeshAsset)
{
	ReplicatedSkeletalMesh = MeshAsset;
	OnSkeletalMeshAssetChanged(MeshAsset);
}

void UInv_ItemComponent::OnSkeletalMeshAssetChanged_Implementation(USkeletalMesh* MeshAsset)
{
	if (USkeletalMeshComponent* MeshComponent = GetOwner()->GetComponentByClass<USkeletalMeshComponent>())
	{
		MeshComponent->SetSkeletalMeshAsset(MeshAsset);
	}
}

void UInv_ItemComponent::OnRep_ReplicatedSkeletalMesh()
{
	USkeletalMesh* LoadedMesh = ReplicatedSkeletalMesh.LoadSynchronous();
	if (IsValid(LoadedMesh))
	{
		if (USkeletalMeshComponent* MeshComponent = GetOwner()->GetComponentByClass<USkeletalMeshComponent>())
		{
			MeshComponent->SetSkeletalMeshAsset(LoadedMesh);
		}
	}
}

