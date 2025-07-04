// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Inv_EquippedGridSlot.generated.h"


class UOverlay;
class UInv_EquippedSlottedItem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquippedGridSlotClicked, UInv_EquippedGridSlot*, GridSlot,
                                             const FGameplayTag&, EquipmentTypeTag);
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_EquippedGridSlot : public UInv_GridSlot
{
	GENERATED_BODY()
public:
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	UInv_EquippedSlottedItem* OnItemEquipped(UInv_InventoryItem* Item, const FGameplayTag& EquipmentTag, const float TileSize);

	void SetEquippedSlottedItem(UInv_EquippedSlottedItem* SlottedItem) { EquippedSlottedItemInstance = SlottedItem; }
	const FGameplayTag& GetEquipmentTypeTag() const { return EquipmentTypeTag; }

	void UnHighlightSlot();
	void HighlightSlot();

	FEquippedGridSlotClicked EquippedGridSlotClicked;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GrayedOutIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Root;

	UPROPERTY(EditAnywhere, Category = "Inventory",meta = (Categories="GameItems.Equipments"))
	FGameplayTag EquipmentTypeTag;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_EquippedSlottedItem> EquippedSlottedItemClass;

	UPROPERTY()
	TObjectPtr< UInv_EquippedSlottedItem> EquippedSlottedItemInstance;
};
