// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Inv_WidgetUtils.generated.h"

class UWidget;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_WidgetUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static int32 GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
	{
		return Position.X + Position.Y * Columns;
	}
	static FIntPoint GetPositionFromIndex(const int32 Index, const int32 Columns);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static FVector2D GetWidgetPosition(UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static FVector2D GetWidgetSize(UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool IsWithinBounds(const FVector2D& Position, const FVector2D& Size, const FVector2D& MousePosition);

	static FVector2D GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize, const FVector2D& MousePos);

	static bool IsRightClick(const FPointerEvent& MouseEvent);

	static bool IsLeftClick(const FPointerEvent& MouseEvent);
};
