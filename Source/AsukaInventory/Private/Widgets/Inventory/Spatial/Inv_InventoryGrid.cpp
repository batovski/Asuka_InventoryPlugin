// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "AsukaInventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/ItemPopUp/InvItemPopUp.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	if(InventoryComponent.IsValid())
	{
		InventoryComponent->OnItemAdded.AddDynamic(this, &UInv_InventoryGrid::AddItem);
		InventoryComponent->OnStackChange.AddDynamic(this, &UInv_InventoryGrid::AddStacks);
		InventoryComponent->OnInventoryMenuToggled.AddDynamic(this, &ThisClass::OnInventoryMenuToggled);
	}
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
	return FIntPoint{
		FMath::FloorToInt32((MousePos.X - CanvasPos.X) / TileSize),
		FMath::FloorToInt32((MousePos.Y - CanvasPos.Y) / TileSize)
	};
}

EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPos, const FVector2D& MousePos) const
{

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

	if(CurrentQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex, ItemDimensions);
		return;
	}
	UnHighlightSlots(LastHighlightIndex, LastHighlightDimensions);

	if(CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		// We can swap item or stack it
		const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(CurrentQueryResult.ValidItem.Get(),FragmentTags::GridFragment);
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
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item, const int32 StackAmountOverride)
{
	return HasRoomForItem(Item->GetItemManifest(),StackAmountOverride);
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& Manifest) const
{
	const FInv_GridFragment* GridFragment = Manifest.GetFragmentOfType<FInv_GridFragment>();
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
	AssignHoverItem(ClickedInventoryItem, GridIndex, GridIndex);
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	if(!IsValid(GetHoverItem()))
	{
		InventoryComponent->GetInventoryMenu()->SetHoverItem(CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass));
	}
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(InventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	GetHoverItem()->SetImageBrush(IconBrush);
	GetHoverItem()->SetGridDimensions(GridFragment->GetGridSize());
	GetHoverItem()->SetInventoryItem(InventoryItem);
	GetHoverItem()->SetIsStackable(InventoryItem->IsStackable());
	GetHoverItem()->SetParentGridItemCategory(ItemCategory);
	GetHoverItem()->SetOwningGrid(this);

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetHoverItem());

	OnHoverItemAssigned.ExecuteIfBound(InventoryItem);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex,
	const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	GetHoverItem()->SetPreviousGridIndex(PreviousGridIndex);
	GetHoverItem()->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);

}

void UInv_InventoryGrid::PutHoverItemBack()
{
	if (!IsValid(GetHoverItem())) return;

	FInv_SlotAvailabilityResult Result = HasRoomForItem(GetHoverItem()->GetInventoryItem(), GetHoverItem()->GetStackCount());
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
	return InventoryComponent->GetInventoryMenu()->GetHoverItem();
}

void UInv_InventoryGrid::RemoveItemFromGrid(UInv_InventoryItem* Item, const int32 GridIndex)
{
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFragment) return;

	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns, [&](UInv_GridSlot* GridSlot)
	{
			GridSlot->SetInventoryItem(nullptr);
			GridSlot->SetUpperLeftIndex(INDEX_NONE);
			GridSlot->SetUnoccupiedTexture();
			GridSlot->SetAvailable(true);
			GridSlot->SetStackCount(0);
	});

	if(SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
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



void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get())) return;

	for(const auto& Availability : Result.SlotsAvailabilities)
	{
		if (Availability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
		}
		else
		{
			AddItemAtIndex(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
			UpdateGridSlots(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
		}
	}
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
	if(IsSameStackable(ClickedInventoryItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoryItem->GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
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

	//if(CurrentQueryResult.ValidItem.IsValid())
	//{
	//	// Swap items
	//	SwapWithHoverItem(ClickedInventoryItem, GridIndex);
	//}
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

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest, const int32 StackAmountOverride)
{
	FInv_SlotAvailabilityResult Result;

	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;

	const int32 MaxStackSize = Result.bStackable ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = Result.bStackable ? StackableFragment->GetStackCount() : 1;
	if(StackAmountOverride != -1 && Result.bStackable)
	{
		AmountToFill = StackAmountOverride;
	}
	if (LastClickedGridIndex == INDEX_NONE)
	{
		TSet<int32> CheckedIndices;
		for (const auto& GridSlot : GridSlots)
		{
			if (AmountToFill == 0) break;
			if (IsIndexClaimed(CheckedIndices, GridSlot->GetTileIndex())) continue;
			if (!IsInGridBounds(GridSlot->GetTileIndex(), GetItemDimensions(Manifest))) continue;

			TSet<int32> MaybeClaimed;
			FIntPoint Dimensions = GetItemDimensions(Manifest);
			if (!HasRoomAtIndex(GridSlot, Dimensions, CheckedIndices, MaybeClaimed, Manifest.GetItemType(), MaxStackSize))
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

			if (AmountToFill == 0) return Result;
		}
	}
	else
	{
		Result.TotalRoomToFill += AmountToFill;
		Result.SlotsAvailabilities.Emplace(
			FInv_SlotAvailability{
				LastClickedGridIndex,
				Result.bStackable ? AmountToFill : 0 ,
				true
			}
		);
		Result.Remainder = 0;
		LastClickedGridIndex = INDEX_NONE;
	}

	return Result;
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!MatchesCategory(Item)) return;
	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);
	AddItemToIndices(Result, Item);
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
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(NewItem, FragmentTags::IconFragment);
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
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
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

	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
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

	SlottedItem->SetIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &UInv_InventoryGrid::OnSlottedItemClicked);

	return SlottedItem;
}

void UInv_InventoryGrid::SetSlottedItemImage(const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,const UInv_SlottedItem* SlottedItem) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Rows * Columns);

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

	auto GridSlot = GridSlots[ItemDropIndex];
	if(!GridSlot->GetInventoryItem().IsValid())
	{
		PutDownOnIndex(ItemDropIndex);
	}
}


void UInv_InventoryGrid::PutDownOnIndex(const int32 GridIndex)
{
	if (!IsItemCategoryValidForGrid(GetHoverItem()->GetInventoryItem()->GetItemManifest().GetItemCategory())) return;
	if(GetHoverItem()->GetParentGridItemCategory() != ItemCategory)
	{
		LastClickedGridIndex = GridIndex;
		InventoryComponent->OnInventoryItemGridChange.Broadcast(GetHoverItem()->GetInventoryItem(), GetHoverItem()->GetStackCount(), GetHoverItem()->GetParentGridItemCategory(), ItemCategory);
	}
	else
	{
		AddItemAtIndex(GetHoverItem()->GetInventoryItem(), GridIndex, GetHoverItem()->IsStackable(), GetHoverItem()->GetStackCount());
		UpdateGridSlots(GetHoverItem()->GetInventoryItem(), GridIndex, GetHoverItem()->IsStackable(), GetHoverItem()->GetStackCount());
	}
	ClearHoverItem();
}

void UInv_InventoryGrid::ClearHoverItem()
{
	if (!IsValid(GetHoverItem())) return;
	OnHoverItemUnAssigned.ExecuteIfBound(GetHoverItem()->GetInventoryItem());
	GetHoverItem()->SetInventoryItem(nullptr);
	GetHoverItem()->SetIsStackable(false);
	GetHoverItem()->SetPreviousGridIndex(INDEX_NONE);
	GetHoverItem()->UpdateStackCount(0);
	GetHoverItem()->SetImageBrush(FSlateNoResource());
	GetHoverItem()->SetOwningGrid(nullptr);

	GetHoverItem()->RemoveFromParent();
	InventoryComponent->GetInventoryMenu()->SetHoverItem(nullptr);
	ShowCursor();
}

void UInv_InventoryGrid::OnHide()
{
	PutHoverItemBack();
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

	InventoryComponent->Server_ConsumeItem(ClickedInventoryItem);

	if(NewStackCount <= 0)
	{
		RemoveItemFromGrid(ClickedInventoryItem, Index);
	}
}

void UInv_InventoryGrid::OnPopUpMenuEquip(int32 Index)
{
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(ClickedInventoryItem)) return;
	OnItemEquipped.ExecuteIfBound(ClickedInventoryItem, Index);
}

void UInv_InventoryGrid::OnInventoryMenuToggled(const bool IsOpen)
{
	if(!IsOpen)
	{
		PutHoverItemBack();
	}
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	return Item && Item->GetItemManifest().GetItemCategory() == ItemCategory;
}

UUserWidget* UInv_InventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer()) || !IsValid(VisibleCursorClass)) return nullptr;
	if(!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), VisibleCursorClass);
	}
	return VisibleCursorWidget;
}

UUserWidget* UInv_InventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer()) || !IsValid(HiddenCursorClass)) return nullptr;
	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), HiddenCursorClass);
	}
	return HiddenCursorWidget;
}

bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	const bool bIsSameItem = ClickedInventoryItem == GetHoverItem()->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable && GetHoverItem()->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

void UInv_InventoryGrid::SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if (!IsValid(GetHoverItem())) return;
	if (!IsItemCategoryValidForGrid(GetHoverItem()->GetInventoryItem()->GetItemManifest().GetItemCategory())) return;

	UInv_InventoryItem* TempInventoryItem = GetHoverItem()->GetInventoryItem();
	const int32 TempStackCount = GetHoverItem()->GetStackCount();
	const bool bIsStackable = GetHoverItem()->IsStackable();

	AssignHoverItem(ClickedInventoryItem, GridIndex, GetHoverItem()->GetPreviousGridIndex());
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
	AddItemAtIndex(TempInventoryItem, ItemDropIndex, bIsStackable, TempStackCount);
	UpdateGridSlots(TempInventoryItem, ItemDropIndex, bIsStackable, TempStackCount);
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

	const FInv_GridFragment* GridFragment = GridSlots[GridIndex]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInv_GridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	HighlightSlots(GridIndex, Dimensions);

	ClearHoverItem();
	ShowCursor();
}

void UInv_InventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 GridIndex)
{
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(GridIndex);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

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
		ItemPopUp->OneEquip.BindDynamic(this, &ThisClass::OnPopUpMenuEquip);
	}
	else
	{
		ItemPopUp->CollapseEquipButton();
	}

	ItemPopUp->OnDrop.BindDynamic(this, &ThisClass::OnPopUpMenuDrop);

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

	InventoryComponent->Server_DropItem(GetHoverItem()->GetInventoryItem(),GetHoverItem()->GetStackCount());

	ClearHoverItem();
	ShowCursor();
}

void UInv_InventoryGrid::ShowCursor()
{
	if(!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetVisibleCursorWidget());
}

void UInv_InventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetHiddenCursorWidget());
}

void UInv_InventoryGrid::SetOwningCanvasPanel(UCanvasPanel* OwningPanel)
{
	OwningCanvasPanel = OwningPanel;
}
