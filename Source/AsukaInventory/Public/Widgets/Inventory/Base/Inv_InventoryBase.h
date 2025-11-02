// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryBase.generated.h"

class IInv_ItemListInterface;
class UInv_InventoryGrid;
class UInv_HoverItem;
class UInv_ItemComponent;
class UInv_InventoryItem;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggled, bool, bIsOpen);

UCLASS()
class ASUKAINVENTORY_API UInv_InventoryBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_ItemComponent* ItemComponent) const { return  FInv_SlotAvailabilityResult(); }
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_InventoryItem* Item, UInv_InventoryGrid* TargetGrid, const int32 StackAmountOverride = -1, const int32 GridIndex = -1) const { return  FInv_SlotAvailabilityResult(); }
	virtual void OnItemHovered(UInv_InventoryItem* Item) {};
	virtual void OnItemUnhovered() {};
	virtual void AssignHoverItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* InventoryItem){};
	virtual void ClearHoverItem() {};
	virtual bool HasHoverItem() const { return false; }
	virtual UInv_HoverItem* GetHoverItem() const { return nullptr; }
	virtual void RotateHoverItem() {};
	virtual void SetHoverItem(UInv_HoverItem* HoverItem) {}
	virtual void ShowInventoryCursor() {};
	virtual void HideInventoryCursor() {};

	virtual void ToggleInventoryMenu() {};

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInventoryMenuToggled OnInventoryMenuToggled;
};
