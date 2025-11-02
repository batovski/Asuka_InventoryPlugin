// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Inv_PopUpInventoryGrid.generated.h"

class UButton;
class UInv_InventoryItem;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPopUpClosed, UInv_InventoryItem*, Item);

/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_PopUpInventoryGrid : public UInv_InventoryGrid
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetOwningItem(UInv_InventoryItem* Item);

	FOnPopUpClosed OnPopUpClosed;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	virtual void OnInventoryMenuToggled(bool IsOpen) override;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;

private:
	TWeakObjectPtr<UInv_InventoryItem> OwningItem;

	FVector2D DragOffset;
	bool bIsDragging{ false };
};
