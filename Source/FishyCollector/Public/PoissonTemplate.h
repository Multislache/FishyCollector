// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PoissonTemplate.generated.h"

UENUM(BlueprintType)
enum class EPoissonRarete : uint8
{
	Commun     UMETA(DisplayName = "Commun"),
	Rare       UMETA(DisplayName = "Rare"),
	Legendaire UMETA(DisplayName = "Légendaire"),
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
};
