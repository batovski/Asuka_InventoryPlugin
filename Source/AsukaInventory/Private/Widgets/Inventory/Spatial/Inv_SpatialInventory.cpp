// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Player/Inv_PlayerControllerBase.h"
#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Widgets/Inventory/Spatial/Inv_LootInventoryGrid.h"
#include "Widgets/Inventory/Spatial/Inv_PopUpInventoryGrid.h"
#include "Widgets/ItemDescription/Inv_ItemDescription.h"

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	FInv_SlotAvailabilityResult Result;
	if(ItemComponent->GetItemManifest().GetItemCategory() == EInv_ItemCategory::Equippable)
	{
		// if so check the equip slots for availability
		if(const auto EquipmentSlot = FindSlotByEquippedType(ItemComponent->GetItemManifest().GetItemType()))
		{
			if(!EquipmentSlot->HasEquippedSlottedItem())
			{
				UInv_EquipmentComponent* EquipmentComponent = UInv_InventoryStatics::GetEquipmentComponent(GetOwningPlayer());
				Result.OwningInventoryComponent = EquipmentComponent;
				Result.TotalRoomToFill = 1;
				return Result;
			}
		}
	}
	if (Result = Grid_Pockets->HasRoomForItem(ItemComponent); Result.TotalRoomToFill > 0)
	{
		return Result;
	}
	if( Result = Grid_Armor->HasRoomForItem(ItemComponent); Result.TotalRoomToFill > 0)
	{
		return Result;
	}
	if (Result = Grid_Backpack->HasRoomForItem(ItemComponent); Result.TotalRoomToFill > 0)
	{
		return Result;
	}
	return FInv_SlotAvailabilityResult();
}

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_InventoryItem* Item, UInv_InventoryGrid* TargetGrid,
	const int32 StackAmountOverride, const int32 GridIndex) const
{
	FInv_StackableFragment* StackableFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment);
	if (TargetGrid)
	{
		return TargetGrid->HasRoomForItem(Item->GetItemManifest(), StackableFragment, GridIndex);
	}
	return FInv_SlotAvailabilityResult();
}

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Grid_Pockets->SetOwningCanvasPanel(CanvasPanel);
	Grid_Loot->SetOwningCanvasPanel(CanvasPanel);
	Grid_Backpack->SetOwningCanvasPanel(CanvasPanel);
	Grid_Armor->SetOwningCanvasPanel(CanvasPanel);

	ShowPocketsGrid(); // Set default view
	SetInActiveGrid(Grid_Backpack);
	SetInActiveGrid(Grid_Armor);

	WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if(UInv_EquippedGridSlot* EquippedGridSlot = Cast<UInv_EquippedGridSlot>(Widget))
			{
				EquippedGridSlots.Add(EquippedGridSlot);
				EquippedGridSlot->EquippedGridSlotClicked.AddDynamic(this, &UInv_SpatialInventory::EmptyEquipmentGridSlotClicked);
				EquippedGridSlot->SetSpatialInventory(this);
			}
		});
	InventoryCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), InventoryCursorWidgetClass);

	if (const auto EquipmentComponent = UInv_InventoryStatics::GetEquipmentComponent(GetOwningPlayer()))
	{
		if (!EquipmentComponent->GetInventoryListMutable().OnItemAdded.IsAlreadyBound(this, &ThisClass::ItemEquipped))
		{
			EquipmentComponent->GetInventoryListMutable().OnItemAdded.AddDynamic(this, &ThisClass::ItemEquipped);
		}
		if (!EquipmentComponent->GetInventoryListMutable().OnItemRemoved.IsAlreadyBound(this, &ThisClass::ItemUnEquipped))
		{
			EquipmentComponent->GetInventoryListMutable().OnItemRemoved.AddDynamic(this, &ThisClass::ItemUnEquipped);
		}
	}

	CloseInventoryMenu();
}

FReply UInv_SpatialInventory::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Get the widget that was actually clicked using hit testing
	FWidgetPath WidgetPath = MouseEvent.GetEventPath() ? *MouseEvent.GetEventPath() : FWidgetPath();
	// Check if the direct target is this widget (not a child)
	// If a child widget (like background) was clicked, the path will contain it
	if (WidgetPath.IsValid() && WidgetPath.Widgets.Num() > 0)
	{
		// If the last widget in the path isn't this widget, a child was clicked
		if (WidgetPath.Widgets.Last().Widget != TakeWidget())
		{
			return FReply::Unhandled();
		}
	}
	
	// Only execute drop logic if we clicked on truly empty space
	if (GetHoverItem())
		DropHoverItem();
		
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
	UInv_ItemDescription* DescriptionWidget = GetItemDescription();
	DescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);

	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(ItemDescriptionTimer);

	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this,Item]()
	{
			GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible);
			//Assimilate the item data into the description widget
			Item->AssimilateInventoryFragments(GetItemDescription());
			
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
	return IsValid(HoverItem) ? true : false;
}

UInv_HoverItem* UInv_SpatialInventory::GetHoverItem() const
{
	if (!IsValid(HoverItem)) return nullptr;
	return HoverItem;
}

void UInv_SpatialInventory::SetHoverItem(UInv_HoverItem* NewHoverItem)
{
	HoverItem = NewHoverItem;
}

void UInv_SpatialInventory::RotateHoverItem()
{
	if (!IsValid(GetHoverItem())) return;
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(InventoryComponent);
	InventoryComponent->Server_RotateItem(GetHoverItem()->GetInventoryItem());
}

void UInv_SpatialInventory::InitGrid(UInv_InventoryGrid* Grid, UInv_ExternalInventoryComponent* ExternalInventoryComponent ,const TArray<UInv_InventoryItem*>& LootList)
{
	if (!IsValid(Grid)) return;

	Grid->CreateGrid(ExternalInventoryComponent, ExternalInventoryComponent->Rows, ExternalInventoryComponent->Columns, ExternalInventoryComponent->GridNameText);

	for(const auto Item : LootList)
	{
		Grid->AddItem(Item);
	}
	Grid->SetVisibility(ESlateVisibility::Visible);
	Grid->BindToOnInventoryToggled(this);
}

void UInv_SpatialInventory::AddDynamicGrid(const FGameplayTag& GridTag,
	UInv_ExternalInventoryComponent* ExternalInventoryComponent, const TArray<UInv_InventoryItem*>& LootList)
{
	if (!Grid_Backpack || !Grid_Armor || !Grid_Loot) return;

	if(GridTag.MatchesTag(Grid_Backpack.Get()->GetOwningGridTag()))
	{
		InitGrid(Grid_Backpack, ExternalInventoryComponent, LootList);
	}
	else if(GridTag.MatchesTag(Grid_Armor.Get()->GetOwningGridTag()))
	{
		InitGrid(Grid_Armor, ExternalInventoryComponent, LootList);
	}
	else if(GridTag.MatchesTag(Grid_Loot.Get()->GetOwningGridTag()))
	{
		InitGrid(Grid_Loot, ExternalInventoryComponent, LootList);
	}
}

void UInv_SpatialInventory::RemoveDynamicGrid(const FGameplayTag& GridTag)
{
	if (!Grid_Backpack || !Grid_Armor || !Grid_Loot) return;

	if (GridTag.MatchesTag(Grid_Backpack.Get()->GetOwningGridTag()))
	{
		SetInActiveGrid(Grid_Backpack);
	}
	else if (GridTag.MatchesTag(Grid_Armor.Get()->GetOwningGridTag()))
	{
		SetInActiveGrid(Grid_Armor);
	}
	else if (GridTag.MatchesTag(Grid_Loot.Get()->GetOwningGridTag()))
	{
		SetInActiveGrid(Grid_Loot);
	}
}

void UInv_SpatialInventory::ShowInventoryCursor()
{
	const auto PlayerController = Cast<AInv_PlayerControllerBase>(GetOwningPlayer());
	FInputModeGameAndUI InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->ChangeCursorWidget(InventoryCursorWidget);
}

void UInv_SpatialInventory::HideInventoryCursor()
{
	const auto PlayerController = Cast<AInv_PlayerControllerBase>(GetOwningPlayer());
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	bKeepCursorActive ? PlayerController->SetShowMouseCursor(true) : PlayerController->SetShowMouseCursor(false);
	PlayerController->ChangeCursorWidget(nullptr);
}

void UInv_SpatialInventory::ShowPocketsGrid()
{
	SetActiveGrid(Grid_Pockets);
}

void UInv_SpatialInventory::ShowBackpackGrid()
{
	SetActiveGrid(Grid_Backpack);
}

void UInv_SpatialInventory::ShowArmorGrid()
{
	SetActiveGrid(Grid_Armor);
}

void UInv_SpatialInventory::EmptyEquipmentGridSlotClicked(UInv_EquippedGridSlot* GridSlot,
                                                    const FGameplayTag& EquipmentTypeTag)
{
	if (!CanEquipHoverItem(GridSlot, EquipmentTypeTag)) return;

	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(InventoryComponent);
	UInv_EquipmentComponent* EquipmentComponent = UInv_InventoryStatics::GetEquipmentComponent(GetOwningPlayer());
	check(EquipmentComponent);

	InventoryComponent->Server_EquipItem(HoverItem->GetOwningInventory(), EquipmentComponent, HoverItem->GetInventoryItem(), nullptr);
	ClearHoverItem();
}

void UInv_SpatialInventory::EquippedSlottedItemClicked(UInv_EquippedSlottedItem* SlottedItem, const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());

	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable()) return;

	if (UInv_WidgetUtils::IsLeftClick(MouseEvent))
	{
		UInv_InventoryItem* ItemToUnEquip = SlottedItem->GetInventoryItem();
		if (UInv_InventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr)
		{
			if (ItemToEquip == ItemToUnEquip) return;

			UInv_EquippedGridSlot* EquippedGridSlot = FindSlotWithEquippedItem(ItemToUnEquip);
			EquippedGridSlot->RemoveEquippedSlottedItem();

			TScriptInterface<IInv_ItemListInterface> SourceInventory = GetHoverItem()->GetOwningInventory();

			UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
			check(InventoryComponent);
			UInv_EquipmentComponent* EquipmentComponent = UInv_InventoryStatics::GetEquipmentComponent(GetOwningPlayer());
			check(EquipmentComponent);
			InventoryComponent->Server_EquipItem(SourceInventory, EquipmentComponent, ItemToEquip, ItemToUnEquip);
			ClearHoverItem();
		}
		else
		{
			UInv_EquipmentComponent* EquipmentComponent = UInv_InventoryStatics::GetEquipmentComponent(GetOwningPlayer());
			check(EquipmentComponent);
			AssignHoverItem(EquipmentComponent,ItemToUnEquip);
		}
	}
	else
	{
		//TODO: Implement right click functionality (Pop up menu, etc.)
	}
}

void UInv_SpatialInventory::AssignHoverItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* InventoryItem)
{
	if (IsItemPopUpGridOpen(InventoryItem)) return;

	if (!IsValid(GetHoverItem()))
	{
		SetHoverItem(CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass));
	}
	const FInv_GridFragment* GridFragment = InventoryItem->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = InventoryItem->GetFragmentStructByTag<FInv_ImageFragment>(FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;

	const FVector2D DrawSize = UInv_InventoryGrid::GetDrawSize(UInv_InventoryGrid::GetTileSize(), GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	GetHoverItem()->SetImageBrush(IconBrush, GridFragment->GetAlignment());
	GetHoverItem()->SetGridDimensions(GridFragment->GetGridSize());
	GetHoverItem()->SetInventoryItem(InventoryItem);
	GetHoverItem()->SetIsStackable(InventoryItem->IsStackable());
	GetHoverItem()->SetOwningInventory(SourceInventory);

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetHoverItem());

	OnHoverItemAssigned.Broadcast(InventoryItem);
}

void UInv_SpatialInventory::ClearHoverItem()
{
	if (!IsValid(GetHoverItem())) return;
	OnHoverItemUnAssigned.Broadcast(GetHoverItem()->GetInventoryItem());
	GetHoverItem()->SetInventoryItem(nullptr);
	GetHoverItem()->SetIsStackable(false);
	GetHoverItem()->SetPreviousGridIndex(INDEX_NONE);
	GetHoverItem()->UpdateStackCount(0);
	GetHoverItem()->SetImageBrush(FSlateNoResource(), EInv_ItemAlignment::Horizontal);
	GetHoverItem()->SetOwningInventory(nullptr);

	GetHoverItem()->RemoveFromParent();
	SetHoverItem(nullptr);
	ShowInventoryCursor();
}

void UInv_SpatialInventory::DropHoverItem()
{
	if (!IsValid(GetHoverItem())) return;
	if (!IsValid(GetHoverItem()->GetInventoryItem())) return;
	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	AuthorityInventory->Server_DropItemFromExternalInventory(GetHoverItem()->GetOwningInventory(), GetHoverItem()->GetInventoryItem(), GetHoverItem()->GetStackCount());
	ClearHoverItem();
}

void UInv_SpatialInventory::CloseInventoryMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
	bIsInventoryMenuOpen = false;

	HideInventoryCursor();
	OnInventoryMenuToggled.Broadcast(bIsInventoryMenuOpen);
	ClearHoverItem();
}

void UInv_SpatialInventory::OpenInventoryMenu()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsInventoryMenuOpen = true;

	ShowInventoryCursor();
	OnInventoryMenuToggled.Broadcast(bIsInventoryMenuOpen);
}

void UInv_SpatialInventory::ToggleInventoryMenu()
{
	if (bIsInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

bool UInv_SpatialInventory::IsItemPopUpGridOpen(UInv_InventoryItem* OwningItem) const
{
	return ActiveItemPopUpGrids.Contains(OwningItem);
}

void UInv_SpatialInventory::CreateItemPopUpGrid(UInv_InventoryItem* OwningItem)
{
	if (!IsValid(OwningItem)) return;

	if (IsItemPopUpGridOpen(OwningItem)) return;

	if (FInv_ContainerFragment* ItemContainer = OwningItem->GetFragmentStructByTagMutable<FInv_ContainerFragment>(FragmentTags::ContainerFragment))
	{
		UInv_PopUpInventoryGrid* PopUpGrid = CreateWidget<UInv_PopUpInventoryGrid>(this, ItemPopupGridClass);

		CanvasPanel->AddChild(PopUpGrid);
		if (FInv_TextFragment* ItemNameFragment = OwningItem->GetFragmentStructByTagMutable<FInv_TextFragment>(ItemDescription::ItemNameFragment))
			ItemContainer->ContainerInventoryComponent->GridNameText = ItemNameFragment->GetText();

		InitGrid(PopUpGrid, ItemContainer->ContainerInventoryComponent, ItemContainer->ContainerInventoryComponent->GetInventoryItems());
		PopUpGrid->SetOwningItem(OwningItem);
		PopUpGrid->OnPopUpClosed.BindDynamic(this, &ThisClass::CloseItemPopUpGrid);
		ActiveItemPopUpGrids.Add(OwningItem, PopUpGrid);

		PopUpGrid->ForceLayoutPrepass();
		const FVector2D PopUpGridSize = PopUpGrid->GetDesiredSize();
		UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(PopUpGrid);
		const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
		CanvasSlot->SetPosition(MousePos);
		CanvasSlot->SetSize(PopUpGridSize);
	}
}

void UInv_SpatialInventory::CloseItemPopUpGrid(UInv_InventoryItem* OwningItem)
{
	UInv_PopUpInventoryGrid* PopUpGrid = ActiveItemPopUpGrids.FindAndRemoveChecked(OwningItem);
	if (!IsValid(PopUpGrid)) return;
	PopUpGrid->OnPopUpClosed.Unbind();
	PopUpGrid->RemoveFromParent();
}

void UInv_SpatialInventory::TryToEquipItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item)
{
	if (!IsValid(Item) || !SourceInventory) return;
	UInv_EquippedGridSlot* EquippedGridSlot = FindSlotByEquippedType(Item->GetItemManifest().GetItemType());
	if (!IsValid(EquippedGridSlot)) return;
	AssignHoverItem(SourceInventory, Item);
	EmptyEquipmentGridSlotClicked(EquippedGridSlot, EquippedGridSlot->GetEquipmentTypeTag());
}

void UInv_SpatialInventory::ItemEquipped(UInv_InventoryItem* Item)
{
	if (!IsValid(Item)) return;
	if(const FInv_ContainerFragment* ContainerFragment = Item->GetFragmentStructByTagMutable<FInv_ContainerFragment>(FragmentTags::EquipmentFragment))
	{
		if (UInv_ExternalInventoryComponent* OwningInventoryComponent = ContainerFragment->ContainerInventoryComponent)
		{
			AddDynamicGrid(ContainerFragment->GridEntityTag, OwningInventoryComponent, OwningInventoryComponent->GetInventoryItems());
		}
	}
}

void UInv_SpatialInventory::ItemUnEquipped(UInv_InventoryItem* Item)
{
	if (!IsValid(Item)) return;
	if (const FInv_ContainerFragment* ContainerFragment = Item->GetFragmentStructByTagMutable<FInv_ContainerFragment>(FragmentTags::EquipmentFragment))
	{
		if (UInv_ExternalInventoryComponent* OwningInventoryComponent = ContainerFragment->ContainerInventoryComponent)
		{
			RemoveDynamicGrid(ContainerFragment->GridEntityTag);
		}
	}
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* GridToActivate)
{
	if (!IsValid(GridToActivate)) return;
	GridToActivate->SetVisibility(ESlateVisibility::Visible);
}

void UInv_SpatialInventory::SetInActiveGrid(UInv_InventoryGrid* GridToDeactivate)
{
	if (!IsValid(GridToDeactivate)) return;
	GridToDeactivate->OnHide();
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
