// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Inv_SpatialInventory.generated.h"

class UInv_PopUpInventoryGrid;
class IInv_ItemListInterface;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHoverItemChanged, const UInv_InventoryItem*, Item);

UCLASS()
class ASUKAINVENTORY_API UInv_SpatialInventory : public UInv_InventoryBase
{
	GENERATED_BODY()
public:
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_ItemComponent* ItemComponent) const override;
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_InventoryItem* Item, UInv_InventoryGrid* TargetGrid, const int32 StackAmountOverride = -1, const int32 GridIndex = -1) const override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void OnItemHovered(UInv_InventoryItem* Item) override;
	virtual void OnItemUnhovered() override;
	virtual bool HasHoverItem() const override;
	virtual UInv_HoverItem* GetHoverItem() const override;
	virtual void SetHoverItem(UInv_HoverItem* NewHoverItem) override;
	virtual void RotateHoverItem() override;

	void AddDynamicGrid(const FGameplayTag& GridTag, UInv_ExternalInventoryComponent* ExternalInventoryComponent, const TArray<UInv_InventoryItem*>& LootList);
	void RemoveDynamicGrid(const FGameplayTag& GridTag);

	virtual void ShowInventoryCursor() override;
	virtual void HideInventoryCursor() override;

	virtual void AssignHoverItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* InventoryItem) override;
	virtual void ClearHoverItem() override;
	void DropHoverItem();

	void CloseInventoryMenu();
	void OpenInventoryMenu();
	virtual void ToggleInventoryMenu() override;

	void CreateItemPopUpGrid(UInv_InventoryItem* OwningItem);

	UFUNCTION()
	void CloseItemPopUpGrid(UInv_InventoryItem* OwningItem);

	UFUNCTION()
	void EquippedSlottedItemClicked(UInv_EquippedSlottedItem* SlottedItem, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void TryToEquipItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item);

	FHoverItemChanged OnHoverItemAssigned;
	FHoverItemChanged OnHoverItemUnAssigned;

private:

	void InitGrid(UInv_InventoryGrid* Grid, UInv_ExternalInventoryComponent* ExternalInventoryComponent, const TArray<UInv_InventoryItem*>& LootList);

	void SetActiveGrid(UInv_InventoryGrid* GridToActivate);
	void SetInActiveGrid(UInv_InventoryGrid* GridToDeactivate);

	UInv_ItemDescription* GetItemDescription();
	void SetItemDescriptionSizeAndPosition(UInv_ItemDescription* DescriptionWidget, UCanvasPanel* Canvas) const;

	bool CanEquipHoverItem(const UInv_EquippedGridSlot* EquippedGridSlot,const FGameplayTag& EquipmentTypeTag ) const;
	UInv_EquippedGridSlot* FindSlotWithEquippedItem(UInv_InventoryItem* EquippedItem) const;
	UInv_EquippedGridSlot* FindSlotByEquippedType(const FGameplayTag& EquipmentTypeTag) const;

	bool IsItemPopUpGridOpen(UInv_InventoryItem* OwningItem) const;

	UFUNCTION()
	void ShowPocketsGrid();
	UFUNCTION()
	void ShowBackpackGrid();
	UFUNCTION()
	void ShowArmorGrid();

	UFUNCTION()
	void EmptyEquipmentGridSlotClicked(UInv_EquippedGridSlot* GridSlot, const FGameplayTag& EquipmentTypeTag);

	UFUNCTION()
	void ItemEquipped(UInv_InventoryItem* Item);
	UFUNCTION()
	void ItemUnEquipped(UInv_InventoryItem* Item);

	FTimerHandle ItemDescriptionTimer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Pockets{ nullptr };
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Backpack{ nullptr };
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Armor {nullptr};
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_LootInventoryGrid> Grid_Loot;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_ItemDescription> ItemDescriptionClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UUserWidget> InventoryCursorWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_PopUpInventoryGrid> ItemPopupGridClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;

	UPROPERTY()
	TObjectPtr<UUserWidget>  InventoryCursorWidget;

	UPROPERTY()
	TObjectPtr<UInv_ItemDescription> ItemDescription;

	UPROPERTY()
	TArray<TObjectPtr<UInv_EquippedGridSlot>> EquippedGridSlots;

	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem;

	UPROPERTY()
	TMap<TObjectPtr<UInv_InventoryItem>, TObjectPtr<UInv_PopUpInventoryGrid>> ActiveItemPopUpGrids;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bKeepCursorActive = false;

	bool bIsInventoryMenuOpen{ false };
};
