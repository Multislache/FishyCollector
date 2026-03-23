// Fill out your copyright notice in the Description page of Project Settings.

#include "FishyBaseWidget.h"
#include "Framework/Application/SlateApplication.h"

void UFishyBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Différer d'une frame pour que Slate ait le temps de layouter le widget
	// avant d'appeler SetUserFocus (sinon le focus est ignoré silencieusement)
	if (GetWorld())
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

FReply UFishyBaseWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Consommer Gamepad_FaceButton_Bottom (bouton Jump/X PS5) avant qu'il atteigne
	// les boutons enfants. Seul AccepterUI() (injection Enter) peut valider le focus.
	if (InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Bottom)
		return FReply::Handled();

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}
