// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Inv_ItemTags.h"

namespace GameItems
{
	namespace Equipment
	{
		namespace Weapons
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,"GameItems.Equipments.Weapons.Axe","Axe");
		}
		namespace Armors
		{
			UE_DEFINE_GAMEPLAY_TAG(Cloak,"GameItems.Equipments.Armors.Cloak");
		}
		namespace Helmets
		{
			UE_DEFINE_GAMEPLAY_TAG(Mask, "GameItems.Equipments.Helmets.Mask");
		}
	}

	namespace Consumables
	{
		namespace Potions
		{
			UE_DEFINE_GAMEPLAY_TAG(Small, "GameItems.Consumables.Potions.Small")
			UE_DEFINE_GAMEPLAY_TAG(Large,"GameItems.Consumables.Potions.Large");
		}
	}

	namespace Craftables
	{
		UE_DEFINE_GAMEPLAY_TAG(FireFernFruit, "GameItems.Craftables.FireFernFruit")
	}
}