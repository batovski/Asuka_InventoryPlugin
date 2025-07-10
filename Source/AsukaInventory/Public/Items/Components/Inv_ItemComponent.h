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
	void InitDynamicData(const TArray<TInstancedStruct<FInv_ItemFragment>>& NewDynamicFragments);
	FString& GetPickupMessage();
	FInv_ItemManifest GetItemManifest() const { return StaticItemManifest; }
	const FPrimaryAssetId& GetStaticItemManifestID() const;

	UFUNCTION()
	void OnRep_DynamicFragments();

	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentOfTypeMutable();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnPickedUp();

private:
	void ApplyDynamicFragmentsToManifest();
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FPrimaryAssetId StaticItemManifestID;
	UPROPERTY(Replicated, ReplicatedUsing= OnRep_DynamicFragments)
	TArray<TInstancedStruct<FInv_ItemFragment>> DynamicFragments;
	UPROPERTY()
	FInv_ItemManifest StaticItemManifest;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage;
};

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* UInv_ItemComponent::GetFragmentOfTypeMutable()
{
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : DynamicFragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}
