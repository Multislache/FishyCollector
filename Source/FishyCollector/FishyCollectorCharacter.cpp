// Copyright Epic Games, Inc. All Rights Reserved.

#include "FishyCollectorCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FishingRod.h"
#include "FishingRodStorage.h"
#include "InputActionValue.h"
#include "PokedexWidget.h"
#include "FishyBaseWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "FishyCollector.h"
#include "Public/PokedexManager.h"
#include "Framework/Application/SlateApplication.h"


void AFishyCollectorCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (FishingRodClass)
    {
       FishingRod = GetWorld()->SpawnActor<AFishingRod>(FishingRodClass);
       if (FishingRod)
       {
          FishingRod->AttachToCharacter(this);
       }
    }
}

AFishyCollectorCharacter::AFishyCollectorCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
       
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void AFishyCollectorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
       
       EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::JumpSiPokedexFerme);
       EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

       EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::Move);
       EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::Look);
       EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::Look);

       if (InteractAction)
          EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::Interact);

       if (PokedexAction)
          EnhancedInputComponent->BindAction(PokedexAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::TogglePokedex);

       if (ResetPokedexAction)
          EnhancedInputComponent->BindAction(ResetPokedexAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ResetPokedex);
       
       if (ClickAction)
          EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ClickInteractionManager);

       if (PokedexRotateLeftAction)
          EnhancedInputComponent->BindAction(PokedexRotateLeftAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::PokedexRoterGauche);
       
       if (PokedexRotateRightAction)
          EnhancedInputComponent->BindAction(PokedexRotateRightAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::PokedexRoterDroite);
       
       if (Echap)
          EnhancedInputComponent->BindAction(Echap, ETriggerEvent::Started, this, &AFishyCollectorCharacter::HandleEscape);

       if (RetourAction)
          EnhancedInputComponent->BindAction(RetourAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::RetourGeneral);

       if (InventaireAction)
          EnhancedInputComponent->BindAction(InventaireAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ToggleInventaire);

       if (Map)
          EnhancedInputComponent->BindAction(Map, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ToggleMap);
    }
}

// --- LOGIQUE QTE SECURISÉE ---
void AFishyCollectorCharacter::ProcessFishingInput()
{
    if (!FishingRod || FishingRod->GetCurrentState() != EFishingRodState::Morsure)
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Empêche le double-clic instantané (E + Clic)
    if (CurrentTime - LastQTEInputTime >= QTEInputCooldown)
    {
        LastQTEInputTime = CurrentTime;
        FishingRod->HandleInput();
    }
}

void AFishyCollectorCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    DoMove(MovementVector.X, MovementVector.Y);
}

void AFishyCollectorCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AFishyCollectorCharacter::DoMove(float Right, float Forward)
{
    if (bUIWidgetOuvert)
    {
       NaviguerUI(Right, Forward);
       return;
    }

    if (GetController() != nullptr)
    {
       const FRotator Rotation = GetController()->GetControlRotation();
       const FRotator YawRotation(0, Rotation.Yaw, 0);
       const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
       const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

       AddMovementInput(ForwardDirection, Forward);
       AddMovementInput(RightDirection, Right);
    }
}

void AFishyCollectorCharacter::DoLook(float Yaw, float Pitch)
{
    if (GetController() != nullptr)
    {
       AddControllerYawInput(Yaw * MouseSensitivity);
       AddControllerPitchInput(Pitch * MouseSensitivity);
    }
}

void AFishyCollectorCharacter::Interact()
{
    if (bUIWidgetOuvert)
    {
       AccepterUI();
       return;
    }

    // Gestion du QTE via la fonction sécurisée
    if (FishingRod && FishingRod->GetCurrentState() == EFishingRodState::Morsure)
    {
       ProcessFishingInput();
       return;
    }

    if (NearbyStorage)
    {
       NearbyStorage->OpenStorage(this);
       return;
    }
    if (bIsInFishingZone && FishingRod)
    {
       DoThrowLine();
    }
    if (bIsInShopZone)
    {
       ToggleShop();
    }
}

void AFishyCollectorCharacter::ClickInteractionManager()
{
    // Gestion du QTE via la fonction sécurisée
    if (FishingRod && FishingRod->GetCurrentState() == EFishingRodState::Morsure)
    {
       ProcessFishingInput();
       return; 
    }
}

void AFishyCollectorCharacter::DoThrowLine_Implementation()
{
    if (!FishingRod) return;

    if (FishingRod->GetCurrentState() == EFishingRodState::Repos)
    {
       FVector LaunchDirection = FVector::ForwardVector;
       if (AController* C = GetController())
       {
          FRotator YawOnly = FRotator(0.f, C->GetControlRotation().Yaw, 0.f);
          LaunchDirection = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
       }
       FishingRod->SetState(EFishingRodState::Lance, LaunchDirection);
    }
    else
    {
       FishingRod->SetState(EFishingRodState::Repos);
    }
}

void AFishyCollectorCharacter::SetFishingZoneActive(bool bActive) { bIsInFishingZone = bActive; }
void AFishyCollectorCharacter::SetShopZoneActive(bool bActive) { bIsInShopZone = bActive; }

void AFishyCollectorCharacter::ResetPokedex()
{
    if (UPokedexManager* Manager = GetGameInstance()->GetSubsystem<UPokedexManager>())
       Manager->ResetPokedex();
}

void AFishyCollectorCharacter::TogglePokedex()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    if (PokedexWidget && PokedexWidget->IsInViewport())
    {
       PokedexWidget->RemoveFromParent();
       FermerWidget(PC);
    }
    else
    {
       if (bUIWidgetOuvert) 
       {
          if (PauseMenuWidget && PauseMenuWidget->IsInViewport()) {
             PauseMenuWidget->RemoveFromParent();
             UGameplayStatics::SetGamePaused(GetWorld(), false);
          }
          if (InventaireWidget && InventaireWidget->IsInViewport()) {
             InventaireWidget->RemoveFromParent();
          }
       }

       if (!PokedexWidgetClass) return;
       if (!PokedexWidget) PokedexWidget = CreateWidget<UPokedexWidget>(PC, PokedexWidgetClass);
       else PokedexWidget->Rafraichir();

       if (PokedexWidget) OuvrirWidget(PokedexWidget, PC);
    }
}

void AFishyCollectorCharacter::ToggleShop()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !ShopWidgetClass) return;

    if (ShopWidget && ShopWidget->IsInViewport())
    {
       ShopWidget->RemoveFromParent();
       FermerWidget(PC);
    }
    else
    {
       if (bUIWidgetOuvert) return;
       if (!ShopWidget) ShopWidget = CreateWidget<UUserWidget>(PC, ShopWidgetClass);
       if (ShopWidget) OuvrirWidget(ShopWidget, PC);
    }
}

void AFishyCollectorCharacter::SetNearbyStorage(AFishingRodStorage* Storage) { NearbyStorage = Storage; }

void AFishyCollectorCharacter::EquipRodFromData(UFishingRodData* RodData)
{
    if (!RodData || !RodData->RodClass) return;

    if (FishingRod)
    {
       FishingRod->DetachFromCharacter();
       FishingRod->Destroy();
    }
    FishingRod = GetWorld()->SpawnActor<AFishingRod>(RodData->RodClass);
    if (FishingRod)
    {
       FishingRod->AttachToCharacter(this);
       FishingRod->SetRodData(RodData);
       EquippedFishingRod = RodData;
    }
}

void AFishyCollectorCharacter::UnequipRod()
{
    if (!FishingRod) return;
    FishingRod->DetachFromCharacter();
    FishingRod->Destroy();
    FishingRod = nullptr;
}

TSubclassOf<AFishingRod> AFishyCollectorCharacter::GetCurrentRodClass() const
{
    return FishingRod ? FishingRod->GetClass() : nullptr;
}

void AFishyCollectorCharacter::JumpSiPokedexFerme()
{
    if (!bUIWidgetOuvert) Jump();
}

UFishyBaseWidget* AFishyCollectorCharacter::GetWidgetOuvert() const
{
    if (PopupActif && PopupActif->IsInViewport()) return PopupActif;
    if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(PokedexWidget);   W && W->IsInViewport()) return W;
    if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(ShopWidget);      W && W->IsInViewport()) return W;
    if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(InventaireWidget);W && W->IsInViewport()) return W;
    if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(MapWidget);       W && W->IsInViewport()) return W;
    return nullptr;
}

void AFishyCollectorCharacter::SetPopupActif(UFishyBaseWidget* Popup) { PopupActif = Popup; }

void AFishyCollectorCharacter::PokedexRoterGauche()
{
    if (bUIWidgetOuvert) if (UFishyBaseWidget* Widget = GetWidgetOuvert()) Widget->NaviguerGauche();
}

void AFishyCollectorCharacter::PokedexRoterDroite()
{
    if (bUIWidgetOuvert) if (UFishyBaseWidget* Widget = GetWidgetOuvert()) Widget->NaviguerDroite();
}

void AFishyCollectorCharacter::ToggleInventaire()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !InventaireWidgetClass) return;

    if (bUIWidgetOuvert && InventaireWidget)
    {
       InventaireWidget->RemoveFromParent();
       InventaireWidget = nullptr;
       FermerWidget(PC);
       return;
    }

    if (bUIWidgetOuvert) return;

    InventaireWidget = CreateWidget<UUserWidget>(PC, InventaireWidgetClass);
    if (InventaireWidget) OuvrirWidget(InventaireWidget, PC);
}

void AFishyCollectorCharacter::NaviguerUI(float X, float Y)
{
    if (PopupActif && PopupActif->IsInViewport()) return;

    constexpr float Seuil = 0.5f;
    if (FMath::Abs(X) < Seuil && FMath::Abs(Y) < Seuil)
    {
       DernierNavigationUI = 0.f;
       return;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    constexpr float Debounce = 0.18f;
    if (Now - DernierNavigationUI < Debounce) return;
    DernierNavigationUI = Now;

    FKey NavKey;
    if (FMath::Abs(X) >= FMath::Abs(Y))
       NavKey = (X > 0.f) ? EKeys::Gamepad_DPad_Right : EKeys::Gamepad_DPad_Left;
    else
       NavKey = (Y > 0.f) ? EKeys::Gamepad_DPad_Up : EKeys::Gamepad_DPad_Down;

    FKeyEvent Down(NavKey, FModifierKeysState(), 0, false, 0, 0);
    FKeyEvent Up(NavKey, FModifierKeysState(), 0, false, 0, 0);
    FSlateApplication::Get().ProcessKeyDownEvent(Down);
    FSlateApplication::Get().ProcessKeyUpEvent(Up);
}

void AFishyCollectorCharacter::AccepterUI()
{
    if (PopupActif && PopupActif->IsInViewport()) return;

    FKeyEvent Down(EKeys::Enter, FModifierKeysState(), 0, false, 0, 0);
    FKeyEvent Up(EKeys::Enter, FModifierKeysState(), 0, false, 0, 0);
    FSlateApplication::Get().ProcessKeyDownEvent(Down);
    FSlateApplication::Get().ProcessKeyUpEvent(Up);
}

void AFishyCollectorCharacter::OuvrirWidget(UUserWidget* Widget, APlayerController* PC)
{
    if (!Widget || !PC) return;
    bUIWidgetOuvert = true;
    Widget->AddToViewport();

    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(Widget->TakeWidget());
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);
    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);

    if (!Cast<UFishyBaseWidget>(Widget)) Widget->SetUserFocus(PC);
}

void AFishyCollectorCharacter::FermerWidget(APlayerController* PC)
{
    if (!PC) return;
    bUIWidgetOuvert = false;
    PC->SetInputMode(FInputModeGameOnly());
    PC->SetShowMouseCursor(false);
    PC->ResetIgnoreMoveInput();
    PC->ResetIgnoreLookInput();
}

void AFishyCollectorCharacter::RetourGeneral()
{
    if (!bUIWidgetOuvert) return;

    if (UFishyBaseWidget* W = GetWidgetOuvert())
    {
       if (W->GererRetour())
       {
          if (W == PopupActif)
          {
             PopupActif = nullptr;
             if (UFishyBaseWidget* Parent = GetWidgetOuvert()) Parent->InitialiserFocusGamepad();
          }
          return;
       }
    }

    if (MapWidget && MapWidget->IsInViewport()) { ToggleMap(); return; }
    if (PokedexWidget && PokedexWidget->IsInViewport()) {
       if (PokedexWidget->EstDetailVisible()) PokedexWidget->RetourListe();
       else TogglePokedex();
       return;
    }
    if (ShopWidget && ShopWidget->IsInViewport()) { ToggleShop(); return; }
    if (InventaireWidget && InventaireWidget->IsInViewport()) { ToggleInventaire(); return; }
    if (NearbyStorage) { NearbyStorage->CloseStorage(); return; }

    FermerWidget(Cast<APlayerController>(GetController()));
}

void AFishyCollectorCharacter::HandleEscape()
{
    if (PauseMenuWidget && PauseMenuWidget->IsInViewport()) { TogglePauseMenu(); return; }
    if (bUIWidgetOuvert) RetourGeneral();
    else TogglePauseMenu();
}

void AFishyCollectorCharacter::TogglePauseMenu()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PauseMenuWidgetClass) return;

    if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
    {
       PauseMenuWidget->RemoveFromParent();
       UGameplayStatics::SetGamePaused(GetWorld(), false);
       FermerWidget(PC); 
    }
    else
    {
       if (!PauseMenuWidget) PauseMenuWidget = CreateWidget<UUserWidget>(PC, PauseMenuWidgetClass);
       if (PauseMenuWidget) {
          OuvrirWidget(PauseMenuWidget, PC);
          UGameplayStatics::SetGamePaused(GetWorld(), true);
       }
    }
}

void AFishyCollectorCharacter::ToggleMap()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !MapWidgetClass) return;

    if (MapWidget && MapWidget->IsInViewport())
    {
       MapWidget->RemoveFromParent();
       FermerWidget(PC);
    }
    else
    {
       if (bUIWidgetOuvert) return;
       if (!MapWidget) MapWidget = CreateWidget<UUserWidget>(PC, MapWidgetClass);
       if (MapWidget) OuvrirWidget(MapWidget, PC);
    }
}