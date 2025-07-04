// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/Inv_Leaf_LabeledValue.h"

#include "Components/TextBlock.h"

void UInv_Leaf_LabeledValue::SetLabelText(const FText& InText, bool bCollapse) const
{
	if(bCollapse)
	{
		Text_Label->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Text_Label->SetText(InText);
}

void UInv_Leaf_LabeledValue::SetValueText(const FText& InText, bool bCollapse) const
{
	if (bCollapse)
	{
		Text_Value->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Text_Value->SetText(InText);
}

void UInv_Leaf_LabeledValue::NativePreConstruct()
{
	Super::NativePreConstruct();

	FSlateFontInfo FontInfo = Text_Label->GetFont();
	FontInfo.Size = FontSize_Label;
	Text_Label->SetFont(FontInfo);

	FontInfo = Text_Value->GetFont();
	FontInfo.Size = FontSize_Value;
	Text_Value->SetFont(FontInfo);
}
