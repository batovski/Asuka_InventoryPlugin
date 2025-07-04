// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetSwitcher.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Widgets/ItemDescription/Inv_ItemDescription.h"

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	switch (UInv_InventoryStatics::GetItemCategoryFromItemComp(ItemComponent))
	{
		case EInv_ItemCategory::Equippable:
			return Grid_Equippables->HasRoomForItem(ItemComponent);
		case EInv_ItemCategory::Consumable:
			return Grid_Consumables->HasRoomForItem(ItemComponent);
		case EInv_ItemCategory::Craftable:
			return Grid_Craftables->HasRoomForItem(ItemComponent);
		default:
			return FInv_SlotAvailabilityResult();
	}
}

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_Equippables->OnClicked.AddDynamic(this, &UInv_SpatialInventory::ShowEquippables);
	Button_Consumables->OnClicked.AddDynamic(this, &UInv_SpatialInventory::ShowConsumables);
	Button_Craftables->OnClicked.AddDynamic(this, &UInv_SpatialInventory::ShowCraftables);

	Grid_Equippables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Equippables->OnHoverItemAssigned.BindDynamic(this, &ThisClass::HoverItemAssigned);
	Grid_Equippables->OnHoverItemUnAssigned.BindDynamic(this, &ThisClass::HoverItemUnAssigned);
	Grid_Equippables->OnItemEquipped.BindDynamic(this, &ThisClass::GridEquippedItemClicked);

	Grid_Consumables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Craftables->SetOwningCanvasPanel(CanvasPanel);

	ShowEquippables(); // Set default view

	WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if(UInv_EquippedGridSlot* EquippedGridSlot = Cast<UInv_EquippedGridSlot>(Widget))
			{
				EquippedGridSlots.Add(EquippedGridSlot);
				EquippedGridSlot->EquippedGridSlotClicked.AddDynamic(this, &UInv_SpatialInventory::EquippedGridSlotClicked);
			}
		});
}

FReply UInv_SpatialInventory::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ActiveGrid->DropHoverItem();
	return FReply::Handled();
}

void UInv_SpatialInventory::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ItemDescription)) return;

	SetItemDescriptionSizeAndPosition(ItemDescription, CanvasPanel);
}

void UInv_SpatialInventory::SetItemDescriptionSizeAndPosition(UInv_ItemDescription* DescriptionWidget,
	UCanvasPanel* Canvas) const
{
	UCanvasPanelSlot* ItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(DescriptionWidget);
	if (!IsValid(ItemDescriptionCPS)) return;

	const FVector2D ItemDescriptionSize = DescriptionWidget->GetBoxSize();
	ItemDescriptionCPS->SetSize(ItemDescriptionSize);

	FVector2D ClampedPosition = UInv_WidgetUtils::GetClampedWidgetPosition(
		UInv_WidgetUtils::GetWidgetSize(Canvas),
		ItemDescriptionSize,
		UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));

	ItemDescriptionCPS->SetPosition(ClampedPosition);
}

void UInv_SpatialInventory::OnItemHovered(UInv_InventoryItem* Item)
{
	const auto& Manifest = Item->GetItemManifest();
	UInv_ItemDescription* DescriptionWidget = GetItemDescription();
	DescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);

	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(ItemDescriptionTimer);

	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this,&Manifest]()
	{
			GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible);
			//Assimilate the item data into the description widget
			Manifest.AssimilateInventoryFragments(GetItemDescription());
			
	});
	GetOwningPlayer()->GetWorldTimerManager().SetTimer(ItemDescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay,false);
}

void UInv_SpatialInventory::OnItemUnhovered()
{
	GetItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(ItemDescriptionTimer);
}

bool UInv_SpatialInventory::HasHoverItem() const
{
	if (Grid_Equippables->HasHoverItem()) return true;
	if (Grid_Consumables->HasHoverItem()) return true;
	if (Grid_Craftables->HasHoverItem()) return true;
	return false;
}

UInv_HoverItem* UInv_SpatialInventory::GetHoverItem() const
{
	if (!ActiveGrid.IsValid()) return nullptr;
	return ActiveGrid->GetHoverItem();
}

float UInv_SpatialInventory::GetTileSize() const
{
	return Grid_Equippables->GetTileSize();
}

void UInv_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippables);
}

void UInv_SpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumables);
}

void UInv_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftables);
}

void UInv_SpatialInventory::EquippedGridSlotClicked(UInv_EquippedGridSlot* GridSlot,
	const FGameplayTag& EquipmentTypeTag)
{
	if (!CanEquipHoverItem(GridSlot, EquipmentTypeTag)) return;
	const float TileSize = UInv_InventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize();
	const auto HoverItem = GetHoverItem();

	UInv_EquippedSlottedItem* EquippedSlottedItem = GridSlot->OnItemEquipped(
		HoverItem->GetInventoryItem(),
		EquipmentTypeTag,
		TileSize);

	if (!EquippedSlottedItem) return;
	EquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);

	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(InventoryComponent);

	InventoryComponent->Server_EquipSlotClicked(HoverItem->GetInventoryItem(), nullptr);

	if(GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(HoverItem->GetInventoryItem());
	}
	Grid_Equippables->ClearHoverItem();
}

void UInv_SpatialInventory::EquippedSlottedItemClicked(UInv_EquippedSlottedItem* SlottedItem, const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());

	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable()) return;

	if (UInv_WidgetUtils::IsLeftClick(MouseEvent))
	{
		UInv_InventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;
		UInv_InventoryItem* ItemToUnEquip = SlottedItem->GetInventoryItem();

		UInv_EquippedGridSlot* EquippedGridSlot = FindSlotWithEquippedItem(ItemToUnEquip);

		ClearSlotOfItem(EquippedGridSlot);

		Grid_Equippables->AssignHoverItem(ItemToUnEquip);

		RemoveEquippedSlottedItem(SlottedItem);

		MakeEquippedSlottedItem(SlottedItem, EquippedGridSlot, ItemToEquip);

		BroadcastSlotClickedDelegates(ItemToEquip, ItemToUnEquip);
	}
	else
	{
		//TODO: Implement right click functionality (Pop up menu, etc.)
	}
}

void UInv_SpatialInventory::GridEquippedItemClicked(UInv_InventoryItem* Item, const int32 GridIndex)
{
	if (!IsValid(Item)) return;
	UInv_EquippedGridSlot* EquippedGridSlot = FindSlotByEquippedType(Item->GetItemManifest().GetItemType());
	if (!IsValid(EquippedGridSlot)) return;

	ActiveGrid->PickUp(Item, GridIndex);
	EquippedGridSlotClicked(EquippedGridSlot, EquippedGridSlot->GetEquipmentTypeTag());
}

void UInv_SpatialInventory::HoverItemAssigned(const UInv_InventoryItem* SlottedItem)
{
	if (!IsValid(SlottedItem)) return;
	UInv_EquippedGridSlot* DesiredEquippedSlot = FindSlotByEquippedType(SlottedItem->GetItemManifest().GetItemType());
	if (!IsValid(DesiredEquippedSlot)) return;
	DesiredEquippedSlot->HighlightSlot();
}

void UInv_SpatialInventory::HoverItemUnAssigned(const UInv_InventoryItem* SlottedItem)
{
	if (!IsValid(SlottedItem)) return;
	UInv_EquippedGridSlot* DesiredEquippedSlot = FindSlotByEquippedType(SlottedItem->GetItemManifest().GetItemType());
	if (!IsValid(DesiredEquippedSlot)) return;
	DesiredEquippedSlot->UnHighlightSlot();
}

void UInv_SpatialInventory::DisableButton(UButton* Button) const
{
	Button_Equippables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* GridToActivate, UButton* Button)
{
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->HideCursor();
		ActiveGrid->OnHide();
	}
	ActiveGrid = GridToActivate;
	if(ActiveGrid.IsValid()) ActiveGrid->ShowCursor();
	DisableButton(Button);
	WidgetSwitcher->SetActiveWidget(GridToActivate);
}

UInv_ItemDescription* UInv_SpatialInventory::GetItemDescription()
{
	if(!IsValid(ItemDescription))
	{
		ItemDescription = CreateWidget<UInv_ItemDescription>(GetOwningPlayer(), ItemDescriptionClass);
		CanvasPanel->AddChild(ItemDescription);
	}
	return ItemDescription;
}

bool UInv_SpatialInventory::CanEquipHoverItem(const UInv_EquippedGridSlot* EquippedGridSlot,
	const FGameplayTag& EquipmentTypeTag) const
{
	if (!IsValid(EquippedGridSlot) || EquippedGridSlot->GetInventoryItem().IsValid()) return false;

	UInv_HoverItem* HoverItem = GetHoverItem();
	if (!IsValid(HoverItem)) return false;

	const UInv_InventoryItem* HeldItem = HoverItem->GetInventoryItem();

	return HasHoverItem() && 
		IsValid(HeldItem) && 
		!HoverItem->IsStackable() && 
		HeldItem->GetItemManifest().GetItemCategory() == EInv_ItemCategory::Equippable &&
		HeldItem->GetItemManifest().GetItemType().MatchesTag(EquipmentTypeTag);
}

UInv_EquippedGridSlot* UInv_SpatialInventory::FindSlotWithEquippedItem(UInv_InventoryItem* EquippedItem) const
{
	auto* FindEquippedGridSlot = EquippedGridSlots.FindByPredicate([EquippedItem](const UInv_EquippedGridSlot* GridSlot)
		{
			return GridSlot->GetInventoryItem() == EquippedItem;
		});
	return FindEquippedGridSlot ? *FindEquippedGridSlot : nullptr;
}

UInv_EquippedGridSlot* UInv_SpatialInventory::FindSlotByEquippedType(const FGameplayTag& EquipmentTypeTag) const
{
	auto* FindEquippedGridSlot = EquippedGridSlots.FindByPredicate([EquipmentTypeTag](const UInv_EquippedGridSlot* GridSlot)
		{
			return EquipmentTypeTag.MatchesTag(GridSlot->GetEquipmentTypeTag());
		});
	return FindEquippedGridSlot ? *FindEquippedGridSlot : nullptr;
}

void UInv_SpatialInventory::ClearSlotOfItem(UInv_EquippedGridSlot* EquippedGridSlot)
{
	if(IsValid(EquippedGridSlot))
	{
		EquippedGridSlot->SetEquippedSlottedItem(nullptr);
		EquippedGridSlot->SetInventoryItem(nullptr);
	}
}

void UInv_SpatialInventory::RemoveEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	if (!IsValid(EquippedSlottedItem)) return;

	if(EquippedSlottedItem->OnEquippedSlottedItemClicked.IsAlreadyBound(this,&ThisClass::EquippedSlottedItemClicked))
	{
		EquippedSlottedItem->OnEquippedSlottedItemClicked.RemoveDynamic(this, &ThisClass::EquippedSlottedItemClicked);
	}
	EquippedSlottedItem->RemoveFromParent();
}

void UInv_SpatialInventory::MakeEquippedSlottedItem(const UInv_EquippedSlottedItem* EquippedSlottedItem, UInv_EquippedGridSlot* EquippedGridSlot, UInv_InventoryItem* ItemToEquip)
{
	if(!IsValid(EquippedGridSlot)) return;
	const float TileSize = UInv_InventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize();
	UInv_EquippedSlottedItem* SlottedItem = EquippedGridSlot->OnItemEquipped(
		ItemToEquip,
		EquippedSlottedItem->GetEquipmentTypeTag(),
		TileSize);

	if (IsValid(SlottedItem)) SlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);

	EquippedGridSlot->SetEquippedSlottedItem(SlottedItem);
}

void UInv_SpatialInventory::BroadcastSlotClickedDelegates(UInv_InventoryItem* ItemToEquip,
	UInv_InventoryItem* ItemToUnEquip) const
{
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(InventoryComponent);

	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUnEquip);

	if(GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(ItemToEquip);
		InventoryComponent->OnItemUnEquipped.Broadcast(ItemToUnEquip);
	}
}
