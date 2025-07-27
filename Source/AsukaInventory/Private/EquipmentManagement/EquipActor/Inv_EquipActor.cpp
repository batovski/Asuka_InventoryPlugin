// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"

#include "Items/Inv_InventoryItem.h"
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
	DOREPLIFETIME(ThisClass, OwningItem);
}

void AInv_EquipActor::SetOwningController_Implementation(AController* Controller)
{
	OwningController = Controller;
}

void AInv_EquipActor::SetOwningItem_Implementation(UInv_InventoryItem* Item)
{
	if (!IsValid(Item)) return;
	OwningItem = Item;
}

AController* AInv_EquipActor::GetOwningController()
{
	return OwningController.Get();
}

UInv_InventoryItem* AInv_EquipActor::GetOwningItem() const
{
	return OwningItem.Get();
}

