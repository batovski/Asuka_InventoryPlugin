// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_LootInventoryGrid.h"

#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"

void UInv_LootInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (IsValid(OwningInventoryComponent.GetObject()))
	{
		OwningInventoryComponent->GetInventoryListMutable().OnItemAdded.RemoveDynamic(this, &ThisClass::AddItem);
		OwningInventoryComponent->GetInventoryListMutable().OnItemRemoved.RemoveDynamic(this, &ThisClass::RemoveItem);
		OwningInventoryComponent->GetInventoryListMutable().OnItemChanged.RemoveDynamic(this, &ThisClass::ChangeItem);
	}
}

void UInv_LootInventoryGrid::RemoveInventoryComponentLinkage()
{
	OwningInventoryComponent->GetInventoryListMutable().OnItemAdded.RemoveDynamic(this, &ThisClass::AddItem);
	OwningInventoryComponent->GetInventoryListMutable().OnItemRemoved.RemoveDynamic(this, &ThisClass::RemoveItem);
	OwningInventoryComponent->GetInventoryListMutable().OnItemChanged.RemoveDynamic(this, &ThisClass::ChangeItem);
	OwningInventoryComponent = nullptr;

	InventoryMenu->OnInventoryMenuToggled.RemoveDynamic(this, &ThisClass::OnInventoryMenuToggled);
	InventoryMenu.Reset();
}

void UInv_LootInventoryGrid::OnInventoryMenuToggled(const bool IsOpen)
{
	Super::OnInventoryMenuToggled(IsOpen);
	if(!IsOpen)
	{
		if (IsValid(OwningInventoryComponent.GetObject()))
		{
			ClearGrid();
			SetVisibility(ESlateVisibility::Collapsed);
			RemoveInventoryComponentLinkage();
		}
	}
}

void UInv_LootInventoryGrid::ClearGrid()
{
	RemoveAllItemFromGrid();
}
