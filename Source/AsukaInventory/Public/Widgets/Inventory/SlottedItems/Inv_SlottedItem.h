// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_SlottedItem.generated.h"

class UTextBlock;
class UInv_InventoryItem;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlottedItemClicked, int32, GridIndex, const FPointerEvent&, MouseEvent);

UCLASS()
class ASUKAINVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;

	void SetIsStackable(const bool bInIsStackable) { bIsStackable = bInIsStackable; }
	UImage* GetIconImage() const { return Image_Icon; }

	void SetGridIndex(const int32 InGridIndex) { GridIndex = InGridIndex; }
	int32 GetGridIndex() const { return GridIndex; }

	void SetGridDimensions(const FIntPoint& InGridDimensions) { GridDimensions = InGridDimensions; }
	FIntPoint GetGridDimensions() const { return GridDimensions; }

	void SetInventoryItem(UInv_InventoryItem* InInventoryItem);
	UInv_InventoryItem* GetInventoryItem() const { return InventoryItem.Get(); }

	void SetImageBrush(const FSlateBrush& InBrush) const;

	void UpdateStackCount(int32 StackCount) const;

	FSlottedItemClicked OnSlottedItemClicked;
	FSlottedItemClicked OnSlottedItemDoubleClicked;
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DoubleClickTreshold{ 0.2f };

	int32 GridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bIsStackable{ false };

	float LastClickTime {0.f};
	FTimerHandle DoubleClickTimerHandle;
};
