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
#include "GameplayEffect.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Inv_ContainerHolderActor.h"


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

void FInv_LabeledNumberFragment::Manifest(UObject* Owner)
{
	FInv_InventoryItemFragmentAbstract::Manifest(Owner);
	if(bRandomizeOnManifest)
	{
		// Randomize the number
		Value = FMath::RandRange(Min, Max);
	}
	bRandomizeOnManifest = false;
}

void FInv_GridFragment::RotateGrid()
{
	GridSize = FIntPoint(GridSize.Y, GridSize.X);
	Alignment = (Alignment == EInv_ItemAlignment::Horizontal) 
		? EInv_ItemAlignment::Vertical 
		: EInv_ItemAlignment::Horizontal;
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

void FInv_ConsumableFragment::Manifest(UObject* Owner)
{
	FInv_InventoryItemFragmentAbstract::Manifest(Owner);

	for (auto& Modifier : ConsumeModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().Manifest(Owner);
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

void FInv_ConsumeModifier::OnConsume(APlayerController* PC)
{
	if (!PC || !PC->GetPawn()) return;
	if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn()))
	{
		if (TSubclassOf<UGameplayEffect> LoadedEffect = Effect.LoadSynchronous())
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(PC->GetPawn());
			FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(LoadedEffect, 1.0f, EffectContext);
			if (EffectSpecHandle.IsValid())
			{
				{
					const FGameplayTag ValueTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Data.Value")), false);
					if (ValueTag.IsValid())
					{
						EffectSpecHandle.Data->SetSetByCallerMagnitude(ValueTag, Value);
					}
				}
				ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
			}
		}
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
					TSubclassOf<UGameplayAbility> LoadedAbility = AbilityClass.LoadSynchronous();
					auto AbilitySpec = ASC->BuildAbilitySpecFromClass(LoadedAbility);
					AbilitySpec.SourceObject = EquippedActor.Get();
					GrantedAbilities.Emplace(ASC->GiveAbility(AbilitySpec));
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
					if (TSubclassOf<UGameplayEffect> LoadedEffect = EffectClass.LoadSynchronous())
					{
						FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
						EffectContext.AddSourceObject(EquippedActor.Get());
						FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(LoadedEffect, 1.0f, EffectContext);
						if (EffectSpecHandle.IsValid())
						{
							auto GrantedEffect = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
							if (GrantedEffect.IsValid())
							{
								GrantedEffects.Emplace(GrantedEffect);
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

void FInv_EquipmentFragment::Manifest(UObject* Owner)
{
	FInv_InventoryItemFragmentAbstract::Manifest(Owner);
	for (auto& Modifier : EquipModifiers)
	{
		if (!Modifier.IsValid()) continue;
		Modifier.GetMutable().Manifest(Owner);
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

void FInv_ContainerFragment::Manifest(UObject* Owner)
{
	FInv_ItemFragment::Manifest(Owner);
	if (!ContainerInventoryComponent)
	{
		if (const UActorComponent* OwnerComponent = Cast<UActorComponent>(Owner))
		{
			if (const AActor* OwnerActor = OwnerComponent->GetOwner())
			{
				if (!OwnerActor->HasAuthority())
				{
					return;
				}
				
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = const_cast<AActor*>(OwnerActor);
				SpawnParams.Instigator = OwnerActor->GetInstigator();
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				const auto SpawnedContainerActor = OwnerActor->GetWorld()->SpawnActor<AInv_ContainerHolderActor>(AInv_ContainerHolderActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

				ContainerInventoryComponent = NewObject<UInv_ExternalInventoryComponent>(
					SpawnedContainerActor,
					UInv_ExternalInventoryComponent::StaticClass(),
					NAME_None,
					RF_Transactional
				);
				
				ContainerInventoryComponent->SetIsReplicated(true);
				ContainerInventoryComponent->OnComponentCreated();
				SpawnedContainerActor->AddInstanceComponent(ContainerInventoryComponent);
				ContainerInventoryComponent->RegisterComponent();

				ContainerInventoryComponent->Rows = ContainerDimension.Y;
				ContainerInventoryComponent->Columns = ContainerDimension.X;
			}
		}
	}
}


