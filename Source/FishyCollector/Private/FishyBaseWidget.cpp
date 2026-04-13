// Fill out your copyright notice in the Description page of Project Settings.

#include "FishyBaseWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "FishyCollectorCharacter.h"

void UFishyBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Indispensable pour que SetUserFocus() fonctionne sans warning.
	bIsFocusable = true;

	// Trouver automatiquement le premier bouton dans le WidgetTree et le désigner
	// comme cible de focus — fonctionne pour Canvas, VerticalBox, HorizontalBox, etc.
	if (WidgetTree)
	{
		UWidget* Premier = nullptr;
		WidgetTree->ForEachWidget([&](UWidget* W)
		{
			if (!Premier && W->GetIsEnabled() && Cast<UButton>(W))
				Premier = W;
		});
		if (Premier)
			SetDesiredFocusWidget(Premier);
	}

	if (IsInViewport() && GetWorld())
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFishyBaseWidget::AppliquerFocusInitial);
}

static void InjecterTouche(FKey Key)
{
	FKeyEvent Down(Key, FModifierKeysState(), 0, false, 0, 0);
	FKeyEvent Up(Key, FModifierKeysState(), 0, false, 0, 0);
	FSlateApplication::Get().ProcessKeyDownEvent(Down);
	FSlateApplication::Get().ProcessKeyUpEvent(Up);
}

void UFishyBaseWidget::InjecterToucheHaut()      { InjecterTouche(EKeys::Gamepad_DPad_Up); }
void UFishyBaseWidget::InjecterToucheBas()       { InjecterTouche(EKeys::Gamepad_DPad_Down); }
void UFishyBaseWidget::InjecterToucheGauche()    { InjecterTouche(EKeys::Gamepad_DPad_Left); }
void UFishyBaseWidget::InjecterToucheDroite()    { InjecterTouche(EKeys::Gamepad_DPad_Right); }
void UFishyBaseWidget::InjecterToucheAccepter()  { InjecterTouche(EKeys::Enter); }

UWidget* UFishyBaseWidget::TrouverPremierFocusable(UPanelWidget* Panel) const
{
	if (!Panel) return nullptr;

	for (int32 i = 0; i < Panel->GetChildrenCount(); i++)
	{
		UWidget* Enfant = Panel->GetChildAt(i);
		if (!Enfant) continue;

		// Si c'est directement focusable (Button…)
		if (Enfant->bIsVariable && Enfant->GetIsEnabled())
			if (Cast<UButton>(Enfant))
				return Enfant;

		// Sinon chercher récursivement dans les containers enfants
		if (UPanelWidget* SousPanel = Cast<UPanelWidget>(Enfant))
		{
			UWidget* Trouve = TrouverPremierFocusable(SousPanel);
			if (Trouve) return Trouve;
		}
	}
	return nullptr;
}

FReply UFishyBaseWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Retour = Gamepad_FaceButton_Bottom
	// Géré en PreviewKeyDown pour bloquer SButton avant qu'il reçoive l'event.
	if (InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Bottom)
	{
		// GererRetour d'abord (popup interne…), sinon toujours déléguer au character.
		// Double-exécution impossible : RetourGeneral() a le guard !bUIWidgetOuvert.
		if (!GererRetour())
		{
			if (AFishyCollectorCharacter* Char = Cast<AFishyCollectorCharacter>(GetOwningPlayerPawn()))
				Char->RetourGeneral();
		}
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UFishyBaseWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Sélection = Gamepad_FaceButton_Right
	// En mode autonome ou jeu en pause, Enhanced Input ne fire pas AccepterUI() :
	// injecter Enter directement pour activer le bouton en focus.
	// En mode normal (jeu actif), Enhanced Input gère AccepterUI() — ne pas double-injecter.
	if (InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right)
	{
		if (bGererInputDirectement || UGameplayStatics::IsGamePaused(GetWorld()))
		{
			InjecterToucheAccepter();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
