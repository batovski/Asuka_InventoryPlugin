// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemComponent.generated.h"


class UInv_ItemDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ASUKAINVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInv_ItemComponent();
	void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void PickedUp();
	void InitItemManifest(FInv_ItemManifest CopyOfManifest);
	FString& GetPickupMessage();
	FInv_ItemManifest GetItemManifest() const { return StaticItemManifest; }
	const FPrimaryAssetId& GetStaticItemManifestID() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnPickedUp();

private:
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FPrimaryAssetId StaticItemManifestID;
	/*UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FInv_ItemManifest ItemManifest;*/
	UPROPERTY()
	FInv_ItemManifest StaticItemManifest;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage;
};
