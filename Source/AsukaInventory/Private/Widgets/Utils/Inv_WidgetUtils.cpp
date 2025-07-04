// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Utils/Inv_WidgetUtils.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"

FIntPoint UInv_WidgetUtils::GetPositionFromIndex(const int32 Index, const int32 Columns)
{
	return FIntPoint(Index % Columns, Index / Columns);
}

FVector2D UInv_WidgetUtils::GetWidgetPosition(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(Widget, Geometry, USlateBlueprintLibrary::GetLocalTopLeft(Geometry), PixelPosition, ViewportPosition);
	return ViewportPosition;
}

FVector2D UInv_WidgetUtils::GetWidgetSize(UWidget* Widget)
{
	return Widget->GetCachedGeometry().GetLocalSize();
}

bool UInv_WidgetUtils::IsWithinBounds(const FVector2D& Position, const FVector2D& Size, const FVector2D& MousePosition)
{
	return MousePosition.X >= Position.X && MousePosition.X <= (Position.X + Size.X) &&
		MousePosition.Y >= Position.Y && MousePosition.Y <= (Position.Y + Size.Y);
}

FVector2D UInv_WidgetUtils::GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize,
	const FVector2D& MousePos)
{
	FVector2D ClampedPosition = MousePos;

	if(MousePos.X + WidgetSize.X > Boundary.X)
	{
		ClampedPosition.X = Boundary.X - WidgetSize.X;
	}
	if(MousePos.X < 0)
	{
		ClampedPosition.X = 0;
	}

	if(MousePos.Y + WidgetSize.Y > Boundary.Y)
	{
		ClampedPosition.Y = Boundary.Y - WidgetSize.Y;
	}
	if(MousePos.Y < 0)
	{
		ClampedPosition.Y = 0;
	}

	return ClampedPosition;
}
bool UInv_WidgetUtils::IsRightClick(const FPointerEvent& MouseEvent)
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInv_WidgetUtils::IsLeftClick(const FPointerEvent& MouseEvent) 
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}
