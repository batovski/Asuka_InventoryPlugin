// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryGrid.generated.h"


class IInv_ItemListInterface;
enum class EInv_GridSlotState : uint8;
struct FGameplayTag;
struct FInv_ImageFragment;
struct FInv_GridFragment;
struct FInv_ItemManifest;
class UInvItemPopUp;
class UInv_HoverItem;
class UInv_SlottedItem;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UCanvasPanel;
class UInv_GridSlot;
/**
 * 
 */

DECLARE_DYNAMIC_DELEGATE_OneParam(FHoverItemAssigned, const UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_DELEGATE_OneParam(FHoverItemUnAssigned, const UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FItemEquipped, UInv_InventoryItem*, Item, const int32, GridIndex, UInv_InventoryGrid*, OwningGrid);

UCLASS()
class ASUKAINVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION()
	virtual void AddItem(UInv_InventoryItem* Item);

	UFUNCTION()
	virtual void RemoveItem(UInv_InventoryItem* Item);

	virtual void NativeOnInitialized() override;
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item, const int32 StackAmountOverride = -1, const int32 GridIndex = -1);
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetOwningCanvasPanel(UCanvasPanel* OwningPanel);

	virtual void DropHoverItem();
	bool HasHoverItem() const;

	void AssignHoverItem(UInv_InventoryItem* InventoryItem);
	void PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);

	UInv_HoverItem* GetHoverItem() const;

	float GetTileSize() const {return TileSize;}
	void ClearHoverItem();

	void OnHide();

	virtual TScriptInterface<IInv_ItemListInterface> GetGridInventoryInterface() const;
	const FGameplayTag& GetOwningGridTag() const { return GridEntityTag; }

	FHoverItemAssigned OnHoverItemAssigned;
	FHoverItemUnAssigned OnHoverItemUnAssigned;
	FItemEquipped OnItemEquipped;

protected:
	void DropHoverItemInGrid(UInv_InventoryGrid* InventoryGrid, const int32 GridIndex) const;

	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& Manifest, const int32 StackAmountOverride = -1, const int32 GridIndex = -1);
	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem);

	void RemoveItemFromGrid(const UInv_InventoryItem* Item, const int32 GridIndex);
	void SetPendingItemInGrid(UInv_InventoryItem* Item, const int32 GridIndex);
	void RemoveAllItemFromGrid();

	virtual UInv_SlottedItem* FindSlottedItem(const UInv_InventoryItem* Item) const;

	virtual bool IsItemCategoryValidForGrid(const EInv_ItemCategory Category) const { return Category == ItemCategory; }
	virtual void PutDownOnIndex(const int32 GridIndex);

	UFUNCTION()
	virtual	void OnInventoryMenuToggled(bool IsOpen);

	UFUNCTION()
	void ChangeItem(UInv_InventoryItem* Item);

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;

private:

	void ConstructGrid();
	bool MatchesCategory(const UInv_InventoryItem* Item) const;
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const;
	void SetSlottedItemImage(const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,
		const UInv_SlottedItem* SlottedItem) const;
	void AddItemAtIndex(UInv_InventoryItem* NewItem, const int32 Index, const bool bStackable, const int32 StackAmount);
	UInv_SlottedItem* CreateSlottedItem(UInv_InventoryItem* NewItem, const FInv_GridFragment* GridFragment,
		const FInv_ImageFragment* ImageFragment, int32 Index, bool bStackable, int32 StackAmount) const;

	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const;

	void UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount);
	bool HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions,
		const TSet<int32>& CheckedIndices, TSet<int32>& OUT OutMaybeClaimed,
		const FGameplayTag& ItemType, const int32 MaxStackSize);

	bool IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const;
	bool HasValidItem(const UInv_GridSlot* SubGridSlot) const;
	bool IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const;
	bool DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const;
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const;
	void MoveHoverItemFromOneGridToAnother(const UInv_InventoryGrid* InventoryGrid, int32 GridIndex) const;

	FIntPoint GetItemDimensions(const FInv_ItemManifest& Manifest) const;
	bool CheckSlotConstraints(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot,
		const TSet<int32>& CheckedIndices, TSet<int32>& OUT OutMaybeClaimed,
		const FGameplayTag& ItemType, const int32 MaxStackSize) const;
	int32 DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountToFill, const UInv_GridSlot* GridSlot) const;
	int32 GetStackAmount(const UInv_GridSlot* GridSlot) const;

	void AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex);
	void AddStacks(const FInv_SlotAvailabilityResult& Result);
	void PutHoverItemBack();

	void UpdateTileParameters(const FVector2D& CanvasPos, const FVector2D& MousePos);
	FIntPoint CalculateHoveredCoordinates(const FVector2D& CanvasPos, const FVector2D& MousePos) const;
	EInv_TileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPos, const FVector2D& MousePos) const;
	void OnTileParametersUpdated(const FInv_TileParameters& Parameters);
	FIntPoint CalculateStaringPoint(const FIntPoint& Coordinates, const FIntPoint& Dimensions, const EInv_TileQuadrant Quadrant) const;
	FInv_SpaceQueryResult CheckHoverPosition(const FIntPoint& Positions, const FIntPoint& Dimensions);
	bool CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundariesSize, const FVector2D& Location);

	void ChangeHoverType(const int32 GridIndex, const FIntPoint& Dimensions, EInv_GridSlotState GridSlotState);

	void HighlightSlots(const int32 Index, const FIntPoint& Dimensions);
	void UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions);

	bool IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const;
	void SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);
	bool ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount, const int32 MaxStackSize) const;
	void SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 GridIndex);
	bool ShouldConsumeHoverItemStacks(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const;
	bool ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const;
	void ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 GridIndex);
	void FillInStack(const int32 FillAmount, const int32 Remainder, const int32 GridIndex);

	void CreateItemPopUp(const int32 GridIndex);

	bool IsItemPresentedAsSlottedItem(const UInv_InventoryItem* Item) const;

	UFUNCTION()
	void OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotUnHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnPopUpMenuSplit(int32 SplitAmount, int32 Index);
	UFUNCTION()
	void OnPopUpMenuDrop(int32 Index);
	UFUNCTION()
	void OnPopUpMenuConsume(int32 Index);
	UFUNCTION()
	void OnPopUpMenuEquip(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	EInv_ItemCategory ItemCategory;


	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInvItemPopUp> ItemPopUpClass;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	UPROPERTY()
	TObjectPtr<UInvItemPopUp> ItemPopUp;


	UPROPERTY()
	TMap<int32, TObjectPtr<UInv_SlottedItem>> SlottedItems;

	UPROPERTY(EditAnywhere,Category= "Inventory")
	int32 Rows;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Columns;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D ItemPopUpOffset;
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "InventoryGrid"))
	FGameplayTag GridEntityTag;

	TWeakObjectPtr <UCanvasPanel> OwningCanvasPanel;

	FInv_TileParameters TileParameters;
	FInv_TileParameters LastTileParameters;
	int32 ItemDropIndex{ INDEX_NONE };
	FInv_SpaceQueryResult CurrentQueryResult;
	bool bMouseWithinCanvas;
	bool bLastMouseWithinCanvas;
	int32 LastHighlightIndex;
	FIntPoint LastHighlightDimensions;
};
