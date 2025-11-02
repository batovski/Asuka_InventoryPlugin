// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"

FReply UInv_SlottedItem::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	GetWorld()->GetTimerManager().ClearTimer(DoubleClickTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(DoubleClickTimerHandle, [this, MouseEvent]()
		{
			OnSlottedItemClicked.Broadcast(GridIndex, MouseEvent);
		}, DoubleClickTreshold, false);
	return FReply::Handled();
}

FReply UInv_SlottedItem::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	GetWorld()->GetTimerManager().ClearTimer(DoubleClickTimerHandle);
	OnSlottedItemDoubleClicked.Broadcast(GridIndex, InMouseEvent);
	return FReply::Handled();
}

void UInv_SlottedItem::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemHovered(GetOwningPlayer(), InventoryItem.Get());
}

void UInv_SlottedItem::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());
}

void UInv_SlottedItem::SetInventoryItem(UInv_InventoryItem* InInventoryItem)
{
	InventoryItem = InInventoryItem;
}

void UInv_SlottedItem::SetImageBrush(const FSlateBrush& InBrush) const
{
	if (Image_Icon)
	{
		Image_Icon->SetBrush(InBrush);
	}
}

void UInv_SlottedItem::UpdateStackCount(int32 StackCount) const
{
	if(StackCount > 0)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
		Text_StackCount->SetText(FText::AsNumber(StackCount));
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}
