#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FishInventoryItem.h"
#include "FishInventorySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(BlueprintType)
class FISHYCOLLECTOR_API UFishInventorySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    void AddFish(UPoissonTemplate* Fish);

    UFUNCTION(BlueprintCallable)
    const TArray<FFishInventoryItem>& GetInventory() const;

    UPROPERTY(BlueprintAssignable)
    FOnInventoryUpdated OnInventoryUpdated;

private:
    UPROPERTY()
    TArray<FFishInventoryItem> Inventory;
};