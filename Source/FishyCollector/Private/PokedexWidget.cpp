// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexWidget.h"
#include "PokedexManager.h"
#include "PoissonTemplate.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UPokedexWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Rafraichir();
}

void UPokedexWidget::Rafraichir()
{
	if (!ListePoissons)
	{
		UE_LOG(LogTemp, Warning, TEXT("PokedexWidget: ListePoissons est null — vérifie que le ScrollBox est nommé exactement 'ListePoissons'"));
		return;
	}

	ListePoissons->ClearChildren();

	UPokedexManager* Manager = GetGameInstance()->GetSubsystem<UPokedexManager>();
	if (!Manager)
	{
		UE_LOG(LogTemp, Warning, TEXT("PokedexWidget: PokedexManager introuvable"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("PokedexWidget: %d entrées trouvées"), Manager->GetNombreTotal());

	for (const FPokedexEntry& Entry : Manager->GetToutesLesEntrees())
	{
		if (!Entry.Poisson) continue;

		// Conteneur horizontal par entrée
		UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();

		// Image du poisson
		UImage* Img = WidgetTree->ConstructWidget<UImage>();
		if (Entry.Poisson->Icone)
		{
			Img->SetBrushFromTexture(Entry.Poisson->Icone);
		}
		Img->SetColorAndOpacity(Entry.bPeche ? FLinearColor::White : FLinearColor::Black);

		UHorizontalBoxSlot* ImgSlot = HBox->AddChildToHorizontalBox(Img);
		ImgSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		// Nom du poisson
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>();
		Txt->SetText(Entry.bPeche ? Entry.Poisson->Nom : FText::FromString(TEXT("???")));

		UHorizontalBoxSlot* TxtSlot = HBox->AddChildToHorizontalBox(Txt);
		TxtSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		ListePoissons->AddChild(HBox);
	}
}
