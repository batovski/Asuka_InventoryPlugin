// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemDataAsset.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ASUKAINVENTORY_API UInv_ItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FInv_ItemManifest ItemManifest;
};
