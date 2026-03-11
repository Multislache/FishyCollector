
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FishInventoryItem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "FishInventorySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS()
class FISHYCOLLECTOR_API UFishInventorySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable)
    void AddFish(UPoissonTemplate* Fish);

    UFUNCTION(BlueprintCallable)
    const TArray<FFishInventoryItem>& GetInventory() const;

    UFUNCTION(BlueprintCallable)
    void SaveInventory();

    UFUNCTION(BlueprintCallable)
    void LoadInventory();

    UPROPERTY(BlueprintAssignable)
    FOnInventoryUpdated OnInventoryUpdated;

private:
    UPROPERTY()
    TArray<FFishInventoryItem> Inventory;

    static const FString SaveSlotName;

    void OnAssetsLoaded(TArray<FSoftObjectPath> Paths, TArray<int32> Quantities);
};