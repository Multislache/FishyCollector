#pragma once

#include "CoreMinimal.h"
#include "PoissonTemplate.h"
#include "FishInventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FFishCatchRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float Poids = 0.f;  

    UPROPERTY(BlueprintReadOnly)
    float Taille = 0.f; 
};

USTRUCT(BlueprintType)
struct FFishInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    UPoissonTemplate* Fish = nullptr;

    UPROPERTY(BlueprintReadOnly)
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadOnly)
    TArray<FFishCatchRecord> Captures;
};