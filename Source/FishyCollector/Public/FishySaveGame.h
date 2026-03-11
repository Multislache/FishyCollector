// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FishySaveGame.generated.h"

USTRUCT(BlueprintType)
struct FFishSaveEntry
{
    GENERATED_BODY()

    // Chemin vers l'asset PoissonTemplate (ex: "/Game/Fish/BP_Truite")
    UPROPERTY(SaveGame)
    FSoftObjectPath FishAssetPath;

    UPROPERTY(SaveGame)
    int32 Quantity = 0;
};

UCLASS()
class FISHYCOLLECTOR_API UFishySaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame)
    TArray<FFishSaveEntry> SavedInventory;
};