
#include "InventoryManagment/Components/Inv_InventoryComponent.h"

#include "Inventorymanagment/FastArray/Inv_FastArray.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"

// Sets default values for this component's properties
UInv_InventoryComponent::UInv_InventoryComponent() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	bIsInventoryMenuOpen = false;
}

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ConstructInventory();
}

void UInv_InventoryComponent::TryAddItemByComponent(UInv_ItemComponent* ItemComponent)
{
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);
	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;

	if(Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	if(Result.Item.IsValid() && Result.bStackable)
	{
		// OnStackChange.Broadcast(Result);
		// Add stacks to an item that already exists in the inventory
		Server_AddStacksToItemByComponent(ItemComponent, Result.TotalRoomToFill, Result.Remainder);
	}
	else if(Result.TotalRoomToFill > 0)
	{
		// This item is new, add it to the inventory
		Server_AddNewItemByComponent(ItemComponent, Result.bStackable ? Result.TotalRoomToFill : 0);
	}
}

void UInv_InventoryComponent::Server_AddNewItemByItem_Implementation(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item,
	const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	UInv_InventoryItem* NewItem = Execute_AddItemToList(SourceInventory.GetObject(), Item->GetStaticItemManifestAssetId(), Item->GetDynamicItemFragments(), NewItemAddingOptions);
	if (FInv_StackableFragment* StackFragment = NewItem->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		StackFragment->SetStackCount(NewItemAddingOptions.StackCount);
	}
}

void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const TScriptInterface <IInv_ItemListInterface>& TargetInventory, UInv_InventoryItem* Item, int32 StackCount, int32 Remainder)
{
	const FGameplayTag ItemType = IsValid(Item) ? Item->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInv_InventoryItem* FoundItem = Execute_FindFirstItemByType(TargetInventory.GetObject(), ItemType);
	if (!FoundItem) return;

	if (FInv_StackableFragment* StackFragment = FoundItem->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		StackFragment->SetStackCount(StackFragment->GetStackCount() + StackCount);
	}

	if (Remainder == 0)
	{
		Execute_RemoveItemFromList(SourceInventory.GetObject(), Item);
	}
	else if (FInv_StackableFragment* StackableFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		// If the item is stackable, we can update the stack count
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInv_InventoryComponent::Server_AddStacksToItemByComponent_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount,
                                                                    int32 Remainder)
{
	const FGameplayTag ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemType);
	if (!FoundItem) return;

	if(FInv_StackableFragment* StackFragment = FoundItem->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		StackFragment->SetStackCount(StackFragment->GetStackCount() + StackCount);
	}

	if(Remainder == 0)
	{
		ItemComponent->PickedUp();
	}
	else if(FInv_StackableFragment* StackableFragment = ItemComponent->GetFragmentOfTypeMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		// If the item is stackable, we can update the stack count
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInv_InventoryComponent::Server_AddNewItemByComponent_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	UInv_InventoryItem* NewItem = InventoryList.AddEntry(ItemComponent);
	if (FInv_StackableFragment* StackFragment = NewItem->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		StackFragment->SetStackCount(StackCount);
	}

	ItemComponent->PickedUp();
}

void UInv_InventoryComponent::Server_AddNewItem_Implementation(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const TScriptInterface <IInv_ItemListInterface>& TargetInventory, UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	if(!Item->IsStackable())
	{
		Item->SetOwningGridEntityTag(NewItemAddingOptions.GridEntityTag);
		Item->SetItemIndex(NewItemAddingOptions.GridIndex);
		Execute_MoveItemToList(TargetInventory.GetObject(), Item);
		Execute_RemoveItemFromList(SourceInventory.GetObject(), Item);
	}
	else
	{

		UInv_InventoryItem* NewItem = Execute_AddItemToList(TargetInventory.GetObject(), Item->GetStaticItemManifestAssetId(), Item->GetDynamicItemFragments(), NewItemAddingOptions);
		if (FInv_StackableFragment* StackFragment = NewItem->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
		{
			StackFragment->SetStackCount(NewItemAddingOptions.StackCount);
		}

		if (FInv_StackableFragment* OldItemStackFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
		{
			if (OldItemStackFragment->GetStackCount() <= NewItemAddingOptions.StackCount)
				Execute_RemoveItemFromList(SourceInventory.GetObject(), Item);
			else
			{
				OldItemStackFragment->SetStackCount(OldItemStackFragment->GetStackCount() - NewItemAddingOptions.StackCount);
			}
		}
		else
		{
			Execute_RemoveItemFromList(SourceInventory.GetObject(), Item);
		}
	}
}


void UInv_InventoryComponent::Server_DropItem_Implementation(UInv_InventoryItem* Item, int32 StackCount)
{
	int32 NewStackCount = 0;
	if (FInv_StackableFragment* StackFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		NewStackCount = StackFragment->GetStackCount()- StackCount;
		StackFragment->SetStackCount(NewStackCount);
	}
	if (NewStackCount <= 0)
	{
		// Remove the item from the inventory
		InventoryList.RemoveEntry(Item);
	}
	/*if (Item->IsEquippable())
		Multicast_EquipSlotClicked(nullptr, Item);*/
	SpawnDroppedItem(Item, StackCount);
}


void UInv_InventoryComponent::Server_ChangeItemGridIndex_Implementation(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	Execute_ChangeItemGridIndex(SourceInventory.GetObject(), Item, NewItemAddingOptions);
}

void UInv_InventoryComponent::Server_DropItemFromExternalInventory_Implementation(
	const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, int32 StackCount)
{
	int32 NewStackCount = 0;
	if (FInv_StackableFragment* StackFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		NewStackCount = StackFragment->GetStackCount() - StackCount;
		StackFragment->SetStackCount(NewStackCount);
	}
	if (NewStackCount <= 0)
	{
		// Remove the item from the inventory
		Execute_RemoveItemFromList(SourceInventory.GetObject(), Item);
	}

	SpawnDroppedItem(Item, StackCount);
}

void UInv_InventoryComponent::SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount)
{
	const APawn* OwnerPawn = OwningController->GetPawn();

	FVector RotatedForward = OwnerPawn->GetActorForwardVector();
	RotatedForward = RotatedForward.RotateAngleAxis(FMath::FRandRange(DropSpawnAngleMin, DropSpawnAngleMax), FVector::UpVector);
	FVector SpawnLocation = OwnerPawn->GetActorLocation() + RotatedForward * FMath::FRandRange(DropSpawnDistanceMin, DropSpawnDistanceMax);
	SpawnLocation.Z += RelativeSpawnElevation;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FInv_PickUpFragment* PickUpFragment = Item->GetFragmentStructByTagMutable<FInv_PickUpFragment>(FragmentTags::DynamicFragmentTags::PickUpActorFragment);
	if (!PickUpFragment) { return; }
	UInv_ItemComponent* NewItemComponent = UInv_ItemComponent::SpawnPickUpActor(PickUpFragment->GetPickUpActorClass(),this, SpawnLocation, SpawnRotation);
	if (!NewItemComponent) return;
	NewItemComponent->InitItemManifest(Item->GetStaticItemManifestAssetId()); //TODO : REDUNDANT
	NewItemComponent->InitDynamicData(Item->GetDynamicItemFragments());
	if(FInv_StackableFragment* StackableFragment = NewItemComponent->GetFragmentOfTypeMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		StackableFragment->SetStackCount(StackCount);
	}
	FInv_SkeletalMeshFragment* SkeletalMeshFragment = Item->GetFragmentStructByTagMutable<FInv_SkeletalMeshFragment>(FragmentTags::SkeltalMeshFragment);
	if (!SkeletalMeshFragment) return;
	
	// Use the replicated function to set the skeletal mesh asset
	USkeletalMesh* MeshAsset = SkeletalMeshFragment->GetDesiredSkeletalMesh();
	if (IsValid(MeshAsset))
	{
		NewItemComponent->SetSkeletalMeshAsset(MeshAsset);
	}
}

void UInv_InventoryComponent::RemoveItemFromList_Implementation(UInv_InventoryItem* Item)
{
	InventoryList.RemoveEntry(Item);
}

UInv_InventoryItem* UInv_InventoryComponent::AddItemToList_Implementation(const FPrimaryAssetId& StaticItemManifestID,
	const TArray<FInstancedStruct>& DynamicFragments, const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	return InventoryList.AddEntry(StaticItemManifestID, NewItemAddingOptions, DynamicFragments);
}

void UInv_InventoryComponent::ChangeItemGridIndex_Implementation(UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	InventoryList.ChangeEntryGridIndex(Item, NewItemAddingOptions.GridIndex, NewItemAddingOptions.GridEntityTag);
}

void UInv_InventoryComponent::MarkItemDirty_Implementation(UInv_InventoryItem* Item)
{
	InventoryList.MarkEntryDirty(Item);
}

void UInv_InventoryComponent::Server_ConsumeItem_Implementation(UInv_InventoryItem* Item)
{
	if (FInv_StackableFragment* StackFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		const int32 NewStackCount = StackFragment->GetStackCount() - 1;
		if (NewStackCount <= 0)
		{
			InventoryList.RemoveEntry(Item);
		}
		else
		{
			StackFragment->SetStackCount(NewStackCount);
		}
	}
	if(FInv_ConsumableFragment* ConsumableFragment = Item->GetFragmentStructByTagMutable<FInv_ConsumableFragment>(FragmentTags::ConsumableFragment))
	{
		ConsumableFragment->OnConsume(OwningController.Get());
	}
}

void UInv_InventoryComponent::Server_EquipSlotClicked_Implementation(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* ItemToEquip,
	UInv_InventoryItem* ItemToUnEquip, const FGameplayTag& EquipSlotTag)
{
	if(SourceInventory != this)
	{
		FInv_ItemAddingOptions ItemToEquipOptions;
		ItemToEquipOptions.GridEntityTag = EquipSlotTag;
		Server_AddNewItem(SourceInventory, this, ItemToEquip, ItemToEquipOptions);
		if(IsValid(ItemToUnEquip))
		{
			ItemToEquipOptions.GridIndex = ItemToEquip->GetItemIndex();
			ItemToEquipOptions.GridEntityTag = ItemToUnEquip->GetOwningGridEntityTag();
			Server_AddNewItem(this, SourceInventory, ItemToUnEquip, ItemToEquipOptions);
		}
	}
	if(IsValid(ItemToEquip) && EquipSlotTag != FGameplayTag::EmptyTag)
	{
		ItemToEquip->SetOwningGridEntityTag(EquipSlotTag);
	}
	Multicast_EquipSlotClicked(ItemToEquip, ItemToUnEquip);
}

void UInv_InventoryComponent::Multicast_EquipSlotClicked_Implementation(UInv_InventoryItem* ItemToEquip,
	UInv_InventoryItem* ItemToUnEquip)
{
	if(ItemToUnEquip)
		OnItemUnEquipped.Broadcast(ItemToUnEquip);
	if (ItemToEquip)
		OnItemEquipped.Broadcast(ItemToEquip);
}

void UInv_InventoryComponent::Server_MarkItemDirty_Implementation(
	const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item)
{
	Execute_MarkItemDirty(SourceInventory.GetObject(), Item);
}

void UInv_InventoryComponent::Server_UpdateItemStackCount_Implementation(UInv_InventoryItem* Item,
	const int32 StackCount)
{
	if (FInv_StackableFragment* StackFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		StackFragment->SetStackCount(StackCount);
	}
}

void UInv_InventoryComponent::Server_RemoveItem_Implementation(
	const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item)
{
	Execute_RemoveItemFromList(SourceInventory.GetObject(), Item);
}

void UInv_InventoryComponent::TryChangeItemGridIndex(const TScriptInterface<IInv_ItemListInterface>& SourceInventory,
                                                     UInv_InventoryItem* Item, const int32 NewGridIndex)
{
	FInv_ItemAddingOptions NewItemAddingOptions;
	NewItemAddingOptions.GridIndex = NewGridIndex;
	Server_ChangeItemGridIndex(SourceInventory, Item, NewItemAddingOptions);
}

const UInv_InventoryItem* UInv_InventoryComponent::FindInventoryItem(const FGameplayTag& ItemType) const
{
	return InventoryList.FindFirstItemByType(ItemType);
}

void UInv_InventoryComponent::ToggleInventoryMenu()
{
	if(bIsInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

void UInv_InventoryComponent::AddRepSubObj(UObject* SubObj)
{
	if(IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		// Add the sub-object to the replication list
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_InventoryComponent::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(),TEXT("InventoryComponent should have player controller as Owner"));
	if(!OwningController->IsLocalController()) return;

	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(), InventoryMenuClass);
	InventoryMenu->AddToViewport();
	CloseInventoryMenu();
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	if (!InventoryMenu) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bIsInventoryMenuOpen = true;

	if (!OwningController.IsValid()) return;
	InventoryMenu->ShowInventoryCursor();
	OnInventoryMenuToggled.Broadcast(bIsInventoryMenuOpen);
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	if (!InventoryMenu) return;
	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bIsInventoryMenuOpen = false;

	if (!OwningController.IsValid()) return;
	InventoryMenu->HideInventoryCursor();
	OnInventoryMenuToggled.Broadcast(bIsInventoryMenuOpen);
}


