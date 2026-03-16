#include "FishingRodStorage.h"

#include "FishyCollector.h"
#include "FishyCollectorCharacter.h"
#include "Blueprint/UserWidget.h"

AFishingRodStorage::AFishingRodStorage()
{
    PrimaryActorTick.bCanEverTick = false;
    ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
    RootComponent = ChestMesh;
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
    StorageWidgetInstance->AddToViewport();
}

void AFishingRodStorage::CloseStorage()
{
    if (StorageWidgetInstance) StorageWidgetInstance->RemoveFromParent();
    StorageWidgetInstance = nullptr;

    if (!InteractingPlayer) return;
    APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController());
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
        PC->ResetIgnoreMoveInput();
        PC->ResetIgnoreLookInput();
    }
    InteractingPlayer = nullptr;
}