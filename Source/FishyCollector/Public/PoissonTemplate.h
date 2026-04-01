// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "PoissonTemplate.generated.h"

UENUM(BlueprintType)
enum class EPoissonRarete : uint8
{
	Commun     UMETA(DisplayName = "Commun"),
	Rare       UMETA(DisplayName = "Rare"),
	Legendaire UMETA(DisplayName = "Légendaire"),
	Secret UMETA(DisplayName = "Secret"),
};

UCLASS(BlueprintType)
class FISHYCOLLECTOR_API UPoissonTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson")
	FText Nom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson")
	EPoissonRarete Rarete = EPoissonRarete::Commun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson")
	TSoftObjectPtr<UMaterialInterface> Materiau;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson")
	UTexture2D* Icone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson|Stats")
	float PoidsMin = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson|Stats")
	float PoidsMax = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson|Stats")
	float TailleMin = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson|Stats")
	float TailleMax = 60.f;
};
