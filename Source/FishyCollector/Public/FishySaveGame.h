    // Fill out your copyright notice in the Description page of Project Settings.

    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/SaveGame.h"
    #include "FishInventoryItem.h"
    #include "FishySaveGame.generated.h"

    USTRUCT(BlueprintType)
    struct FFishSaveEntry
    {
        GENERATED_BODY()

        UPROPERTY(SaveGame)
        FSoftObjectPath FishAssetPath;

        UPROPERTY(SaveGame)
        int32 Quantity = 0;

        UPROPERTY(SaveGame)
        FFishCatchRecord Capture;
    };

    UCLASS()
    class FISHYCOLLECTOR_API UFishySaveGame : public USaveGame
    {
        GENERATED_BODY()

    public:
        UPROPERTY(SaveGame)
        TArray<FFishSaveEntry> SavedInventory;

        UPROPERTY(SaveGame)
        int32 PlayerMoney;

        UPROPERTY(SaveGame)
        TArray<FPrimaryAssetId> OwnedRods;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
        bool bRodPurchased = false;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float Sensitivity = 1.0f;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float GlobalVolume = 1.0f;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float MusicVolume = 0.8f;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float SFXVolume = 0.8f;
    };