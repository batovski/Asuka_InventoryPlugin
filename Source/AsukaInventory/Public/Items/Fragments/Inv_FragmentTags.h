// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

namespace FragmentTags
{
	namespace DynamicFragmentTags
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StackDynamicFragment)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(PickUpActorFragment)
	}
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GridFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IconFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemNameFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StackableFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConsumableFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EquipmentFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WeaponFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SkeltalMeshFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AnimLayerFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayAbilitiesFragment)
	namespace WeaponFragmentTags
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(WeaponDamage)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(WeapomMaxAmmo)
	}
}
namespace ItemDescription
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryStatFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTypeFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DescriptionTextFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SellValueFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(RequiredLevelFragment)
	namespace StatMod
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatMod_1)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatMod_2)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatMod_3)
	}
}