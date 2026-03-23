#include "FishingRodStorage.h"

#include "FishyCollector.h"
#include "FishyCollectorCharacter.h"
#include "Blueprint/UserWidget.h"

AFishingRodStorage::AFishingRodStorage()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
    
    ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
    ChestMesh->SetupAttachment(RootComponent);
}

void AFishingRodStorage::OpenStorage(AFishyCollectorCharacter* PlayerCharacter)
{
    if (!StorageWidgetClass || !PlayerCharacter)
    {
        return;
    }
    InteractingPlayer = PlayerCharacter;
    APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
    if (!PC)
    {
        return;
    }
    StorageWidgetInstance = CreateWidget<UUserWidget>(PC, StorageWidgetClass);
    if (!StorageWidgetInstance)
    {
        return;
    }
    UFunction* InitFunc = StorageWidgetInstance->FindFunction(FName("InitStorage"));
    if (InitFunc)
    {
        struct { AFishingRodStorage* InStorage; AFishyCollectorCharacter* InCharacter; } Params;
        Params.InStorage = this;
        Params.InCharacter = PlayerCharacter;
        StorageWidgetInstance->ProcessEvent(InitFunc, &Params);
    }
    if (InteractWidgetInstance)
    {
        InteractWidgetInstance->RemoveFromParent();
        InteractWidgetInstance = nullptr;
    } 
    PlayerCharacter->OuvrirWidget(StorageWidgetInstance, PC);
}

void AFishingRodStorage::CloseStorage()
{
    if (StorageWidgetInstance) StorageWidgetInstance->RemoveFromParent();
    StorageWidgetInstance = nullptr;

    if (!InteractingPlayer) return;
    APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController());
    InteractingPlayer->FermerWidget(PC);
    InteractingPlayer = nullptr;
}