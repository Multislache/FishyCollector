// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PokedexManager.generated.h"

class UPoissonTemplate;
class UPokedexRegistre;
class UPokedexSaveGame;

USTRUCT(BlueprintType)
struct FPokedexEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pokedex")
	UPoissonTemplate* Poisson = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Pokedex")
	bool bPeche = false;
};

UCLASS()
class FISHYCOLLECTOR_API UPokedexManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Getter explicite pour Blueprint (évite les problèmes de world context)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pokedex", meta = (DefaultToSelf = "GameInstance"))
	static UPokedexManager* GetInstance(UGameInstance* GameInstance);

	// A appeler depuis le GameInstance BP (Init)
	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void InitialiserRegistre(UPokedexRegistre* Registre);

	// Marque un poisson comme pêché et sauvegarde
	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void MarquerCommePeche(UPoissonTemplate* Poisson);

	// Retourne true si ce poisson a déjà été pêché
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pokedex")
	bool EstPeche(UPoissonTemplate* Poisson) const;

	// Retourne toutes les entrées pour l'UI (Poisson + bPeche)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pokedex")
	TArray<FPokedexEntry> GetToutesLesEntrees() const;

	// Nombre de poissons déjà pêchés
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pokedex")
	int32 GetNombrePeches() const;

	// Nombre total de poissons dans le registre
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pokedex")
	int32 GetNombreTotal() const;

	// Remet à zéro tous les poissons pêchés
	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void ResetPokedex();

private:
	UPROPERTY()
	UPokedexRegistre* RegistreActuel = nullptr;

	UPROPERTY()
	UPokedexSaveGame* SaveGame = nullptr;

	static const FString SaveSlotName;

	void ChargerSauvegarde();
	void Sauvegarder();
	FName GetPoissonId(UPoissonTemplate* Poisson) const;
};
