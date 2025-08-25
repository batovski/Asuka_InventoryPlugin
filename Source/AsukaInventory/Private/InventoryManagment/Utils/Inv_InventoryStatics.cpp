// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagment/Utils/Inv_InventoryStatics.h"

#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"
#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"
#include "Widgets/Inventory/Spatial/Inv_GridsTags.h"

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

const FInstancedStruct& UInv_InventoryStatics::GetFragmentFromItem(UInv_InventoryItem* Item, FGameplayTag FragmentType,
	bool& IsFound)
{
	static FInstancedStruct FoundFragment;
	if(!IsValid(Item))
	{
		IsFound = false;
		return FoundFragment;
	}
	if(const TInstancedStruct<FInv_ItemFragment>* Fragment = Item->GetFragmentStructByTag(FragmentType))
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

const UInv_InventoryItem* UInv_InventoryStatics::GetEquipmentItemFromPlayerController(
	const APlayerController* PlayerController, FGameplayTag ItemType)
{
	if (const AInv_EquipActor* ItemActor = GetEquipmentActorFromPlayerController(PlayerController, ItemType))
	{
		return ItemActor->GetOwningItem();
	}
	return nullptr;
}

const AInv_EquipActor* UInv_InventoryStatics::GetEquipmentActorFromPlayerController(
	const APlayerController* PlayerController, FGameplayTag ItemType)
{
	check(IsValid(PlayerController));
	if (UInv_EquipmentComponent* EquipmentComponent = PlayerController->FindComponentByClass<UInv_EquipmentComponent>())
	{
		if (AInv_EquipActor* ItemActor = EquipmentComponent->FindEquippedActorByType(ItemType))
		{
			return ItemActor;
		}
	}
	return nullptr;
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

AActor* UInv_InventoryStatics::CreateExternalInventoryActor(UObject* WorldContextObject,
	UInv_InventoryComponent* InventoryComponent, 
    const FString& PickupMessage,
    TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform)
{
    if (!IsValid(InventoryComponent))
    {
        return nullptr;
    }

    if (!ActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateExternalInventoryComponent: ActorClass is null"));
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    // Spawn the actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
    if (!SpawnedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateExternalInventoryComponent: Failed to spawn actor of class %s"), 
               ActorClass ? *ActorClass->GetName() : TEXT("NULL"));
        return nullptr;
    }

    // Create and attach the external inventory component
    UInv_ExternalInventoryComponent* ExternalInventoryComponent = NewObject<UInv_ExternalInventoryComponent>(
        SpawnedActor, UInv_ExternalInventoryComponent::StaticClass());
    
    if (!ExternalInventoryComponent)
    {
        SpawnedActor->Destroy();
        return nullptr;
    }

    // Add component to the actor
    SpawnedActor->AddInstanceComponent(ExternalInventoryComponent);
    ExternalInventoryComponent->RegisterComponent();

    // Set pickup message if provided
    if (!PickupMessage.IsEmpty())
    {
        ExternalInventoryComponent->GetPickupMessage() = PickupMessage;
    }

    // Copy items from source inventory component
    const TArray<UInv_InventoryItem*>& SourceItems = InventoryComponent->GetInventoryList().GetAllItems();
    for (UInv_InventoryItem* SourceItem : SourceItems)
    {
        if (IsValid(SourceItem))
        {
            FInv_ItemAddingOptions ItemToEquipOptions;
            ItemToEquipOptions.GridIndex = INDEX_NONE;
            ItemToEquipOptions.GridEntityTag = InventoryGrid::External::LootGrid;
			InventoryComponent->Server_AddNewItem(InventoryComponent, ExternalInventoryComponent, SourceItem, ItemToEquipOptions);
        }
    }

    return SpawnedActor;
}
