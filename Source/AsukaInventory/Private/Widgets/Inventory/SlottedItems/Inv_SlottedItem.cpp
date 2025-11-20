// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"

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
	if(nullptr == InInventoryItem)
	{
		InventoryItem->OnFragmentModified.RemoveDynamic(this, &ThisClass::OnItemFragmentModified);
	}
	else
	{
		if (!InInventoryItem->OnFragmentModified.IsAlreadyBound(this, &ThisClass::OnItemFragmentModified))
		{
			InInventoryItem->OnFragmentModified.AddDynamic(this, &ThisClass::OnItemFragmentModified);
		}
	}
	InventoryItem = InInventoryItem;
}

void UInv_SlottedItem::SetImageBrush(const FSlateBrush& InBrush) const
{
	if (Image_Icon)
	{
		Image_Icon->SetBrush(InBrush);
	}
	const auto GridFragment = InventoryItem->GetFragmentStructByTagMutable<FInv_GridFragment>(FragmentTags::GridFragment);
	const auto GridAlignment = GridFragment->GetAlignment();
	UInv_HoverItem::RotateImage(Image_Icon, FVector2D(0.f, 0.f), GridAlignment);

	if (GridAlignment == EInv_ItemAlignment::Vertical)
		Image_Icon->SetRenderTranslation(FVector2D(0, Image_Icon->GetBrush().GetImageSize().X));
	else
	{
		Image_Icon->SetRenderTransformPivot(FVector2D(0.5, 0.5));
		Image_Icon->SetRenderTranslation(FVector2D::ZeroVector);
	}
}

void UInv_SlottedItem::UpdateStackCount(int32 NewStackCount)
{
	if(NewStackCount > 0)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
		Text_StackCount->SetText(FText::AsNumber(NewStackCount));
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
	StackCount = NewStackCount;
}

void UInv_SlottedItem::OnItemFragmentModified(const FGameplayTag& FragmentTag)
{
	if(FragmentTag == FragmentTags::StackableFragment)
	{
		if(const auto StackFragment = InventoryItem->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
		{
			UpdateStackCount(StackFragment->GetStackCount());
		}
	}
}
