// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace InventoryGrid
{
	namespace Player
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Backpack);

		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consumables);

		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Craftables);

		namespace Equipment
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Armor);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Helmet);
		}

	}

	namespace External
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(LootGrid)
	}
}