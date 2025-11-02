// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Fragments/Inv_WeaponFragments.h"

void FInv_WeaponFragment::Manifest(UObject* Owner)
{
	FInv_InventoryItemFragmentAbstract::Manifest(Owner);
	Damage.Manifest(Owner);
	MagazineSize.Manifest(Owner);
	CurrentAmmo.Manifest(Owner);
	AimDistance.Manifest(Owner);
	AimViewAngle.Manifest(Owner);
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