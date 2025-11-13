// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace GameItems
{
	namespace Equipments
	{
		namespace Weapons
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
		}
		namespace Armors
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cloak);
		}
		namespace Helmets
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mask);
		}
		namespace Backpacks
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Backpack);
		}
	}

	namespace Consumables
	{
		namespace Potions
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Small)
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Large);
		}
	}

	namespace Craftables
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(FireFernFruit)
	}
}