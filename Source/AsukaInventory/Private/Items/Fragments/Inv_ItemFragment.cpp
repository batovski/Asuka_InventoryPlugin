// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Fragments/Inv_ItemFragment.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Widgets/Composite/Inv_Leaf_Image.h"
#include "Widgets/Composite/Inv_Leaf_LabeledValue.h"
#include "Widgets/Composite/Inv_Leaf_Text.h"


void FInv_ImageFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragmentAbstract::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;

	const UInv_Leaf_Image* ImageLeaf = Cast<UInv_Leaf_Image>(Composite);
	if (!IsValid(ImageLeaf)) return;
	ImageLeaf->SetImage(Icon);
	ImageLeaf->SetImageSize(IconDimensions);
	ImageLeaf->SetBoxSize(IconDimensions);
}

void FInv_TextFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragmentAbstract::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;
	const UInv_Leaf_Text* TextLeaf = Cast<UInv_Leaf_Text>(Composite);
	if (!IsValid(TextLeaf)) return;
	TextLeaf->SetText(FragmentText);
}

void FInv_LabeledNumberFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragmentAbstract::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;
	const UInv_Leaf_LabeledValue* LabeledLeaf = Cast<UInv_Leaf_LabeledValue>(Composite);

	LabeledLeaf->SetLabelText(Text_Label, bCollapseLabel);

	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = MinFractionalDigits;
	Options.MaximumFractionalDigits = MaxFractionalDigits;

	LabeledLeaf->SetValueText(FText::AsNumber(Value, &Options), bCollapseValue);
}

void FInv_LabeledNumberFragment::Manifest()
{
	FInv_InventoryItemFragmentAbstract::Manifest();
	if(bRandomizeOnManifest)
	{
		// Randomize the number
		Value = FMath::RandRange(Min, Max);
	}
	bRandomizeOnManifest = false;
}

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	FInv_ConsumeModifier::OnConsume(PC);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,FString::Printf(TEXT("health Potion consumed! Healing by: %f"), GetValue()));
}

void FInv_InventoryItemFragmentAbstract::Assimilate(UInv_CompositeBase* Composite) const
{
	if (!MatchesWidgetTag(Composite)) return;
	Composite->Expand();
}

bool FInv_InventoryItemFragmentAbstract::MatchesWidgetTag(const UInv_CompositeBase* Composite) const
{
	return Composite->GetFragmentTag() == FragmentTag;
}

bool FInv_UIElementFragmentAbstract::MatchesWidgetTag(const UInv_CompositeBase* Composite) const
{
	return Composite->GetFragmentTag().MatchesTag(UIElementTag);
}

void FInv_ConsumableFragment::Manifest()
{
	FInv_InventoryItemFragmentAbstract::Manifest();

	for (auto& Modifier : ConsumeModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().Manifest();
	}
}

void FInv_ConsumableFragment::OnConsume(APlayerController* PC)
{
	for (auto& Modifier : ConsumeModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().OnConsume(PC);
	}
}

void FInv_ConsumableFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragmentAbstract::Assimilate(Composite);
	for(const auto& Modifier : ConsumeModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.Get().Assimilate(Composite);
	}
}


void FInv_ArmorModifier::OnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnEquip(PC);
	GEngine->AddOnScreenDebugMessage(-1,
		5.f,
		FColor::Green,
		FString::Printf(TEXT("Strength Increased by: %f"), GetValue()));
}
void FInv_ArmorModifier::OnUnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnUnEquip(PC);
	GEngine->AddOnScreenDebugMessage(-1,
		5.f,
		FColor::Green,
		FString::Printf(TEXT("Strength Decreased by: %f"), GetValue()));
}

void FInv_EquipmentFragment::Manifest()
{
	FInv_InventoryItemFragmentAbstract::Manifest();
	for (auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().Manifest();
	}
}

void FInv_EquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped) return; // Already equipped, do nothing
	for (auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().OnEquip(PC);
		return;
	}
	bEquipped = true;
}

void FInv_EquipmentFragment::OnUnEquip(APlayerController* PC)
{
	if (!bEquipped) return; // Not equipped, do nothing
	for (auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().OnUnEquip(PC);
		return;
	}
	bEquipped = false;
}
void FInv_EquipmentFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragmentAbstract::Assimilate(Composite);
	for (const auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.Get().Assimilate(Composite);
	}
}

AInv_EquipActor* FInv_EquipmentFragment::SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const
{
	if (!IsValid(AttachMesh) || !IsValid(EquipActorClass)) return nullptr;

	AInv_EquipActor* SpawnedActor = AttachMesh->GetWorld()->SpawnActor<AInv_EquipActor>(EquipActorClass);
	SpawnedActor->AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint);
	return SpawnedActor;
}

void FInv_EquipmentFragment::DestroyAttachedActor() const
{
	if (!EquippedActor.IsValid()) return;
	EquippedActor->Destroy();
}

void FInv_EquipmentFragment::SetEquippedActor(AInv_EquipActor* NewActor)
{
	EquippedActor = NewActor;
}

