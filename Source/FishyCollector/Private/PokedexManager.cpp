// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexManager.h"
#include "PoissonTemplate.h"
#include "PokedexRegistre.h"
#include "PokedexSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "FishyCollector.h"

const FString UPokedexManager::SaveSlotName = TEXT("PokedexSave");

UPokedexManager* UPokedexManager::GetInstance(UGameInstance* GameInstance)
{
	if (!GameInstance) return nullptr;
	return GameInstance->GetSubsystem<UPokedexManager>();
}

void UPokedexManager::InitialiserRegistre(UPokedexRegistre* Registre)
{
	if (!Registre)
	{
		UE_LOG(LogFishyCollector, Warning, TEXT("PokedexManager: Registre null"));
		return;
	}

	RegistreActuel = Registre;
	ChargerSauvegarde();

	UE_LOG(LogFishyCollector, Display, TEXT("PokedexManager: %d poisson(s), %d pêché(s)"),
		GetNombreTotal(), GetNombrePeches());
}

void UPokedexManager::ChargerSauvegarde()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		SaveGame = Cast<UPokedexSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}

	if (!SaveGame)
	{
		SaveGame = Cast<UPokedexSaveGame>(UGameplayStatics::CreateSaveGameObject(UPokedexSaveGame::StaticClass()));
	}
}

void UPokedexManager::Sauvegarder()
{
	if (SaveGame)
	{
		UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
	}
}

FName UPokedexManager::GetPoissonId(UPoissonTemplate* Poisson) const
{
	if (!Poisson) return NAME_None;
	return FName(*Poisson->GetName());
}

void UPokedexManager::MarquerCommePeche(UPoissonTemplate* Poisson)
{
	if (!Poisson || !SaveGame) return;

	SaveGame->PoissonsPeches.Add(GetPoissonId(Poisson));
	Sauvegarder();

	UE_LOG(LogFishyCollector, Display, TEXT("PokedexManager: '%s' pêché"), *Poisson->Nom.ToString());
}

bool UPokedexManager::EstPeche(UPoissonTemplate* Poisson) const
{
	if (!Poisson || !SaveGame) return false;
	return SaveGame->PoissonsPeches.Contains(GetPoissonId(Poisson));
}

TArray<FPokedexEntry> UPokedexManager::GetToutesLesEntrees() const
{
	TArray<FPokedexEntry> Entrees;
	if (!RegistreActuel) return Entrees;

	for (UPoissonTemplate* P : RegistreActuel->TousLesPoissons)
	{
		FPokedexEntry Entry;
		Entry.Poisson = P;
		Entry.bPeche = EstPeche(P);
		Entrees.Add(Entry);
	}
	return Entrees;
}

int32 UPokedexManager::GetNombrePeches() const
{
	return SaveGame ? SaveGame->PoissonsPeches.Num() : 0;
}

int32 UPokedexManager::GetNombreTotal() const
{
	return RegistreActuel ? RegistreActuel->TousLesPoissons.Num() : 0;
}
