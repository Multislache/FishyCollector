// Fill out your copyright notice in the Description page of Project Settings.


#include "FishInventorySubsystem.h"

void UFishInventorySubsystem::AddFish(UPoissonTemplate* Fish)
{
    if (!Fish) return;
    for (FFishInventoryItem& Item : Inventory)
    {
        if (Item.Fish == Fish)
        {
            Item.Quantity++;
            OnInventoryUpdated.Broadcast(); 
            return;
        }
    }
    FFishInventoryItem NewItem;
    NewItem.Fish = Fish;
    NewItem.Quantity = 1;
    Inventory.Add(NewItem);
    OnInventoryUpdated.Broadcast();
}

const TArray<FFishInventoryItem>& UFishInventorySubsystem::GetInventory() const
{
    return Inventory;
}