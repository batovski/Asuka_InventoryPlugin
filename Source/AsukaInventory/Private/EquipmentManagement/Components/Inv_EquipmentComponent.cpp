// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"


void UInv_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	InitPlayerController();
	SetIsReplicated(true);
}

void UInv_EquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, EquippedActors);
}

void UInv_EquipmentComponent::InitPlayerController()
{
	if (OwningPlayerController = Cast<APlayerController>(GetOwner()); OwningPlayerController.IsValid())
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn());
		if (IsValid(OwnerCharacter))
		{
			OnPossessedPawnChanged(nullptr, OwnerCharacter);
		}
		else
		{
			OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
		}
	}
}

void UInv_EquipmentComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}
	InitInventoryComponent();
}

void UInv_EquipmentComponent::InitInventoryComponent()
{
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(OwningPlayerController.Get());
	if (!InventoryComponent.IsValid()) return;

	if(!InventoryComponent->GetInventoryListMutable().OnItemRemoved.IsAlreadyBound(this, &ThisClass::OnItemRemoved))
	{
		InventoryComponent->GetInventoryListMutable().OnItemRemoved.AddDynamic(this, &ThisClass::OnItemRemoved);
	}
	if(!InventoryComponent->OnItemEquipped.IsAlreadyBound(this,&ThisClass::OnItemEquipped))
	{
		InventoryComponent->OnItemEquipped.AddDynamic(this, &ThisClass::OnItemEquipped);
	}
	if(!InventoryComponent->OnItemUnEquipped.IsAlreadyBound(this,&ThisClass::OnItemUnEquipped))
	{
		InventoryComponent->OnItemUnEquipped.AddDynamic(this, &ThisClass::OnItemUnEquipped);
	}
}

AInv_EquipActor* UInv_EquipmentComponent::SpawnedEquippedActor(FInv_EquipmentFragment* EquipmentFragment,
	const FInv_ItemManifest&, USkeletalMeshComponent* AttachMesh) const
{
	AInv_EquipActor* SpawnedEquipActor = EquipmentFragment->SpawnAttachedActor(AttachMesh);
	SpawnedEquipActor->SetEquipmentType(EquipmentFragment->GetEquipmentType());
	SpawnedEquipActor->SetOwner(GetOwner());
	SpawnedEquipActor->SetOwningController(OwningPlayerController.Get());
	EquipmentFragment->SetEquippedActor(SpawnedEquipActor);
	return SpawnedEquipActor;
}

AInv_EquipActor* UInv_EquipmentComponent::FindEquippedActorByType(const FGameplayTag& EquipmentType)
{
	const auto FoundActor = EquippedActors.FindByPredicate([&EquipmentType](const AInv_EquipActor* Actor)
	{
		return Actor && Actor->GetEquipmentType().MatchesTagExact(EquipmentType);
		});
	return FoundActor ? FoundActor->Get() : nullptr;
}

void UInv_EquipmentComponent::RemoveEquippedActor(const FGameplayTag& EquipmentType)
{
	if(AInv_EquipActor* EquippedActor = FindEquippedActorByType(EquipmentType); IsValid(EquippedActor))
	{
		EquippedActors.Remove(EquippedActor);
		EquippedActor->Destroy();
	}
}

void UInv_EquipmentComponent::OnItemEquipped(UInv_InventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem)) return;
	if (!OwningPlayerController->HasAuthority()) return;

	FInv_ItemManifest& ItemManifest = EquippedItem->GetItemManifestMutable();
	FInv_EquipmentFragment* EquipmentFragment = EquippedItem->GetFragmentOfTypeMutable<FInv_EquipmentFragment>();
	if (!EquipmentFragment) return;

	if (!OwningSkeletalMesh.IsValid()) return;
	AInv_EquipActor* NewEquippedActor = SpawnedEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());
	NewEquippedActor->SetOwningItem(EquippedItem);

	EquipmentFragment->OnEquip(OwningPlayerController.Get());

	EquippedActors.Add(NewEquippedActor);

	FInv_SkeletalMeshFragment* SkeletalMeshFragment = EquippedItem->GetFragmentOfTypeMutable<FInv_SkeletalMeshFragment>();
	if (!SkeletalMeshFragment) return;

	USkeletalMesh* MeshAsset = SkeletalMeshFragment->GetDesiredSkeletalMesh();
	if (IsValid(MeshAsset))
	{
		NewEquippedActor->SetSkeletalMeshAsset(MeshAsset);
	}

}

void UInv_EquipmentComponent::OnItemUnEquipped(UInv_InventoryItem* UnEquippedItem)
{
	if (!IsValid(UnEquippedItem)) return;
	if (!OwningPlayerController->HasAuthority()) return;

	FInv_EquipmentFragment* EquipmentFragment = UnEquippedItem->GetFragmentOfTypeMutable<FInv_EquipmentFragment>();
	if (!EquipmentFragment) return;

	EquipmentFragment->OnUnEquip(OwningPlayerController.Get());

	RemoveEquippedActor(EquipmentFragment->GetEquipmentType());
}

void UInv_EquipmentComponent::OnItemRemoved(UInv_InventoryItem* UnEquippedItem)
{
	auto ActorToUnequip = EquippedActors.FindByPredicate([UnEquippedItem](const AInv_EquipActor* Actor)
	{
			return Actor->GetOwningItem() == UnEquippedItem;
	});
	if(ActorToUnequip)
	{
		OnItemUnEquipped(UnEquippedItem);
	}
}
