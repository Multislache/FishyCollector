// Copyright Epic Games, Inc. All Rights Reserved.

#include "FishyCollectorGameMode.h"

AFishyCollectorGameMode::AFishyCollectorGameMode()
{
	// stub
}

EPoissonRarete AFishyCollectorGameMode::TirerRarete(const FLieuRow& Lieu)
{
	const float Total = Lieu.ChanceCommun + Lieu.ChanceRare + Lieu.ChanceLegendaire + Lieu.ChanceSecret;
	const float Roll  = FMath::FRandRange(0.f, Total);

	if (Roll < Lieu.ChanceSecret)
		return EPoissonRarete::Secret;

	if (Roll < Lieu.ChanceSecret + Lieu.ChanceLegendaire)
		return EPoissonRarete::Legendaire;

	if (Roll < Lieu.ChanceSecret + Lieu.ChanceLegendaire + Lieu.ChanceRare)
		return EPoissonRarete::Rare;

	return EPoissonRarete::Commun;
}

UPoissonTemplate* AFishyCollectorGameMode::TirerUnPoissonDepuisTable(UDataTable* Table, FName LieuNom, bool& bSucces)
{
	bSucces = false;

	if (!Table)
		return nullptr;

	const FLieuRow* Lieu = Table->FindRow<FLieuRow>(LieuNom, TEXT("TirerUnPoisson"));
	if (!Lieu)
		return nullptr;

	const EPoissonRarete RareteCible = TirerRarete(*Lieu);

	// Filtrer les poissons du lieu selon la rareté tirée
	TArray<UPoissonTemplate*> Candidats;
	for (UPoissonTemplate* Poisson : Lieu->Poissons)
	{
		if (Poisson && Poisson->Rarete == RareteCible)
			Candidats.Add(Poisson);
	}

	// Fallback : si aucun poisson de cette rareté, prendre n'importe lequel
	if (Candidats.IsEmpty())
	{
		for (UPoissonTemplate* Poisson : Lieu->Poissons)
		{
			if (Poisson)
				Candidats.Add(Poisson);
		}
	}

	if (Candidats.IsEmpty())
		return nullptr;

	bSucces = true;
	return Candidats[FMath::RandRange(0, Candidats.Num() - 1)];
}

UPoissonTemplate* AFishyCollectorGameMode::TirerUnPoisson(FName LieuNom, bool& bSucces)
{
	return TirerUnPoissonDepuisTable(LieuTable, LieuNom, bSucces);
}
