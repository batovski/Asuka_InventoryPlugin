// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Inv_InventoryItem.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"

void UInv_InventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, DynamicItemFragments);
	DOREPLIFETIME(ThisClass, StaticItemManifestAssetId);
}

void UInv_InventoryItem::SetDynamicItemFragments(const TArray<TInstancedStruct<FInv_ItemFragment>>& Fragments)
{
	DynamicItemFragments = Fragments;
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
	UpdateManifestData(GetItemManifestMutable().GetFragmentsMutable(), DynamicItemFragments);
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

void UInv_InventoryItem::UpdateManifestData(TArray<TInstancedStruct<FInv_ItemFragment>>& StaticFragments, TArray <TInstancedStruct<FInv_ItemFragment>>& DynamicFragments)
{
	//TODO:: OPTIMIZE THIS with hashmap if fragments are unique 
	for (TInstancedStruct<FInv_ItemFragment>& DynamicFragment : DynamicFragments)
	{
		for (TInstancedStruct<FInv_ItemFragment>& StaticItemFragment : StaticFragments)
		{
			if (StaticItemFragment.Get().IsDynamicFragment() && StaticItemFragment.Get().GetFragmentTag().MatchesTagExact(DynamicFragment.Get().GetFragmentTag()))
			{
				StaticItemFragment = DynamicFragment;
			}
		}
	}
}

void UInv_InventoryItem::OnRep_DynamicItemFragments()
{
	UpdateManifestData(GetItemManifestMutable().GetFragmentsMutable(),DynamicItemFragments);
}
