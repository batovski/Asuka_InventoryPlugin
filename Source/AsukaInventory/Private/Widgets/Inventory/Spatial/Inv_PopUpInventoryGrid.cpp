// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_PopUpInventoryGrid.h"
#include "Components/Button.h"
#include "Items/Inv_InventoryItem.h"
#include "Components/CanvasPanelSlot.h"

void UInv_PopUpInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Close->OnClicked.AddDynamic(this, &UInv_PopUpInventoryGrid::OnCloseButtonClicked);
}

void UInv_PopUpInventoryGrid::SetOwningItem(UInv_InventoryItem* Item)
{
	OwningItem = Item;
}

void UInv_PopUpInventoryGrid::OnInventoryMenuToggled(bool IsOpen)
{
	Super::OnInventoryMenuToggled(IsOpen);

	if (!IsOpen)
	{
		OnCloseButtonClicked();
	}
}

void UInv_PopUpInventoryGrid::OnCloseButtonClicked()
{
	if (OnPopUpClosed.IsBound())
	{
		OnPopUpClosed.Execute(OwningItem.Get());
	}
}

FReply UInv_PopUpInventoryGrid::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return Reply;
}

FReply UInv_PopUpInventoryGrid::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Reply;
}

FReply UInv_PopUpInventoryGrid::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (bIsDragging)
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
		if (CanvasSlot)
		{
			FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
			FVector2D NewPosition = MousePosition - (DragOffset * InGeometry.Scale);

			if (UWidget* ParentWidget = GetParent())
			{
				FGeometry ParentGeometry = ParentWidget->GetCachedGeometry();
				NewPosition = ParentGeometry.AbsoluteToLocal(MousePosition) - DragOffset;
			}

			CanvasSlot->SetPosition(NewPosition);
		}

		return FReply::Handled();
	}

	return Reply;
}
