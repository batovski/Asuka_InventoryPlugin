// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Widgets/Composite/Inv_Leaf.h"
#include "Inv_Leaf_Image.generated.h"

class USizeBox;
class UImage;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API UInv_Leaf_Image : public UInv_Leaf
{
	GENERATED_BODY()
public:
	UInv_Leaf_Image() { FragmentTag = FragmentTags::IconFragment; }
	void SetImage(UTexture2D* Texture) const;
	void SetBoxSize(const FVector2D& Size) const;
	void SetImageSize(const FVector2D& Size) const;
	FVector2D GetImageSize() const;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox>Size_Box;
};
