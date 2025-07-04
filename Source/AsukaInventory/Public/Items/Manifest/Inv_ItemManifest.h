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

class UInv_CompositeBase;
struct FInv_ItemFragment;

USTRUCT(BlueprintType)
struct ASUKAINVENTORY_API FInv_ItemManifest
{
	GENERATED_BODY()
	UInv_InventoryItem* Manifest(UObject* NewOuter);
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	FGameplayTag GetItemType() const { return ItemType; }

	void SetPickupActorClass(const TSubclassOf<AActor>& NewActorClass) { PickupActorClass = NewActorClass; }

	void SpawnPickUpActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation);

	void AssimilateInventoryFragments(UInv_CompositeBase* Composite) const;
	TArray< TInstancedStruct<FInv_ItemFragment>>& GetFragmentsMutable() { return Fragments; }

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	TArray<const T*> GetAllFragmentsOfType() const;

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentOfTypeWithTag(const FGameplayTag& FragmentType) const;

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentOfType() const;

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentOfTypeMutable();


private:

	void ClearFragments();

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_ItemFragment>> Fragments;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	EInv_ItemCategory ItemCategory{ EInv_ItemCategory::None };

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "GameItems"))
	FGameplayTag ItemType;

	UPROPERTY()
	TSubclassOf<AActor> PickupActorClass;
};

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
TArray<const T*> FInv_ItemManifest::GetAllFragmentsOfType() const
{
	TArray<const T*> FragmentsOfType;
	for(const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if(const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			FragmentsOfType.Add(FragmentPtr);
		}
	}
	return FragmentsOfType;
}

template<typename T>
requires std::derived_from<T, FInv_ItemFragment>
const T* FInv_ItemManifest::GetFragmentOfTypeWithTag(const FGameplayTag& FragmentType) const
{
	for(const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if( const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			if(FragmentPtr->GetFragmentTag().MatchesTagExact(FragmentType))
				return FragmentPtr;
		}
	}
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
const T* FInv_ItemManifest::GetFragmentOfType() const
{
	for (const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* FInv_ItemManifest::GetFragmentOfTypeMutable()
{
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}
