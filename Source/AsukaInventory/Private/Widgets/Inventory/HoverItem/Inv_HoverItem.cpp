// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryManagment/FastArray/Inv_FastArray.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

void UInv_HoverItem::SetImageBrush(const FSlateBrush& Brush, EInv_ItemAlignment Alignment)
{
	Image_Icon->SetBrush(Brush);
	Alignment == EInv_ItemAlignment::Horizontal
	? SetRenderTransformAngle(0.f)
	: SetRenderTransformAngle(90.f);
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

void UInv_HoverItem::UpdateImage(FGameplayTag ModifiedFragment)
{
	if (ModifiedFragment == FragmentTags::GridFragment)
	{
		const auto GridFragment = InventoryItem->GetFragmentStructByTagMutable<FInv_GridFragment>(FragmentTags::GridFragment);
		const EInv_ItemAlignment Alignment = GridFragment->GetAlignment();
		Alignment == EInv_ItemAlignment::Horizontal
			? SetRenderTransformAngle(0.f)
			: SetRenderTransformAngle(90.f);
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
	if(Item == nullptr && InventoryItem.IsValid())
	{
		InventoryItem->OnItemFragmentModified.RemoveDynamic(this, &ThisClass::UpdateImage);
	}
	InventoryItem = Item;

	if(InventoryItem.IsValid() && !InventoryItem->OnItemFragmentModified.IsAlreadyBound(this, &ThisClass::UpdateImage))
	{
		InventoryItem->OnItemFragmentModified.AddDynamic(this, &ThisClass::UpdateImage);
	}
}

void UInv_HoverItem::SetOwningInventory(const TScriptInterface<IInv_ItemListInterface>& Inventory)
{
	OwningInventory = Inventory;
}
