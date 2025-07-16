// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Manifest/Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Composite/Inv_CompositeBase.h"


void FInv_ItemManifest::Manifest()
{
	for(auto& Fragment : GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}
}

void FInv_ItemManifest::AssimilateInventoryFragments(UInv_CompositeBase* Composite) const
{
	const auto& ItemFragments = GetAllFragmentsOfType<FInv_InventoryItemFragmentAbstract>();
	for(const auto* Fragment : ItemFragments)
	{
		Composite->ApplyFunction([Fragment](UInv_CompositeBase* Child)
		{
			Fragment->Assimilate(Child);
		});
	}
}
