// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manifest/Inv_ItemManifest.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_InventoryItem.generated.h"


class UInv_ItemDataAsset;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetItemManifest(const FInv_ItemManifest& NewManifest);
	void SetStaticItemManifestAssetId(const FPrimaryAssetId& NewAssetId);
	void LoadStaticItemManifest();
	const FInv_ItemManifest& GetItemManifest() const;
	FInv_ItemManifest& GetItemManifestMutable() { return StaticItemManifest.GetMutable<FInv_ItemManifest>();}

	bool IsStackable() const;
	bool IsConsumable() const;
	bool IsEquippable() const;

	int32 GetTotalStackCount() const { return TotalStackCount; }
	void SetTotalStackCount(int32 NewCount) { TotalStackCount = NewCount; }

private:

	UPROPERTY(VisibleAnywhere, meta = (BaseStuct = "/Scripts/AsukaInventory.Inv_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;

	UPROPERTY(VisibleAnywhere, meta = (BaseStuct = "/Scripts/AsukaInventory.Inv_ItemManifest"))
	FInstancedStruct StaticItemManifest;

	UPROPERTY(VisibleAnywhere, Replicated)
	FPrimaryAssetId StaticItemManifestAssetId;

	UPROPERTY(Replicated)
	int32 TotalStackCount{ 0 };
};

template<typename FragmentType>
const FragmentType* GetFragment(const UInv_InventoryItem* Item, const FGameplayTag& Tag)
{
	if (!IsValid(Item)) return nullptr;

	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	return Item->GetItemManifest().GetFragmentOfTypeWithTag<FragmentType>(Tag);
}