#include "FishingRod.h"
#include "CableComponent.h"
#include "FishingHook.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
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
        FTransform SpawnTransform = LineAttachPoint->GetComponentTransform();
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        FishingHook = GetWorld()->SpawnActor<AFishingHook>(FishingHookClass, SpawnTransform, SpawnParams);
        if (FishingHook && FishingLine)
        {
            FishingLine->SetAttachEndToComponent(FishingHook->GetRootComponent());
        }
    }

    if (FishingLine) FishingLine->SetVisibility(false);
    if (FishingHook) FishingHook->SetActorHiddenInGame(true);
}

void AFishingRod::AttachToCharacter(ACharacter* Character, FName SocketName)
{
    if (!Character) return;
    OwnerCharacter = Character;
    AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
}

void AFishingRod::DetachFromCharacter()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AFishingRod::SetState(EFishingRodState NewState, FVector LaunchDirection)
{
    if (!CanTransitionTo(NewState))
    {
        UE_LOG(LogFishyCollector, Warning, TEXT("FishingRod: Transition invalide de %d vers %d"), (uint8)CurrentState, (uint8)NewState);
        return;
    }

    EFishingRodState OldState = CurrentState;
    CurrentState = NewState;
    UE_LOG(LogFishyCollector, Display, TEXT("FishingRod: %d -> %d"), (uint8)OldState, (uint8)CurrentState);
    OnStateChanged(OldState, CurrentState, LaunchDirection);
}

bool AFishingRod::CanTransitionTo(EFishingRodState NewState) const
{
    if (NewState == CurrentState) return false;
    if (NewState == EFishingRodState::Repos) return true;

    switch (CurrentState)
    {
    case EFishingRodState::Repos:
        return NewState == EFishingRodState::Lance;

    case EFishingRodState::Lance:
        return NewState == EFishingRodState::Attente;

    case EFishingRodState::Attente:
        return NewState == EFishingRodState::Morsure;

    case EFishingRodState::Morsure:
        return NewState == EFishingRodState::Tirer;

    default:
        return false;
    }
}

void AFishingRod::OnStateChanged_Implementation(EFishingRodState OldState, EFishingRodState NewState, FVector LaunchDirection)
{
    switch (NewState)
    {
    case EFishingRodState::Repos:
        if (FishingLine) FishingLine->SetVisibility(false);
        if (FishingHook) FishingHook->SetActorHiddenInGame(true);
        break;

    case EFishingRodState::Lance:
        if (FishingLine) FishingLine->SetVisibility(true);
        if (FishingHook)
        {
            if (RodSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, RodSound, GetActorLocation());
            }

            // On utilise la direction passée en paramètre
            FVector ForwardDirection = LaunchDirection.IsNearlyZero()
                ? OwnerCharacter->GetActorForwardVector() // fallback
                : LaunchDirection;

            FVector TraceStart = OwnerCharacter->GetActorLocation()
                + ForwardDirection * 500.f
                + FVector(0.f, 0.f, 500.f);

            FVector TraceEnd = TraceStart + FVector(0.f, 0.f, -2000.f);

            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);
            QueryParams.AddIgnoredActor(OwnerCharacter);
            QueryParams.AddIgnoredActor(FishingHook);

            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult, TraceStart, TraceEnd,
                ECC_Visibility, QueryParams
            );

            FVector HookLocation = bHit
                ? HitResult.ImpactPoint
                : OwnerCharacter->GetActorLocation() + ForwardDirection * 500.f;

            FishingHook->SetActorLocation(HookLocation);

            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 2.f);
            DrawDebugSphere(GetWorld(), HookLocation, 10.f, 8, FColor::Red, false, 2.f);
            
            GetWorldTimerManager().SetTimer(
                RodTimerHandle,
                this,
                &AFishingRod::ShowHook,
                1.5f,
                false
            );
        }
        break;

    case EFishingRodState::Attente:
        if (SplashSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, SplashSound, FishingHook->GetActorLocation());
        }
        StartWaitingForBite();
        break;
    case EFishingRodState::Morsure:
        if (FishBiteSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, FishBiteSound, FishingHook->GetActorLocation());
        }
        break;
    case EFishingRodState::Tirer:
        if (FishingLine) FishingLine->SetVisibility(true);
        if (FishingHook) FishingHook->SetActorHiddenInGame(false);
        break;

    default:
        break;
    }
}

void AFishingRod::ShowHook()
{
    FishingHook->SetActorHiddenInGame(false);
    SetState(EFishingRodState::Attente);
}

void AFishingRod::StartWaitingForBite()
{
    float RandomDelay = FMath::RandRange(5.0f, 10.0f);
    UE_LOG(LogFishyCollector, Display, TEXT("En attente d'une morsure... (%.1f secondes)"), RandomDelay);

    GetWorldTimerManager().SetTimer(
        FishBiteTimerHandle,
        this,
        &AFishingRod::TriggerFishBite,
        RandomDelay,
        false
    );
}

void AFishingRod::TriggerFishBite()
{
    UE_LOG(LogFishyCollector, Display, TEXT("Un poisson a mordu !"));
    SetState(EFishingRodState::Morsure);
}