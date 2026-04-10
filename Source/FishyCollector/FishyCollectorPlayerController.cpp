// Copyright Epic Games, Inc. All Rights Reserved.


#include "FishyCollectorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "FishyCollector.h"
#include "Kismet/GameplayStatics.h"
#include "FishySaveGame.h"
#include "Animation/UMGSequencePlayer.h"
#include "Widgets/Input/SVirtualJoystick.h"

static const FString SaveSlot = TEXT("FishySlot");

void AFishyCollectorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		}
		else {
			UE_LOG(LogFishyCollector, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}


void AFishyCollectorPlayerController::TravelToLevel(const FName& LevelName)
{
	UGameplayStatics::OpenLevel(this, LevelName);
}

void AFishyCollectorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AFishyCollectorPlayerController::SaveGame(bool bRodPurchased)
{
	UFishySaveGame* Save = Cast<UFishySaveGame>(
		UGameplayStatics::CreateSaveGameObject(UFishySaveGame::StaticClass()));
	if (!Save) return;

	Save->bRodPurchased = bRodPurchased;

	UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0);
}

UFishySaveGame* AFishyCollectorPlayerController::LoadGame()
{
	return Cast<UFishySaveGame>(
		UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
}
