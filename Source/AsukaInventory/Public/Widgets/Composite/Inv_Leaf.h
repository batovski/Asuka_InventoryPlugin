// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Inv_Leaf.generated.h"

/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_Leaf : public UInv_CompositeBase
{
	GENERATED_BODY()
public:
	virtual void ApplyFunction(FuncType Func) override;
};
