#include "FishingRod.h"
#include "CableComponent.h"
#include "FishingHook.h"
#include "PoissonTemplate.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FishyCollector.h"
#include "Components/WidgetComponent.h"
#include "FishyCollectorGameMode.h"
#include "Blueprint/UserWidget.h"
#include "PokedexManager.h"
#include "FishInventorySubsystem.h"

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
    if (FishingHook)
    {
        FishingHook->SetActorHiddenInGame(true);
        FishingHook->OnHookLanded.AddDynamic(this, &AFishingRod::OnHookLandedInWater);
        FishingHook->OnHookInvalidSurface.AddDynamic(this, &AFishingRod::OnHookInvalidSurface);
    }
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
    if (FishingHook && FishingHook->GetQTEWidgetComponent())
    {
        if (NewState == EFishingRodState::Morsure)
        {
            // La visibilité est gérée dans TriggerFishBite, on peut la forcer ici au besoin
        }
        else
        {
            // On cache le widget pour tous les autres états
            FishingHook->GetQTEWidgetComponent()->SetVisibility(false);
            FishingWidgetInstance = nullptr;
        }
    }
    switch (NewState)
    {
    case EFishingRodState::Repos:
        GetWorldTimerManager().ClearAllTimersForObject(this);
        if (FishingLine) FishingLine->SetVisibility(false);
        if (FishingHook) FishingHook->SetActorHiddenInGame(true);
        break;

    case EFishingRodState::Lance:
        if (!OwnerCharacter || !FishingHook) return;
        if (FishingLine) FishingLine->SetVisibility(true);
        if (RodSound) UGameplayStatics::PlaySoundAtLocation(this, RodSound, GetActorLocation());

        {
            FVector ForwardDirection = LaunchDirection.IsNearlyZero() ? OwnerCharacter->GetActorForwardVector() : LaunchDirection;
            FVector TraceStart = OwnerCharacter->GetActorLocation() + ForwardDirection * 500.f + FVector(0.f, 0.f, 500.f);
            FVector TraceEnd = TraceStart + FVector(0.f, 0.f, -2000.f);

            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);
            QueryParams.AddIgnoredActor(OwnerCharacter);
            QueryParams.AddIgnoredActor(FishingHook);

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
            FVector HookLocation = bHit ? HitResult.ImpactPoint : TraceStart + (FVector::DownVector * 500.f);
            
            FishingHook->SetActorLocation(HookLocation);
            GetWorldTimerManager().SetTimer(RodTimerHandle, this, &AFishingRod::ShowHook, 1.5f, false);
        }
        break;

    case EFishingRodState::Attente:
        if (FishingHook && SplashSound) UGameplayStatics::PlaySoundAtLocation(this, SplashSound, FishingHook->GetActorLocation());
        StartWaitingForBite();
        break;

    case EFishingRodState::Morsure:
        if (FishingHook && FishBiteSound) UGameplayStatics::PlaySoundAtLocation(this, FishBiteSound, FishingHook->GetActorLocation());
        break;

    case EFishingRodState::Tirer:
        if (FishingLine) FishingLine->SetVisibility(true);
        if (FishingHook) FishingHook->SetActorHiddenInGame(false);
        break;
    }
}

void AFishingRod::ShowHook()
{
    if (!FishingHook) return;

    FishingHook->SetActorHiddenInGame(false);

    // Faire un overlap check manuel à la position du hook
    TArray<AActor*> OverlappingActors;
    FishingHook->GetOverlappingActors(OverlappingActors);

    bool bInWater = false;
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->ActorHasTag(FName("water")))
        {
            bInWater = true;
            break;
        }
    }

    if (bInWater)
    {
        GetWorldTimerManager().ClearTimer(RodTimerHandle);
        SetState(EFishingRodState::Attente);
    }
    else
    {
        // Pas dans l'eau → timeout de sécurité
        GetWorldTimerManager().SetTimer(RodTimerHandle, this, &AFishingRod::OnHookTimeout, 2.0f, false);
    }
}

void AFishingRod::StartWaitingForBite()
{
    float RandomDelay = FMath::RandRange(5.0f, 10.0f);
    GetWorldTimerManager().SetTimer(FishBiteTimerHandle, this, &AFishingRod::TriggerFishBite, RandomDelay, false);
}

void AFishingRod::TriggerFishBite()
{
    // 1. On demande au GameMode de choisir un poisson selon le lieu actuel
    AFishyCollectorGameMode* GM = GetWorld()->GetAuthGameMode<AFishyCollectorGameMode>();
    if (GM && !CurrentLieuName.IsNone())
    {
        bool bSucces;
        // On stocke le poisson choisi dans la canne
        CurrentFishBiting = GM->TirerUnPoisson(CurrentLieuName, bSucces);
    }

    SetState(EFishingRodState::Morsure);

    if (FishingWidgetClass && FishingHook && CurrentFishBiting)
    {
        UWidgetComponent* HookWidgetComp = FishingHook->GetQTEWidgetComponent();
        if (HookWidgetComp)
        {
            HookWidgetComp->SetVisibility(true);
            UUserWidget* NewWidget = HookWidgetComp->GetUserWidgetObject();
            
            if (NewWidget)
            {
                FishingWidgetInstance = NewWidget;

                FProperty* FishProp = NewWidget->GetClass()->FindPropertyByName(FName("FishData"));
                if (FObjectProperty* ObjProp = CastField<FObjectProperty>(FishProp))
                {
                    ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(NewWidget), CurrentFishBiting);
                }

                FProperty* RodProp = NewWidget->GetClass()->FindPropertyByName(FName("ParentRod"));
                if (FObjectProperty* ObjProp = CastField<FObjectProperty>(RodProp))
                {
                    ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(NewWidget), this);
                }

                UFunction* ResetFunc = NewWidget->FindFunction(FName("ResetWidget"));
                if (ResetFunc) NewWidget->ProcessEvent(ResetFunc, nullptr);
            }
        }
    }
}


void AFishingRod::EndMiniGame(bool bSuccess, UPoissonTemplate* CaughtFish)
{
    FishingWidgetInstance = nullptr;
    if (bSuccess && CaughtFish)
    {
        UFishInventorySubsystem* Inventory =
            GetWorld()->GetGameInstance()->GetSubsystem<UFishInventorySubsystem>();

        if (Inventory)
        {
            Inventory->AddFish(CaughtFish);
        }

        CurrentFishBiting = CaughtFish;

        UPokedexManager* Manager = UPokedexManager::GetInstance(GetWorld()->GetGameInstance());
        if (Manager)
        {
            Manager->MarquerCommePeche(CaughtFish);
        }

        SetState(EFishingRodState::Repos);
        if (FishNotifyWidgetClass)
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            UUserWidget* NotifyWidget = CreateWidget<UUserWidget>(PC, FishNotifyWidgetClass);
            if (NotifyWidget)
            {
                FProperty* Prop = NotifyWidget->GetClass()->FindPropertyByName(FName("FishData"));
                if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
                {
                    ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(NotifyWidget), CaughtFish);
                }

                NotifyWidget->AddToViewport();
                
                UFunction* PlayAnimFunc = NotifyWidget->FindFunction(FName("PlaySwingIn")); 
                if (PlayAnimFunc) NotifyWidget->ProcessEvent(PlayAnimFunc, nullptr);
            }
        }
    }
    else
    {
        SetState(EFishingRodState::Repos);
    }
}

void AFishingRod::HandleInput()
{
    if (CurrentState == EFishingRodState::Morsure && FishingWidgetInstance)
    {
        UFunction* Func = FishingWidgetInstance->FindFunction(FName("OnInputReceived"));
        if (Func)
        {
            FishingWidgetInstance->ProcessEvent(Func, nullptr);
        }
    }
}

void AFishingRod::OnHookLandedInWater()
{
    if (CurrentState == EFishingRodState::Lance)
    {
        GetWorldTimerManager().ClearTimer(RodTimerHandle);
        if (FishingHook) FishingHook->SetActorHiddenInGame(false);
        SetState(EFishingRodState::Attente);
    }
}

void AFishingRod::OnHookInvalidSurface()
{
    if (CurrentState == EFishingRodState::Lance)
    {
        GetWorldTimerManager().ClearTimer(RodTimerHandle);
        GetWorldTimerManager().ClearTimer(FishBiteTimerHandle);
        if (FishingHook) FishingHook->SetActorHiddenInGame(true);
        SetState(EFishingRodState::Repos);
    }
}

void AFishingRod::OnHookTimeout()
{
    UE_LOG(LogFishyCollector, Warning, TEXT("OnHookTimeout appelé, état: %d"), (uint8)CurrentState);

    if (CurrentState == EFishingRodState::Lance)
    {
        if (FishingHook && FishingHook->WrongSurfaceSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, FishingHook->WrongSurfaceSound,
                FishingHook->GetActorLocation());
        }

        if (FishingHook) FishingHook->SetActorHiddenInGame(true);
        SetState(EFishingRodState::Repos);
    }
}

void AFishingRod::setLieuName(FName NewLieuName)
{
    CurrentLieuName = NewLieuName;
}