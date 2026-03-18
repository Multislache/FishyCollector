// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RodSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOwnedRodsUpdated);

UCLASS()
class FISHYCOLLECTOR_API URodSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	TArray<FPrimaryAssetId> OwnedRods;

	UPROPERTY()
	float PlayerMoney;

	UFUNCTION(BlueprintCallable)
	void AddRod(FPrimaryAssetId RodId);

	UFUNCTION(BlueprintCallable)
	bool HasRod(FPrimaryAssetId RodId) const;

	UFUNCTION(BlueprintCallable)
	void AddMoney(float Amount);

	UFUNCTION(BlueprintCallable)
	bool RemoveMoney(float Amount);

	UFUNCTION(BlueprintCallable)
	float GetMoney() const;

	UPROPERTY(BlueprintAssignable)
	FOnOwnedRodsUpdated OnOwnedRodsUpdated;
	
};
