// Fill out your copyright notice in the Description page of Project Settings.

#include "FishInventorySubsystem.h"
#include "PoissonTemplate.h"
#include "FishySaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UFishInventorySubsystem::SaveSlotName = TEXT("FishyInventorySave");

void UFishInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadInventory();
}

void UFishInventorySubsystem::AddFish(UPoissonTemplate* Fish)
{
    if (!Fish) return;

    FFishCatchRecord Record;
    // Taille aléatoire
    Record.Taille = FMath::RandRange(Fish->TailleMin, Fish->TailleMax);

    // Normalisation (0 → 1)
    float Range = Fish->TailleMax - Fish->TailleMin;
    float Alpha = (Range > 0.f) ? (Record.Taille - Fish->TailleMin) / Range : 0.f;

    // Courbe réaliste (poids dépend du volume ≈ cube)
    float WeightFactor = FMath::Pow(Alpha, 3);

    // Interpolation du poids
    Record.Poids = FMath::Lerp(Fish->PoidsMin, Fish->PoidsMax, WeightFactor);

    // Petite variation naturelle
    float RandomFactor = FMath::RandRange(0.85f, 1.15f);
    Record.Poids *= RandomFactor;
    
    if (Fish->Rarete == EPoissonRarete::Legendaire)
    {
        Record.Prix = round((Record.Poids) * 10.f + Record.Taille*0.3);
    }
    else if (Fish->Rarete == EPoissonRarete::Rare)
    {
        Record.Prix = round((Record.Poids) * 6.f + Record.Taille*0.3);
    }
    else
    {
        Record.Prix = round((Record.Poids) * 3.f + Record.Taille*0.3);
    }

    for (FFishInventoryItem& Item : Inventory)
    {
        if (Item.Fish == Fish
            && Item.Capture.Poids == Record.Poids
            && Item.Capture.Taille == Record.Taille)
        {
            Item.Quantity++;
            OnInventoryUpdated.Broadcast();
            SaveInventory();
            return;
        }
    }

    FFishInventoryItem NewItem;
    NewItem.Fish = Fish;
    NewItem.Quantity = 1;
    NewItem.Capture = Record;
    Inventory.Add(NewItem);
    OnInventoryUpdated.Broadcast();
    SaveInventory();
}

void UFishInventorySubsystem::RemoveFish(int index )
{
    Inventory.RemoveAt(index);
    OnInventoryUpdated.Broadcast();
    SaveInventory();
}

const TArray<FFishInventoryItem>& UFishInventorySubsystem::GetInventory() const
{
    return Inventory;
}

void UFishInventorySubsystem::SaveInventory()
{
    UFishySaveGame* SaveGame = Cast<UFishySaveGame>(
        UGameplayStatics::CreateSaveGameObject(UFishySaveGame::StaticClass()));
    if (!SaveGame) return;

    for (const FFishInventoryItem& Item : Inventory)
    {
        if (!Item.Fish) continue;

        FFishSaveEntry Entry;
        Entry.FishAssetPath = FSoftObjectPath(Item.Fish);
        Entry.Quantity = Item.Quantity;
        Entry.Capture = Item.Capture;
        SaveGame->SavedInventory.Add(Entry);
    }

    UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
    UE_LOG(LogTemp, Display, TEXT("FishInventory: Sauvegarde OK (%d entrées)"), Inventory.Num());
}

void UFishInventorySubsystem::LoadInventory()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0)) return;

    USaveGame* RawSave = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
    if (!RawSave) return;

    UFishySaveGame* SaveGame = Cast<UFishySaveGame>(RawSave);
    if (!SaveGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("FishInventory: Sauvegarde incompatible, reset."));
        UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
        return;
    }

    if (SaveGame->SavedInventory.IsEmpty()) return;

    TArray<FSoftObjectPath>      PathsToLoad;
    TArray<int32>                Quantities;
    TArray<FFishCatchRecord>     Captures;

    for (const FFishSaveEntry& Entry : SaveGame->SavedInventory)
    {
        PathsToLoad.Add(Entry.FishAssetPath);
        Quantities.Add(Entry.Quantity);
        Captures.Add(Entry.Capture);
    }

    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    Streamable.RequestAsyncLoad(PathsToLoad,
        FStreamableDelegate::CreateUObject(this,
            &UFishInventorySubsystem::OnAssetsLoaded,
            PathsToLoad, Quantities, Captures)
    );
}

void UFishInventorySubsystem::OnAssetsLoaded(
    TArray<FSoftObjectPath>  Paths,
    TArray<int32>            Quantities,
    TArray<FFishCatchRecord> Captures)
{
    Inventory.Empty();

    for (int32 i = 0; i < Paths.Num(); i++)
    {
        UPoissonTemplate* Fish = Cast<UPoissonTemplate>(Paths[i].ResolveObject());
        if (!Fish) continue;

        FFishInventoryItem Item;
        Item.Fish = Fish;
        Item.Quantity = Quantities[i];
        Item.Capture = Captures[i];
        Inventory.Add(Item);
    }

    OnInventoryUpdated.Broadcast();
    UE_LOG(LogTemp, Display, TEXT("FishInventory: Chargement async OK (%d entrées)"), Inventory.Num());
}

