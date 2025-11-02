// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"


void UInv_EquippedGridSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	EquipmentComponent = UInv_InventoryStatics::GetEquipmentComponent(GetOwningPlayer());
	if (EquipmentComponent.IsValid())
	{
		if (!EquipmentComponent->GetInventoryListMutable().OnItemAdded.IsAlreadyBound(this, &ThisClass::AddItem))
		{
			EquipmentComponent->GetInventoryListMutable().OnItemAdded.AddDynamic(this, &ThisClass::AddItem);
		}
		if (!EquipmentComponent->GetInventoryListMutable().OnItemRemoved.IsAlreadyBound(this, &ThisClass::RemoveItem))
		{
			EquipmentComponent->GetInventoryListMutable().OnItemRemoved.AddDynamic(this, &ThisClass::RemoveItem);
		}
	}
}

void UInv_EquippedGridSlot::AddItem(UInv_InventoryItem* Item)
{
	if(auto EquipmentFragment = Item->GetFragmentStructByTag<FInv_EquipmentFragment>(FragmentTags::EquipmentFragment))
	{
		if(EquipmentFragment->GetEquipmentType().MatchesTag(EquipmentTypeTag))
		{
			CreateEquippedSlottedItem(Item);
		}
	}
}

void UInv_EquippedGridSlot::RemoveItem(UInv_InventoryItem* Item)
{
	if (IsValid(EquippedSlottedItemInstance) && Item->IsEquippable() && Item == EquippedSlottedItemInstance->GetInventoryItem())
	{
		RemoveEquippedSlottedItem();
	}
}


void UInv_EquippedGridSlot::HighlightSlot(const UInv_InventoryItem* HoverItem)
{
	if (HoverItem->GetItemManifest().GetItemType().MatchesTag(GetEquipmentTypeTag()))
	{
		SetSelectedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInv_EquippedGridSlot::SetOccupiedSlot()
{
	SetOccupiedTexture();
	Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
}

void UInv_EquippedGridSlot::UnHighlightSlot(const UInv_InventoryItem* HoverItem)
{
	if (IsValid(EquippedSlottedItemInstance)) return;
	if (HoverItem->GetItemManifest().GetItemType().MatchesTag(GetEquipmentTypeTag()))
	{
		SetUnoccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInv_EquippedGridSlot::UnHighlightSlot()
{
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

	if (SpatialInventory.IsValid())
	{
		if (!SpatialInventory->OnHoverItemAssigned.IsAlreadyBound(this, &ThisClass::HighlightSlot))
		{
			SpatialInventory->OnHoverItemAssigned.AddDynamic(this, &ThisClass::HighlightSlot);
		}
		if (!SpatialInventory->OnHoverItemUnAssigned.IsAlreadyBound(this, &ThisClass::UnHighlightSlot))
		{
			SpatialInventory->OnHoverItemUnAssigned.AddDynamic(this, &ThisClass::UnHighlightSlot);
		}
	}
}

UInv_EquippedSlottedItem* UInv_EquippedGridSlot::CreateEquippedSlottedItem(UInv_InventoryItem* Item)
{
	//if (!EquipmentTag.MatchesTagExact(EquipmentTypeTag)) return nullptr;
	const FInv_GridFragment* GridFragment = Item->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
	if (!GridFragment) return nullptr;

	const FIntPoint Dimensions = GridFragment->GetGridSize();
	const float TileSize = UInv_InventoryGrid::GetTileSize();
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FVector2D DrawSize = Dimensions * IconTileWidth;

	EquippedSlottedItemInstance = CreateWidget<UInv_EquippedSlottedItem>(GetOwningPlayer(), EquippedSlottedItemClass);
	EquippedSlottedItemInstance->SetInventoryItem(Item);
	EquippedSlottedItemInstance->SetEquipmentTypeTag(EquipmentTypeTag);
	EquippedSlottedItemInstance->UpdateStackCount(0);

	SetInventoryItem(Item);
	SetOccupiedSlot();

	const FInv_ImageFragment* ImageFragment = Item->GetFragmentStructByTag<FInv_ImageFragment>(FragmentTags::IconFragment);
	if (!ImageFragment) return nullptr;

	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = DrawSize;

	EquippedSlottedItemInstance->SetImageBrush(Brush);

	Overlay_Root->AddChild(EquippedSlottedItemInstance);
	const FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry();
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

bool UInv_EquippedGridSlot::HasEquippedSlottedItem() const
{
	return IsValid(EquippedSlottedItemInstance);
}
