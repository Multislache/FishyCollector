// Fill out your copyright notice in the Description page of Project Settings.

#include "ClickSoundSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Kismet/GameplayStatics.h"

// ─── Preprocessor ────────────────────────────────────────────────────────────

bool FBoutonSonPreprocessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		return false;

	if (!Son.IsValid() || !GameInstance.IsValid())
		return false;

	// Récupérer tous les widgets sous le curseur
	TArray<TSharedRef<SWindow>> Windows;
	FSlateApplication::Get().GetAllVisibleWindowsOrdered(Windows);

	FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(
		MouseEvent.GetScreenSpacePosition(), Windows);

	// Jouer le son si un SButton est dans la hiérarchie
	for (int32 i = WidgetPath.Widgets.Num() - 1; i >= 0; --i)
	{
		if (WidgetPath.Widgets[i].Widget->GetType() == FName("SButton"))
		{
			UGameplayStatics::PlaySound2D(GameInstance.Get(), Son.Get());
			break;
		}
	}

	return false; // Ne pas consommer l'événement
}

// ─── Subsystem ────────────────────────────────────────────────────────────────

void UClickSoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!FSlateApplication::IsInitialized()) return;

	// Charger et garder le son en mémoire (UPROPERTY protège du GC)
	TSoftObjectPtr<USoundBase> SoftSon(FSoftObjectPath(
		TEXT("/Game/Asset/Sound/musheran-beep-313342.musheran-beep-313342")));
	SonClic = SoftSon.LoadSynchronous();

	if (!SonClic)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClickSoundSubsystem : son introuvable"));
		return;
	}

	Preprocessor = MakeShared<FBoutonSonPreprocessor>();
	Preprocessor->Son          = SonClic;
	Preprocessor->GameInstance = GetGameInstance();

	FSlateApplication::Get().RegisterInputPreProcessor(Preprocessor);
}

void UClickSoundSubsystem::Deinitialize()
{
	if (Preprocessor.IsValid() && FSlateApplication::IsInitialized())
		FSlateApplication::Get().UnregisterInputPreProcessor(Preprocessor);

	Preprocessor.Reset();
	Super::Deinitialize();
}
