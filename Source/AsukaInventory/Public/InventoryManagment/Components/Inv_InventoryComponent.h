

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagment/FastArray/Inv_FastArray.h"
#include <Types/Inv_GridTypes.h>
#include "Inv_InventoryComponent.generated.h"


class UInv_ExternalInventoryComponent;
struct FInv_SlotAvailabilityResult;
class UInv_ItemComponent;
class UInv_InventoryBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryStackChange, const FInv_SlotAvailabilityResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquipStatusChanges, UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggled, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FInventoryItemGridChange, UInv_InventoryItem*, Item, int32, StackCount, EInv_ItemCategory, OldGridCategory, EInv_ItemCategory, NewGridCategory);
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

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void TryAddItemToInventory(TScriptInterface<IInv_ItemListInterface> SourceInventory, TScriptInterface <IInv_ItemListInterface> TargetInventor,
		UInv_InventoryItem* Item, int32 StackCount, const EInv_ItemCategory GridCategory = EInv_ItemCategory::None);

	UFUNCTION(Server,Reliable)
	void Server_AddNewItemByComponent(UInv_ItemComponent* ItemComponent, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_AddStacksToItemByComponent(UInv_ItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	UFUNCTION(Server, Reliable)
	void Server_DropItem(UInv_InventoryItem* Item, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_DropItemFromExternalInventory(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, UInv_InventoryItem* Item, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_ConsumeItem(UInv_InventoryItem* Item);

	UFUNCTION(Server, Reliable)
	void Server_EquipSlotClicked(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnEquip);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipSlotClicked(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnEquip);

	const UInv_InventoryItem* FindInventoryItem(const FGameplayTag& ItemType) const;

	void ToggleInventoryMenu();
	void OpenInventoryMenu();

	void AddRepSubObj(UObject* SubObj);

	void SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount);

	UInv_InventoryBase* GetInventoryMenu() const { return InventoryMenu; }

	// IInv_ItemListInterface interface:
	virtual UInv_InventoryItem* FindFirstItemByType_Implementation(const FGameplayTag& ItemType) const override { return InventoryList.FindFirstItemByType(ItemType); }
	virtual void RemoveItemFromList_Implementation(UInv_InventoryItem* Item) override;
	virtual UInv_InventoryItem* AddItemToList_Implementation(const FPrimaryAssetId& StaticItemManifestID) override;

	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FInventoryStackChange OnStackChange;
	FItemEquipStatusChanges OnItemEquipped;
	FItemEquipStatusChanges OnItemUnEquipped;
	FInventoryMenuToggled OnInventoryMenuToggled;
	FInventoryItemGridChange OnInventoryItemGridChange;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UFUNCTION(Server, Reliable)
	void Server_AddNewItem(const TScriptInterface<IInv_ItemListInterface>& SourceInventory, const TScriptInterface <IInv_ItemListInterface>& TargetInventory, UInv_InventoryItem* Item, int32 StackCount);

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
