// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Inv_EquipActor.generated.h"

class UInv_InventoryItem;

UCLASS()
class ASUKAINVENTORY_API AInv_EquipActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInv_EquipActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FGameplayTag GetEquipmentType() const { return EquipmentType; }
	UFUNCTION(Server, Reliable)
	void SetEquipmentType(const FGameplayTag& NewType);
	UFUNCTION(Server, Reliable)
	void SetOwningController(AController* Controller);
	UFUNCTION(Server, Reliable)
	void SetOwningItem(UInv_InventoryItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	AController* GetOwningController();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInv_InventoryItem* GetOwningItem() const;

	UFUNCTION(Server, Reliable)
	void SetSkeletalMeshAsset(USkeletalMesh* MeshAsset);
	UFUNCTION(Server, Reliable)
	void SetSkeletalMeshAnimationLayer(TSubclassOf<UAnimInstance>  NewAnimLayer);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void OnSkeletalMeshAssetChanged(USkeletalMesh* MeshAsset);
	UFUNCTION(NetMulticast, Reliable)
	void OnSkeletalMeshAnimationLayerChanged(TSubclassOf<UAnimInstance>  NewAnimLayer);

	UFUNCTION()
	void OnRep_ReplicatedSkeletalMesh();
	UFUNCTION()
	void OnRep_ReplicatedAnimationLayer();

private:
	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory")
	FGameplayTag EquipmentType;
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Inventory")
	TWeakObjectPtr<AController> OwningController {nullptr};
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Inventory")
	TWeakObjectPtr<UInv_InventoryItem> OwningItem{ nullptr };
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ReplicatedSkeletalMesh, Category = "Inventory")
	TSoftObjectPtr<USkeletalMesh> ReplicatedSkeletalMesh;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ReplicatedAnimationLayer, Category = "Inventory")
	TSoftClassPtr<UAnimInstance> AnimationLayer{ nullptr };
};
