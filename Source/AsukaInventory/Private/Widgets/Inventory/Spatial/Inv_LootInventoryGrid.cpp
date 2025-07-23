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
		InventoryComponent->OnItemAdded.RemoveDynamic(this, &ThisClass::AddItem);
		InventoryComponent->OnStackChange.RemoveDynamic(this, &ThisClass::AddStacks);
	}
}

void UInv_LootInventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	const FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);
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
	ShowCursor();
}

void UInv_LootInventoryGrid::SetExternalInventoryComponent(UInv_ExternalInventoryComponent* ExternalInventoryComp)
{
	ExternalInventoryComponent = ExternalInventoryComp;
	ExternalInventoryComponent->GetItemAddDelegate().AddDynamic(this, &UInv_InventoryGrid::AddItem);
	ExternalInventoryComponent->GetItemRemoveDelegate().AddDynamic(this, &ThisClass::RemoveItem);
}

void UInv_LootInventoryGrid::RemoveExternalInventoryComponentLinkage()
{
	ExternalInventoryComponent->GetItemAddDelegate().RemoveDynamic(this, &UInv_InventoryGrid::AddItem);
	ExternalInventoryComponent->GetItemRemoveDelegate().RemoveDynamic(this, &ThisClass::RemoveItem);
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

void UInv_LootInventoryGrid::RemoveItem(UInv_InventoryItem* Item)
{
	if(UInv_SlottedItem* SlottedItem = FindSlottedItem(Item))
	{
		const int32 Index = SlottedItem->GetGridIndex();
		RemoveItemFromGrid(SlottedItem->GetInventoryItem(), Index);
	}
}

void UInv_LootInventoryGrid::ClearGrid()
{
	RemoveAllItemFromGrid();
}
