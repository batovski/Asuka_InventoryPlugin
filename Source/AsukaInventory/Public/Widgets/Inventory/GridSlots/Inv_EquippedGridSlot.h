// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Inv_EquippedGridSlot.generated.h"


class UInv_SpatialInventory;
class UInv_InventoryComponent;
class FEquippedSlottedItemClicked;
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
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	void SetSpatialInventory(UInv_SpatialInventory* NewSpatialInventory);
	UInv_EquippedSlottedItem* CreateEquippedSlottedItem(UInv_InventoryItem* Item);
	void RemoveEquippedSlottedItem();

	void SetEquippedSlottedItem(UInv_EquippedSlottedItem* SlottedItem) { EquippedSlottedItemInstance = SlottedItem; }
	const FGameplayTag& GetEquipmentTypeTag() const { return EquipmentTypeTag; }
	const FGameplayTag& GetOwningEntityGridTag() const { return OwningEntityGridTag; }
	

	void UnHighlightSlot();
	void HighlightSlot();

	FEquippedGridSlotClicked EquippedGridSlotClicked;
	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);
	UFUNCTION()
	void RemoveItem(UInv_InventoryItem* Item);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GrayedOutIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Root;

	UPROPERTY(EditAnywhere, Category = "Inventory",meta = (Categories="GameItems.Equipments"))
	FGameplayTag EquipmentTypeTag;

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "InventoryGrid.Player.Equipment"))
	FGameplayTag OwningEntityGridTag;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_EquippedSlottedItem> EquippedSlottedItemClass;

	UPROPERTY()
	TObjectPtr< UInv_EquippedSlottedItem> EquippedSlottedItemInstance;

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	TWeakObjectPtr<UInv_SpatialInventory> SpatialInventory;

};
