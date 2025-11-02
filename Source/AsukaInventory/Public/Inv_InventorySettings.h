// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Inv_InventorySettings.generated.h"

/**
 * Global Inventory System Settings
 * Accessible from Project Settings -> Plugins -> Asuka Inventory
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Asuka Inventory Settings"))
class ASUKAINVENTORY_API UInv_InventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UInv_InventorySettings();

	/** Get the settings singleton */
	static const UInv_InventorySettings* Get();

	/** Size of each tile in the inventory grid (in pixels) */
	UPROPERTY(Config, EditAnywhere, Category = "Grid", meta = (ClampMin = "10.0", ClampMax = "200.0", UIMin = "10.0", UIMax = "200.0"))
	float TileSize;

	// UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif
};

