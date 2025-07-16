// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manifest/Inv_ItemManifest.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_InventoryItem.generated.h"


class UInv_ItemDataAsset;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetDynamicItemFragments(const TArray<TInstancedStruct<FInv_ItemFragment>>& Fragments);
	void SetStaticItemManifestAssetId(const FPrimaryAssetId& NewAssetId);
	const FPrimaryAssetId& GetStaticItemManifestAssetId() const { return StaticItemManifestAssetId; }
	void LoadStaticItemManifest();
	const FInv_ItemManifest& GetItemManifest() const;
	FInv_ItemManifest& GetItemManifestMutable() { return StaticItemManifest.GetMutable<FInv_ItemManifest>();}

	const TArray<TInstancedStruct<FInv_ItemFragment>>& GetDynamicItemFragmentsMutable() { return DynamicItemFragments; }

	bool IsStackable() const;
	bool IsConsumable() const;
	bool IsEquippable() const;
	static void UpdateManifestData(TArray<TInstancedStruct<FInv_ItemFragment>>& StaticFragments, TArray <TInstancedStruct<FInv_ItemFragment>>& DynamicFragments);

	UFUNCTION()
	void OnRep_DynamicItemFragments();


	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentOfTypeMutable();

private:

	UPROPERTY(meta = (BaseStuct = "/Scripts/AsukaInventory.FInv_DynamicItemFragment"), ReplicatedUsing = OnRep_DynamicItemFragments)
	TArray<TInstancedStruct<FInv_ItemFragment>> DynamicItemFragments {};

	UPROPERTY(VisibleAnywhere, Replicated)
	FPrimaryAssetId StaticItemManifestAssetId;

	UPROPERTY(meta = (BaseStuct = "/Scripts/AsukaInventory.Inv_ItemManifest"))
	FInstancedStruct StaticItemManifest;
};


template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* UInv_InventoryItem::GetFragmentOfTypeMutable()
{
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : DynamicItemFragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}

	auto& StaticFragments = GetItemManifestMutable().GetFragmentsMutable();
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : StaticFragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			if (FInv_ItemFragment* FragmentBasePtr = Fragment.GetMutablePtr<FInv_ItemFragment>(); FragmentBasePtr->IsDynamicFragment())
			{
				TInstancedStruct<T> BaseStruct = TInstancedStruct<T>::Make(*FragmentPtr);
;				TInstancedStruct<FInv_ItemFragment>& NewDynamicFragment = DynamicItemFragments.Add_GetRef(BaseStruct);
				return NewDynamicFragment.GetMutablePtr<T>();
			}
			return FragmentPtr;
		}
	}
	return nullptr;
}

template<typename FragmentType>
const FragmentType* GetFragment(const UInv_InventoryItem* Item, const FGameplayTag& Tag)
{
	if (!IsValid(Item)) return nullptr;

	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	return Item->GetItemManifest().GetFragmentOfTypeWithTag<FragmentType>(Tag);
}

