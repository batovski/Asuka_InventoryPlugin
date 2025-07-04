// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "GameplayTagContainer.h"
#include "Inv_EquippedSlottedItem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquippedSlottedItemClicked, class UInv_EquippedSlottedItem*, SlottedItem,const FPointerEvent&, MouseEvent);
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_EquippedSlottedItem : public UInv_SlottedItem
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SetEquipmentTypeTag(const FGameplayTag& InEquipmentTypeTag) { EquipmentTypeTag = InEquipmentTypeTag; }
	const FGameplayTag& GetEquipmentTypeTag() const { return EquipmentTypeTag; }

	FEquippedSlottedItemClicked OnEquippedSlottedItemClicked;

private:

	UPROPERTY()
	FGameplayTag EquipmentTypeTag;
	
};
