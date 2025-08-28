// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemComponent.generated.h"


class UInv_ItemDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ASUKAINVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

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

	TArray<FInstancedStruct>& GetDynamicFragmentsMutable() { return DynamicFragments; }

	UFUNCTION()
	void OnRep_DynamicFragments();

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentOfTypeMutable(const FGameplayTag& FragmentType);

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
	void UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, TArray<FInstancedStruct>& NewDynamicFragments);
	void ApplyDynamicFragmentsToManifest();

	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FPrimaryAssetId StaticItemManifestID;
	UPROPERTY(Replicated, ReplicatedUsing= OnRep_DynamicFragments)
	TArray<FInstancedStruct> DynamicFragments;

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
	if (FInstancedStruct** Fragment = FragmentsMap.Find(FragmentType))
	{
		return (*Fragment)->GetMutablePtr<T>();
	}
	return nullptr;
}
