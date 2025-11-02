// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual void Manifest(UObject* Owner) override;
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier MagazineSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier BulletsPerCartridge;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier FireRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier AimDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInv_WeaponModifier AimViewAngle;
};

USTRUCT(BlueprintType)
struct FInv_WeaponAnimationsFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	FInv_WeaponAnimationsFragment() { FragmentTag = FragmentTags::WeaponAnimationsFragment; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSoftObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSoftObjectPtr<UAnimMontage> ReloadMontage;
};
class UMetaSoundSource;
USTRUCT(BlueprintType)
struct FInv_WeaponMetaSoundsFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	FInv_WeaponMetaSoundsFragment() { FragmentTag = FragmentTags::WeaponMetaSoundsFragment; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSoftObjectPtr<UMetaSoundSource> FireAudioSource;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSoftObjectPtr<UMetaSoundSource> ReloadAudioSource;
};