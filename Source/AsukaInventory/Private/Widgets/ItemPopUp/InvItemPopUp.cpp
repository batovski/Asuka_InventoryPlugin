// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemPopUp/InvItemPopUp.h"

#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

void UInvItemPopUp::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Split->OnClicked.AddDynamic(this, &UInvItemPopUp::SplitButtonClicked);
	Button_Equip->OnClicked.AddDynamic(this, &UInvItemPopUp::EquipButtonClicked);
	Button_Drop->OnClicked.AddDynamic(this, &UInvItemPopUp::DropButtonClicked);
	Button_Consume->OnClicked.AddDynamic(this, &UInvItemPopUp::ConsumeButtonClicked);
	Slider_Split->OnValueChanged.AddDynamic(this, &UInvItemPopUp::SliderValueChanged);
}

void UInvItemPopUp::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	RemoveFromParent();
}

void UInvItemPopUp::SplitButtonClicked()
{
	if(OnSplit.ExecuteIfBound(FMath::FloorToInt32(Slider_Split->GetValue()), GridIndex))
	{
		RemoveFromParent();
	}
}

void UInvItemPopUp::EquipButtonClicked()
{
	if(OneEquip.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UInvItemPopUp::DropButtonClicked()
{
	if(OnDrop.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UInvItemPopUp::ConsumeButtonClicked()
{
	if(OnConsume.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UInvItemPopUp::SliderValueChanged(float Value)
{
	Text_Split->SetText(FText::AsNumber(FMath::FloorToInt32(Value)));
}

void UInvItemPopUp::CollapseSplitButton() const
{
	Button_Split->SetVisibility(ESlateVisibility::Collapsed);
	Slider_Split->SetVisibility(ESlateVisibility::Collapsed);
	Text_Split->SetVisibility(ESlateVisibility::Collapsed);
}

void UInvItemPopUp::CollapseConsumeButton() const
{
	Button_Consume->SetVisibility(ESlateVisibility::Collapsed);
}

void UInvItemPopUp::CollapseEquipButton() const
{
	Button_Equip->SetVisibility(ESlateVisibility::Collapsed);
}

void UInvItemPopUp::SetSliderParams(const float Max, const float Value) const
{
	Slider_Split->SetMaxValue(Max);
	Slider_Split->SetMinValue(1);
	Slider_Split->SetValue(Value);
	Text_Split->SetText(FText::AsNumber(FMath::FloorToInt32(Value)));
}

FVector2D UInvItemPopUp::GetBoxSize() const
{
	return FVector2D(SizeBox_Root->GetWidthOverride(), SizeBox_Root->GetHeightOverride());
}
