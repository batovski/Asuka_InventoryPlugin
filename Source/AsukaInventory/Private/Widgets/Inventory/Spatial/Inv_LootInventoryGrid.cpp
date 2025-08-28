// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_LootInventoryGrid.h"

#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"

void UInv_LootInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->GetInventoryListMutable().OnItemAdded.RemoveDynamic(this, &ThisClass::AddItem);
		InventoryComponent->GetInventoryListMutable().OnItemRemoved.RemoveDynamic(this, &ThisClass::RemoveItem);
		InventoryComponent->GetInventoryListMutable().OnItemChanged.RemoveDynamic(this, &ThisClass::ChangeItem);
	}
}

void UInv_LootInventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	FInv_StackableFragment* StackableFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment);
	const FInv_SlotAvailabilityResult Result = HasRoomForItem(Item->GetItemManifest(), StackableFragment, Item->GetItemIndex());
	AddItemToIndices(Result, Item);
}

void UInv_LootInventoryGrid::DropHoverItem()
{
	if (!IsValid(GetHoverItem())) return;
	if (!IsValid(GetHoverItem()->GetInventoryItem())) return;
	if (ExternalInventoryComponent.IsValid())
	{
		InventoryComponent->Server_DropItemFromExternalInventory(ExternalInventoryComponent.Get(),
			GetHoverItem()->GetInventoryItem(), GetHoverItem()->GetStackCount());
	}

	ClearHoverItem();
}

void UInv_LootInventoryGrid::SetExternalInventoryComponent(UInv_ExternalInventoryComponent* ExternalInventoryComp)
{
	ExternalInventoryComponent = ExternalInventoryComp;
	ExternalInventoryComponent->GetItemAddDelegate().AddDynamic(this, &ThisClass::AddItem);
	ExternalInventoryComponent->GetItemRemoveDelegate().AddDynamic(this, &ThisClass::RemoveItem);
	ExternalInventoryComponent->GetItemChangedDelegate().AddDynamic(this, &ThisClass::ChangeItem);
}

void UInv_LootInventoryGrid::RemoveExternalInventoryComponentLinkage()
{
	ExternalInventoryComponent->GetItemAddDelegate().RemoveDynamic(this, &ThisClass::AddItem);
	ExternalInventoryComponent->GetItemRemoveDelegate().RemoveDynamic(this, &ThisClass::RemoveItem);
	ExternalInventoryComponent->GetItemChangedDelegate().RemoveDynamic(this, &ThisClass::ChangeItem);
	ExternalInventoryComponent = nullptr;
}

void UInv_LootInventoryGrid::OnInventoryMenuToggled(const bool IsOpen)
{
	Super::OnInventoryMenuToggled(IsOpen);
	if(!IsOpen)
	{
		if (ExternalInventoryComponent.IsValid())
		{
			ClearGrid();
			SetVisibility(ESlateVisibility::Collapsed);
			RemoveExternalInventoryComponentLinkage();
		}
	}
}

TScriptInterface<IInv_ItemListInterface> UInv_LootInventoryGrid::GetGridInventoryInterface() const
{
	return ExternalInventoryComponent.IsValid() ? ExternalInventoryComponent.Get() : nullptr;
}

void UInv_LootInventoryGrid::ClearGrid()
{
	RemoveAllItemFromGrid();
}
