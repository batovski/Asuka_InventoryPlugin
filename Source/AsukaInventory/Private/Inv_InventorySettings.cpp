// Fill out your copyright notice in the Description page of Project Settings.

#include "Inv_InventorySettings.h"

UInv_InventorySettings::UInv_InventorySettings()
{
	// Default tile size
	TileSize = 64.0f;
}

const UInv_InventorySettings* UInv_InventorySettings::Get()
{
	return GetDefault<UInv_InventorySettings>();
}

FName UInv_InventorySettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

#if WITH_EDITOR
FText UInv_InventorySettings::GetSectionText() const
{
	return NSLOCTEXT("AsukaInventory", "InventorySettingsSection", "Asuka Inventory");
}

FText UInv_InventorySettings::GetSectionDescription() const
{
	return NSLOCTEXT("AsukaInventory", "InventorySettingsDescription", "Configure global settings for the Asuka Inventory System");
}
#endif

