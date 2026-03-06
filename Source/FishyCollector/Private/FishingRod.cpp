#include "FishingRod.h"
#include "CableComponent.h"
#include "FishingHook.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "FishyCollector.h"

AFishingRod::AFishingRod()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    RodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RodMesh"));
    RodMesh->SetupAttachment(RootComponent);

    LineAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LineAttachPoint"));
    LineAttachPoint->SetupAttachment(RodMesh);

    FishingLine = CreateDefaultSubobject<UCableComponent>(TEXT("FishingLine"));
    FishingLine->SetupAttachment(LineAttachPoint);
    FishingLine->CableLength = 300.f;
    FishingLine->NumSegments = 12;
    FishingLine->CableWidth = 0.5f;
    FishingLine->bAttachEnd = true;
    FishingLine->SetVisibility(false);
}

void AFishingRod::BeginPlay()
{
    Super::BeginPlay();

    if (FishingHookClass)
    {
        FishingHook = GetWorld()->SpawnActor<AFishingHook>(FishingHookClass);
        if (FishingHook && FishingLine)
        {
            FishingLine->SetAttachEndToComponent(FishingHook->GetRootComponent());
        }
    }
}

void AFishingRod::AttachToCharacter(ACharacter* Character, FName SocketName)
{
    if (!Character) return;
    AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
}

void AFishingRod::DetachFromCharacter()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AFishingRod::SetState(EFishingRodState NewState)
{
    if (!CanTransitionTo(NewState))
    {
        UE_LOG(LogFishyCollector, Warning, TEXT("FishingRod: Transition invalide de %d vers %d"), (uint8)CurrentState, (uint8)NewState);
        return;
    }

    EFishingRodState OldState = CurrentState;
    CurrentState = NewState;
    UE_LOG(LogFishyCollector, Display, TEXT("FishingRod: %d -> %d"), (uint8)OldState, (uint8)CurrentState);
    OnStateChanged(OldState, CurrentState);
}

bool AFishingRod::CanTransitionTo(EFishingRodState NewState) const
{
    if (NewState == CurrentState) return false;
    switch (CurrentState)
    {
        case EFishingRodState::Repos:
            return NewState == EFishingRodState::Lance;

        case EFishingRodState::Lance:
            return NewState == EFishingRodState::Attente || NewState == EFishingRodState::Repos;

        case EFishingRodState::Attente:
            return NewState == EFishingRodState::Morsure || NewState == EFishingRodState::Repos;

        case EFishingRodState::Morsure:
            return NewState == EFishingRodState::Tirer || NewState == EFishingRodState::Repos;

        case EFishingRodState::Tirer:
            return NewState == EFishingRodState::Repos;

        default:
            return false;
    }
}

void AFishingRod::OnStateChanged_Implementation(EFishingRodState OldState, EFishingRodState NewState)
{
    
}