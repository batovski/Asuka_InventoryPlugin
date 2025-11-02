// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Inv_LootInventoryGrid.generated.h"

class UInv_InventoryBase;
class UInv_ExternalInventoryComponent;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_LootInventoryGrid : public UInv_InventoryGrid
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;

	virtual void AddItem(UInv_InventoryItem* Item) override;

protected:
	virtual void OnInventoryMenuToggled(const bool IsOpen) override;

private:
	void ClearGrid();

	void RemoveInventoryComponentLinkage();
};
