// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Inv_SpatialInventory.generated.h"

class UInv_ExternalInventoryComponent;
class UInv_LootInventoryGrid;
class UInv_EquippedSlottedItem;
struct FGameplayTag;
class UInv_EquippedGridSlot;
class UInv_ItemDescription;
class UCanvasPanel;
class UInv_InventoryGrid;
class UWidgetSwitcher;
class UButton;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_SpatialInventory : public UInv_InventoryBase
{
	GENERATED_BODY()
public:
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_ItemComponent* ItemComponent) const override;
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_InventoryItem* Item, const int32 StackAmountOverride = -1, const EInv_ItemCategory GridCategory = EInv_ItemCategory::None) const override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void OnItemHovered(UInv_InventoryItem* Item) override;
	virtual void OnItemUnhovered() override;
	virtual bool HasHoverItem() const override;
	virtual UInv_HoverItem* GetHoverItem() const override;
	virtual void SetHoverItem(UInv_HoverItem* NewHoverItem) override;
	virtual float GetTileSize() const override;

	void InitLootGrid(UInv_ExternalInventoryComponent* ExternalInventoryComponent, const TArray<UInv_InventoryItem*>& LootList) const;

	virtual void ShowInventoryCursor() override;
	virtual void HideInventoryCursor() override;

private:

	void DisableButton(UButton* Button) const;
	void SetActiveGrid(UInv_InventoryGrid* GridToActivate, UButton* Button);

	UInv_ItemDescription* GetItemDescription();
	void SetItemDescriptionSizeAndPosition(UInv_ItemDescription* DescriptionWidget, UCanvasPanel* Canvas) const;

	bool CanEquipHoverItem(const UInv_EquippedGridSlot* EquippedGridSlot,const FGameplayTag& EquipmentTypeTag ) const;
	UInv_EquippedGridSlot* FindSlotWithEquippedItem(UInv_InventoryItem* EquippedItem) const;
	UInv_EquippedGridSlot* FindSlotByEquippedType(const FGameplayTag& EquipmentTypeTag) const;
	void ClearSlotOfItem(UInv_EquippedGridSlot* EquippedGridSlot);
	void RemoveEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem);

	void MakeEquippedSlottedItem(const UInv_EquippedSlottedItem* EquippedSlottedItem, UInv_EquippedGridSlot* EquippedGridSlot, UInv_InventoryItem* ItemToEquip);
	void BroadcastSlotClickedDelegates(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnEquip) const;

	UFUNCTION()
	void ShowEquippables();
	UFUNCTION()
	void ShowConsumables();
	UFUNCTION()
	void ShowCraftables();

	UFUNCTION()
	void EquippedGridSlotClicked(UInv_EquippedGridSlot* GridSlot, const FGameplayTag& EquipmentTypeTag);
	UFUNCTION()
	void EquippedSlottedItemClicked(UInv_EquippedSlottedItem* SlottedItem, const FPointerEvent& MouseEvent);
	UFUNCTION()
	void GridEquippedItemClicked(UInv_InventoryItem* Item, const int32 GridIndex);
	UFUNCTION()
	void HoverItemAssigned(const UInv_InventoryItem* SlottedItem);
	UFUNCTION()
	void HoverItemUnAssigned(const UInv_InventoryItem* SlottedItem);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Equippables;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Consumables;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Craftables;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_LootInventoryGrid> Grid_Loot;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Equippables;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consumables;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Craftables;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_ItemDescription> ItemDescriptionClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bKeepCursorActive = false;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UUserWidget> InventoryCursorWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget>  InventoryCursorWidget;

	UPROPERTY()
	TObjectPtr<UInv_ItemDescription> ItemDescription;

	UPROPERTY()
	TArray<TObjectPtr<UInv_EquippedGridSlot>> EquippedGridSlots;

	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem;

	FTimerHandle ItemDescriptionTimer;

	TWeakObjectPtr<UInv_InventoryGrid> ActiveGrid;
};
