// Fill out your copyright notice in the Description page of Project Settings.


#include "RodSubsystem.h"
#include "FishySaveGame.h"
#include "Kismet/GameplayStatics.h"

void URodSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadMoney();
	LoadRod();
}
void URodSubsystem::AddRod(FPrimaryAssetId RodId)
{
	OwnedRods.Add(RodId);
	SaveRod();
	OnOwnedRodsUpdated.Broadcast();
}

bool URodSubsystem::HasRod(FPrimaryAssetId RodId) const
{
	return OwnedRods.Contains(RodId);
}

void URodSubsystem::AddMoney(float Amount)
{
	PlayerMoney += Amount;
	SaveMoney();
}

bool URodSubsystem::RemoveMoney(float Amount)
{
	if (PlayerMoney - Amount < 0) return false;
	PlayerMoney -= Amount;
	SaveMoney();
	return true;
}

float URodSubsystem::GetMoney() const
{
	return PlayerMoney;
}

void URodSubsystem::SaveMoney()
{
	if ( UFishySaveGame* SaveGameInstance = Cast<UFishySaveGame>(UGameplayStatics::CreateSaveGameObject(UFishySaveGame::StaticClass())))
	{
		SaveGameInstance->PlayerMoney = PlayerMoney;
		UE_LOG(LogTemp, Warning, TEXT("SAVED: %d"), SaveGameInstance->PlayerMoney);
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("MoneySave"), 0);
	}
}

void URodSubsystem::LoadMoney()
{
	if (UFishySaveGame* LoadedGame = Cast<UFishySaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MoneySave"), 0)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Money LOADED: %d"), LoadedGame->PlayerMoney);
		PlayerMoney = LoadedGame->PlayerMoney;
	}
	else UE_LOG(LogTemp, Warning, TEXT("No save game found or failed to load."));
}

void URodSubsystem::SaveRod()
{
	if ( UFishySaveGame* SaveGameInstance = Cast<UFishySaveGame>(UGameplayStatics::CreateSaveGameObject(UFishySaveGame::StaticClass())))
	{
		SaveGameInstance->OwnedRods = OwnedRods;
		for (const FPrimaryAssetId& RodId : OwnedRods)
		{
			UE_LOG(LogTemp, Warning, TEXT("Rod to save: %s"), *RodId.ToString());
		}
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("RodSave"), 0);
	}
}

void URodSubsystem::LoadRod()
{
	if (UFishySaveGame* LoadedGame = Cast<UFishySaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("RodSave"), 0)))
	{
		OwnedRods = LoadedGame->OwnedRods;
		for (const FPrimaryAssetId& RodId : OwnedRods)
		{
			UE_LOG(LogTemp, Warning, TEXT("Rod LOADED: %s"), *RodId.ToString());
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("No save game found or failed to load."));
	
}
