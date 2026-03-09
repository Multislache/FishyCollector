// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PokedexSaveGame.generated.h"

UCLASS()
class FISHYCOLLECTOR_API UPokedexSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Noms des assets UPoissonTemplate déjà pêchés
	UPROPERTY()
	TSet<FName> PoissonsPeches;
};
