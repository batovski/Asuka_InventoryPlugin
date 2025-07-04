// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Manifest/Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Composite/Inv_CompositeBase.h"

UInv_InventoryItem* FInv_ItemManifest::Manifest(UObject* NewOuter)
{
	UInv_InventoryItem* Item = NewObject<UInv_InventoryItem>(NewOuter, UInv_InventoryItem::StaticClass());
	Item->SetItemManifest(*this);
	for(auto& Fragment : Item->GetItemManifestMutable().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}
	ClearFragments();
	return Item;
}

void FInv_ItemManifest::SpawnPickUpActor(const UObject* WorldContextObject, const FVector& SpawnLocation,
	const FRotator& SpawnRotation)
{
	if (!PickupActorClass || !WorldContextObject) return;

	AActor* SpawnedACtor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedACtor)) return;

	// Set the item manifest, on the spawned actor

	UInv_ItemComponent* ItemComponent = SpawnedACtor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComponent);

	ItemComponent->InitItemManifest(*this);
}

void FInv_ItemManifest::AssimilateInventoryFragments(UInv_CompositeBase* Composite) const
{
	const auto& ItemFragments = GetAllFragmentsOfType<FInv_InventoryItemFragmentAbstract>();
	for(const auto* Fragment : ItemFragments)
	{
		Composite->ApplyFunction([Fragment](UInv_CompositeBase* Child)
		{
			Fragment->Assimilate(Child);
		});
	}
}

void FInv_ItemManifest::ClearFragments()
{
	for(auto& Fragment : Fragments)
	{
		Fragment.Reset();
	}
	Fragments.Empty();
}
