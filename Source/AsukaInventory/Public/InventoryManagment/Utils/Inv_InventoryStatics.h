// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "InventoryManagment/ItemData/Inv_ItemDataAsset.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Inv_InventoryStatics.generated.h"

class AInv_EquipActor;
class UInv_ExternalInventoryComponent;
class UInv_InventoryBase;
class UInv_HoverItem;
class UInv_InventoryComponent;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_InventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static UInv_InventoryComponent* GetInventoryComponent(const APlayerController* PlayerController);

	static EInv_ItemCategory GetItemCategoryFromItemComp(const UInv_ItemComponent* ItemTag)
	{
		if (!IsValid(ItemTag)) return EInv_ItemCategory::None;
		return ItemTag->GetItemManifest().GetItemCategory();
	}
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static void ItemHovered(APlayerController* PC, UInv_InventoryItem* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemUnhovered(APlayerController* PC);

	template<typename T,typename FuncT>
	static void ForEach2D(TArray<T>& Array,const int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInv_HoverItem* GetHoverItem(const APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static const FInstancedStruct& GetFragmentFromItem(UInv_InventoryItem* Item,
		UPARAM(meta = (Categories = "FragmentTags"))
		FGameplayTag FragmentType,
		 bool& IsFound);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void SetFragmentValuesByTag(UInv_InventoryItem* Item,
		UPARAM(Ref)
		const FInstancedStruct& Fragment,
		UPARAM(meta = (Categories = "FragmentTags"))
		FGameplayTag ItemType,
		bool& IsSucceeded);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static const UInv_InventoryItem* GetInventoryItemFromPlayerController(const APlayerController* PlayerController,
		UPARAM(meta = (Categories = "GameItems")) FGameplayTag ItemType);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static const UInv_InventoryItem* GetEquipmentItemFromPlayerController(const APlayerController* PlayerController,
		UPARAM(meta = (Categories = "GameItems")) FGameplayTag ItemType);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static const AInv_EquipActor* GetEquipmentActorFromPlayerController(const APlayerController* PlayerController,
		UPARAM(meta = (Categories = "GameItems")) FGameplayTag ItemType);

	static UInv_InventoryBase* GetInventoryWidget(const APlayerController* PlayerController);

	static FInv_ItemManifest GetItemManifestFromID(const FPrimaryAssetId& ItemId);

	static UInv_InventoryItem* CreateInventoryItemFromManifest(const FPrimaryAssetId& ItemId, UObject* WorldContextObject, const TArray<TInstancedStruct<FInv_ItemFragment>>& DynamicFragments = {});

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static AActor* CreateExternalInventoryActor(UObject* WorldContextObject, 
	UInv_InventoryComponent* InventoryComponent, 
    const FString& PickupMessage,
    TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform);
};

template<typename T, typename FuncT>
void UInv_InventoryStatics::ForEach2D(TArray<T>& Array,const int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function)
{
	for(int32 j = 0; j < Range2D.Y; ++j)
	{
		for(int32 i = 0; i < Range2D.X; ++i)
		{
			const FIntPoint Coordinates = UInv_WidgetUtils::GetPositionFromIndex(Index, GridColumns) + FIntPoint(i,j);
			const int32 TileIndex = UInv_WidgetUtils::GetIndexFromPosition(Coordinates, GridColumns);
			if(Array.IsValidIndex(TileIndex))
			{
				Function(Array[TileIndex]);
			}
		}
	}
}