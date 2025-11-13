// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"

UInv_EquipmentComponent::UInv_EquipmentComponent() : EquipmentItemsList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}


void UInv_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	EquippedActors.SetOwnerComponent(this);
	InitPlayerController();
}

void UInv_EquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, EquippedActors);
	DOREPLIFETIME(ThisClass, EquipmentItemsList);
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
		OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
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

	if(!EquipmentItemsList.OnItemAdded.IsAlreadyBound(this, &ThisClass::OnItemEquipped))
	{
		EquipmentItemsList.OnItemAdded.AddDynamic(this, &ThisClass::OnItemEquipped);
	}
	if(!EquipmentItemsList.OnItemRemoved.IsAlreadyBound(this,&ThisClass::OnItemUnEquipped))
	{
		EquipmentItemsList.OnItemRemoved.AddDynamic(this, &ThisClass::OnItemUnEquipped);
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
	return EquippedActors.FindByType(EquipmentType);
}

UInv_InventoryItem* UInv_EquipmentComponent::FindFirstItemByType_Implementation(const FGameplayTag& ItemType) const
{
	for(const auto Item : EquipmentItemsList.GetAllItems())
	{
		if(Item->GetItemManifest().GetItemType().MatchesTag(ItemType))
		{
			return Item;
		}
		else if(const FInv_ContainerFragment* ContainerFragment = Item->GetFragmentStructByTag<FInv_ContainerFragment>(FragmentTags::ContainerFragment))
		{
			if(const auto SubItem = ContainerFragment->ContainerInventoryComponent->FindFirstItemByType_Implementation(ItemType))
			{
				return SubItem;
			}
		}
	}
	return IInv_ItemListInterface::FindFirstItemByType_Implementation(ItemType);
}

UInv_InventoryItem* UInv_EquipmentComponent::AddItemToList_Implementation(const FPrimaryAssetId& StaticItemManifestID,
                                                                          const TArray<FInstancedStruct>& DynamicFragments, const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	return EquipmentItemsList.AddEntry(StaticItemManifestID, NewItemAddingOptions, DynamicFragments);
}

void UInv_EquipmentComponent::ChangeItemGridIndex_Implementation(UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	EquipmentItemsList.ChangeEntryGridIndex(Item, NewItemAddingOptions.GridIndex);
}

void UInv_EquipmentComponent::MarkItemDirty_Implementation(UInv_InventoryItem* Item)
{
	EquipmentItemsList.MarkEntryDirty(Item);
}

void UInv_EquipmentComponent::AddRepSubObj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		// Add the sub-object to the replication list
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_EquipmentComponent::RemoveEquippedActor(const FGameplayTag& EquipmentType)
{
	if(AInv_EquipActor* EquippedActor = FindEquippedActorByType(EquipmentType); IsValid(EquippedActor))
	{
		// Remove from fast array (this will automatically broadcast the delegate)
		EquippedActors.RemoveEquippedActor(EquippedActor);
		
		EquippedActor->Destroy();
	}
}

void UInv_EquipmentComponent::OnItemEquipped(UInv_InventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem)) return;
	if (!OwningPlayerController->HasAuthority()) return;
	FInv_ItemManifest& ItemManifest = EquippedItem->GetItemManifestMutable();
	FInv_EquipmentFragment* EquipmentFragment = EquippedItem->GetFragmentStructByTagMutable<FInv_EquipmentFragment>(FragmentTags::EquipmentFragment);
	if (!EquipmentFragment) return;

	if (!OwningSkeletalMesh.IsValid()) return;
	AInv_EquipActor* NewEquippedActor = SpawnedEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());
	NewEquippedActor->SetOwningItem(EquippedItem);

	EquipmentFragment->OnEquip(OwningPlayerController.Get());

	// Add to fast array (this will automatically broadcast the delegate)
	EquippedActors.AddEquippedActor(NewEquippedActor);

	const FInv_SkeletalMeshFragment* SkeletalMeshFragment = EquippedItem->GetFragmentStructByTag<FInv_SkeletalMeshFragment>(FragmentTags::SkeltalMeshFragment);
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

	FInv_EquipmentFragment* EquipmentFragment = UnEquippedItem->GetFragmentStructByTagMutable<FInv_EquipmentFragment>(FragmentTags::EquipmentFragment);
	if (!EquipmentFragment) return;

	EquipmentFragment->OnUnEquip(OwningPlayerController.Get());

	RemoveEquippedActor(EquipmentFragment->GetEquipmentType());
}

void UInv_EquipmentComponent::OnItemRemoved(UInv_InventoryItem* UnEquippedItem)
{
	const FEquippedActorEntry* ActorToUnequip = EquippedActors.GetItems().FindByPredicate([UnEquippedItem](const FEquippedActorEntry& Entry)
	{
		return Entry.EquippedActor && Entry.EquippedActor->GetOwningItem() == UnEquippedItem;
	});
	if(ActorToUnequip)
	{
		OnItemUnEquipped(UnEquippedItem);
	}
}

// Fast Array Serializer implementations

void FEquippedActorFastArray::SetOwnerComponent(UInv_EquipmentComponent* InOwnerComponent)
{
	OwnerComponent = InOwnerComponent;
}

void FEquippedActorFastArray::AddEquippedActor(AInv_EquipActor* NewActor)
{
	if (!IsValid(NewActor))
	{
		return;
	}

	FEquippedActorEntry NewEntry(NewActor);
	Items.Add(NewEntry);
	MarkItemDirty(NewEntry);

	// Broadcast on server immediately
	if (OwnerComponent.IsValid())
	{
		OwnerComponent->OnEquippedActorAdded.Broadcast(NewActor);
	}
}

bool FEquippedActorFastArray::RemoveEquippedActor(AInv_EquipActor* ActorToRemove)
{
	if (!IsValid(ActorToRemove))
	{
		return false;
	}

	const int32 RemovedIndex = Items.IndexOfByPredicate([ActorToRemove](const FEquippedActorEntry& Entry)
	{
		return Entry.EquippedActor == ActorToRemove;
	});

	if (RemovedIndex != INDEX_NONE)
	{
		Items.RemoveAt(RemovedIndex);
		MarkArrayDirty();

		// Broadcast on server immediately
		if (OwnerComponent.IsValid())
		{
			OwnerComponent->OnEquippedActorRemoved.Broadcast(ActorToRemove);
		}
		return true;
	}
	return false;
}

AInv_EquipActor* FEquippedActorFastArray::FindByType(const FGameplayTag& EquipmentType) const
{
	const FEquippedActorEntry* FoundEntry = Items.FindByPredicate([&EquipmentType](const FEquippedActorEntry& Entry)
	{
		return Entry.EquippedActor && Entry.EquippedActor->GetEquipmentType().MatchesTagExact(EquipmentType);
	});

	return FoundEntry ? FoundEntry->EquippedActor : nullptr;
}

// Fast Array Item implementations

void FEquippedActorEntry::PostReplicatedAdd(const FEquippedActorFastArray& InArraySerializer)
{
	// Called when this item is added during replication on clients
	if (InArraySerializer.OwnerComponent.IsValid() && IsValid(EquippedActor))
	{
		InArraySerializer.OwnerComponent->OnEquippedActorAdded.Broadcast(EquippedActor);
	}
}

void FEquippedActorEntry::PreReplicatedRemove(const FEquippedActorFastArray& InArraySerializer)
{
	// Called when this item is removed during replication on clients
	if (InArraySerializer.OwnerComponent.IsValid())
	{
		InArraySerializer.OwnerComponent->OnEquippedActorRemoved.Broadcast(EquippedActor);
	}
}

void FEquippedActorEntry::PostReplicatedChange(const FEquippedActorFastArray& InArraySerializer)
{
	// Called when this item is changed during replication on clients
	// This is typically used when the item's properties change but the item itself doesn't get added/removed
}
