// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvItemPopUp.generated.h"

class USizeBox;
class UTextBlock;
class UButton;
class USlider;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPopUpMenuSplit, int32, SplitAmount, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuDrop, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuConsume, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuEquip, int32, Index);
/**     
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInvItemPopUp : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void CollapseSplitButton() const;
	void CollapseConsumeButton() const;
	void CollapseEquipButton() const;
	void SetSliderParams(const float Max, const float Value) const;
	FVector2D GetBoxSize() const;
	void SetGridIndex(const int32 Index) { GridIndex = Index; }
	int32 GetGridIndex() const { return GridIndex; }

	FPopUpMenuSplit OnSplit;
	FPopUpMenuDrop OnDrop;
	FPopUpMenuConsume OnConsume;
	FPopUpMenuEquip OnEquip;

private:

	UFUNCTION()
	void SplitButtonClicked();

	UFUNCTION()
	void EquipButtonClicked();

	UFUNCTION()
	void DropButtonClicked();

	UFUNCTION()
	void ConsumeButtonClicked(); 

	UFUNCTION()
	void SliderValueChanged(float Value);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Equip;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Drop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	int32 GridIndex{ INDEX_NONE };
};
