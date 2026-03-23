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
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AFishyCollectorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping – bloqué si le pokédex est ouvert
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::JumpSiPokedexFerme);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::Look);

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::Interact);
		}

		if (PokedexAction)
		{
			EnhancedInputComponent->BindAction(PokedexAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::TogglePokedex);
		}

		if (ResetPokedexAction)
		{
			EnhancedInputComponent->BindAction(ResetPokedexAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ResetPokedex);
		}
		if (ClickAction)
		{
			EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ClickInteractionManager);
		}

		// Rotation du modèle 3D pokédex via L1/R1
		if (PokedexRotateLeftAction)
		{
			EnhancedInputComponent->BindAction(PokedexRotateLeftAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::PokedexRoterGauche);
		}
		if (PokedexRotateRightAction)
		{
			EnhancedInputComponent->BindAction(PokedexRotateRightAction, ETriggerEvent::Triggered, this, &AFishyCollectorCharacter::PokedexRoterDroite);
		}

		// IA_Retour – fermer le widget actuellement ouvert (pokédex, shop, inventaire…)
		if (RetourAction)
		{
			EnhancedInputComponent->BindAction(RetourAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::RetourGeneral);
		}

		// I / bouton manette – inventaire
		if (InventaireAction)
		{
			EnhancedInputComponent->BindAction(InventaireAction, ETriggerEvent::Started, this, &AFishyCollectorCharacter::ToggleInventaire);
		}
	}
	else
	{
		UE_LOG(LogFishyCollector, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AFishyCollectorCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AFishyCollectorCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AFishyCollectorCharacter::DoMove(float Right, float Forward)
{
	// Quand un widget UI est ouvert, le joystick navigue dans la grille
	if (bUIWidgetOuvert)
	{
		NaviguerUI(Right, Forward);
		return;
	}

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AFishyCollectorCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AFishyCollectorCharacter::DoJumpStart()
{
	Jump();
}

void AFishyCollectorCharacter::DoJumpEnd()
{
	StopJumping();
}

void AFishyCollectorCharacter::Interact()
{
	// Quand un widget UI est ouvert, cliquer le bouton en focus
	if (bUIWidgetOuvert)
	{
		AccepterUI();
		return;
	}

	if (FishingRod && FishingRod->GetCurrentState() == EFishingRodState::Morsure)
	{
		FishingRod->HandleInput();
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
	if (FishingRod && FishingRod->GetCurrentState() == EFishingRodState::Morsure)
	{
		FishingRod->HandleInput();
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

void AFishyCollectorCharacter::SetFishingZoneActive(bool bActive)
{
	bIsInFishingZone = bActive;
}

void AFishyCollectorCharacter::SetShopZoneActive(bool bActive)
{
	bIsInShopZone = bActive;
}

void AFishyCollectorCharacter::ResetPokedex()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UPokedexManager* Manager = GI->GetSubsystem<UPokedexManager>();
	if (Manager)
	{
		Manager->ResetPokedex();
	}
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
		if (!PokedexWidgetClass) return;

		if (!PokedexWidget)
		{
			PokedexWidget = CreateWidget<UPokedexWidget>(PC, PokedexWidgetClass);
		}
		else
		{
			PokedexWidget->Rafraichir();
		}

		if (PokedexWidget)
		{
			OuvrirWidget(PokedexWidget, PC);
		}
	}
}

void AFishyCollectorCharacter::ToggleShop()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	if (!ShopWidgetClass) return;

	if (ShopWidget && ShopWidget->IsInViewport())
	{
		ShopWidget->RemoveFromParent();
		FermerWidget(PC);
	}
	else
	{
		if (!ShopWidget)
		{
			ShopWidget = CreateWidget<UUserWidget>(PC, ShopWidgetClass);
		}

		if (ShopWidget)
		{
			OuvrirWidget(ShopWidget, PC);
		}
	}
}

void AFishyCollectorCharacter::SetNearbyStorage(AFishingRodStorage* Storage)
{
	NearbyStorage = Storage;
}
/*
void AFishyCollectorCharacter::EquipRod(TSubclassOf<AFishingRod> NewRodClass)
{
	if (!NewRodClass) return;

	if (FishingRod)
	{
		FishingRod->DetachFromCharacter();
		FishingRod->Destroy();
		FishingRod = nullptr;
	}
	FishingRod = GetWorld()->SpawnActor<AFishingRod>(NewRodClass);
	if (FishingRod)
	{
		FishingRod->AttachToCharacter(this);
	}
}*/

void AFishyCollectorCharacter::EquipRodFromData(UFishingRodData* RodData)
{
	if (!RodData || !RodData->RodClass) return;

	if (FishingRod)
	{
		FishingRod->DetachFromCharacter();
		FishingRod->Destroy();
		FishingRod = nullptr;
	}
	FishingRod = GetWorld()->SpawnActor<AFishingRod>(RodData->RodClass);
	if (FishingRod)
	{
		FishingRod->AttachToCharacter(this);
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
	if (!FishingRod) return nullptr;
	return FishingRod->GetClass();
}

void AFishyCollectorCharacter::JumpSiPokedexFerme()
{
	if (bUIWidgetOuvert) return;
	Jump();
}

UFishyBaseWidget* AFishyCollectorCharacter::GetWidgetOuvert() const
{
	// Le popup a priorité : il est "au-dessus" du widget parent
	if (PopupActif && PopupActif->IsInViewport()) return PopupActif;
	if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(PokedexWidget);   W && W->IsInViewport()) return W;
	if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(ShopWidget);      W && W->IsInViewport()) return W;
	if (UFishyBaseWidget* W = Cast<UFishyBaseWidget>(InventaireWidget);W && W->IsInViewport()) return W;
	return nullptr;
}

void AFishyCollectorCharacter::SetPopupActif(UFishyBaseWidget* Popup)
{
	PopupActif = Popup;
}

void AFishyCollectorCharacter::PokedexRoterGauche()
{
	if (!bUIWidgetOuvert) return;
	if (UFishyBaseWidget* Widget = GetWidgetOuvert())
		Widget->NaviguerGauche();
}

void AFishyCollectorCharacter::PokedexRoterDroite()
{
	if (!bUIWidgetOuvert) return;
	if (UFishyBaseWidget* Widget = GetWidgetOuvert())
		Widget->NaviguerDroite();
}

void AFishyCollectorCharacter::ToggleInventaire()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	if (!InventaireWidgetClass) return;

	// Inventaire déjà ouvert → fermer
	if (bUIWidgetOuvert && InventaireWidget)
	{
		InventaireWidget->RemoveFromParent();
		InventaireWidget = nullptr;
		FermerWidget(PC);
		return;
	}

	// Tout autre widget ouvert → ne rien faire
	if (bUIWidgetOuvert) return;

	// Toujours recréer pour éviter les doublons
	InventaireWidget = CreateWidget<UUserWidget>(PC, InventaireWidgetClass);

	if (InventaireWidget)
	{
		OuvrirWidget(InventaireWidget, PC);
	}
}

void AFishyCollectorCharacter::NaviguerUI(float X, float Y)
{
	// Popup ouvert → ne pas naviguer dans le widget parent derrière
	if (PopupActif && PopupActif->IsInViewport()) return;

	constexpr float Seuil = 0.5f;

	// Joystick au centre → reset debounce pour réactivité immédiate au prochain push
	if (FMath::Abs(X) < Seuil && FMath::Abs(Y) < Seuil)
	{
		DernierNavigationUI = 0.f;
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	constexpr float Debounce = 0.18f;
	if (Now - DernierNavigationUI < Debounce) return;
	DernierNavigationUI = Now;

	// Direction dominante → événement D-pad correspondant injecté dans Slate
	FKey NavKey;
	if (FMath::Abs(X) >= FMath::Abs(Y))
		NavKey = (X > 0.f) ? EKeys::Gamepad_DPad_Right : EKeys::Gamepad_DPad_Left;
	else
		NavKey = (Y > 0.f) ? EKeys::Gamepad_DPad_Up : EKeys::Gamepad_DPad_Down;

	FKeyEvent Down(NavKey, FModifierKeysState(), 0, false, 0, 0);
	FKeyEvent Up  (NavKey, FModifierKeysState(), 0, false, 0, 0);
	FSlateApplication::Get().ProcessKeyDownEvent(Down);
	FSlateApplication::Get().ProcessKeyUpEvent(Up);
}

void AFishyCollectorCharacter::AccepterUI()
{
	// Popup ouvert → ne pas cliquer sur le widget parent derrière
	if (PopupActif && PopupActif->IsInViewport()) return;

	// Injecter Enter (pas Gamepad_FaceButton_Bottom qui est bloqué par NativeOnPreviewKeyDown)
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

	if (!Cast<UFishyBaseWidget>(Widget))
		Widget->SetUserFocus(PC);
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

void AFishyCollectorCharacter::FermerInventaire()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (InventaireWidget && InventaireWidget->IsInViewport())
	{
		InventaireWidget->RemoveFromParent();
		InventaireWidget = nullptr;
	}
	FermerWidget(PC);
}

void AFishyCollectorCharacter::RetourGeneral()
{
	if (!bUIWidgetOuvert) return;

	// Laisser le widget gérer le retour lui-même (ex : fermer un popup interne)
	if (UFishyBaseWidget* W = GetWidgetOuvert())
	{
		if (W->GererRetour())
		{
			// Si c'était un popup, l'effacer et remettre le focus sur le widget parent
			if (W == PopupActif)
			{
				PopupActif = nullptr;
				if (UFishyBaseWidget* Parent = GetWidgetOuvert())
					Parent->InitialiserFocusGamepad();
			}
			return;
		}
	}

	// Pokédex : détail → liste, liste → fermer
	if (PokedexWidget && PokedexWidget->IsInViewport())
	{
		if (PokedexWidget->EstDetailVisible())
			PokedexWidget->RetourListe();
		else
			TogglePokedex();
		return;
	}

	// Shop
	if (ShopWidget && ShopWidget->IsInViewport())
	{
		ToggleShop();
		return;
	}

	// Inventaire
	if (InventaireWidget && InventaireWidget->IsInViewport())
	{
		ToggleInventaire();
		return;
	}

	// Storage
	if (NearbyStorage)
	{
		NearbyStorage->CloseStorage();
		return;
	}

	// Fallback : widget inconnu ouvert depuis Blueprint → reset propre
	APlayerController* PC = Cast<APlayerController>(GetController());
	FermerWidget(PC);
}

