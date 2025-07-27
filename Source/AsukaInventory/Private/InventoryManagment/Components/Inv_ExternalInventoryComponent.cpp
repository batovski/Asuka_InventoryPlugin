// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"

#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

UInv_ExternalInventoryComponent::UInv_ExternalInventoryComponent() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInv_ExternalInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, InitialItemsIDs);
}

void UInv_ExternalInventoryComponent::OpenItemsContainer(APlayerController* PlayerController)
{
	check(PlayerController);
	UInv_InventoryComponent* PlayerInventoryComponent = UInv_InventoryStatics::GetInventoryComponent(PlayerController);
	check(PlayerInventoryComponent);
	if (PlayerInventoryComponent->OnInventoryMenuToggled.IsAlreadyBound(this, &UInv_ExternalInventoryComponent::OnInventoryMenuToggled)) { return; }
	UsingPlayerController = PlayerController;

	PlayerInventoryComponent->OpenInventoryMenu();
	UInv_InventoryBase* InventoryWidget = UInv_InventoryStatics::GetInventoryWidget(PlayerController);
	if(!InventoryWidget) return;

	UInv_SpatialInventory* SpatialInventoryWidget = Cast<UInv_SpatialInventory>(InventoryWidget);
	if (!SpatialInventoryWidget) return;

	PlayerInventoryComponent->OnInventoryMenuToggled.AddDynamic(this, &UInv_ExternalInventoryComponent::OnInventoryMenuToggled);
	PlayerInventoryComponent->OnInventoryItemGridChange.AddDynamic(this, &UInv_ExternalInventoryComponent::OnInventoryItemGridChange);
	SpatialInventoryWidget->InitLootGrid(this, InventoryList.GetAllItems());
}

void UInv_ExternalInventoryComponent::Server_AddNewItem_Implementation(const FPrimaryAssetId StaticItemManifestID)
{
	InventoryList.AddEntry(this, StaticItemManifestID);
}

void UInv_ExternalInventoryComponent::AddRepSubObj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		// Add the sub-object to the replication list
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_ExternalInventoryComponent::RemoveItemFromList_Implementation(UInv_InventoryItem* Item)
{
	InventoryList.RemoveEntry(Item);
}


UInv_InventoryItem* UInv_ExternalInventoryComponent::AddItemToList_Implementation(
	const FPrimaryAssetId& StaticItemManifestID,
	const TArray<TInstancedStruct<FInv_ItemFragment>>& DynamicFragments)
{
	return InventoryList.AddEntry(this, StaticItemManifestID, DynamicFragments);
}

// Called when the game starts
void UInv_ExternalInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	//Items are not replicated for some reason:

	for(auto ItemAssetsIDs : InitialItemsIDs)
	{
		Server_AddNewItem(ItemAssetsIDs);
	}
	
}

void UInv_ExternalInventoryComponent::OnInventoryMenuToggled(const bool IsOpen)
{
	if(!IsOpen)
	{
		UInv_InventoryComponent* PlayerInventoryComponent = UInv_InventoryStatics::GetInventoryComponent(UsingPlayerController.Get());
		check(PlayerInventoryComponent);
		PlayerInventoryComponent->OnInventoryMenuToggled.RemoveDynamic(this, &UInv_ExternalInventoryComponent::OnInventoryMenuToggled);
		PlayerInventoryComponent->OnInventoryItemGridChange.RemoveDynamic(this, &UInv_ExternalInventoryComponent::OnInventoryItemGridChange);
	}
}

void UInv_ExternalInventoryComponent::OnInventoryItemGridChange(UInv_InventoryItem* Item, int32 StackCount,
	EInv_ItemCategory OldGridCategory, EInv_ItemCategory NewGridCategory)
{
	UInv_InventoryComponent* PlayerInventoryComponent = UInv_InventoryStatics::GetInventoryComponent(UsingPlayerController.Get());
	check(PlayerInventoryComponent);
	if(OldGridCategory == EInv_ItemCategory::External && NewGridCategory != EInv_ItemCategory::External)
	{
		PlayerInventoryComponent->TryAddItemToInventory(this, PlayerInventoryComponent, Item, StackCount);
	}
	else if(NewGridCategory == EInv_ItemCategory::External && OldGridCategory != EInv_ItemCategory::External)
	{
		PlayerInventoryComponent->TryAddItemToInventory(PlayerInventoryComponent, this, Item, StackCount, EInv_ItemCategory::External);
	}
}

