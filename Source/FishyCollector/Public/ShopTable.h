// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ShopTable.generated.h"

UCLASS()
class FISHYCOLLECTOR_API AShopTable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShopTable();

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* TableMesh;

	UPROPERTY(EditAnywhere)
	UBoxComponent* InteractionZone;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
