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
	void SetEquipmentType(const FGameplayTag& NewType) { EquipmentType = NewType; }
	UFUNCTION(Server, Reliable)
	void SetOwningController(AController* Controller);
	UFUNCTION(Server, Reliable)
	void SetOwningItem(UInv_InventoryItem* Item);

	UFUNCTION(BlueprintCallable)
	AController* GetOwningController();
	UFUNCTION(BlueprintCallable)
	UInv_InventoryItem* GetOwningItem() const;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Inventory")
	TWeakObjectPtr<AController> OwningController {nullptr};
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Inventory")
	TWeakObjectPtr<UInv_InventoryItem> OwningItem{ nullptr };
};
