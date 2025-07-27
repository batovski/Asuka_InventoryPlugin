// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_ItemFragment.h"
#include "Inv_WeaponFragments.generated.h"

USTRUCT(BlueprintType)
struct FInv_WeaponModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FInv_WeaponFragment : public FInv_InventoryItemFragmentAbstract
{
	GENERATED_BODY()
	FInv_WeaponFragment()
	{
		bDynamicFragment = true;
		FragmentTag = FragmentTags::WeaponFragment;
	}
	virtual void Manifest() override;
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier MagazineSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier CurrentAmmo;

};