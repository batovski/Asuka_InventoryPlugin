// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/HUD/Inv_HUDWidget.h"
#include "Inv_PlayerControllerBase.generated.h"

class UInputMappingContext;
class UInv_InventoryComponent;
/**
 * 
 */
UCLASS()
class ASUKAINVENTORY_API AInv_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	AInv_PlayerControllerBase();

	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ChangeCursorWidget(UUserWidget* NewCursorWidget);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetDefaultCursorWidget(UUserWidget* NewCursorWidget);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void PrimaryInteract();
	void CreateHUDWidget();
	void TraceForItem();

	void RotateHoverItem();

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> RotateHoverItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInv_HUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInv_HUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	double TraceLength;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	TWeakObjectPtr<AActor> ThisActor;
	TWeakObjectPtr<AActor> LastActor;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UUserWidget> DefaultMouseWidget;
};
