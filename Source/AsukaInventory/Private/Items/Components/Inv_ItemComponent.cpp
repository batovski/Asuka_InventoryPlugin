// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/Inv_ItemComponent.h"

#include "InventoryManagment/Utils/Inv_InventoryStatics.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Net/UnrealNetwork.h"

UInv_ItemComponent::UInv_ItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickupMessage = FString("E - Pick Up");
	SetIsReplicatedByDefault(true);
}

void UInv_ItemComponent::BeginPlay()
{
	Super::BeginPlay();
	if (StaticItemManifestID.IsValid() && StaticItemManifest.GetItemCategory() == EInv_ItemCategory::None)
	{
		StaticItemManifest = UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestID);
		if(GetOwner())
			StaticItemManifest.SetPickupActorClass(GetOwner()->GetClass());
	}
}

void UInv_ItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, StaticItemManifestID);
	DOREPLIFETIME(ThisClass, DynamicFragments);
}

void UInv_ItemComponent::PickedUp()
{
	OnPickedUp();
	GetOwner()->Destroy();
}

void UInv_ItemComponent::InitItemManifest(const FPrimaryAssetId& NewItemManifestID)
{
	StaticItemManifestID = NewItemManifestID;
}

void UInv_ItemComponent::InitDynamicData(const TArray<TInstancedStruct<FInv_ItemFragment>>& NewDynamicFragments)
{
	DynamicFragments = NewDynamicFragments;
}

FString& UInv_ItemComponent::GetPickupMessage()
{
	return PickupMessage;
}

const FPrimaryAssetId& UInv_ItemComponent::GetStaticItemManifestID() const
{
	return StaticItemManifestID;
}
void UInv_ItemComponent::OnRep_DynamicFragments()
{
	ApplyDynamicFragmentsToManifest();
}

void UInv_ItemComponent::ApplyDynamicFragmentsToManifest()
{
	if (StaticItemManifest.GetItemCategory() == EInv_ItemCategory::None)
	{
		StaticItemManifest = UInv_InventoryStatics::GetItemManifestFromID(StaticItemManifestID);
		if (GetOwner())
			StaticItemManifest.SetPickupActorClass(GetOwner()->GetClass());
	}
	TArray<TInstancedStruct<FInv_ItemFragment>>& StaticFragments = StaticItemManifest.GetFragmentsMutable();
	for (TInstancedStruct<FInv_ItemFragment>& DynamicFragment : DynamicFragments)
	{
		for (TInstancedStruct<FInv_ItemFragment>& StaticItemFragment : StaticFragments)
		{
			if (StaticItemFragment.Get().IsDynamicFragment() && StaticItemFragment.Get().GetFragmentTag().MatchesTagExact(DynamicFragment.Get().GetFragmentTag()))
			{
				StaticItemFragment = DynamicFragment;
			}
		}
	}
}

