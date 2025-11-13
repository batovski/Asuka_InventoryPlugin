// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "InventoryManagment/FastArray/Inv_FastArray.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Inv_EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedActorCnahged, AInv_EquipActor*, EquipActor);

struct FInv_ItemManifest;
struct FInv_EquipmentFragment;
class AInv_EquipActor;
class UInv_InventoryItem;
class UInv_InventoryComponent;
class UInv_EquipmentComponent;

// Fast Array Item for individual equipped actors
USTRUCT()
struct ASUKAINVENTORY_API FEquippedActorEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FEquippedActorEntry()
		: EquippedActor(nullptr)
	{
	}

	FEquippedActorEntry(AInv_EquipActor* InEquippedActor)
		: EquippedActor(InEquippedActor)
	{
	}

	UPROPERTY()
	TObjectPtr<AInv_EquipActor> EquippedActor;

	// Called when this item is added to the array during replication
	void PostReplicatedAdd(const struct FEquippedActorFastArray& InArraySerializer);
	
	// Called when this item is removed from the array during replication
	void PreReplicatedRemove(const struct FEquippedActorFastArray& InArraySerializer);
	
	// Called when this item is changed in the array during replication
	void PostReplicatedChange(const struct FEquippedActorFastArray& InArraySerializer);

	bool operator==(const FEquippedActorEntry& Other) const
	{
		return EquippedActor == Other.EquippedActor;
	}
};


USTRUCT()
struct ASUKAINVENTORY_API FEquippedActorFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FEquippedActorEntry> Items;

	// Owner component reference
	TWeakObjectPtr<UInv_EquipmentComponent> OwnerComponent;

	void SetOwnerComponent(UInv_EquipmentComponent* InOwnerComponent);

	// Add a new equipped actor
	void AddEquippedActor(AInv_EquipActor* NewActor);
	
	// Remove an equipped actor
	bool RemoveEquippedActor(AInv_EquipActor* ActorToRemove);
	
	// Find an equipped actor by type
	AInv_EquipActor* FindByType(const FGameplayTag& EquipmentType) const;

	// Get all actors
	const TArray<FEquippedActorEntry>& GetItems() const { return Items; }

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FEquippedActorEntry, FEquippedActorFastArray>(Items, DeltaParms, *this);
	}
};

// Template specialization for the fast array
template<>
struct TStructOpsTypeTraits<FEquippedActorFastArray> : public TStructOpsTypeTraitsBase2<FEquippedActorFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ASUKAINVENTORY_API UInv_EquipmentComponent : public UActorComponent, public IInv_ItemListInterface
{
	GENERATED_BODY()

public:
	UInv_EquipmentComponent();

	AInv_EquipActor* FindEquippedActorByType(const FGameplayTag& EquipmentType);

	// IInv_ItemListInterface interface:
	virtual UInv_InventoryItem* FindFirstItemByType_Implementation(const FGameplayTag& ItemType) const override;
	virtual void RemoveItemFromList_Implementation(UInv_InventoryItem* Item) override { EquipmentItemsList.RemoveEntry(Item); }
	virtual UInv_InventoryItem* AddItemToList_Implementation(const FPrimaryAssetId& StaticItemManifestID, const TArray<FInstancedStruct>& DynamicFragments, const FInv_ItemAddingOptions& NewItemAddingOptions) override;
	virtual UInv_InventoryItem* MoveItemToList_Implementation(UInv_InventoryItem* Item) override { return EquipmentItemsList.AddEntry(Item); }
	virtual void ChangeItemGridIndex_Implementation(UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions) override;
	virtual void MarkItemDirty_Implementation(UInv_InventoryItem* Item) override;
	virtual void AddRepSubObj(UObject* SubObj) override;
	virtual FInv_InventoryFastArray& GetInventoryListMutable() override { return EquipmentItemsList; }

	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquippedActorCnahged OnEquippedActorAdded;

	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquippedActorCnahged OnEquippedActorRemoved;
protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetOwningSkeletalMesh(USkeletalMeshComponent* NewSkeletalMesh){ OwningSkeletalMesh = NewSkeletalMesh;}

	UPROPERTY(Replicated)
	FEquippedActorFastArray EquippedActors;
	UPROPERTY(Replicated)
	FInv_InventoryFastArray EquipmentItemsList;

private:	
	UFUNCTION()
	void OnItemEquipped(UInv_InventoryItem* EquippedItem);
	UFUNCTION()
	void OnItemUnEquipped(UInv_InventoryItem* UnEquippedItem);
	UFUNCTION()
	void OnItemRemoved(UInv_InventoryItem* UnEquippedItem);
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void InitPlayerController();
	void InitInventoryComponent();
	AInv_EquipActor* SpawnedEquippedActor(FInv_EquipmentFragment* EquipmentFragment, const FInv_ItemManifest&, USkeletalMeshComponent* AttachMesh) const;
	void RemoveEquippedActor(const FGameplayTag& EquipmentType);

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMesh;
};
