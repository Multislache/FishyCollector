// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LieuTemplate.h"
#include "PoissonTemplate.h"
#include "FishyCollectorGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AFishyCollectorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AFishyCollectorGameMode();

	// DataTable des lieux de pêche (row type : FLieuRow)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pêche")
	UDataTable* LieuTable;

	// Tire un poisson depuis le lieu indiqué en utilisant LieuTable du GameMode
	UFUNCTION(BlueprintCallable, Category = "Pêche")
	UPoissonTemplate* TirerUnPoisson(FName LieuNom, bool& bSucces);

	// Version statique : utilisable sans instance GameMode (ex: tests)
	static UPoissonTemplate* TirerUnPoissonDepuisTable(UDataTable* Table, FName LieuNom, bool& bSucces);

private:
	static EPoissonRarete TirerRarete(const FLieuRow& Lieu);
};
