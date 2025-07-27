// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"

#include "Net/UnrealNetwork.h"


AInv_EquipActor::AInv_EquipActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AInv_EquipActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, OwningController);
}

void AInv_EquipActor::SetOwningController_Implementation(AController* Controller)
{
	OwningController = Controller;
}

AController* AInv_EquipActor::GetOwningController()
{
	return OwningController.Get();
}

