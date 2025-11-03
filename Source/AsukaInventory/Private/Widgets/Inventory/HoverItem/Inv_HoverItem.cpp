// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryManagment/FastArray/Inv_FastArray.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

void UInv_HoverItem::RotateImage(UImage* Image_Icon, const FVector2D& PivotPoint, EInv_ItemAlignment Alignment)
{
	Image_Icon->SetRenderTransformPivot(PivotPoint);

	if (Alignment == EInv_ItemAlignment::Horizontal)
	{
		Image_Icon->SetRenderTransformAngle(0.f);
	}
	else
	{
		Image_Icon->SetRenderTransformAngle(-90.f);
	}
}

bool UInv_HoverItem::IsHoverItemRotated() const
{
	return ItemAlignment != OriginalItemAlignment;
}

void UInv_HoverItem::RotateHoverItem()
{
	if (ItemAlignment == EInv_ItemAlignment::Horizontal)
	{
		ItemAlignment = EInv_ItemAlignment::Vertical;
	}
	else
	{
		ItemAlignment = EInv_ItemAlignment::Horizontal;
	}
	SetGridDimensions(FIntPoint(GridDimensions.Y, GridDimensions.X));
	RotateImage(Image_Icon, FVector2D(0.5f, 0.5f), ItemAlignment);
}

void UInv_HoverItem::SetImageBrush(const FSlateBrush& Brush)
{
	Image_Icon->SetBrush(Brush);
}

void UInv_HoverItem::UpdateStackCount(const int32 NewStackCount)
{
	StackCount = NewStackCount;
	if(NewStackCount > 0)
	{
		Text_StackCount->SetText(FText::AsNumber(NewStackCount));
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FGameplayTag UInv_HoverItem::GetItemType() const
{
	if(InventoryItem.IsValid())
	{
		return InventoryItem->GetItemManifest().GetItemType();
	}
	return FGameplayTag::EmptyTag;
}

void UInv_HoverItem::SetIsStackable(const bool bStackable)
{
	bIsStackable = bStackable;
	if(!bIsStackable)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UInv_InventoryItem* UInv_HoverItem::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UInv_HoverItem::SetInventoryItem(UInv_InventoryItem* Item)
{
	InventoryItem = Item;
	if (nullptr != InventoryItem)
	{
		if (const auto GridFragment = InventoryItem->GetFragmentStructByTagMutable<FInv_GridFragment>(FragmentTags::GridFragment))
		{
			OriginalItemAlignment = GridFragment->GetAlignment();
			ItemAlignment = OriginalItemAlignment;
			RotateImage(Image_Icon, FVector2D(0.5f, 0.5f), ItemAlignment);
		}
	}
}

void UInv_HoverItem::SetOwningInventory(const TScriptInterface<IInv_ItemListInterface>& Inventory)
{
	OwningInventory = Inventory;
}
