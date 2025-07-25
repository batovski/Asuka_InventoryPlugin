// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagment/Utils/Inv_InventoryStatics.h"

#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"

UInv_InventoryComponent* UInv_InventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return nullptr;

	return PlayerController->FindComponentByClass<UInv_InventoryComponent>();

}

void UInv_InventoryStatics::ItemHovered(APlayerController* PC, UInv_InventoryItem* Item)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PC);
	if (!IsValid(IC)) return;
	UInv_InventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return;

	if(InventoryBase->HasHoverItem()) return;

	InventoryBase->OnItemHovered(Item);
}

void UInv_InventoryStatics::ItemUnhovered(APlayerController* PC)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PC);
	if (!IsValid(IC)) return;
	UInv_InventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return;

	InventoryBase->OnItemUnhovered();
}

UInv_HoverItem* UInv_InventoryStatics::GetHoverItem(const APlayerController* PlayerController)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PlayerController);
	if (!IsValid(IC)) return nullptr;
	UInv_InventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return nullptr;

	return InventoryBase->GetHoverItem();
}

FInstancedStruct UInv_InventoryStatics::GetFragmentFromItem(const UInv_InventoryItem* Item, FGameplayTag ItemType,
	bool& IsFound)
{
	const TArray<TInstancedStruct<FInv_ItemFragment>>& Fragments = Item->GetItemManifest().GetFragments();
	auto FoundFragment = Fragments.FindByPredicate([ItemType](const TInstancedStruct<FInv_ItemFragment>& StaticFragment)
	{
			return StaticFragment.Get().GetFragmentTag().MatchesTagExact(ItemType);
	});
	FInstancedStruct TempStruct;
	if (!FoundFragment)
	{
		IsFound = false;
		return TempStruct;
	}
	TempStruct.InitializeAs(FoundFragment->GetScriptStruct());
	IsFound = true;
	return TempStruct;
}

const UInv_InventoryItem* UInv_InventoryStatics::GetInventoryItemFromPlayerController(const APlayerController* PlayerController, FGameplayTag ItemType)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PlayerController);
	if (!IsValid(IC)) return nullptr;
	const UInv_InventoryItem* Item = IC->FindInventoryItem(ItemType);
	if (!IsValid(Item)) return nullptr;
	return Item;
}

UInv_InventoryBase* UInv_InventoryStatics::GetInventoryWidget(const APlayerController* PlayerController)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PlayerController);
	if (!IsValid(IC)) return nullptr;
	return IC->GetInventoryMenu();
}

FInv_ItemManifest UInv_InventoryStatics::GetItemManifestFromID(const FPrimaryAssetId& ItemId)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	UObject* Asset = AssetManager.GetPrimaryAssetObject(ItemId);
	if (!IsValid(Asset))
	{
		auto Handle = AssetManager.LoadPrimaryAsset(ItemId);
		Handle->WaitUntilComplete();
		Asset = Handle->GetLoadedAsset();
	}
	return Cast<UInv_ItemDataAsset>(Asset)->ItemManifest;
}

UInv_InventoryItem* UInv_InventoryStatics::CreateInventoryItemFromManifest(const FPrimaryAssetId& ItemId, UObject* WorldContextObject)
{
	UInv_InventoryItem* NewItem = NewObject<UInv_InventoryItem>(WorldContextObject, UInv_InventoryItem::StaticClass());;
	NewItem->SetStaticItemManifestAssetId(ItemId);
	NewItem->LoadStaticItemManifest();
	NewItem->GetItemManifestMutable().Manifest();
	return NewItem;
}
