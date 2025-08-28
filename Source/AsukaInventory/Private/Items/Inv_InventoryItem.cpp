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
	DOREPLIFETIME(ThisClass, ItemIndex);
	DOREPLIFETIME(ThisClass, OwningGridEntityTag);
}

void UInv_InventoryItem::SetDynamicItemFragments(const TArray<FInstancedStruct>& Fragments)
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
	UpdateManifestData(GetItemManifestMutable().GetFragmentsMutable(), DynamicItemFragments, this);
}

const FInv_ItemManifest& UInv_InventoryItem::GetItemManifest() const
{
	return StaticItemManifest.Get<FInv_ItemManifest>(); 
}

void UInv_InventoryItem::SetOwningGridEntityTag(const FGameplayTag& NewTag)
{
	OwningGridEntityTag = FGameplayTag(NewTag);
}

bool UInv_InventoryItem::IsStackable() const
{
	const FInv_StackableFragment* StackableFragment = GetFragmentStructByTag<FInv_StackableFragment>(FragmentTags::StackableFragment);
	return StackableFragment != nullptr;
}

bool UInv_InventoryItem::IsConsumable() const
{
	return GetItemManifest().GetItemCategory() == EInv_ItemCategory::Consumable;
}

bool UInv_InventoryItem::IsEquippable() const
{
	const FInv_EquipmentFragment* EquipmentFragment = GetFragmentStructByTag<FInv_EquipmentFragment>(FragmentTags::EquipmentFragment);
	return EquipmentFragment != nullptr;
}

void UInv_InventoryItem::UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, TArray <FInstancedStruct>& DynamicFragments,
	const UInv_InventoryItem* Item)
{
	// Process static fragments first
	for (FInstancedStruct& StaticItemFragment : StaticFragments)
	{
		if (const FInv_ItemFragment* FragmentBase = StaticItemFragment.GetPtr<FInv_ItemFragment>())
		{
			const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
			if (!FragmentsMap.Contains(FragmentTag))
			{
				FragmentsMap.Add(FragmentTag, &StaticItemFragment);
				
				if (Item)
				{
					// Notify listeners about the modification
					Item->OnItemFragmentModified.Broadcast(FragmentTag);
				}
			}
		}
	}
	
	// Process dynamic fragments - they override static fragments
	for (FInstancedStruct& DynamicFragment : DynamicFragments)
	{
		if (const FInv_ItemFragment* FragmentBase = DynamicFragment.GetPtr<FInv_ItemFragment>())
		{
			const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
			// Dynamic fragments always override static ones
			FragmentsMap.FindOrAdd(FragmentTag) = &DynamicFragment;
			
			if (Item)
			{
				// Notify listeners about the modification
				Item->OnItemFragmentModified.Broadcast(FragmentTag);
			}
		}
	}
}

void UInv_InventoryItem::OnRep_DynamicItemFragments()
{
	UpdateManifestData(GetItemManifestMutable().GetFragmentsMutable(),DynamicItemFragments, this);
}

FInstancedStruct* UInv_InventoryItem::GetFragmentStructByTagMutable(const FGameplayTag& FragmentType)
{
	if (FInstancedStruct** Fragment = FragmentsMap.Find(FragmentType))
	{
		auto FoundFragment = (*Fragment)->GetPtr<FInv_ItemFragment>();
		auto DynamicFragment = DynamicItemFragments.FindByPredicate([FoundFragment](const FInstancedStruct& Element)
			{
				if (const FInv_ItemFragment* ElementFragment = Element.GetPtr<FInv_ItemFragment>())
				{
					return ElementFragment->GetFragmentTag() == FoundFragment->GetFragmentTag();
				}
				return false;
			});

		if (!DynamicFragment && FoundFragment->IsDynamicFragment())
		{
			const int32 index = DynamicItemFragments.Add(*(*Fragment));
			FragmentsMap.FindOrAdd(FragmentType) = &DynamicItemFragments[index];
			Fragment = FragmentsMap.Find(FragmentType);
		}
		return (*Fragment);
	}
	return nullptr;
}
