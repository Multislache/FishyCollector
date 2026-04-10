// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FishingRod.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	UTexture2D* Icone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	float Chance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	float Prix = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod")
	TSubclassOf<AFishingRod> RodClass;


	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod|Bonus Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BonusCommun = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod|Bonus Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BonusRare = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod|Bonus Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BonusLegendaire = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FishingRod|Bonus Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BonusSecret = 0.f;
	
};
