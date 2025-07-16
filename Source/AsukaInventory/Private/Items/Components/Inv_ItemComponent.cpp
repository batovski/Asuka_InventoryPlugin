// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/Inv_ItemComponent.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"

UInv_ItemComponent::UInv_ItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickupMessage = FString("E - Pick Up");
	SetIsReplicatedByDefault(true);
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

void UInv_ItemComponent::InitDynamicData(const TArray<TInstancedStruct<FInv_ItemFragment>>& NewDynamicFragments)
{
	DynamicFragments = NewDynamicFragments;
}

UInv_InventoryItem* UInv_ItemComponent::CreateInventoryItemFromComponent(UObject* WorldContextObject)
{
	UInv_InventoryItem* NewItem = NewObject<UInv_InventoryItem>(WorldContextObject, UInv_InventoryItem::StaticClass());;
	NewItem->SetStaticItemManifestAssetId(GetStaticItemManifestID());
	NewItem->LoadStaticItemManifest();
	DynamicFragments.Add(TInstancedStruct<FInv_PickUpFragment>::Make(FInv_PickUpFragment(GetOwner()->GetClass())));
	NewItem->SetDynamicItemFragments(DynamicFragments);
	NewItem->GetItemManifestMutable().Manifest();
	return NewItem;
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
void UInv_ItemComponent::OnRep_DynamicFragments()
{
	ApplyDynamicFragmentsToManifest();
}

void UInv_ItemComponent::ApplyDynamicFragmentsToManifest()
{
	if (StaticItemManifest.GetItemCategory() == EInv_ItemCategory::None)
	{
		StaticItemManifest = UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestID);
	}
	UInv_InventoryItem::UpdateManifestData(StaticItemManifest.GetFragmentsMutable(), DynamicFragments);
}

