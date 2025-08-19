// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Fragments/Inv_ItemFragment.h"

#include "AbilitySystemComponent.h"
#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Widgets/Composite/Inv_Leaf_Image.h"
#include "Widgets/Composite/Inv_Leaf_LabeledValue.h"
#include "Widgets/Composite/Inv_Leaf_Text.h"
#include "Engine/SkeletalMesh.h"
#include "AbilitySystemGlobals.h"
#include "InventoryManagment/Utils/Inv_InventoryStatics.h"


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
	if (!IsValid(LabeledLeaf)) return;
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


void FInv_AnimLayerModifier::OnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnEquip(PC);
	if (!PC || !PC->GetPawn()) return;
	if(EquippedActor.IsValid())
	{
		EquippedActor->SetSkeletalMeshAnimationLayer(AnimationLayer.LoadSynchronous());
	}
}
void FInv_AnimLayerModifier::OnUnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnUnEquip(PC);
	if (!PC || !PC->GetPawn()) return;
	if (EquippedActor.IsValid())
	{
		EquippedActor->SetSkeletalMeshAnimationLayer(nullptr);
	}
}

void FInv_GameplayAbilitiesModifier::OnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnEquip(PC);
	if (!PC || !PC->GetPawn()) return;
	if (EquippedActor.IsValid())
	{
		if(const AActor* ParentActor = EquippedActor->GetAttachParentActor())
		{
			if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ParentActor))
			{
				for (auto AbilityClass : Abilities)
				{
					if (AbilityClass.IsValid())
					{
						TSubclassOf<UGameplayAbility> LoadedAbility = AbilityClass.LoadSynchronous();
						GrantedAbilities.Emplace(ASC->GiveAbility(ASC->BuildAbilitySpecFromClass(LoadedAbility)));
					}
				}
			}
		}
	}
}

void FInv_GameplayAbilitiesModifier::OnUnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnUnEquip(PC);
	if (!PC || !PC->GetPawn()) return;
	if (EquippedActor.IsValid())
	{
		if (const AActor* ParentActor = EquippedActor->GetAttachParentActor())
		{
			if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ParentActor))
			{
				for(auto AbilitySpecHandle : GrantedAbilities)
				{
					ASC->ClearAbility(AbilitySpecHandle);
				}
				GrantedAbilities.Empty();
			}
		}
	}
}

void FInv_GameplayEffectsModifier::OnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnEquip(PC);
	if (!PC || !PC->GetPawn()) return;
	if (EquippedActor.IsValid())
	{
		if(const AActor* ParentActor = EquippedActor->GetAttachParentActor())
		{
			if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ParentActor))
			{
				for (auto EffectClass : Effects)
				{
					if (EffectClass.IsValid())
					{
						if (TSubclassOf<UGameplayEffect> LoadedEffect = EffectClass.LoadSynchronous())
						{
							FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
							FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(LoadedEffect, 1.0f, EffectContext);
							if (EffectSpecHandle.IsValid())
							{
								GrantedEffects.Emplace(ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()));
							}
						}
					}
				}
			}
		}
	}
}

void FInv_GameplayEffectsModifier::OnUnEquip(APlayerController* PC)
{
	FInv_EquipModifier::OnUnEquip(PC);
	if (!PC || !PC->GetPawn()) return;
	if (EquippedActor.IsValid())
	{
		if (const AActor* ParentActor = EquippedActor->GetAttachParentActor())
		{
			if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ParentActor))
			{
				for(auto EffectHandle : GrantedEffects)
				{
					ASC->RemoveActiveGameplayEffect(EffectHandle);
				}
				GrantedEffects.Empty();
			}
		}
	}
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
	for (auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().EquippedActor = EquippedActor;
		Modifier.GetMutable().OnEquip(PC);
	}
}

void FInv_EquipmentFragment::OnUnEquip(APlayerController* PC)
{
	for (auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().OnUnEquip(PC);
	}
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

USkeletalMesh* FInv_SkeletalMeshFragment::GetDesiredSkeletalMesh() const
{
	if (SkeletalMesh.IsValid())
	{
		return SkeletalMesh.Get();
	}
	USkeletalMesh* LoadedMesh = SkeletalMesh.LoadSynchronous();

	if (!IsValid(LoadedMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load skeletal mesh from soft reference"));
		return nullptr;
	}
	
	return LoadedMesh;
}


