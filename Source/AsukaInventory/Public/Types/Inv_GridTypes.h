// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inv_GridTypes.generated.h"
class UInv_InventoryItem;
/**
 * 
 */
UENUM(BlueprintType)
enum class EInv_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	External,
	None
};

USTRUCT()
struct FInv_SlotAvailability
{
	GENERATED_BODY()
	FInv_SlotAvailability(){}
	FInv_SlotAvailability(int32 InIndex, int32 InAmountToFill, bool bInItemAtIndex)
		: Index(InIndex), AmountToFill(InAmountToFill), bItemAtIndex(bInItemAtIndex) {}
	int32 Index{ INDEX_NONE };
	int32 AmountToFill{ 0 };
	bool bItemAtIndex{ false };
};

USTRUCT()
struct FInv_SlotAvailabilityResult
{
	GENERATED_BODY()
	FInv_SlotAvailabilityResult(){}
	TWeakObjectPtr<UInv_InventoryItem> Item;
	int32 TotalRoomToFill{ 0 };
	int32 Remainder{ 0 };
	bool bStackable{ false };
	TArray<FInv_SlotAvailability> SlotsAvailabilities;
};

UENUM(BlueprintType)
enum class EInv_TileQuadrant : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	None
};

USTRUCT(BlueprintType)
struct FInv_TileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	FIntPoint TileCoordinates{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	int32 TileIndex{ INDEX_NONE };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	EInv_TileQuadrant TileQuadrant{ EInv_TileQuadrant::None };
};

inline bool operator==(const FInv_TileParameters& A, const FInv_TileParameters& B)
{
	return A.TileCoordinates == B.TileCoordinates &&
		   A.TileIndex == B.TileIndex &&
		A.TileQuadrant == B.TileQuadrant;
}

USTRUCT()
struct FInv_SpaceQueryResult
{
	GENERATED_BODY()

	//
	bool bHasSpace{ false };

	//Valid if there is a single item we can swap with
	TWeakObjectPtr<UInv_InventoryItem> ValidItem = nullptr;

	int32 UpperLeftIndex{ INDEX_NONE };
};