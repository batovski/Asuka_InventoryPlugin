// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"
#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

void UInv_EquippedGridSlot::HighlightSlot()
{
	SetOccupiedTexture();
	Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
}

void UInv_EquippedGridSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	if (InventoryComponent.IsValid())
	{
		if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &ThisClass::AddItem))
		{
			InventoryComponent->OnItemEquipped.AddDynamic(this, &ThisClass::AddItem);
		}
		if (!InventoryComponent->OnItemUnEquipped.IsAlreadyBound(this, &ThisClass::RemoveItem))
		{
			InventoryComponent->OnItemUnEquipped.AddDynamic(this, &ThisClass::RemoveItem);
		}
		if (!InventoryComponent->GetInventoryListMutable().OnItemRemoved.IsAlreadyBound(this, &ThisClass::RemoveItem))
		{
			InventoryComponent->GetInventoryListMutable().OnItemRemoved.AddDynamic(this, &ThisClass::RemoveItem);
		}
	}
}

void UInv_EquippedGridSlot::AddItem(UInv_InventoryItem* Item)
{
	if(Item->IsEquippable() && Item->GetOwningGridEntityTag() == OwningEntityGridTag)
	{
		if (IsValid(EquippedSlottedItemInstance)) return;
		CreateEquippedSlottedItem(Item);
	}
}

void UInv_EquippedGridSlot::RemoveItem(UInv_InventoryItem* Item)
{
	if (IsValid(EquippedSlottedItemInstance) && Item->IsEquippable() && Item == EquippedSlottedItemInstance->GetInventoryItem())
	{
		RemoveEquippedSlottedItem();
	}
}

void UInv_EquippedGridSlot::UnHighlightSlot()
{
	if (IsValid(EquippedSlottedItemInstance)) return;
	SetUnoccupiedTexture();
	Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
}

FReply UInv_EquippedGridSlot::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	EquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}

void UInv_EquippedGridSlot::SetSpatialInventory(UInv_SpatialInventory* NewSpatialInventory)
{
	SpatialInventory = NewSpatialInventory;
}

UInv_EquippedSlottedItem* UInv_EquippedGridSlot::CreateEquippedSlottedItem(UInv_InventoryItem* Item)
{
	//if (!EquipmentTag.MatchesTagExact(EquipmentTypeTag)) return nullptr;
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFragment) return nullptr;

	const FIntPoint Dimensions = GridFragment->GetGridSize();
	const float TileSize = UInv_InventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize();
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FVector2D DrawSize = Dimensions * IconTileWidth;

	EquippedSlottedItemInstance = CreateWidget<UInv_EquippedSlottedItem>(GetOwningPlayer(), EquippedSlottedItemClass);
	EquippedSlottedItemInstance->SetInventoryItem(Item);
	EquippedSlottedItemInstance->SetEquipmentTypeTag(EquipmentTypeTag);
	EquippedSlottedItemInstance->UpdateStackCount(0);

	SetInventoryItem(Item);

	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	if (!ImageFragment) return nullptr;

	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = DrawSize;

	EquippedSlottedItemInstance->SetImageBrush(Brush);

	Overlay_Root->AddChild(EquippedSlottedItemInstance);
	const FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry();
	auto OverlayPos = OverlayGeometry.Position;
	const auto OverlaySize = OverlayGeometry.Size;

	const float LeftPadding = OverlaySize.X / 2.f - DrawSize.X / 2.f;
	const float TopPadding = OverlaySize.Y / 2.f - DrawSize.Y / 2.f;

	UOverlaySlot* OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(EquippedSlottedItemInstance);
	OverlaySlot->SetPadding(FMargin(LeftPadding, TopPadding));
	EquippedSlottedItemInstance->OnEquippedSlottedItemClicked.AddDynamic(SpatialInventory.Get(), &UInv_SpatialInventory::EquippedSlottedItemClicked);
	return EquippedSlottedItemInstance;
}

void UInv_EquippedGridSlot::RemoveEquippedSlottedItem()
{
	if (!IsValid(EquippedSlottedItemInstance)) return;

	if (EquippedSlottedItemInstance->OnEquippedSlottedItemClicked.IsAlreadyBound(SpatialInventory.Get(), &UInv_SpatialInventory::EquippedSlottedItemClicked))
	{
		EquippedSlottedItemInstance->OnEquippedSlottedItemClicked.RemoveDynamic(SpatialInventory.Get(), &UInv_SpatialInventory::EquippedSlottedItemClicked);
	}
	EquippedSlottedItemInstance->RemoveFromParent();

	SetEquippedSlottedItem(nullptr);
	SetInventoryItem(nullptr);

	UnHighlightSlot();
}
