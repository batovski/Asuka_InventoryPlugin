// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Inv_EquipmentComponent.generated.h"


struct FInv_ItemManifest;
struct FInv_EquipmentFragment;
class AInv_EquipActor;
class UInv_InventoryItem;
class UInv_InventoryComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ASUKAINVENTORY_API UInv_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetOwningSkeletalMesh(USkeletalMeshComponent* NewSkeletalMesh){ OwningSkeletalMesh = NewSkeletalMesh;}

private:	
	UFUNCTION()
	void OnItemEquipped(UInv_InventoryItem* EquippedItem);
	UFUNCTION()
	void OnItemUnEquipped(UInv_InventoryItem* UnEquippedItem);
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void InitPlayerController();
	void InitInventoryComponent();
	AInv_EquipActor* SpawnedEquippedActor(FInv_EquipmentFragment* EquipmentFragment, const FInv_ItemManifest&, USkeletalMeshComponent* AttachMesh) const;
	AInv_EquipActor* FindEquippedActorByType(const FGameplayTag& EquipmentType);
	void RemoveEquippedActor(const FGameplayTag& EquipmentType);

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMesh;

	UPROPERTY()
	TArray<TObjectPtr<AInv_EquipActor>> EquippedActors;
};
