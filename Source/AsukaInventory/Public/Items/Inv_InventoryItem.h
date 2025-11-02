// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manifest/Inv_ItemManifest.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FInv_ItemAddingOptions
{
	GENERATED_BODY()
	FInv_ItemAddingOptions() {};
	UPROPERTY()
	int32 StackCount { -1 };
	UPROPERTY()
	int32 GridIndex{ INDEX_NONE };
};

class UInv_ItemDataAsset;
/**
 * 
 */


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemFragmentModified, FGameplayTag, ModifiedFragment);

UCLASS()
class ASUKAINVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetDynamicItemFragments(const TArray<FInstancedStruct>& Fragments);
	void SetStaticItemManifestAssetId(const FPrimaryAssetId& NewAssetId);
	const FPrimaryAssetId& GetStaticItemManifestAssetId() const { return StaticItemManifestAssetId; }
	void LoadStaticItemManifest();
	const FInv_ItemManifest& GetItemManifest() const;
	FInv_ItemManifest& GetItemManifestMutable() { return StaticItemManifest.GetMutable<FInv_ItemManifest>();}

	void SetItemIndex(const int32 Index) { ItemIndex = Index; }
	int32 GetItemIndex() const { return ItemIndex; }

	const TArray<FInstancedStruct>& GetDynamicItemFragments() const { return DynamicItemFragments; }

	void AssimilateInventoryFragments(UInv_CompositeBase* Composite) const;

	void InitManifestDynamicFragments(UObject* Outer);

	bool IsStackable() const;
	bool IsConsumable() const;
	bool IsEquippable() const;
	void UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, TArray <FInstancedStruct>& DynamicFragments, const UInv_InventoryItem* Item = nullptr);

	UFUNCTION()
	void OnRep_DynamicItemFragments();

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentStructByTag(const FGameplayTag& FragmentType) const;
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentStructByTagMutable(const FGameplayTag& FragmentType);
	FInstancedStruct* GetFragmentStructByTagMutable(const FGameplayTag& FragmentType);

	static FInstancedStruct* GetFragmentStructByTagMutable(TArray<FInstancedStruct>& DynamicFragments, TMap<FGameplayTag, FInstancedStruct*>& FragmentsMap, const FGameplayTag& FragmentType);

	UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = "Inventory")
	FItemFragmentModified OnItemFragmentModified;

private:

	UPROPERTY(meta = (BaseStuct = "/Script/AsukaInventory.FInv_ItemFragment"), ReplicatedUsing = OnRep_DynamicItemFragments)
	TArray<FInstancedStruct> DynamicItemFragments {};

	TMap<FGameplayTag, FInstancedStruct*> FragmentsMap;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Inventory")
	FPrimaryAssetId StaticItemManifestAssetId;

	UPROPERTY(meta = (BaseStuct = "/Script/AsukaInventory.Inv_ItemManifest"))
	FInstancedStruct StaticItemManifest;
	UPROPERTY(Replicated)
	int32 ItemIndex{ -1 };
};

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
const T* UInv_InventoryItem::GetFragmentStructByTag(const FGameplayTag& FragmentType) const
{
	if ( FInstancedStruct* const* Fragment = FragmentsMap.Find(FragmentType))
	{
		return	(*Fragment)->GetPtr<T>();
	}
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* UInv_InventoryItem::GetFragmentStructByTagMutable(const FGameplayTag& FragmentType)
{
	FInstancedStruct* FoundFragment = GetFragmentStructByTagMutable(DynamicItemFragments, FragmentsMap, FragmentType);
	return FoundFragment ? FoundFragment->GetMutablePtr<T>() : nullptr;
}
