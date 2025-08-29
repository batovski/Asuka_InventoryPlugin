// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/Inv_GridTypes.h"
#include "StructUtils/InstancedStruct.h"

#include "Inv_ItemManifest.generated.h"

/**
 * Necessary Data for creating a new Inventory Item
 */

class UInv_ItemComponent;
class UInv_CompositeBase;
struct FInv_ItemFragment;

USTRUCT(BlueprintType)
struct ASUKAINVENTORY_API FInv_ItemManifest
{
	friend class UInv_InventoryItem;
	friend class UInv_ItemComponent;

	GENERATED_BODY()
	void Manifest();
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	FGameplayTag GetItemType() const { return ItemType; }

	const TArray<FInstancedStruct>& GetFragments() const { return StaticFragments; }

	void AddStaticFragment(const FInstancedStruct& Fragment) { StaticFragments.Add(Fragment);}

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	TArray<const T*> GetAllFragmentsOfType() const;
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentOfType() const;

private:
	TArray<FInstancedStruct>& GetFragmentsMutable() { return StaticFragments; }

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct, BaseStuct = "/Script/AsukaInventory.FInv_ItemFragment"))
	TArray<FInstancedStruct> StaticFragments;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	EInv_ItemCategory ItemCategory{ EInv_ItemCategory::None };

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "GameItems"))
	FGameplayTag ItemType;
};

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
TArray<const T*> FInv_ItemManifest::GetAllFragmentsOfType() const
{
	TArray<const T*> FragmentsOfType;
	for (const FInstancedStruct& Fragment : StaticFragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			FragmentsOfType.Add(FragmentPtr);
		}
	}
	return FragmentsOfType;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
const T* FInv_ItemManifest::GetFragmentOfType() const
{
	for (const FInstancedStruct& Fragment : StaticFragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}
