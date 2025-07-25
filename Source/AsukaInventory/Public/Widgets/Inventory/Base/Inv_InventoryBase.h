// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryBase.generated.h"

class UInv_HoverItem;
class UInv_ItemComponent;
class UInv_InventoryItem;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_InventoryBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_ItemComponent* ItemComponent) const { return  FInv_SlotAvailabilityResult(); }
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_InventoryItem* Item, const int32 StackAmountOverride = -1,  const EInv_ItemCategory GridCategory = EInv_ItemCategory::None) const { return  FInv_SlotAvailabilityResult(); }
	virtual void OnItemHovered(UInv_InventoryItem* Item) {};
	virtual void OnItemUnhovered() {};
	virtual bool HasHoverItem() const { return false; }
	virtual UInv_HoverItem* GetHoverItem() const { return nullptr; }
	virtual void SetHoverItem(UInv_HoverItem* HoverItem) {}
	virtual float GetTileSize() const { return 0.f; }
};
