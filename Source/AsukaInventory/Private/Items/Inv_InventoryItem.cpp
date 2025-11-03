// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Inv_InventoryItem.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Composite/Inv_CompositeBase.h"

void FInv_ItemFragmentArray::UpdateFragmentsMap(TMap<FGameplayTag, FInstancedStruct*>& FragmentsMap, const TArrayView<int32> Indices)
{
	for (int32 Index : Indices)
	{
		if (Items.IsValidIndex(Index))
		{
			const FInstancedStruct& Fragment = Items[Index].Fragment;
			if (const FInv_ItemFragment* FragmentBase = Fragment.GetPtr<FInv_ItemFragment>())
			{
				const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
				FragmentsMap.FindOrAdd(FragmentTag) = const_cast<FInstancedStruct*>(&Fragment);
			}
		}
	}
}

void FInv_ItemFragmentArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (!Owner)
	{
		return;
	}

	if (UInv_InventoryItem* OwnerItem = Cast<UInv_InventoryItem>(Owner))
	{
		UpdateFragmentsMap(OwnerItem->FragmentsMap, AddedIndices);

		for (int32 Index : AddedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				if (const FInv_ItemFragment* FragmentBase = Items[Index].Fragment.GetPtr<FInv_ItemFragment>())
				{
					const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
					OwnerItem->OnFragmentAdded.Broadcast(FragmentTag);
				}
			}
		}
	}

	else if (UInv_ItemComponent* OwnerComponent = Cast<UInv_ItemComponent>(Owner))
	{
		UpdateFragmentsMap(OwnerComponent->FragmentsMap, AddedIndices);

		for (int32 Index : AddedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				if (const FInv_ItemFragment* FragmentBase = Items[Index].Fragment.GetPtr<FInv_ItemFragment>())
				{
					const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
					OwnerComponent->OnFragmentAdded.Broadcast(FragmentTag);
				}
			}
		}
	}
}

void FInv_ItemFragmentArray::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (!Owner)
	{
		return;
	}

	if (UInv_InventoryItem* OwnerItem = Cast<UInv_InventoryItem>(Owner))
	{
		UpdateFragmentsMap(OwnerItem->FragmentsMap, ChangedIndices);

		for (int32 Index : ChangedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				if (const FInv_ItemFragment* FragmentBase = Items[Index].Fragment.GetPtr<FInv_ItemFragment>())
				{
					const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
					OwnerItem->OnFragmentModified.Broadcast(FragmentTag);
				}
			}
		}
	}
	else if (UInv_ItemComponent* OwnerComponent = Cast<UInv_ItemComponent>(Owner))
	{
		UpdateFragmentsMap(OwnerComponent->FragmentsMap, ChangedIndices);

		for (int32 Index : ChangedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				if (const FInv_ItemFragment* FragmentBase = Items[Index].Fragment.GetPtr<FInv_ItemFragment>())
				{
					const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
					OwnerComponent->OnFragmentModified.Broadcast(FragmentTag);
				}
			}
		}
	}
}

void FInv_ItemFragmentArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (!Owner)
	{
		return;
	}

	if (UInv_InventoryItem* OwnerItem = Cast<UInv_InventoryItem>(Owner))
	{
		UpdateFragmentsMap(OwnerItem->FragmentsMap, RemovedIndices);

		for (int32 Index : RemovedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				if (const FInv_ItemFragment* FragmentBase = Items[Index].Fragment.GetPtr<FInv_ItemFragment>())
				{
					const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
					OwnerItem->OnFragmentRemoved.Broadcast(FragmentTag);
				}
			}
		}
	}
	else if (UInv_ItemComponent* OwnerComponent = Cast<UInv_ItemComponent>(Owner))
	{
		UpdateFragmentsMap(OwnerComponent->FragmentsMap, RemovedIndices);

		for (int32 Index : RemovedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				if (const FInv_ItemFragment* FragmentBase = Items[Index].Fragment.GetPtr<FInv_ItemFragment>())
				{
					const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
					OwnerComponent->OnFragmentRemoved.Broadcast(FragmentTag);
				}
			}
		}
	}
}

void UInv_InventoryItem::PostInitProperties()
{
	Super::PostInitProperties();
	DynamicItemFragments.Owner = this;
}

void UInv_InventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, DynamicItemFragments);
	DOREPLIFETIME(ThisClass, StaticItemManifestAssetId);
	DOREPLIFETIME(ThisClass, ItemIndex);
}

void UInv_InventoryItem::SetDynamicItemFragments(const TArray<FInstancedStruct>& Fragments)
{
	DynamicItemFragments.Items.Empty();
	DynamicItemFragments.Owner = this;
	for (const FInstancedStruct& Fragment : Fragments)
	{
		DynamicItemFragments.Items.Add(FInv_ItemFragmentArrayItem(Fragment));
	}
	DynamicItemFragments.MarkArrayDirty();
	UpdateManifestData(GetItemManifestMutable().GetFragmentsMutable(), DynamicItemFragments, this);
}

TArray<FInstancedStruct> UInv_InventoryItem::GetDynamicItemFragments() const
{
	TArray<FInstancedStruct> Result;
	for (const FInv_ItemFragmentArrayItem& Item : DynamicItemFragments.Items)
	{
		Result.Add(Item.Fragment);
	}
	return Result;
}

void UInv_InventoryItem::SetStaticItemManifestAssetId(const FPrimaryAssetId& NewAssetId)
{
	StaticItemManifestAssetId = NewAssetId;
}

void UInv_InventoryItem::LoadStaticItemManifest()
{
	DynamicItemFragments.Owner = this;
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

void UInv_InventoryItem::AssimilateInventoryFragments(UInv_CompositeBase* Composite) const
{
	for(const auto FragmentTuple : FragmentsMap)
	{
		if(const auto* ItemFragment = FragmentTuple.Value->GetPtr<FInv_InventoryItemFragmentAbstract>())
		{
			Composite->ApplyFunction([ItemFragment](UInv_CompositeBase* Child)
			{
				ItemFragment->Assimilate(Child);
			});
		}
	}
}

void UInv_InventoryItem::InitManifestDynamicFragments(UObject* Outer)
{
	for(auto& Fragment : GetItemManifest().GetFragments())
	{
		if(const FInv_ItemFragment* FragmentBase = Fragment.GetPtr<FInv_ItemFragment>())
		{
			if(FragmentBase->IsDynamicFragment() && FragmentBase->ShouldReplicateFromStart())
			{
				auto DynamicFragment = GetFragmentStructByTagMutable<FInv_ItemFragment>(FragmentBase->GetFragmentTag());
				DynamicFragment->Manifest(Outer);
			}
		}
	}
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

void UInv_InventoryItem::UpdateManifestData(TArray<FInstancedStruct>& StaticFragments, const FInv_ItemFragmentArray& DynamicFragments,
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
			}
		}
	}
	// Process dynamic fragments - they override static fragments
	for (const FInv_ItemFragmentArrayItem& DynamicFragmentItem : DynamicFragments.Items)
	{
		if (const FInv_ItemFragment* FragmentBase = DynamicFragmentItem.Fragment.GetPtr<FInv_ItemFragment>())
		{
			const FGameplayTag& FragmentTag = FragmentBase->GetFragmentTag();
			FragmentsMap.FindOrAdd(FragmentTag) = const_cast<FInstancedStruct*>(&DynamicFragmentItem.Fragment);
		}
	}
}

void UInv_InventoryItem::OnDynamicFragmentUpdated()
{
	UpdateManifestData(GetItemManifestMutable().GetFragmentsMutable(), DynamicItemFragments, this);
}

FInstancedStruct* UInv_InventoryItem::GetFragmentStructByTagMutable(const FGameplayTag& FragmentType)
{
	return GetFragmentStructByTagMutable(DynamicItemFragments, FragmentsMap, FragmentType);
}

FInstancedStruct* UInv_InventoryItem::GetFragmentStructByTagMutable(FInv_ItemFragmentArray& DynamicFragments, TMap<FGameplayTag, FInstancedStruct*>& FragmentsMap, const FGameplayTag& FragmentType)
{
	if (FInstancedStruct** Fragment = FragmentsMap.Find(FragmentType))
	{
		auto FoundFragment = (*Fragment)->GetPtr<FInv_ItemFragment>();
		auto* DynamicFragmentItem = DynamicFragments.Items.FindByPredicate([FoundFragment](const FInv_ItemFragmentArrayItem& Element)
			{
				if (const FInv_ItemFragment* ElementFragment = Element.Fragment.GetPtr<FInv_ItemFragment>())
				{
					return ElementFragment->GetFragmentTag() == FoundFragment->GetFragmentTag();
				}
				return false;
			});

		if (!DynamicFragmentItem && FoundFragment->IsDynamicFragment())
		{
			const int32 index = DynamicFragments.Items.Add(FInv_ItemFragmentArrayItem(*(*Fragment)));
			FragmentsMap.FindOrAdd(FragmentType) = &DynamicFragments.Items[index].Fragment;
			DynamicFragments.MarkItemDirty(DynamicFragments.Items[index]);
			Fragment = FragmentsMap.Find(FragmentType);
		}
		return (*Fragment);
	}
	return nullptr;
}
