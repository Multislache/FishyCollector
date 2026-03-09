// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexWidget.h"
#include "PokedexManager.h"
#include "PoissonTemplate.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/GameInstance.h"

void UPokedexBoutonHelper::OnClique()
{
	if (Widget) Widget->AfficherDetail(Poisson, bPeche);
}

void UPokedexWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BoutonRetour)
		BoutonRetour->OnClicked.AddDynamic(this, &UPokedexWidget::RetourListe);

	Rafraichir();
}

void UPokedexWidget::Rafraichir()
{
	UPokedexManager* Manager = UPokedexManager::GetInstance(GetGameInstance());
	if (!Manager || !ListePoissons) return;

	ListePoissons->ClearChildren();

	int32 Index = 1;
	for (const FPokedexEntry& Entry : Manager->GetToutesLesEntrees())
	{
		if (!Entry.Poisson) continue;

		// Bouton contenant toute la ligne
		UButton* Btn = WidgetTree->ConstructWidget<UButton>();
		UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();
		Btn->AddChild(HBox);

		// ID (#001, #002...)
		UTextBlock* TxtId = WidgetTree->ConstructWidget<UTextBlock>();
		TxtId->SetText(FText::FromString(FString::Printf(TEXT("#%03d"), Index++)));
		UHorizontalBoxSlot* IdSlot = HBox->AddChildToHorizontalBox(TxtId);
		IdSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		// Icone (noire si non découvert)
		UImage* Img = WidgetTree->ConstructWidget<UImage>();
		if (Entry.Poisson->Icone)
			Img->SetBrushFromTexture(Entry.Poisson->Icone);
		Img->SetColorAndOpacity(Entry.bPeche ? FLinearColor::White : FLinearColor::Black);
		UHorizontalBoxSlot* ImgSlot = HBox->AddChildToHorizontalBox(Img);
		ImgSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		// Nom ("???" si non découvert)
		UTextBlock* TxtNom = WidgetTree->ConstructWidget<UTextBlock>();
		TxtNom->SetText(Entry.bPeche ? Entry.Poisson->Nom : FText::FromString(TEXT("???")));
		UHorizontalBoxSlot* NomSlot = HBox->AddChildToHorizontalBox(TxtNom);
		NomSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Helper pour le clic
		UPokedexBoutonHelper* Helper = NewObject<UPokedexBoutonHelper>(this);
		Helper->Poisson = Entry.Poisson;
		Helper->Widget  = this;
		Helper->bPeche  = Entry.bPeche;
		Btn->OnClicked.AddDynamic(Helper, &UPokedexBoutonHelper::OnClique);

		ListePoissons->AddChild(Btn);
	}

	RetourListe();
}

void UPokedexWidget::AfficherDetail(UPoissonTemplate* Poisson, bool bPeche)
{
	if (!Poisson) return;

	if (NomPoisson)
		NomPoisson->SetText(bPeche ? Poisson->Nom : FText::FromString(TEXT("???")));

	if (DescriptionPoisson)
		DescriptionPoisson->SetText(bPeche ? Poisson->Description : FText::FromString(TEXT("???")));

	if (IconePoisson)
	{
		if (Poisson->Icone)
			IconePoisson->SetBrushFromTexture(Poisson->Icone);
		IconePoisson->SetColorAndOpacity(bPeche ? FLinearColor::White : FLinearColor::Black);
	}

	SetDetailVisible(true);
}

void UPokedexWidget::RetourListe()
{
	SetDetailVisible(false);
}

void UPokedexWidget::SetDetailVisible(bool bVisible)
{
	ESlateVisibility DetailVis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	ESlateVisibility ListeVis  = bVisible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;

	if (IconePoisson)        IconePoisson->SetVisibility(DetailVis);
	if (NomPoisson)          NomPoisson->SetVisibility(DetailVis);
	if (DescriptionPoisson)  DescriptionPoisson->SetVisibility(DetailVis);
	if (BoutonRetour)        BoutonRetour->SetVisibility(DetailVis);
	if (ListePoissons)       ListePoissons->SetVisibility(ListeVis);
}
