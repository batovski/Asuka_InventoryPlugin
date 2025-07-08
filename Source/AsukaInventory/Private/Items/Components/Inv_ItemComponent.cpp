// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/Inv_ItemComponent.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Net/UnrealNetwork.h"

UInv_ItemComponent::UInv_ItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickupMessage = FString("E - Pick Up");
	SetIsReplicatedByDefault(true);
}

void UInv_ItemComponent::BeginPlay()
{
	Super::BeginPlay();
	if (StaticItemManifestID.IsValid())
	{
		StaticItemManifest = UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestID);
		if(GetOwner())
			StaticItemManifest.SetPickupActorClass(GetOwner()->GetClass());
	}
}

void UInv_ItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(ThisClass, ItemManifest);
	DOREPLIFETIME(ThisClass, StaticItemManifestID);
}

void UInv_ItemComponent::PickedUp()
{
	OnPickedUp();
	GetOwner()->Destroy();
}

void UInv_ItemComponent::InitItemManifest(FInv_ItemManifest CopyOfManifest)
{
	StaticItemManifest = CopyOfManifest;
}

FString& UInv_ItemComponent::GetPickupMessage()
{
	return PickupMessage;
}

const FPrimaryAssetId& UInv_ItemComponent::GetStaticItemManifestID() const
{
	return StaticItemManifestID;
}

