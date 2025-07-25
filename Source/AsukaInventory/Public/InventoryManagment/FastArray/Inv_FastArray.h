

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Inv_FastArray.generated.h"

class UInv_ExternalInventoryComponent;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UInv_InventoryItem;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryFastArrayItemChange, UInv_InventoryItem*, Item);

/**
 A single entry in the inventory array.
 */
USTRUCT(BlueprintType)
struct FInv_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	FInv_InventoryEntry(){}

private:
	friend UInv_InventoryComponent;
	friend struct FInv_InventoryFastArray;

	UPROPERTY()
	TObjectPtr<UInv_InventoryItem> Item = nullptr;
};

USTRUCT(BlueprintType)
struct FInv_InventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()
public:
	FInv_InventoryFastArray() : OwnerComponent(nullptr) {}
	FInv_InventoryFastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<UInv_InventoryItem*> GetAllItems() const;

	// FastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// End of FastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInv_InventoryEntry, FInv_InventoryFastArray>(Entries, DeltaParams, *this);
	}

	UInv_InventoryItem* AddEntry(UInv_ItemComponent* ItemComponent);
	UInv_InventoryItem* AddEntry(UInv_ExternalInventoryComponent* ExternalComponent, const FPrimaryAssetId& StaticItemManifestID);
	UInv_InventoryItem* AddEntry(const FPrimaryAssetId& StaticItemManifestID);
	void RemoveEntry(UInv_InventoryItem* Item);
	UInv_InventoryItem* FindFirstItemByType(const FGameplayTag& ItemType) const;

	FInventoryFastArrayItemChange OnItemAdded;
	FInventoryFastArrayItemChange OnItemRemoved;

private:
	friend UInv_InventoryComponent;
	UPROPERTY()
	TArray<FInv_InventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInv_InventoryFastArray> : TStructOpsTypeTraitsBase2<FInv_InventoryFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

UINTERFACE(MinimalAPI)
class UInv_ItemListInterface : public UInterface
{
	GENERATED_BODY()
};

class IInv_ItemListInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	UInv_InventoryItem* FindFirstItemByType(const FGameplayTag& ItemType) const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	void RemoveItemFromList(UInv_InventoryItem* Item);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	UInv_InventoryItem* AddItemToList(const FPrimaryAssetId& StaticItemManifestID);
};
