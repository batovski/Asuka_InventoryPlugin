


#include "InventoryManagment/FastArray/Inv_FastArray.h"

#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Components/Inv_ItemComponent.h"

TArray<UInv_InventoryItem*> FInv_InventoryFastArray::GetAllItems() const
{
	TArray<UInv_InventoryItem*> Results;
	Results.Reserve(Entries.Num());
	for(const FInv_InventoryEntry& Entry : Entries)
	{
		if (Entry.Item)
		{
			Results.Add(Entry.Item);
		}
	}
	return Results;
}

void FInv_InventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UInv_InventoryComponent* InventoryComponent = Cast<UInv_InventoryComponent>(OwnerComponent);
	const bool bIsInventoryComponentAvailable = IsValid(InventoryComponent);

	for(int32 Index : RemovedIndices)
	{
		if (bIsInventoryComponentAvailable)
			InventoryComponent->OnItemRemoved.Broadcast(Entries[Index].Item);
		OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInv_InventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UInv_InventoryComponent* InventoryComponent = Cast<UInv_InventoryComponent>(OwnerComponent);
	const bool bIsInventoryComponentAvailable = IsValid(InventoryComponent);
	for (int32 Index : AddedIndices)
	{
		Entries[Index].Item->LoadStaticItemManifest();
		OnItemAdded.Broadcast(Entries[Index].Item);
		if (bIsInventoryComponentAvailable)
			InventoryComponent->OnItemAdded.Broadcast(Entries[Index].Item);
	}
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_ItemComponent* ItemComponent)
{
	check(OwnerComponent);
	AActor* OwnerActor = OwnerComponent->GetOwner();
	check(OwnerActor->HasAuthority());
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return nullptr;

	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = UInv_InventoryStatics::CreateInventoryItemFromManifest(ItemComponent->GetStaticItemManifestID(), ItemComponent, ItemComponent->GetDynamicFragmentsMutable());

	IC->AddRepSubObj(NewEntry.Item);
	MarkItemDirty(NewEntry);

	return NewEntry.Item;
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_ExternalInventoryComponent* ExternalComponent, const FPrimaryAssetId& StaticItemManifestID,
	const TArray<TInstancedStruct<FInv_ItemFragment>>& DynamicFragments)
{
	check(OwnerComponent)
	AActor* OwnerActor = OwnerComponent->GetOwner();
	check(OwnerActor->HasAuthority());

	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = UInv_InventoryStatics::CreateInventoryItemFromManifest(StaticItemManifestID, ExternalComponent, DynamicFragments);
	ExternalComponent->AddRepSubObj(NewEntry.Item);

	MarkItemDirty(NewEntry);
	return NewEntry.Item;
}
UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(const FPrimaryAssetId& StaticItemManifestID, const TArray<TInstancedStruct<FInv_ItemFragment>>& DynamicFragments)
{
	check(OwnerComponent);
	AActor* OwnerActor = OwnerComponent->GetOwner();
	check(OwnerActor->HasAuthority());
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return nullptr;

	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = UInv_InventoryStatics::CreateInventoryItemFromManifest(StaticItemManifestID, IC, DynamicFragments);
	IC->AddRepSubObj(NewEntry.Item);
	MarkItemDirty(NewEntry);

	return NewEntry.Item;
}

void FInv_InventoryFastArray::RemoveEntry(UInv_InventoryItem* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		if (EntryIt->Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
			return;
		}
	}
}

UInv_InventoryItem* FInv_InventoryFastArray::FindFirstItemByType(const FGameplayTag& ItemType) const
{
	auto* FoundItem = Entries.FindByPredicate([ItemType](const FInv_InventoryEntry& Entry)
	{
			return IsValid(Entry.Item) && Entry.Item->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
	});
	return FoundItem ? FoundItem->Item : nullptr;
}
