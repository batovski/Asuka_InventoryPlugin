// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Inv_ItemComponent.generated.h"


class UInv_ItemDataAsset;
class UInv_ItemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ASUKAINVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()
	friend struct FInv_ItemFragmentArray;

public:	
	// Sets default values for this component's properties
	UInv_ItemComponent();
	void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void PickedUp();
	void InitItemManifest(const FPrimaryAssetId& NewItemManifestID);
	void InitDynamicData(const TArray<FInstancedStruct>& NewDynamicFragments);

	static UInv_ItemComponent* SpawnPickUpActor(const TSubclassOf<AActor>& ActorToSpawn, const UObject* WorldContextObject, const FVector& SpawnLocation,
		const FRotator& SpawnRotation);
	FString& GetPickupMessage();
	FInv_ItemManifest GetItemManifest() const { return StaticItemManifest; }
	const FPrimaryAssetId& GetStaticItemManifestID() const;

	TArray<FInstancedStruct> GetDynamicFragments() const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Fragments")
	FInv_ItemFragmentArray& GetDynamicFragmentsArray() { return DynamicFragments; }

	void OnDynamicFragmentUpdated();

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentOfTypeMutable(const FGameplayTag& FragmentType);

	// Mark a dynamic fragment as dirty for replication
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	void MarkDynamicFragmentDirty(T* Fragment);

	// Blueprint-accessible fragment change events
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Fragments")
	FItemFragmentArrayItemChange OnFragmentAdded;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Fragments")
	FItemFragmentArrayItemChange OnFragmentModified;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Fragments")
	FItemFragmentArrayItemChange OnFragmentRemoved;

	// Skeletal mesh replication
	UFUNCTION(Server, Reliable)
	void SetSkeletalMeshAsset(USkeletalMesh* MeshAsset);
	UFUNCTION(NetMulticast, Reliable)
	void OnSkeletalMeshAssetChanged(USkeletalMesh* MeshAsset);
	UFUNCTION()
	void OnRep_ReplicatedSkeletalMesh();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnPickedUp();

private:
	void UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, const FInv_ItemFragmentArray& NewDynamicFragments);
	void ApplyDynamicFragmentsToManifest();

	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FPrimaryAssetId StaticItemManifestID;
	UPROPERTY(Replicated)
	FInv_ItemFragmentArray DynamicFragments;

	TMap<FGameplayTag, FInstancedStruct*> FragmentsMap;

	UPROPERTY()
	FInv_ItemManifest StaticItemManifest;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ReplicatedSkeletalMesh, Category = "Inventory")
	TSoftObjectPtr<USkeletalMesh> ReplicatedSkeletalMesh;
};

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* UInv_ItemComponent::GetFragmentOfTypeMutable(const FGameplayTag& FragmentType)
{
	FInstancedStruct* FoundFragment = UInv_InventoryItem::GetFragmentStructByTagMutable(DynamicFragments, FragmentsMap, FragmentType);
	return FoundFragment ? FoundFragment->GetMutablePtr<T>() : nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
void UInv_ItemComponent::MarkDynamicFragmentDirty(T* Fragment)
{
	if (!Fragment)
	{
		return;
	}

	// Find the matching item in the fast array
	for (FInv_ItemFragmentArrayItem& Item : DynamicFragments.Items)
	{
		if (T* ItemFragment = Item.Fragment.GetMutablePtr<T>())
		{
			// Check if this is the same memory address
			if (ItemFragment == Fragment)
			{
				const FGameplayTag& FragmentTag = Item.Fragment.Get<FInv_ItemFragment>().GetFragmentTag();
				OnFragmentModified.Broadcast(FragmentTag);
				DynamicFragments.MarkItemDirty(Item);
				return;
			}
		}
	}
}
