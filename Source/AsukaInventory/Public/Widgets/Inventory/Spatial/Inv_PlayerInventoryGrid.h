// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Inv_PlayerInventoryGrid.generated.h"

/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_PlayerInventoryGrid : public UInv_InventoryGrid
{
	GENERATED_BODY()
public:

	virtual void NativeOnInitialized() override;
	virtual void OnHide() override;
	virtual void CreateGrid(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const int32 NewRows, const int32 NewColumns, const FText& NewGridName) override;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bIsInitializedOnStart;
};
