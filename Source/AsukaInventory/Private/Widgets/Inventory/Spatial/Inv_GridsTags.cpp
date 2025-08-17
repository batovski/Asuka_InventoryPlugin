// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_GridsTags.h"

namespace InventoryGrid
{
	namespace Player
	{
		UE_DEFINE_GAMEPLAY_TAG(Backpack, "InventoryGrid.Player.Backpack");

		UE_DEFINE_GAMEPLAY_TAG(Consumables, "InventoryGrid.Player.Consumables");

		UE_DEFINE_GAMEPLAY_TAG(Craftables, "InventoryGrid.Player.Craftables");
		namespace Equipment
		{
			UE_DEFINE_GAMEPLAY_TAG(Weapon, "InventoryGrid.Player.Equipment.Weapon");
			UE_DEFINE_GAMEPLAY_TAG(Armor, "InventoryGrid.Player.Equipment.Armor");
			UE_DEFINE_GAMEPLAY_TAG(Helmet, "InventoryGrid.Player.Equipment.Helmet");
		}
	}

	namespace External
	{
		UE_DEFINE_GAMEPLAY_TAG(LootGrid, "InventoryGrid.External.LootGrid");
	}
}