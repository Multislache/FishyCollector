// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PoissonTemplate.h"
#include "LieuTemplate.generated.h"

USTRUCT(BlueprintType)
struct FISHYCOLLECTOR_API FLieuRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lieu")
	TArray<UPoissonTemplate*> Poissons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lieu|Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ChanceCommun = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lieu|Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ChanceRare = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lieu|Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ChanceLegendaire = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lieu|Rareté", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ChanceSecret = 0.f;
};
