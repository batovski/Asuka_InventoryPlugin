// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemDescription/Inv_ItemDescription.h"

#include "Components/SizeBox.h"

void UInv_ItemDescription::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);

	if(InVisibility == ESlateVisibility::Collapsed)
	{
		for (UInv_CompositeBase* Child : GetChildren())
		{
			if (IsValid(Child))
			{
				Child->Collapse();
			}
		}
	}
}

FVector2D UInv_ItemDescription::GetBoxSize() const
{
	return SizeBox->GetDesiredSize();
}
