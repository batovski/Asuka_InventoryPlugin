// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manifest/Inv_ItemManifest.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "StructUtils/InstancedStruct.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Inv_InventoryItem.generated.h"

class UInv_InventoryItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemFragmentArrayItemChange,const FGameplayTag&, ModifiedTag);

/**
 * Wrapper for FInstancedStruct to work with FFastArraySerializer
 */
USTRUCT(BlueprintType)
struct FInv_ItemFragmentArrayItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInv_ItemFragmentArrayItem() {}
	FInv_ItemFragmentArrayItem(const FInstancedStruct& InFragment)
		: Fragment(InFragment)
	{}
private:
	friend struct FInv_ItemFragmentArray;
	friend UInv_InventoryItem;
	friend UInv_ItemComponent;
	UPROPERTY()
	FInstancedStruct Fragment;
};

/**
 * Fast array container for dynamic item fragments
 * Can be used by both UInv_InventoryItem and UInv_ItemComponent
 */
USTRUCT(BlueprintType)
struct FInv_ItemFragmentArray : public FFastArraySerializer
{
	GENERATED_BODY()

	FInv_ItemFragmentArray() : Owner(nullptr) {}
	FInv_ItemFragmentArray(UObject* InOwner) : Owner(InOwner) {}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FInv_ItemFragmentArrayItem, FInv_ItemFragmentArray>(Items, DeltaParms, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	
	// Generic helper to update fragments map
	void UpdateFragmentsMap(TMap<FGameplayTag, FInstancedStruct*>& FragmentsMap, const TArrayView<int32> Indices);

	UPROPERTY()
	TArray<FInv_ItemFragmentArrayItem> Items;

	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FInv_ItemFragmentArray> : public TStructOpsTypeTraitsBase2<FInv_ItemFragmentArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

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


UCLASS()
class ASUKAINVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()
	friend struct FInv_ItemFragmentArray;
public:
	virtual void PostInitProperties() override;
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

	TArray<FInstancedStruct> GetDynamicItemFragments() const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Fragments")
	FInv_ItemFragmentArray& GetDynamicItemFragmentsArray() { return DynamicItemFragments; }

	void AssimilateInventoryFragments(UInv_CompositeBase* Composite) const;

	void InitManifestDynamicFragments(UObject* Outer);

	bool IsStackable() const;
	bool IsConsumable() const;
	bool IsEquippable() const;
	void UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, const FInv_ItemFragmentArray& DynamicFragments, const UInv_InventoryItem* Item = nullptr);

	void OnDynamicFragmentUpdated();

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentStructByTag(const FGameplayTag& FragmentType) const;
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentStructByTagMutable(const FGameplayTag& FragmentType);
	FInstancedStruct* GetFragmentStructByTagMutable(const FGameplayTag& FragmentType);

	// Mark a dynamic fragment as dirty for replication
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	void MarkDynamicFragmentDirty(T* Fragment);

	static FInstancedStruct* GetFragmentStructByTagMutable(FInv_ItemFragmentArray& DynamicFragments, TMap<FGameplayTag, FInstancedStruct*>& FragmentsMap, const FGameplayTag& FragmentType);

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Fragments")
	FItemFragmentArrayItemChange OnFragmentAdded;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Fragments")
	FItemFragmentArrayItemChange OnFragmentModified;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Fragments")
	FItemFragmentArrayItemChange OnFragmentRemoved;

private:

	UPROPERTY(Replicated)
	FInv_ItemFragmentArray DynamicItemFragments;

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

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
void UInv_InventoryItem::MarkDynamicFragmentDirty(T* Fragment)
{
	if (!Fragment){return;}
	for (FInv_ItemFragmentArrayItem& Item : DynamicItemFragments.Items)
	{
		if (T* ItemFragment = Item.Fragment.GetMutablePtr<T>())
		{
			if (ItemFragment == Fragment)
			{
				const FGameplayTag& FragmentTag = Item.Fragment.Get<FInv_ItemFragment>().GetFragmentTag();
				OnFragmentModified.Broadcast(FragmentTag);
				DynamicItemFragments.MarkItemDirty(Item);
				return;
			}
		}
	}
}
