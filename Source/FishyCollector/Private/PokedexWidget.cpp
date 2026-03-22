// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexWidget.h"
#include "PokedexManager.h"
#include "PoissonTemplate.h"
#include "LieuTemplate.h"
#include "PokedexViewerActor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"

void UPokedexBoutonHelper::OnClique()
{
	if (Widget) Widget->AfficherDetail(Poisson, bPeche, Bouton);
}

void UPokedexWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BoutonRetour)
		BoutonRetour->OnClicked.AddDynamic(this, &UPokedexWidget::RetourListe);

	if (BoutonTriNumero)
		BoutonTriNumero->OnClicked.AddDynamic(this, &UPokedexWidget::TrierParNumero);

	if (BoutonTriRarete)
		BoutonTriRarete->OnClicked.AddDynamic(this, &UPokedexWidget::TrierParRarete);

	if (BoutonTriLieu)
		BoutonTriLieu->OnClicked.AddDynamic(this, &UPokedexWidget::TrierParLieu);

	Rafraichir();
}

void UPokedexWidget::Rafraichir()
{
	UPokedexManager* Manager = UPokedexManager::GetInstance(GetGameInstance());
	if (!Manager || !ListePoissons) return;

	// Sauvegarder l'ordre original (numéro)
	EntreesOriginales.Empty();
	for (const FPokedexEntry& Entry : Manager->GetToutesLesEntrees())
	{
		if (Entry.Poisson)
			EntreesOriginales.Add(Entry);
	}

	TriActuel = ETriPokedex::Numero;
	RemplirGrille(EntreesOriginales);
	MettreAJourBoutonsTri();
	RetourListe();
}

// ─── Tris ────────────────────────────────────────────────────────────────────

void UPokedexWidget::TrierParNumero()
{
	TriActuel = ETriPokedex::Numero;
	RemplirGrille(EntreesOriginales);
	MettreAJourBoutonsTri();
}

void UPokedexWidget::TrierParRarete()
{
	TriActuel = ETriPokedex::Rarete;

	TArray<FPokedexEntry> Tries = EntreesOriginales;
	Tries.Sort([](const FPokedexEntry& A, const FPokedexEntry& B)
	{
		return (uint8)A.Poisson->Rarete < (uint8)B.Poisson->Rarete;
	});

	RemplirGrille(Tries);
	MettreAJourBoutonsTri();
}

void UPokedexWidget::TrierParLieu()
{
	TriActuel = ETriPokedex::Lieu;

	TArray<FPokedexEntry> Tries = EntreesOriginales;
	Tries.Sort([this](const FPokedexEntry& A, const FPokedexEntry& B)
	{
		return TrouverLieu(A.Poisson).LexicalLess(TrouverLieu(B.Poisson));
	});

	RemplirGrille(Tries);
	MettreAJourBoutonsTri();
}

FName UPokedexWidget::TrouverLieu(UPoissonTemplate* Poisson) const
{
	if (!TableLieux || !Poisson) return NAME_None;

	for (const FName& RowName : TableLieux->GetRowNames())
	{
		FLieuRow* Row = TableLieux->FindRow<FLieuRow>(RowName, TEXT(""));
		if (Row && Row->Poissons.Contains(Poisson))
			return RowName;
	}
	return NAME_None;
}

// ─── Remplissage de la grille ─────────────────────────────────────────────────

static FLinearColor CouleurRarete(EPoissonRarete Rarete)
{
	switch (Rarete)
	{
	case EPoissonRarete::Rare:       return FLinearColor(0.55f, 0.75f, 1.0f);   // bleu clair
	case EPoissonRarete::Legendaire: return FLinearColor(1.0f,  0.85f, 0.25f);  // or
	default:                         return FLinearColor(0.88f, 0.88f, 0.88f);  // gris (Commun)
	}
}

void UPokedexWidget::RemplirGrille(const TArray<FPokedexEntry>& Entrees)
{
	if (!ListePoissons) return;

	ListePoissons->ClearChildren();
	Helpers.Empty();
	BoutonActif = nullptr;

	int32 Index = 1;
	for (const FPokedexEntry& Entry : Entrees)
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>();
		Btn->SetBackgroundColor(CouleurRarete(Entry.Poisson->Rarete));

		// VerticalBox : image en haut, numéro en bas
		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Btn->AddChild(VBox);

		// Icone (noire si non découvert)
		UImage* Img = WidgetTree->ConstructWidget<UImage>();
		if (Entry.Poisson->Icone)
			Img->SetBrushFromTexture(Entry.Poisson->Icone);
		Img->SetColorAndOpacity(Entry.bPeche ? FLinearColor::White : FLinearColor::Black);
		UVerticalBoxSlot* ImgSlot = VBox->AddChildToVerticalBox(Img);
		ImgSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ImgSlot->SetHorizontalAlignment(HAlign_Fill);

		// Numéro (position dans l'ordre original)
		int32 NumeroOriginal = EntreesOriginales.IndexOfByPredicate(
			[&](const FPokedexEntry& E){ return E.Poisson == Entry.Poisson; }) + 1;
		UTextBlock* TxtId = WidgetTree->ConstructWidget<UTextBlock>();
		TxtId->SetText(FText::FromString(FString::Printf(TEXT("#%03d"), NumeroOriginal)));
		UVerticalBoxSlot* IdSlot = VBox->AddChildToVerticalBox(TxtId);
		IdSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		IdSlot->SetHorizontalAlignment(HAlign_Center);

		// Helper pour le clic
		UPokedexBoutonHelper* Helper = NewObject<UPokedexBoutonHelper>(this);
		Helper->Poisson = Entry.Poisson;
		Helper->Widget  = this;
		Helper->bPeche  = Entry.bPeche;
		Helper->Bouton  = Btn;
		Helpers.Add(Helper);
		Btn->OnClicked.AddDynamic(Helper, &UPokedexBoutonHelper::OnClique);

		// Taille fixe via SizeBox
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>();
		SizeBox->SetWidthOverride(100.f);
		SizeBox->SetHeightOverride(140.f);
		SizeBox->AddChild(Btn);

		int32 Position = Index - 1;
		UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(
			ListePoissons->AddChildToUniformGrid(SizeBox, Position / 4, Position % 4));
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}
		Index++;
	}
}

// ─── Boutons de tri : surligner le tri actif ─────────────────────────────────

void UPokedexWidget::MettreAJourBoutonsTri()
{
	const FLinearColor Actif  = FLinearColor(0.2f, 0.6f, 1.0f);  // bleu = tri actif
	const FLinearColor Inactif = FLinearColor::White;

	if (BoutonTriNumero) BoutonTriNumero->SetBackgroundColor(TriActuel == ETriPokedex::Numero ? Actif : Inactif);
	if (BoutonTriRarete) BoutonTriRarete->SetBackgroundColor(TriActuel == ETriPokedex::Rarete ? Actif : Inactif);
	if (BoutonTriLieu)   BoutonTriLieu  ->SetBackgroundColor(TriActuel == ETriPokedex::Lieu   ? Actif : Inactif);
}

// ─── Détail ──────────────────────────────────────────────────────────────────

void UPokedexWidget::AfficherDetail(UPoissonTemplate* Poisson, bool bPeche, UButton* BoutonSource)
{
	if (!Poisson) return;

	if (NomPoisson)
		NomPoisson->SetText(bPeche ? Poisson->Nom : FText::FromString(TEXT("???")));

	if (DescriptionPoisson)
		DescriptionPoisson->SetText(bPeche ? Poisson->Description : FText::FromString(TEXT("???")));

	// Viewer 3D dans IconePoisson
	if (IconePoisson && GetWorld())
	{
		if (!ViewerActor)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ViewerActor = GetWorld()->SpawnActor<APokedexViewerActor>(
				APokedexViewerActor::StaticClass(),
				FVector(0.f, 0.f, -5000.f),
				FRotator::ZeroRotator,
				Params
			);
		}

		if (ViewerActor)
		{
			UStaticMesh* Mesh = Poisson->Mesh.LoadSynchronous();
			UMaterialInterface* Mat = bPeche ? Poisson->Materiau.LoadSynchronous() : MateriauSilhouette;
			ViewerActor->SetMesh(Mesh, Mat, bPeche);

			UTextureRenderTarget2D* RT = ViewerActor->GetRenderTarget();
			if (RT)
			{
				FSlateBrush Brush;
				Brush.SetResourceObject(RT);
				Brush.ImageSize = FVector2D(512.f, 512.f);
				Brush.DrawAs = ESlateBrushDrawType::Image;
				IconePoisson->SetBrush(Brush);
			}
		}
	}

	// Curseur de sélection : reset l'ancien, assombrit le nouveau
	SurlignerBouton(BoutonActif, false);
	BoutonActif = BoutonSource;
	SurlignerBouton(BoutonActif, true);

	SetDetailVisible(true);
}

void UPokedexWidget::SurlignerBouton(UButton* Bouton, bool bSurligne)
{
	if (!Bouton) return;

	for (UPokedexBoutonHelper* H : Helpers)
	{
		if (H && H->Bouton == Bouton && H->Poisson)
		{
			FLinearColor Couleur = CouleurRarete(H->Poisson->Rarete);
			// Sélectionné : assombrir la couleur de rareté (×0.5)
			if (bSurligne)
				Couleur = FLinearColor(Couleur.R * 0.5f, Couleur.G * 0.5f, Couleur.B * 0.5f, 1.0f);
			Bouton->SetBackgroundColor(Couleur);
			return;
		}
	}
}

// ─── Visibilité ───────────────────────────────────────────────────────────────

void UPokedexWidget::NativeDestruct()
{
	if (ViewerActor)
	{
		ViewerActor->Destroy();
		ViewerActor = nullptr;
	}
	Super::NativeDestruct();
}

FReply UPokedexWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ViewerActor)
	{
		bDragging = true;
		TSharedPtr<SWidget> Widget = GetCachedWidget();
		if (Widget.IsValid())
			return FReply::Handled().CaptureMouse(Widget.ToSharedRef());
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPokedexWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging && ViewerActor)
	{
		float DeltaYaw = InMouseEvent.GetCursorDelta().X * 0.5f;
		ViewerActor->AjouterRotationYaw(DeltaYaw);
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UPokedexWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UPokedexWidget::RetourListe()
{
	SurlignerBouton(BoutonActif, false);
	BoutonActif = nullptr;
	SetDetailVisible(false);
}

void UPokedexWidget::SetDetailVisible(bool bVisible)
{
	ESlateVisibility DetailVis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (IconePoisson)        IconePoisson->SetVisibility(DetailVis);
	if (NomPoisson)          NomPoisson->SetVisibility(DetailVis);
	if (DescriptionPoisson)  DescriptionPoisson->SetVisibility(DetailVis);
	if (BoutonRetour)        BoutonRetour->SetVisibility(DetailVis);
	// ScrollListe reste toujours visible
}
