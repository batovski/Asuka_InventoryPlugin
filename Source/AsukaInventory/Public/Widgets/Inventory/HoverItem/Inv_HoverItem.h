// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include <Types/Inv_GridTypes.h>

#include "Inv_HoverItem.generated.h"

class UInv_InventoryGrid;
struct FGameplayTag;
class UTextBlock;
class UInv_InventoryItem;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHoverItemDropped, UInv_InventoryItem*, Item);
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_HoverItem : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetImageBrush(const FSlateBrush& Brush) const;
	void UpdateStackCount(const int32 NewStackCount);

	FGameplayTag GetItemType() const;
	int32 GetStackCount() const { return StackCount; }
	bool IsStackable() const { return bIsStackable; }
	void SetIsStackable(const bool bStackable);
	int32 GetPreviousGridIndex() const { return PreviousGridIndex; }
	void SetPreviousGridIndex(const int32 Index) { PreviousGridIndex = Index; }
	void SetGridDimensions(const FIntPoint& Dimensions) { GridDimensions = Dimensions; }

	UInv_InventoryItem* GetInventoryItem() const;
	void SetInventoryItem(UInv_InventoryItem* Item);
	FIntPoint GetGridDimensions() const { return GridDimensions; }

	void SetOwningGrid(UInv_InventoryGrid* Grid);
	UInv_InventoryGrid* GetOwningGrid() const;

	FHoverItemDropped OnHoverItemPutDown;
	FHoverItemDropped OnHoverItemDropped;

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon{ nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount{ nullptr };

	int32 PreviousGridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem {nullptr};
	bool bIsStackable{ false };
	int32 StackCount {0};
	TWeakObjectPtr<UInv_InventoryGrid> OwningGrid {nullptr};
};
