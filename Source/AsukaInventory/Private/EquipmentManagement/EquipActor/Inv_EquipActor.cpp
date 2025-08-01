// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"

#include "ComponentUtils.h"
#include "Items/Inv_InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"


AInv_EquipActor::AInv_EquipActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AInv_EquipActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, OwningController);
	DOREPLIFETIME(ThisClass, OwningItem);
	DOREPLIFETIME(ThisClass, ReplicatedSkeletalMesh);
	DOREPLIFETIME(ThisClass, EquipmentType);
}

void AInv_EquipActor::SetEquipmentType_Implementation(const FGameplayTag& NewType)
{
	EquipmentType = NewType;
}

void AInv_EquipActor::SetOwningController_Implementation(AController* Controller)
{
	OwningController = Controller;
}

void AInv_EquipActor::SetOwningItem_Implementation(UInv_InventoryItem* Item)
{
	if (!IsValid(Item)) return;
	OwningItem = Item;
}

AController* AInv_EquipActor::GetOwningController()
{
	return OwningController.Get();
}

UInv_InventoryItem* AInv_EquipActor::GetOwningItem() const
{
	return OwningItem.Get();
}

void AInv_EquipActor::SetSkeletalMeshAsset_Implementation(USkeletalMesh* MeshAsset)
{
	ReplicatedSkeletalMesh = MeshAsset;
	OnSkeletalMeshAssetChanged(MeshAsset);
}

void AInv_EquipActor::OnSkeletalMeshAssetChanged_Implementation(USkeletalMesh* MeshAsset)
{
	if (USkeletalMeshComponent* MeshComponent = GetComponentByClass<USkeletalMeshComponent>())
	{
		MeshComponent->SetSkeletalMeshAsset(MeshAsset);
	}
}
void AInv_EquipActor::OnRep_ReplicatedSkeletalMesh()
{
	USkeletalMesh* LoadedMesh = ReplicatedSkeletalMesh.LoadSynchronous();
	if (IsValid(LoadedMesh))
	{
		if (USkeletalMeshComponent* MeshComponent = GetComponentByClass<USkeletalMeshComponent>())
		{
			MeshComponent->SetSkeletalMeshAsset(LoadedMesh);
		}
	}
}

void AInv_EquipActor::SetSkeletalMeshAnimationLayer_Implementation(TSubclassOf<UAnimInstance> NewAnimLayer)
{
	AnimationLayer = NewAnimLayer;
	OnSkeletalMeshAnimationLayerChanged(NewAnimLayer);
}

void AInv_EquipActor::OnSkeletalMeshAnimationLayerChanged_Implementation(TSubclassOf<UAnimInstance> NewAnimLayer)
{
	if(const AActor* ParentActor = GetAttachParentActor())
	{
		if(USkeletalMeshComponent* OwnerMesh = ParentActor->GetComponentByClass<USkeletalMeshComponent>())
		{
			OwnerMesh->LinkAnimClassLayers(NewAnimLayer);
		}
	}
}

void AInv_EquipActor::OnRep_ReplicatedAnimationLayer()
{
	TSubclassOf<UAnimInstance> LoadedAnimLayer = AnimationLayer.LoadSynchronous();
	if (!IsValid(LoadedAnimLayer)) return;

	if (const AActor* ParentActor = GetAttachParentActor())
	{
		if (USkeletalMeshComponent* OwnerMesh = ParentActor->GetComponentByClass<USkeletalMeshComponent>())
		{
			OwnerMesh->LinkAnimClassLayers(LoadedAnimLayer);
		}
	}
}

