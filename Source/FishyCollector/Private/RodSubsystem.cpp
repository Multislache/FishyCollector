// Fill out your copyright notice in the Description page of Project Settings.


#include "RodSubsystem.h"

void URodSubsystem::AddRod(FPrimaryAssetId RodId)
{
	OwnedRods.Add(RodId);
	OnOwnedRodsUpdated.Broadcast();
}

bool URodSubsystem::HasRod(FPrimaryAssetId RodId) const
{
	return OwnedRods.Contains(RodId);
}

void URodSubsystem::AddMoney(float Amount)
{
	PlayerMoney += Amount;
}

bool URodSubsystem::RemoveMoney(float Amount)
{
	if (PlayerMoney - Amount < 0) return false;
	PlayerMoney -= Amount;
	return true;
}

float URodSubsystem::GetMoney() const
{
	return PlayerMoney;
}
