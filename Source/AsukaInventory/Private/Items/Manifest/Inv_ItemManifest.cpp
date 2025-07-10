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
	//Item->SetDynamicItemFragments(GetAllDynamicFragmentsFromManifest());
	for(auto& Fragment : Item->GetItemManifestMutable().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}
	ClearFragments();
	return Item;
}

UInv_ItemComponent* FInv_ItemManifest::SpawnPickUpActor(const UObject* WorldContextObject, const FVector& SpawnLocation,
	const FRotator& SpawnRotation)
{
	if (!PickupActorClass || !WorldContextObject) return nullptr;

	AActor* SpawnedACtor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedACtor)) return nullptr;

	// Set the item manifest, on the spawned actor

	UInv_ItemComponent* ItemComponent = SpawnedACtor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComponent);

	return ItemComponent;
}

//TArray<TInstancedStruct<FInv_ItemFragment>> FInv_ItemManifest::GetAllDynamicFragmentsFromManifest()
//{
//	TArray<TInstancedStruct<FInv_ItemFragment>> DynamicFragments;
//	for (auto& Fragment : StaticFragments)
//	{
//		if (Fragment.GetPtr<FInv_ItemFragment>()->IsDynamicFragment())
//		{
//			DynamicFragments.Emplace(TInstancedStruct<FInv_ItemFragment>::Make(Fragment));
//		}
//	}
//	return DynamicFragments;
//}

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
	for(auto& Fragment : StaticFragments)
	{
		Fragment.Reset();
	}
	StaticFragments.Empty();
}
