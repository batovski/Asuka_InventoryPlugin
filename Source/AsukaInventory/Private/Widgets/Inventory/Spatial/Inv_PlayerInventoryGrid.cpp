// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_PlayerInventoryGrid.h"

#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"

void UInv_PlayerInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	GridName = Text_GridName->GetText();

	if (bIsInitializedOnStart)
	{
		CreateGrid(UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer()), Rows, Columns, GridName);
	}
}

void UInv_PlayerInventoryGrid::OnHide()
{
	SizeBox_GridFrame->SetVisibility(ESlateVisibility::Hidden);
}

void UInv_PlayerInventoryGrid::CreateGrid(const TScriptInterface<IInv_ItemListInterface>& SourceInventory,
	const int32 NewRows, const int32 NewColumns, const FText& NewGridName)
{
	Super::CreateGrid(SourceInventory, NewRows, NewColumns, GridName);
}
