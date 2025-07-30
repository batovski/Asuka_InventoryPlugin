// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagment/Utils/Inv_InventoryStatics.h"

#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
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

const FInstancedStruct& UInv_InventoryStatics::GetFragmentFromItem(UInv_InventoryItem* Item, FGameplayTag ItemType,
	bool& IsFound)
{
	static FInstancedStruct FoundFragment;
	if(!IsValid(Item))
	{
		IsFound = false;
		return FoundFragment;
	}
	if(const TInstancedStruct<FInv_ItemFragment>* Fragment = Item->GetFragmentStructByTag(ItemType))
	{
		FoundFragment.InitializeAs(Fragment->GetScriptStruct(), Fragment->GetMemory());
		IsFound = true;
		return FoundFragment;
	}
	IsFound = false;
	return FoundFragment;
}

void UInv_InventoryStatics::SetFragmentValuesByTag(UInv_InventoryItem* Item, const FInstancedStruct& Fragment,
	FGameplayTag ItemType, bool& IsSucceeded)
{
	if(!IsValid(Item))
	{
		IsSucceeded = false;
		return;
	}
	if (TInstancedStruct<FInv_ItemFragment>* DesiredFragment = Item->GetFragmentStructByTagMutable(ItemType))
	{
		DesiredFragment->InitializeAsScriptStruct(Fragment.GetScriptStruct(),Fragment.GetMemory());
		IsSucceeded = true;
	}
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
		if (Handle)
		{
			Handle->WaitUntilComplete();
			Asset = Handle->GetLoadedAsset();
		}
	}
	return Asset ? Cast<UInv_ItemDataAsset>(Asset)->ItemManifest : FInv_ItemManifest();
}

UInv_InventoryItem* UInv_InventoryStatics::CreateInventoryItemFromManifest(const FPrimaryAssetId& ItemId, UObject* WorldContextObject, const TArray<TInstancedStruct<FInv_ItemFragment>>& DynamicFragments)
{
	UInv_InventoryItem* NewItem = NewObject<UInv_InventoryItem>(WorldContextObject, UInv_InventoryItem::StaticClass());;
	NewItem->SetStaticItemManifestAssetId(ItemId);
	NewItem->LoadStaticItemManifest();
	if(!DynamicFragments.IsEmpty())
	{
		NewItem->SetDynamicItemFragments(DynamicFragments);
	}
	NewItem->GetItemManifestMutable().Manifest();
	return NewItem;
}

UInv_ExternalInventoryComponent* UInv_InventoryStatics::CreateExternalInventoryComponent(UObject* WorldContextObject,
	const UInv_InventoryComponent* InventoryComponent, const FString& PickupMessage)
{
	if (!IsValid(InventoryComponent))
	{
		return nullptr;
	}

	UInv_ExternalInventoryComponent* ExternalInventoryComponent = NewObject<UInv_ExternalInventoryComponent>(WorldContextObject, UInv_ExternalInventoryComponent::StaticClass());

	if(!PickupMessage.IsEmpty())
	{
		ExternalInventoryComponent->GetPickupMessage() = PickupMessage;
	}

	const TArray<UInv_InventoryItem*>& SourceItems = InventoryComponent->GetInventoryList().GetAllItems();
	for (UInv_InventoryItem* SourceItem : SourceItems)
	{
		if (IsValid(SourceItem))
		{
			UInv_ExternalInventoryComponent::Execute_AddItemToList(ExternalInventoryComponent, SourceItem->GetStaticItemManifestAssetId(), SourceItem->GetDynamicItemFragments());
		}
	}

	return ExternalInventoryComponent;
}
