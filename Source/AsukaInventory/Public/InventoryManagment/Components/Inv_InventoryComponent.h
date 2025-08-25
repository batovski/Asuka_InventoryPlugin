

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagment/FastArray/Inv_FastArray.h"
#include <Types/Inv_GridTypes.h>
#include "Inv_InventoryComponent.generated.h"


struct FInv_ItemAddingOptions;
class UInv_ExternalInventoryComponent;
struct FInv_SlotAvailabilityResult;
class UInv_ItemComponent;
class UInv_InventoryBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquipStatusChanges, UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggled, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class ASUKAINVENTORY_API UInv_InventoryComponent : public UActorComponent, public IInv_ItemListInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInv_InventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void TryAddItemByComponent(UInv_ItemComponent* ItemComponent);

	UFUNCTION(Server, Reliable)
	void Server_AddNewItemByItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions);

	UFUNCTION(Server,Reliable)
	void Server_AddNewItemByComponent(UInv_ItemComponent* ItemComponent, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_AddStacksToItemByComponent(UInv_ItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	UFUNCTION(Server, Reliable)
	void Server_DropItem(UInv_InventoryItem* Item, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_DropItemFromExternalInventory(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, int32 StackCount);
	UFUNCTION(Server, Reliable)
	void Server_ChangeItemGridIndex(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions);
	UFUNCTION(Server, Reliable)
	void Server_RemoveItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item);
	UFUNCTION(Server, Reliable)
	void Server_MarkItemDirty(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item);

	UFUNCTION(Server, Reliable)
	void Server_ConsumeItem(UInv_InventoryItem* Item);

	UFUNCTION(Server, Reliable)
	void Server_UpdateItemStackCount(UInv_InventoryItem* Item, const int32 StackCount);

	void TryChangeItemGridIndex(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, const int32 NewGridIndex);


	UFUNCTION(Server, Reliable)
	void Server_EquipSlotClicked(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnEquip, const FGameplayTag& EquipSlotTag = {});
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipSlotClicked(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnEquip);

	UFUNCTION(Server, Reliable)
	void Server_AddNewItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const TScriptInterface <IInv_ItemListInterface>& TargetInventory, UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions);

	const UInv_InventoryItem* FindInventoryItem(const FGameplayTag& ItemType) const;

	void ToggleInventoryMenu();
	void OpenInventoryMenu();

	void SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount);

	UInv_InventoryBase* GetInventoryMenu() const { return InventoryMenu; }
	FInv_InventoryFastArray& GetInventoryListMutable() { return InventoryList; }
	const FInv_InventoryFastArray& GetInventoryList() const { return InventoryList; }


	// IInv_ItemListInterface interface:
	virtual UInv_InventoryItem* FindFirstItemByType_Implementation(const FGameplayTag& ItemType) const override { return InventoryList.FindFirstItemByType(ItemType); }
	virtual void RemoveItemFromList_Implementation(UInv_InventoryItem* Item) override;
	virtual UInv_InventoryItem* AddItemToList_Implementation(const FPrimaryAssetId& StaticItemManifestID, const TArray<TInstancedStruct<FInv_ItemFragment>>& DynamicFragments, const FInv_ItemAddingOptions& NewItemAddingOptions) override;
	virtual UInv_InventoryItem* MoveItemToList_Implementation(UInv_InventoryItem* Item) override { return InventoryList.AddEntry(Item); }
	virtual void ChangeItemGridIndex_Implementation(UInv_InventoryItem* Item, const FInv_ItemAddingOptions& NewItemAddingOptions) override;
	virtual void MarkItemDirty_Implementation(UInv_InventoryItem* Item) override;
	virtual void AddRepSubObj(UObject* SubObj) override;

	FNoRoomInInventory NoRoomInInventory;
	FItemEquipStatusChanges OnItemEquipped;
	FItemEquipStatusChanges OnItemUnEquipped;
	FInventoryMenuToggled OnInventoryMenuToggled;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UFUNCTION(Server, Reliable)
	void Server_AddStacksToItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const TScriptInterface <IInv_ItemListInterface>& TargetInventory, UInv_InventoryItem* Item, int32 StackCount, int32 Remainder);

	void ConstructInventory();
	void CloseInventoryMenu();

	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList;

	TWeakObjectPtr<APlayerController> OwningController;

	UPROPERTY()
	TObjectPtr<UInv_InventoryBase> InventoryMenu;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_InventoryBase> InventoryMenuClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnAngleMin{ -85.0f };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnAngleMax{ 85.0f };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnDistanceMin{ 50.0f };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnDistanceMax{ 120.0f };

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float RelativeSpawnElevation{ -70.0f };

	bool bIsInventoryMenuOpen = false;
};
