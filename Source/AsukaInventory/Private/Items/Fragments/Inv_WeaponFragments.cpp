// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Fragments/Inv_WeaponFragments.h"

void FInv_WeaponFragment::Manifest()
{
	FInv_InventoryItemFragmentAbstract::Manifest();
	Damage.Manifest();
	MagazineSize.Manifest();
	CurrentAmmo.Manifest();
	AimDistance.Manifest();
	AimViewAngle.Manifest();
}
void FInv_WeaponFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragmentAbstract::Assimilate(Composite);
	Damage.Assimilate(Composite);
	MagazineSize.Assimilate(Composite);
	CurrentAmmo.Assimilate(Composite);
	AimDistance.Assimilate(Composite);
	AimViewAngle.Assimilate(Composite);
}