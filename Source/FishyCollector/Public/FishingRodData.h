// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FishingRodData.generated.h"

/**
 * 
 */
UCLASS()
class FISHYCOLLECTOR_API UFishingRodData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("FishingRod", GetFName());
    }
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	FText Nom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	UStaticMesh* Mesh;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poisson")
	//TSoftObjectPtr<UMaterialInterface> Materiau;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	UTexture2D* Icone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	float Chance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	float Prix = 0.0f;
	
};
