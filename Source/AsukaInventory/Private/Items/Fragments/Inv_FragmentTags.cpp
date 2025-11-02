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
	UE_DEFINE_GAMEPLAY_TAG(StackableFragment, "FragmentTags.StackableFragment")
	UE_DEFINE_GAMEPLAY_TAG(ConsumableFragment, "FragmentTags.ConsumableFragment")
	UE_DEFINE_GAMEPLAY_TAG(EquipmentFragment, "FragmentTags.EquipmentFragment")
	UE_DEFINE_GAMEPLAY_TAG(WeaponFragment, "FragmentTags.WeaponFragment")
	UE_DEFINE_GAMEPLAY_TAG(WeaponAnimationsFragment, "FragmentTags.WeaponAnimationsFragment")
	UE_DEFINE_GAMEPLAY_TAG(WeaponMetaSoundsFragment, "FragmentTags.WeaponMetaSoundsFragment")
	UE_DEFINE_GAMEPLAY_TAG(SkeltalMeshFragment, "FragmentTags.SkeltonMeshFragment")
	UE_DEFINE_GAMEPLAY_TAG(AnimLayerFragment, "FragmentTags.AnimLayerFragment")
	UE_DEFINE_GAMEPLAY_TAG(GameplayAbilitiesFragment, "FragmentTags.GameplayAbilitiesFragment")
	UE_DEFINE_GAMEPLAY_TAG(GameplayEffectsFragment, "FragmentTags.GameplayEffectsFragment")
	UE_DEFINE_GAMEPLAY_TAG(ContainerFragment, "FragmentTags.ContainerFragment")
}

namespace ItemDescription
{
	UE_DEFINE_GAMEPLAY_TAG(ItemNameFragment, "UIElementTag.ItemDescription.ItemNameFragment")
	UE_DEFINE_GAMEPLAY_TAG(PrimaryStatFragment, "UIElementTag.ItemDescription.PrimaryStatFragment")
	UE_DEFINE_GAMEPLAY_TAG(ItemTypeFragment, "UIElementTag.ItemDescription.ItemTypeFragment")
	UE_DEFINE_GAMEPLAY_TAG(DescriptionTextFragment, "UIElementTag.ItemDescription.DescriptionTextFragment")
	UE_DEFINE_GAMEPLAY_TAG(SellValueFragment, "UIElementTag.ItemDescription.SellValueFragment")
	UE_DEFINE_GAMEPLAY_TAG(RequiredLevelFragment, "UIElementTag.ItemDescription.RequiredLevelFragment")
	namespace StatMod
	{
		UE_DEFINE_GAMEPLAY_TAG(StatMod_1, "UIElementTag.StatMod.StatMod_1")
		UE_DEFINE_GAMEPLAY_TAG(StatMod_2, "UIElementTag.StatMod.StatMod_2")
		UE_DEFINE_GAMEPLAY_TAG(StatMod_3, "UIElementTag.StatMod.StatMod_3")
	}
}