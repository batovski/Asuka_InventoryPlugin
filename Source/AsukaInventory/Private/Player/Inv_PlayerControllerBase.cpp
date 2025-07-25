// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Inv_PlayerControllerBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Kismet/GameplayStatics.h"

AInv_PlayerControllerBase::AInv_PlayerControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceLength = 500.0;
	ItemTraceChannel = ECC_Camera;
}

void AInv_PlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	InventoryComponent = FindComponentByClass<UInv_InventoryComponent>();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (IsValid(Subsystem))
	{
		Subsystem->AddMappingContext(DefaultIMC, 0);
	}

	CreateHUDWidget();
}

void AInv_PlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AInv_PlayerControllerBase::ToggleInventory);
	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &AInv_PlayerControllerBase::PrimaryInteract);
}

void AInv_PlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TraceForItem();
}

void AInv_PlayerControllerBase::ToggleInventory()
{
	if(InventoryComponent.IsValid())
	{
		InventoryComponent->ToggleInventoryMenu();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Component is not valid!"));
	}
}

void AInv_PlayerControllerBase::PrimaryInteract()
{
	if (!ThisActor.IsValid()) return;

	UInv_ItemComponent* ItemComp = ThisActor->FindComponentByClass<UInv_ItemComponent>();
	if (IsValid(ItemComp) && InventoryComponent.IsValid())
	{
		InventoryComponent->TryAddItemByComponent(ItemComp);
	}
	UInv_ExternalInventoryComponent* ItemsContainer = ThisActor->FindComponentByClass<UInv_ExternalInventoryComponent>();
	if(IsValid(ItemsContainer))
	{
		ItemsContainer->OpenItemsContainer(this);
	}
}

void AInv_PlayerControllerBase::CreateHUDWidget()
{
	if (!IsLocalController()) return;
	HUDWidget = CreateWidget<UInv_HUDWidget>(this, HUDWidgetClass);
	if (IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
}

void AInv_PlayerControllerBase::TraceForItem()
{
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport)) return;
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D ViewportCenter = ViewportSize / 2.f;
	FVector TraceStart;
	FVector Forward;
	if (!UGameplayStatics::DeprojectScreenToWorld(this, ViewportCenter, TraceStart, Forward)) return;

	const FVector TraceEnd = TraceStart + Forward * TraceLength;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel);

	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	if (!ThisActor.IsValid())
	{
		if (IsValid(HUDWidget)) HUDWidget->HidePickupMessage();
	}

	if (ThisActor == LastActor) return;

	if (ThisActor.IsValid())
	{
		UInv_ItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInv_ItemComponent>();
		UInv_ExternalInventoryComponent* ItemContainerComponent = ThisActor->FindComponentByClass<UInv_ExternalInventoryComponent>();

		// TODO: Could be refactored to Interface call
		if (IsValid(HUDWidget))
		{
			if(IsValid(ItemComponent))
				HUDWidget->ShowPickupMessage(ItemComponent->GetPickupMessage());
			if(IsValid(ItemContainerComponent))
				HUDWidget->ShowPickupMessage(ItemContainerComponent->GetPickupMessage());
		}
	}

	if (LastActor.IsValid())
	{
	}
}
