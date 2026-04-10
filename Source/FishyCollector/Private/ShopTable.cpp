// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopTable.h"

// Sets default values
AShopTable::AShopTable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
	RootComponent = TableMesh;
	
	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
	InteractionZone->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AShopTable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShopTable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

