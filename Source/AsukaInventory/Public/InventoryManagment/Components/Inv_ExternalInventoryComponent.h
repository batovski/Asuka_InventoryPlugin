// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagment/FastArray/Inv_FastArray.h"
#include "Types/Inv_GridTypes.h"
#include "Widgets/Inventory/Spatial/Inv_GridsTags.h"
#include "Inv_ExternalInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInv_ExternalInventorySpawnParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Rows{ 8 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Columns{ 6 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FText GridNameText{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (Categories = "InventoryGrid"))
	FGameplayTag GridEntityTag{ InventoryGrid::Player::Loot };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FString PickupMessage{ "E - To loot" };
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ASUKAINVENTORY_API UInv_ExternalInventoryComponent : public UActorComponent, public IInv_ItemListInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInv_ExternalInventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	TArray<UInv_InventoryItem* > GetInventoryItems() const { return InventoryList.GetAllItems(); }
	FInventoryFastArrayItemChange& GetItemAddDelegate() { return InventoryList.OnItemAdded; }
	FInventoryFastArrayItemChange& GetItemRemoveDelegate() { return InventoryList.OnItemRemoved; }
	FInventoryFastArrayItemChange& GetItemChangedDelegate() { return InventoryList.OnItemChanged; }


	FString& GetPickupMessage() { return PickupMessage; }
	FGameplayTag& GetGridTagMutable() { return GridEntityTag; }

	// IInv_ItemListInterface interface:
	virtual UInv_InventoryItem* FindFirstItemByType_Implementation(const FGameplayTag& ItemType) const override { return InventoryList.FindFirstItemByType(ItemType); }
	virtual void RemoveItemFromList_Implementation(UInv_InventoryItem* Item) override;
	virtual UInv_InventoryItem* AddItemToList_Implementation(const FPrimaryAssetId& StaticItemManifestID, const TArray<FInstancedStruct>& DynamicFragments, const FInv_ItemAddingOptions& NewItemAddingOptions) override;
	virtual UInv_InventoryItem* MoveItemToList_Implementation(UInv_InventoryItem* Item) override { return InventoryList.AddEntry(Item); }
	virtual void ChangeItemGridIndex_Implementation(UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions) override;
	virtual void MarkItemDirty_Implementation(UInv_InventoryItem* Item) override;
	virtual void AddRepSubObj(UObject* SubObj) override;
	virtual FInv_InventoryFastArray& GetInventoryListMutable() override { return InventoryList; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenItemsContainer(APlayerController* PlayerController);

	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory")
	int32 Rows{0};
	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory")
	int32 Columns{0};

	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory")
	FText GridNameText{};

protected:

	UFUNCTION(Server, Reliable)
	void Server_AddNewItem(const FPrimaryAssetId StaticItemManifestID);

	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnInventoryMenuToggled(const bool IsOpen);

	UPROPERTY(EditAnywhere, Category = "Inventory", Replicated)
	TArray<FPrimaryAssetId> InitialItemsIDs;
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "InventoryGrid"))
	FGameplayTag GridEntityTag {InventoryGrid::Player::Loot};
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage {"E - To loot"};
	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList;

	TWeakObjectPtr<APlayerController> UsingPlayerController;
};
