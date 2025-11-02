// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"

#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
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
	DOREPLIFETIME(ThisClass, Rows);
	DOREPLIFETIME(ThisClass, Columns);
	DOREPLIFETIME(ThisClass, GridNameText);
}

void UInv_ExternalInventoryComponent::OpenItemsContainer(APlayerController* PlayerController)
{
	check(PlayerController);
	UInv_InventoryComponent* PlayerInventoryComponent = UInv_InventoryStatics::GetInventoryComponent(PlayerController);
	check(PlayerInventoryComponent);
	if (PlayerInventoryComponent->GetInventoryMenu()->OnInventoryMenuToggled.IsAlreadyBound(this, &UInv_ExternalInventoryComponent::OnInventoryMenuToggled)) { return; }
	UsingPlayerController = PlayerController;

	UInv_InventoryBase* InventoryWidget = UInv_InventoryStatics::GetInventoryWidget(PlayerController);
	if(!InventoryWidget) return;

	UInv_SpatialInventory* SpatialInventoryWidget = Cast<UInv_SpatialInventory>(InventoryWidget);
	if (!SpatialInventoryWidget) return;

	SpatialInventoryWidget->OpenInventoryMenu();

	PlayerInventoryComponent->GetInventoryMenu()->OnInventoryMenuToggled.AddDynamic(this, &UInv_ExternalInventoryComponent::OnInventoryMenuToggled);
	SpatialInventoryWidget->AddDynamicGrid(GridEntityTag, this, InventoryList.GetAllItems());
}

void UInv_ExternalInventoryComponent::Server_AddNewItem_Implementation(const FPrimaryAssetId StaticItemManifestID)
{
	FInv_ItemAddingOptions NewItemAddingOptions;
	InventoryList.AddEntry(this, StaticItemManifestID, NewItemAddingOptions);
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
	const TArray<FInstancedStruct>& DynamicFragments,
	const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	return InventoryList.AddEntry(this, StaticItemManifestID, NewItemAddingOptions, DynamicFragments);
}

void UInv_ExternalInventoryComponent::ChangeItemGridIndex_Implementation(UInv_InventoryItem* Item,
	const FInv_ItemAddingOptions& NewItemAddingOptions)
{
	InventoryList.ChangeEntryGridIndex(Item, NewItemAddingOptions.GridIndex);
}

void UInv_ExternalInventoryComponent::MarkItemDirty_Implementation(UInv_InventoryItem* Item)
{
	InventoryList.MarkEntryDirty(Item);
}

// Called when the game starts
void UInv_ExternalInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

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
		PlayerInventoryComponent->GetInventoryMenu()->OnInventoryMenuToggled.RemoveDynamic(this, &UInv_ExternalInventoryComponent::OnInventoryMenuToggled);
	}
}

