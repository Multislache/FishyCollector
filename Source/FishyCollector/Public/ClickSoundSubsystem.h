// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Framework/Application/IInputProcessor.h"
#include "ClickSoundSubsystem.generated.h"

class USoundBase;

// Intercepte tous les clics souris et joue le son si un SButton est sous le curseur
class FBoutonSonPreprocessor : public IInputProcessor
{
public:
	TWeakObjectPtr<UGameInstance> GameInstance;
	TWeakObjectPtr<USoundBase>    Son;

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
};

UCLASS()
class FISHYCOLLECTOR_API UClickSoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	TSharedPtr<FBoutonSonPreprocessor> Preprocessor;

	// UPROPERTY pour empêcher le GC de collecter le son
	UPROPERTY()
	USoundBase* SonClic = nullptr;
};
