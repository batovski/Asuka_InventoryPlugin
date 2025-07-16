// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Fragments/Inv_FragmentTags.h"

namespace FragmentTags
{
	namespace DynamicFragmentTags
	{
		UE_DEFINE_GAMEPLAY_TAG(StackDynamicFragment, "FragmentTags.StackDynamicFragment")
		UE_DEFINE_GAMEPLAY_TAG(PickUpActorFragment, "FragmentTags.PickUpActorFragment")
	}

	UE_DEFINE_GAMEPLAY_TAG(GridFragment, "FragmentTags.GridFragment")
	UE_DEFINE_GAMEPLAY_TAG(IconFragment, "FragmentTags.IconFragment")
	UE_DEFINE_GAMEPLAY_TAG(ItemNameFragment, "FragmentTags.ItemNameFragment")
	UE_DEFINE_GAMEPLAY_TAG(StackableFragment, "FragmentTags.StackableFragment")
	UE_DEFINE_GAMEPLAY_TAG(ConsumableFragment, "FragmentTags.ConsumableFragment")
	UE_DEFINE_GAMEPLAY_TAG(EquipmentFragment, "FragmentTags.EquipmentFragment")

	namespace ItemDescription
	{
		UE_DEFINE_GAMEPLAY_TAG(PrimaryStatFragment, "FragmentTags.ItemDescription.PrimaryStatFragment")
		UE_DEFINE_GAMEPLAY_TAG(ItemTypeFragment, "FragmentTags.ItemDescription.ItemTypeFragment")
		UE_DEFINE_GAMEPLAY_TAG(DescriptionTextFragment, "FragmentTags.ItemDescription.DescriptionTextFragment")
		UE_DEFINE_GAMEPLAY_TAG(SellValueFragment, "FragmentTags.ItemDescription.SellValueFragment")
		UE_DEFINE_GAMEPLAY_TAG(RequiredLevelFragment, "FragmentTags.ItemDescription.RequiredLevelFragment")
	}

	namespace StatMod
	{
		UE_DEFINE_GAMEPLAY_TAG(StatMod_1, "FragmentTags.StatMod.StatMod_1")
		UE_DEFINE_GAMEPLAY_TAG(StatMod_2, "FragmentTags.StatMod.StatMod_2")
		UE_DEFINE_GAMEPLAY_TAG(StatMod_3, "FragmentTags.StatMod.StatMod_3")
	}
}