// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Inv_EquipActor.generated.h"

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

	UFUNCTION(BlueprintCallable)
	AController* GetOwningController();

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Inventory")
	TWeakObjectPtr<AController> OwningController {nullptr};
};
