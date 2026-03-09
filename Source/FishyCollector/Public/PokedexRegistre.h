// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PokedexRegistre.generated.h"

class UPoissonTemplate;

UCLASS(BlueprintType)
class FISHYCOLLECTOR_API UPokedexRegistre : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pokedex")
	TArray<UPoissonTemplate*> TousLesPoissons;
};
