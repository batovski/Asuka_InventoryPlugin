// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "AsukaInventory.h"
#include "IDetailTreeNode.h"
#include "Inv_InventorySettings.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Widgets/ItemPopUp/InvItemPopUp.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}
void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector2D CanvasPos = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);

	if(CursorExitedCanvas(CanvasPos, UInv_WidgetUtils::GetWidgetSize(CanvasPanel), MousePos))
	{
		return;
	}

	UpdateTileParameters(CanvasPos, MousePos);
}
void UInv_InventoryGrid::UpdateTileParameters(const FVector2D& CanvasPos, const FVector2D& MousePos)
{
	if (!bMouseWithinCanvas) return;

	const FIntPoint Coordinates = CalculateHoveredCoordinates(CanvasPos, MousePos);

	LastTileParameters = TileParameters;
	TileParameters.TileCoordinates = Coordinates;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(Coordinates, Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPos, MousePos);

	OnTileParametersUpdated(TileParameters);
}

FIntPoint UInv_InventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPos, const FVector2D& MousePos) const
{
	const float TileSize = GetTileSize();
	return FIntPoint{
		FMath::FloorToInt32((MousePos.X - CanvasPos.X) / TileSize),
		FMath::FloorToInt32((MousePos.Y - CanvasPos.Y) / TileSize)
	};
}

EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPos, const FVector2D& MousePos) const
{
	const float TileSize = GetTileSize();
	const float TileLocalX = FMath::Fmod(MousePos.X - CanvasPos.X, TileSize);
	const float TileLocalY = FMath::Fmod(MousePos.Y - CanvasPos.Y, TileSize);

	//Determine the quadrant based on the local position within the tile
	const bool bIsTop = TileLocalY < TileSize / 2.f;
	const bool bIsLeft = TileLocalX < TileSize / 2.f;

	EInv_TileQuadrant HoverTileQuadrant;
	if(bIsTop && bIsLeft)
	{
		HoverTileQuadrant = EInv_TileQuadrant::TopLeft;
	}
	else if(bIsTop && !bIsLeft)
	{
		HoverTileQuadrant = EInv_TileQuadrant::TopRight;
	}
	else if(!bIsTop && bIsLeft)
	{
		HoverTileQuadrant = EInv_TileQuadrant::BottomLeft;
	}
	else
	{
		HoverTileQuadrant = EInv_TileQuadrant::BottomRight;
	}
	return HoverTileQuadrant;
}

void UInv_InventoryGrid::OnTileParametersUpdated(const FInv_TileParameters& Parameters)
{
	if (!IsValid(GetHoverItem())) return;

	const FIntPoint ItemDimensions = GetHoverItem()->GetGridDimensions();
	const FIntPoint StartingCoordinates = CalculateStaringPoint(Parameters.TileCoordinates, ItemDimensions, Parameters.TileQuadrant);
	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinates, Columns);

	CurrentQueryResult = CheckHoverPosition(StartingCoordinates, ItemDimensions);

	if(CurrentQueryResult.bHasSpace ||
		(CurrentQueryResult.ValidItem.IsValid() && CurrentQueryResult.ValidItem.Get() == GetHoverItem()->GetInventoryItem()))
	{
		HighlightSlots(ItemDropIndex, ItemDimensions);
		return;
	}
	UnHighlightSlots(LastHighlightIndex, LastHighlightDimensions);

	if(CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		// We can swap item or stack it
		const FInv_GridFragment* GridFragment = CurrentQueryResult.ValidItem.Get()->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
		if (!GridFragment) return;
		ChangeHoverType(CurrentQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EInv_GridSlotState::GrayedOut);
	}

}


FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions)
{
	FInv_SpaceQueryResult Result;
	const int32 Index = UInv_WidgetUtils::GetIndexFromPosition(Position, Columns);
	if (!IsInGridBounds(Index, Dimensions)) return Result;

	Result.bHasSpace = true;

	TSet<int32> OccupiedUpperLeftIndices;
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&]( const UInv_GridSlot* GridSlot)
	{
		if(GridSlot->GetInventoryItem().IsValid())
		{
			OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
			Result.bHasSpace = false;
		}
	});
	if(OccupiedUpperLeftIndices.Num() == 1) // We can swap with single item
	{
		const int32 NewUpperLeftIndex = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[NewUpperLeftIndex]->GetInventoryItem();
		Result.UpperLeftIndex = GridSlots[NewUpperLeftIndex]->GetUpperLeftIndex();
		Result.GridIndex = Index;
	}
	return Result;
}

bool UInv_InventoryGrid::CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundariesSize,
	const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UInv_WidgetUtils::IsWithinBounds(BoundaryPos, BoundariesSize, Location);
	if(!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		// We exited the canvas
		// Unhighlight the slots:
		UnHighlightSlots(LastHighlightIndex, LastHighlightDimensions);
		return true;
	}
	return false;
}

void UInv_InventoryGrid::ChangeHoverType(const int32 GridIndex, const FIntPoint& Dimensions,
	EInv_GridSlotState GridSlotState)
{
	UnHighlightSlots(LastHighlightIndex, LastHighlightDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		switch (GridSlotState)
		{
		case EInv_GridSlotState::Unoccupied:
			GridSlot->SetUnoccupiedTexture();
			break;
		case EInv_GridSlotState::Occupied:
			GridSlot->SetOccupiedTexture();
			break;
		case EInv_GridSlotState::Selected:
			GridSlot->SetSelectedTexture();
			break;
		case EInv_GridSlotState::GrayedOut:
			GridSlot->SetGrayedOutTexture();
			break;
		default:
			break;
		}
	});
	LastHighlightIndex = GridIndex;
	LastHighlightDimensions = Dimensions;
}

void UInv_InventoryGrid::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas) return;

	UnHighlightSlots(LastHighlightIndex, LastHighlightDimensions);

	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
			GridSlot->SetOccupiedTexture();
	});
	LastHighlightDimensions = Dimensions;
	LastHighlightIndex = Index;
}

void UInv_InventoryGrid::UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		if (GridSlot->IsAvailable())
		{
			GridSlot->SetUnoccupiedTexture();
		}
		else
		{
			GridSlot->SetOccupiedTexture();
		}
	});
}

FIntPoint UInv_InventoryGrid::CalculateStaringPoint(const FIntPoint& Coordinates, const FIntPoint& Dimensions,
                                                    const EInv_TileQuadrant Quadrant) const
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoord;

	switch (Quadrant)
	{
	case EInv_TileQuadrant::TopLeft:
		StartingCoord.X = Coordinates.X - FMath::FloorToInt32(0.5f * Dimensions.X);
		StartingCoord.Y = Coordinates.Y - FMath::FloorToInt32(0.5f * Dimensions.Y);
		break;
	case EInv_TileQuadrant::TopRight:
		StartingCoord.X = Coordinates.X - FMath::FloorToInt32(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinates.Y - FMath::FloorToInt32(0.5f * Dimensions.Y);
		break;

	case EInv_TileQuadrant::BottomLeft:
		StartingCoord.X = Coordinates.X - FMath::FloorToInt32(0.5f * Dimensions.X);
		StartingCoord.Y = Coordinates.Y - FMath::FloorToInt32(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	case EInv_TileQuadrant::BottomRight:
		StartingCoord.X = Coordinates.X - FMath::FloorToInt32(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinates.Y - FMath::FloorToInt32(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
		default:
			return FIntPoint(-1,-1);
	}
	return StartingCoord;
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest().GetFragmentOfType<FInv_GridFragment>(), ItemComponent->GetItemManifest().GetItemType());
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_GridFragment* GridFragment) const
{
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlot* GridSlot,
												const UInv_GridSlot* SubGridSlot,
												const TSet<int32>& CheckedIndices,
												TSet<int32>& OUT OutMaybeClaimed,
												const FGameplayTag& ItemType,
												const int32 MaxStackSize) const
{
	// Check if the index claimed:
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetTileIndex())) return false;

	// Check if we have an item here:
	if (!HasValidItem(SubGridSlot))
	{
		// TODO:: is it really necessary?
		OutMaybeClaimed.Add(SubGridSlot->GetTileIndex());
		return true;
	}
	// CHeck if it is parent slot:
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot)) return false;

	//is it stackable?
	const UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable()) return false;

	// Check if the item type matches:
	if (!DoesItemTypeMatch(SubItem, ItemType)) return false;

	if (GridSlot->GetStackCount() >= MaxStackSize) return false;

	return true;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,
	const int32 AmountToFill, const UInv_GridSlot* GridSlot) const
{
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1;
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlot* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	if( const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlot* UpperLeftSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftSlot->GetStackCount();
	}
	return CurrentSlotStackCount;
}

void UInv_InventoryGrid::PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if(AssignHoverItem(ClickedInventoryItem, GridIndex, GridIndex))
		SetPendingItemInGrid(ClickedInventoryItem, GridIndex);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	AuthorityInventory->GetInventoryMenu()->AssignHoverItem(GetGridInventoryInterface(), InventoryItem);
}

bool UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex,
	const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	GetHoverItem()->SetPreviousGridIndex(PreviousGridIndex);
	GetHoverItem()->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
	return true;

}

void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	for (const auto& Availability : Result.SlotsAvailabilities)
	{
		if (Availability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
		}
	}
}

void UInv_InventoryGrid::PutHoverItemBack()
{
	if (!IsValid(GetHoverItem())) return;

	FInv_SlotAvailabilityResult Result = HasRoomForItem(GetHoverItem()->GetInventoryItem()->GetFragmentStructByTagMutable<FInv_GridFragment>(FragmentTags::GridFragment),
		GetHoverItem()->GetInventoryItem()->GetItemManifest().GetItemType(),
		GetHoverItem()->GetInventoryItem()->GetFragmentStructByTag<FInv_StackableFragment>(FragmentTags::StackableFragment),
		GetHoverItem()->GetStackCount());
	Result.Item = GetHoverItem()->GetInventoryItem();

	AddStacks(Result);
	ClearHoverItem();

}

bool UInv_InventoryGrid::HasHoverItem() const
{
	return IsValid(GetHoverItem());
}

UInv_HoverItem* UInv_InventoryGrid::GetHoverItem() const
{
	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	return AuthorityInventory ? AuthorityInventory->GetInventoryMenu()->GetHoverItem() : nullptr;
}

float UInv_InventoryGrid::GetTileSize()
{
	const UInv_InventorySettings* Settings = UInv_InventorySettings::Get();
	check(Settings && "Inventory Settings not found! Make sure the plugin is properly configured.");
	return Settings->TileSize;
}

void UInv_InventoryGrid::RemoveItemFromGrid(const UInv_InventoryItem* Item, const FIntPoint& Dimensions, const int32 GridIndex)
{
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
			GridSlot->SetInventoryItem(nullptr);
			GridSlot->SetUpperLeftIndex(INDEX_NONE);
			GridSlot->SetUnoccupiedTexture();
			GridSlot->SetAvailable(true);
			GridSlot->SetStackCount(0);
	});

	if(SlottedItems.Contains(GridIndex) && SlottedItems[GridIndex].Get()->GetInventoryItem() == Item)
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

void UInv_InventoryGrid::SetPendingItemInGrid(UInv_InventoryItem* Item, const int32 GridIndex)
{
	const FInv_GridFragment* GridFragment = Item->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
	if (!GridFragment) return;

	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns, [&](UInv_GridSlot* GridSlot)
		{
			//GridSlot->SetInventoryItem(nullptr);
			//GridSlot->SetUpperLeftIndex(INDEX_NONE);
			GridSlot->SetGrayedOutTexture();
			//GridSlot->SetAvailable(true);
			//GridSlot->SetStackCount(0);
		});

	if (SlottedItems.Contains(GridIndex))
	{
		//TODO:: tint image
	}
}

void UInv_InventoryGrid::RemoveAllItemFromGrid()
{
	for (const TObjectPtr<UInv_GridSlot> GridSlot : GridSlots)
	{
		GridSlot->SetInventoryItem(nullptr);
		GridSlot->SetUpperLeftIndex(INDEX_NONE);
		GridSlot->SetUnoccupiedTexture();
		GridSlot->SetAvailable(true);
		GridSlot->SetStackCount(0);
	}
	TArray<int32> Keys;
	SlottedItems.GetKeys(Keys);
	for(const auto & Key : Keys)
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(Key, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

TScriptInterface<IInv_ItemListInterface> UInv_InventoryGrid::GetGridInventoryInterface() const
{
	return IsValid(OwningInventoryComponent.GetObject()) ? OwningInventoryComponent : nullptr;
}

void UInv_InventoryGrid::OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());

	check(GridSlots.IsValidIndex(GridIndex));
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();

	if(!IsValid(GetHoverItem()) && UInv_WidgetUtils::IsLeftClick(MouseEvent))
	{
		PickUp(ClickedInventoryItem, GridIndex);
		return;
	}

	if(UInv_WidgetUtils::IsRightClick(MouseEvent))
	{
		CreateItemPopUp(GridIndex);
		return;
	}

	// Stack Item if possible
	if(ClickedInventoryItem && IsSameStackable(ClickedInventoryItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoryItem->GetFragmentStructByTag<FInv_StackableFragment>(FragmentTags::StackableFragment);
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = GetHoverItem()->GetStackCount();

		if(ShouldSwapStackCounts(RoomInClickedSlot, HoveredStackCount, MaxStackSize))
		{
			// Swap stack counts
			SwapStackCounts(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}

		if(ShouldConsumeHoverItemStacks(RoomInClickedSlot,HoveredStackCount))
		{
			ConsumeHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}

		if(ShouldFillInStack(ClickedStackCount,HoveredStackCount))
		{

			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return;
		}

		if(RoomInClickedSlot == 0)
		{
			return;
		}
	}

	if(CurrentQueryResult.ValidItem.IsValid() && CurrentQueryResult.ValidItem.Get() == GetHoverItem()->GetInventoryItem())
	{
		PutDownOnIndex(CurrentQueryResult.GridIndex);
	}
}

void UInv_InventoryGrid::OnSlottedItemDoubleClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());

	check(GridSlots.IsValidIndex(GridIndex));
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();

	if (!IsValid(GetHoverItem()) && UInv_WidgetUtils::IsLeftClick(MouseEvent))
	{
		// Display Grid Widget:
		CreateItemPopUpGrid(GridIndex);
	}
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices,const int32 Index) const
{
	return (CheckedIndices.Contains(Index));
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetTileIndex();
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const
{
	if(StartIndex < 0 || StartIndex >= GridSlots.Num()) return false;

	const int32 EndColumn = StartIndex % Columns + ItemDimensions.X;
	const int32 EndRow = StartIndex / Columns + ItemDimensions.Y;
	return EndColumn <= Columns && EndRow <= Rows;
}

void UInv_InventoryGrid::MoveHoverItemFromOneGridToAnother(const int32 GridIndex) const
{
	const auto InventoryInterface = GetGridInventoryInterface();
	const auto HoverItemGridInterface = GetHoverItem()->GetOwningInventory();
	//Simple move should be called in parent
	if (InventoryInterface == HoverItemGridInterface)
	{
		FInv_ItemAddingOptions NewItemAddingOptions;
		NewItemAddingOptions.GridIndex = GridIndex;
		auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
		AuthorityInventory->Server_ChangeItemGridIndex(InventoryInterface,
			GetHoverItem()->GetInventoryItem(), NewItemAddingOptions);
	}
	else
	{
		if (InventoryInterface && HoverItemGridInterface)
		{
			FInv_ItemAddingOptions NewItemAddingOptions;
			NewItemAddingOptions.StackCount = GetHoverItem()->GetStackCount();
			NewItemAddingOptions.GridIndex = GridIndex;
			const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
			AuthorityInventory->Server_AddNewItemByItem(InventoryInterface, GetHoverItem()->GetInventoryItem(), NewItemAddingOptions);
			AuthorityInventory->Server_RemoveItem(HoverItemGridInterface, GetHoverItem()->GetInventoryItem());
		}
	}
}

void UInv_InventoryGrid::CreateGrid(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const int32 NewRows,
	const int32 NewColumns, const FText& NewGridName)
{
	Rows = NewRows;
	Columns = NewColumns;
	GridName = NewGridName;

	ConstructGrid();

	if(IsValid(OwningInventoryComponent.GetObject()))
	{
		OwningInventoryComponent->GetInventoryListMutable().OnItemAdded.RemoveDynamic(this, &UInv_InventoryGrid::AddItem);
		OwningInventoryComponent->GetInventoryListMutable().OnItemRemoved.RemoveDynamic(this, &ThisClass::RemoveItem);
		OwningInventoryComponent->GetInventoryListMutable().OnItemChanged.RemoveDynamic(this, &ThisClass::ChangeItem);
	}

	OwningInventoryComponent = SourceInventory;
	if (IsValid(OwningInventoryComponent.GetObject()))
	{
		OwningInventoryComponent->GetInventoryListMutable().OnItemAdded.AddDynamic(this, &UInv_InventoryGrid::AddItem);
		OwningInventoryComponent->GetInventoryListMutable().OnItemRemoved.AddDynamic(this, &ThisClass::RemoveItem);
		OwningInventoryComponent->GetInventoryListMutable().OnItemChanged.AddDynamic(this, &ThisClass::ChangeItem);
	}
}

void UInv_InventoryGrid::BindToOnInventoryToggled(UInv_InventoryBase* MenuBase)
{
	if (MenuBase)
	{
		InventoryMenu = MenuBase;
		if (MenuBase->OnInventoryMenuToggled.IsAlreadyBound(this, &ThisClass::OnInventoryMenuToggled))
			MenuBase->OnInventoryMenuToggled.RemoveDynamic(this, &ThisClass::OnInventoryMenuToggled);
		MenuBase->OnInventoryMenuToggled.AddDynamic(this, &ThisClass::OnInventoryMenuToggled);
	}
}

void UInv_InventoryGrid::DropHoverItemInGrid(const int32 GridIndex) const
{
	if (!IsValid(GetHoverItem()->GetInventoryItem())) return;

	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	if (GetHoverItem()->IsHoverItemRotated())
	{
		AuthorityInventory->Server_RotateItem(GetHoverItem()->GetInventoryItem());
	}

	if (const FInv_StackableFragment* StackableFragment = GetHoverItem()->GetInventoryItem()->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment))
	{
		if (StackableFragment->GetStackCount() != GetHoverItem()->GetStackCount())
		{
			//modify old item stack count

			AuthorityInventory->Server_UpdateItemStackCount(GetHoverItem()->GetInventoryItem(), StackableFragment->GetStackCount() - GetHoverItem()->GetStackCount());
			if (const auto InventoryInterface = GetGridInventoryInterface())
			{
				FInv_ItemAddingOptions NewItemAddingOptions;
				NewItemAddingOptions.StackCount = GetHoverItem()->GetStackCount();
				NewItemAddingOptions.GridIndex = GridIndex;
				AuthorityInventory->Server_AddNewItemByItem(InventoryInterface, GetHoverItem()->GetInventoryItem(), NewItemAddingOptions);
			}
			return;
		}
	}

	MoveHoverItemFromOneGridToAnother(GridIndex);
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_GridFragment* GridFragment, const FGameplayTag& ItemType,
	const FInv_StackableFragment* StackableFragment, const int32 GridIndex)
{
	FInv_SlotAvailabilityResult Result;
	Result.bStackable = StackableFragment != nullptr;
	Result.OwningInventoryComponent = GetGridInventoryInterface();

	const int32 MaxStackSize = Result.bStackable ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = Result.bStackable ? StackableFragment->GetStackCount() : 1;

	if (GridIndex == INDEX_NONE)
	{
		TSet<int32> CheckedIndices;
		for (const auto& GridSlot : GridSlots)
		{
			if (AmountToFill == 0) break;
			if (IsIndexClaimed(CheckedIndices, GridSlot->GetTileIndex())) continue;
			if (!IsInGridBounds(GridSlot->GetTileIndex(), GetItemDimensions(GridFragment))) continue;

			TSet<int32> MaybeClaimed;
			FIntPoint Dimensions = GetItemDimensions(GridFragment);
			if (!HasRoomAtIndex(GridSlot, Dimensions, CheckedIndices, MaybeClaimed, ItemType, MaxStackSize))
			{
				continue;
			}

			//How much to fill this slot?
			const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill, GridSlot);
			if (AmountToFillInSlot == 0) continue;

			CheckedIndices.Append(MaybeClaimed);

			// Add item to the result
			Result.TotalRoomToFill += AmountToFillInSlot;
			Result.SlotsAvailabilities.Emplace(
				FInv_SlotAvailability{
					HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
					Result.bStackable ? AmountToFillInSlot : 0 ,
					HasValidItem(GridSlot)
				}
			);
			AmountToFill -= AmountToFillInSlot;
			Result.Remainder = AmountToFill;
			if(HasValidItem(GridSlot))
				Result.Item = GridSlot->GetInventoryItem();
			if (AmountToFill == 0) return Result;
		}
	}
	else
	{
		Result.TotalRoomToFill += AmountToFill;
		Result.SlotsAvailabilities.Emplace(
			FInv_SlotAvailability{
				GridIndex,
				Result.bStackable ? AmountToFill : 0 ,
				true
			}
		);
		Result.Remainder = 0;
	}

	return Result;
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!IsValid(Item)) return;
	FInv_StackableFragment* StackableFragment = Item->GetFragmentStructByTagMutable<FInv_StackableFragment>(FragmentTags::StackableFragment);
	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item->GetFragmentStructByTagMutable<FInv_GridFragment>(FragmentTags::GridFragment),
		Item->GetItemManifest().GetItemType(),
		StackableFragment, Item->GetItemIndex());
	AddItemToIndices(Result, Item);
}

void UInv_InventoryGrid::RemoveItem(UInv_InventoryItem* Item)
{
	if (UInv_SlottedItem* SlottedItem = FindSlottedItem(Item))
	{
		const int32 Index = SlottedItem->GetGridIndex();
		RemoveItemFromGrid(SlottedItem->GetInventoryItem(), SlottedItem->GetGridDimensions(), Index);
	}
}

void UInv_InventoryGrid::ChangeItem(UInv_InventoryItem* Item)
{
	RemoveItem(Item);
	AddItem(Item);
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	for (const auto& Availability : Result.SlotsAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* NewItem, const int32 Index, const bool bStackable, const int32 StackAmount)
{
	const FInv_GridFragment* GridFragment = NewItem->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = NewItem->GetFragmentStructByTag<FInv_ImageFragment>(FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment)
	{
		UE_LOG(LogInventory, Warning, TEXT("Grid or Image Fragment missing for item: %s"), *NewItem->GetName());
		return;
	}
	UInv_SlottedItem* SlottedItem = CreateSlottedItem(NewItem, GridFragment, ImageFragment,Index, bStackable, StackAmount);
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	SlottedItems.Add(Index, SlottedItem);
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GetTileSize(),GridFragment));
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * GetTileSize();
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));

	if(bStackableItem)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}

	const FInv_GridFragment* GridFragment = NewItem->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
	if (!GridFragment) return;
	const FIntPoint Dimension = GridFragment->GetGridSize();
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimension, Columns, [&](UInv_GridSlot* GridSlot)
	{
			GridSlot->SetInventoryItem(NewItem);
			GridSlot->SetUpperLeftIndex(Index);
			GridSlot->SetOccupiedTexture();
			GridSlot->SetAvailable(false);
	});
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions,
	const TSet<int32>& CheckedIndices, TSet<int32>& OutMaybeClaimed,
	const FGameplayTag& ItemType, const int32 MaxStackSize)
{
	bool bHasRoomAtIndex = true;
	UInv_InventoryStatics::ForEach2D(GridSlots, GridSlot->GetTileIndex(), Dimensions, Columns, [&](const UInv_GridSlot* SubGridSlot)
	{
		if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices, OutMaybeClaimed, ItemType, MaxStackSize))
		{
			OutMaybeClaimed.Add(SubGridSlot->GetTileIndex());
		}
		else
		{
			bHasRoomAtIndex = false;
		}
	});
	return bHasRoomAtIndex;
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* NewItem, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment, int32 Index, bool bStackable, int32 StackAmount) const
{
	//Create a widget for the item
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetInventoryItem(NewItem);
	SetSlottedItemImage(GridFragment, ImageFragment, SlottedItem);
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetGridDimensions(GridFragment->GetGridSize());
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &UInv_InventoryGrid::OnSlottedItemClicked);
	SlottedItem->OnSlottedItemDoubleClicked.AddDynamic(this, &UInv_InventoryGrid::OnSlottedItemDoubleClicked);

	return SlottedItem;
}

void UInv_InventoryGrid::SetSlottedItemImage(const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,const UInv_SlottedItem* SlottedItem) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GetTileSize(),GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const float TileSize, const FInv_GridFragment* GridFragment)
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FIntPoint RealDrawSize = GridFragment->GetAlignment() == EInv_ItemAlignment::Horizontal
	? GridFragment->GetGridSize()
	: FIntPoint(GridFragment->GetGridSize().Y, GridFragment->GetGridSize().X);
	return RealDrawSize * IconTileWidth;
}

void UInv_InventoryGrid::ConstructGrid()
{
	GridSlots.Empty();
	GridSlots.Reserve(Rows * Columns);
	const float TileSize = GetTileSize();
	for (int32 j = 0; j < Rows; ++j)
	{
		for (int32 i = 0; i < Columns; ++i)
		{
			UInv_GridSlot* NewGridSlot = CreateWidget<UInv_GridSlot>(this, GridSlotClass);
			CanvasPanel->AddChildToCanvas(NewGridSlot);
			const FIntPoint TilePosition(i, j);
			NewGridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(TilePosition, Columns));

			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(NewGridSlot);
			GridCPS->SetSize(FVector2D(TileSize));
			GridCPS->SetPosition(TilePosition * TileSize);

			GridSlots.Add(NewGridSlot);

			NewGridSlot->OnGridSlotClicked.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotClicked);
			NewGridSlot->OnGridSlotHovered.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotHovered);
			NewGridSlot->OnGridSlotUnHovered.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotUnHovered);
		}
	}

	Text_GridName->SetText(GridName);

	const float GridWidth = Columns * TileSize + GridFramePadding.X;
	const float GridHeight = Rows * TileSize + GridFramePadding.Y;
	SizeBox_GridFrame->SetWidthOverride(GridWidth);
	SizeBox_GridFrame->SetHeightOverride(GridHeight);
}

void UInv_InventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(GetHoverItem())) return;
	if (!GridSlots.IsValidIndex(ItemDropIndex)) return;

	if(CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		OnSlottedItemClicked(CurrentQueryResult.UpperLeftIndex, MouseEvent);
		return;
	}
	if (!CurrentQueryResult.bHasSpace) return;
	auto GridSlot = GridSlots[ItemDropIndex];
	if(!GridSlot->GetInventoryItem().IsValid())
	{
		PutDownOnIndex(ItemDropIndex);
	}
}

void UInv_InventoryGrid::PutDownOnIndex(const int32 GridIndex)
{
	DropHoverItemInGrid(GridIndex);
	ClearHoverItem();
}

void UInv_InventoryGrid::ClearHoverItem() const
{
	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	AuthorityInventory->GetInventoryMenu()->ClearHoverItem();
}

void UInv_InventoryGrid::OnHide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInv_InventoryGrid::OnVisible()
{
	SetVisibility(ESlateVisibility::Visible);
}


void UInv_InventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(GetHoverItem())) return;

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInv_InventoryGrid::OnGridSlotUnHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(GetHoverItem())) return;

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetUnoccupiedTexture();
	}
}

void UInv_InventoryGrid::OnPopUpMenuSplit(int32 SplitAmount, int32 Index)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;
	if(!ClickedInventoryItem->IsStackable()) return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInv_GridSlot* UpperLeftSlot = GridSlots[UpperLeftIndex];
	const int32 ClickedStackCount = UpperLeftSlot->GetStackCount();
	const int32 NewStackCount = ClickedStackCount - SplitAmount;

	UpperLeftSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	AssignHoverItem(ClickedInventoryItem, UpperLeftIndex, UpperLeftIndex);
	GetHoverItem()->UpdateStackCount(SplitAmount);
}

void UInv_InventoryGrid::OnPopUpMenuDrop(int32 Index)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;

	PickUp(ClickedInventoryItem, Index);
	DropHoverItem();
}

void UInv_InventoryGrid::OnPopUpMenuConsume(int32 Index)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);
	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	AuthorityInventory->Server_ConsumeItem(ClickedInventoryItem);
}

void UInv_InventoryGrid::OnPopUpMenuEquip(int32 Index)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;
	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	Cast<UInv_SpatialInventory>(AuthorityInventory->GetInventoryMenu())->TryToEquipItem(GetGridInventoryInterface(), ClickedInventoryItem);
}

bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsStackable && GetHoverItem()->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

bool UInv_InventoryGrid::ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
	const int32 MaxStackSize) const
{
	return RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize;
}

void UInv_InventoryGrid::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 GridIndex)
{
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	GridSlot->SetStackCount(HoveredStackCount);

	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(GridIndex);
	ClickedSlottedItem->UpdateStackCount(HoveredStackCount);

	GetHoverItem()->UpdateStackCount(ClickedStackCount);
}

bool UInv_InventoryGrid::ShouldConsumeHoverItemStacks(const int32 RoomInClickedSlot,
	const int32 HoveredStackCount) const
{
	return RoomInClickedSlot >= HoveredStackCount;
}

bool UInv_InventoryGrid::ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const
{
	return RoomInClickedSlot < HoveredStackCount;
}

void UInv_InventoryGrid::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount,
                                                const int32 GridIndex)
{
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount + AmountToTransfer;

	GridSlots[GridIndex]->SetStackCount(NewClickedStackCount);
	SlottedItems.FindChecked(GridIndex)->UpdateStackCount(NewClickedStackCount);

	const FInv_GridFragment* GridFragment = GridSlots[GridIndex]->GetInventoryItem()->GetFragmentStructByTag<FInv_GridFragment>(FragmentTags::GridFragment);
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	HighlightSlots(GridIndex, Dimensions);

	const auto InventoryInterface = GetGridInventoryInterface();
	const auto HoverItemGridInterface = GetHoverItem()->GetOwningInventory();;
	if (InventoryInterface && HoverItemGridInterface && GetHoverItem()->GetInventoryItem() != GridSlots[GridIndex]->GetInventoryItem().Get())
	{
		const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
		AuthorityInventory->Server_RemoveItem(HoverItemGridInterface, GetHoverItem()->GetInventoryItem());
		AuthorityInventory->Server_UpdateItemStackCount(GridSlots[GridIndex]->GetInventoryItem().Get(), NewClickedStackCount);
	}

	ClearHoverItem();
}

void UInv_InventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 GridIndex)
{
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(GridIndex);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

	const auto InventoryInterface = GetGridInventoryInterface();
	const auto HoverItemGridInterface = GetHoverItem()->GetOwningInventory();
	if (InventoryInterface && HoverItemGridInterface && GetHoverItem()->GetInventoryItem() != GridSlots[GridIndex]->GetInventoryItem().Get())
	{
		const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
		AuthorityInventory->Server_UpdateItemStackCount(GetHoverItem()->GetInventoryItem(), Remainder);
		AuthorityInventory->Server_UpdateItemStackCount(GridSlots[GridIndex]->GetInventoryItem().Get(), NewStackCount);
	}

	GetHoverItem()->UpdateStackCount(Remainder);

}

void UInv_InventoryGrid::CreateItemPopUp(const int32 GridIndex)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;
	if (IsValid(ItemPopUp) && IsValid(UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp))) return;
	ItemPopUp = CreateWidget<UInvItemPopUp>(this, ItemPopUpClass);
	ItemPopUp->SetGridIndex(GridIndex);

	OwningCanvasPanel->AddChild(ItemPopUp);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp);
	const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasSlot->SetPosition(MousePos - ItemPopUpOffset);
	CanvasSlot->SetSize(ItemPopUp->GetBoxSize());

	const int32 SliderMax = GridSlots[GridIndex]->GetStackCount() - 1;
	if(ClickedInventoryItem->IsStackable() && SliderMax > 0)
	{
		ItemPopUp->OnSplit.BindDynamic(this, &ThisClass::OnPopUpMenuSplit);
		ItemPopUp->SetSliderParams(SliderMax, FMath::Max(1, GridSlots[GridIndex]->GetStackCount() / 2));
	}
	else
	{
		ItemPopUp->CollapseSplitButton();
	}

	if(ClickedInventoryItem->IsConsumable())
	{
		ItemPopUp->OnConsume.BindDynamic(this, &ThisClass::OnPopUpMenuConsume);
	}
	else
	{
		ItemPopUp->CollapseConsumeButton();
	}

	if(ClickedInventoryItem->IsEquippable())
	{
		ItemPopUp->OnEquip.BindDynamic(this, &ThisClass::OnPopUpMenuEquip);
	}
	else
	{
		ItemPopUp->CollapseEquipButton();
	}

	ItemPopUp->OnDrop.BindDynamic(this, &ThisClass::OnPopUpMenuDrop);

}

void UInv_InventoryGrid:: CreateItemPopUpGrid(const int32 GridIndex)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;

	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	Cast<UInv_SpatialInventory>(AuthorityInventory->GetInventoryMenu())->CreateItemPopUpGrid(ClickedInventoryItem);
}

bool UInv_InventoryGrid::IsItemPresentedAsSlottedItem(const UInv_InventoryItem* Item) const
{
	for(auto& SlottedItem : SlottedItems)
	{
		if(SlottedItem.Value.Get()->GetInventoryItem() == Item)
		{
			return true;
		}
	}
	return false;
}

UInv_SlottedItem* UInv_InventoryGrid::FindSlottedItem(const UInv_InventoryItem* Item) const
{
	for (auto& SlottedItem : SlottedItems)
	{
		if (SlottedItem.Value.Get()->GetInventoryItem() == Item)
		{
			return SlottedItem.Value.Get();
		}
	}
	return nullptr;
}

void UInv_InventoryGrid::DropHoverItem()
{
	if (!IsValid(GetHoverItem())) return;
	if (!IsValid(GetHoverItem()->GetInventoryItem())) return;
	const auto AuthorityInventory = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	AuthorityInventory->Server_DropItemFromExternalInventory(OwningInventoryComponent, GetHoverItem()->GetInventoryItem(),GetHoverItem()->GetStackCount());
	ClearHoverItem();
}

void UInv_InventoryGrid::SetOwningCanvasPanel(UCanvasPanel* OwningPanel)
{
	OwningCanvasPanel = OwningPanel;
}
