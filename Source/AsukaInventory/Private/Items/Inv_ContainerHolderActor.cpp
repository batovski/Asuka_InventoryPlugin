// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Inv_ContainerHolderActor.h"

// Sets default values
AInv_ContainerHolderActor::AInv_ContainerHolderActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetReplicates(true);
}

// Called when the game starts or when spawned
void AInv_ContainerHolderActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInv_ContainerHolderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

