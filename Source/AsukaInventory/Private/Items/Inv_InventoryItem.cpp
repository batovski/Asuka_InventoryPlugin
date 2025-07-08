// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Inv_InventoryItem.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"

void UInv_InventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ItemManifest);
	DOREPLIFETIME(ThisClass, TotalStackCount);
	DOREPLIFETIME(ThisClass, StaticItemManifestAssetId);
}

void UInv_InventoryItem::SetItemManifest(const FInv_ItemManifest& NewManifest)
{
	StaticItemManifest = FInstancedStruct::Make<FInv_ItemManifest>(NewManifest);
}

void UInv_InventoryItem::SetStaticItemManifestAssetId(const FPrimaryAssetId& NewAssetId)
{
	StaticItemManifestAssetId = NewAssetId;
}

void UInv_InventoryItem::LoadStaticItemManifest()
{
	if(!StaticItemManifest.IsValid())
	{
		StaticItemManifest = FInstancedStruct::Make<FInv_ItemManifest>(UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestAssetId));
	}
}

const FInv_ItemManifest& UInv_InventoryItem::GetItemManifest() const
{
	return StaticItemManifest.Get<FInv_ItemManifest>(); 
}

bool UInv_InventoryItem::IsStackable() const
{
	const FInv_StackableFragment* StackableFragment = GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
	return StackableFragment != nullptr;
}

bool UInv_InventoryItem::IsConsumable() const
{
	return GetItemManifest().GetItemCategory() == EInv_ItemCategory::Consumable;
}

bool UInv_InventoryItem::IsEquippable() const
{
	const FInv_EquipmentFragment* EquipmentFragment = GetItemManifest().GetFragmentOfType<FInv_EquipmentFragment>();
	return EquipmentFragment != nullptr;
}
