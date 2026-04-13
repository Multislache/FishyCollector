// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexWidget.h"
#include "PokedexManager.h"
#include "PoissonTemplate.h"
#include "LieuTemplate.h"
#include "PokedexViewerActor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Border.h"
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

	if (Bord)
	{
		FSlateBrush BrushVide;
		BrushVide.DrawAs = ESlateBrushDrawType::NoDrawType;
		Bord->SetBrush(BrushVide);
	}

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

	TArray<FName> ClesGroupe;
	ClesGroupe.Reserve(Tries.Num());
	for (const FPokedexEntry& Entry : Tries)
		ClesGroupe.Add(FName(*UEnum::GetValueAsString(Entry.Poisson->Rarete)));

	RemplirGrille(Tries, ClesGroupe);
	MettreAJourBoutonsTri();
}

void UPokedexWidget::TrierParLieu()
{
	TriActuel = ETriPokedex::Lieu;

	// Construire l'index de position de chaque lieu dans la DataTable (pour respecter l'ordre défini)
	TMap<FName, int32> OrdreLieux;
	if (TableLieux)
	{
		const TArray<FName>& RowNames = TableLieux->GetRowNames();
		for (int32 i = 0; i < RowNames.Num(); i++)
			OrdreLieux.Add(RowNames[i], i);
	}

	TArray<FPokedexEntry> Tries = EntreesOriginales;
	Tries.Sort([this, &OrdreLieux](const FPokedexEntry& A, const FPokedexEntry& B)
	{
		const FName LieuA = TrouverLieu(A.Poisson);
		const FName LieuB = TrouverLieu(B.Poisson);
		// Non assignés toujours à la fin
		if (LieuA == NAME_None && LieuB != NAME_None) return false;
		if (LieuA != NAME_None && LieuB == NAME_None) return true;
		if (LieuA == LieuB) return false;
		return OrdreLieux.FindRef(LieuA) < OrdreLieux.FindRef(LieuB);
	});

	// Calculer la clé de groupe (lieu) pour chaque entrée triée
	TArray<FName> ClesGroupe;
	ClesGroupe.Reserve(Tries.Num());
	for (const FPokedexEntry& Entry : Tries)
		ClesGroupe.Add(TrouverLieu(Entry.Poisson));

	RemplirGrille(Tries, ClesGroupe);
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
	case EPoissonRarete::Rare:       return FLinearColor(0.25f, 0.45f, 0.75f);   // bleu
	case EPoissonRarete::Legendaire: return FLinearColor(0.75f, 0.55f, 0.05f);  // or foncé
	case EPoissonRarete::Secret:     return FLinearColor(0.45f, 0.10f, 0.65f);  // violet
	default:                         return FLinearColor(0.45f, 0.45f, 0.45f);  // gris foncé
	}
}

void UPokedexWidget::RemplirGrille(const TArray<FPokedexEntry>& Entrees, const TArray<FName>& ClesGroupe)
{
	if (!ListePoissons) return;

	ListePoissons->ClearChildren();
	ListePoissons->SetSlotPadding(FMargin(4.f));
	Helpers.Empty();
	BoutonActif = nullptr;
	BoutonEnFocus = nullptr;
	PremierBouton = nullptr;

	int32 Index = 1;
	int32 Ligne   = 0;
	int32 Colonne = 0;
	for (const FPokedexEntry& Entry : Entrees)
	{
		// Nouveau groupe de lieu : forcer un retour à la ligne si on n'est pas déjà en début de ligne
		const bool bGroupeActif = ClesGroupe.IsValidIndex(Index - 1);
		if (bGroupeActif && Index > 1 && Colonne != 0 && ClesGroupe[Index - 1] != ClesGroupe[Index - 2])
		{
			Ligne++;
			Colonne = 0;
		}
		UButton* Btn = WidgetTree->ConstructWidget<UButton>();

		// Brush arrondi pour tous les états du bouton
		auto MakeBrush = [this](FLinearColor Couleur) -> FSlateBrush
		{
			FSlateBrush B;
			B.DrawAs = ESlateBrushDrawType::RoundedBox;
			B.OutlineSettings.CornerRadii = FVector4(RayonCarte, RayonCarte, RayonCarte, RayonCarte);
			B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			B.TintColor = FSlateColor(Couleur);
			return B;
		};
		FLinearColor C = CouleurRarete(Entry.Poisson->Rarete);
		FButtonStyle Style = Btn->GetStyle();
		Style.Normal  = MakeBrush(C);
		Style.Hovered = MakeBrush(C * 1.2f);
		Style.Pressed = MakeBrush(C * 0.7f);
		Btn->SetStyle(Style);

		// VerticalBox : image en haut, numéro en bas
		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Btn->AddChild(VBox);

		// Icone (noire si non découvert)
		UImage* Img = WidgetTree->ConstructWidget<UImage>();
		if (Entry.Poisson->Icone)
		{
			FSlateBrush ImgBrush;
			ImgBrush.SetResourceObject(Entry.Poisson->Icone);
			ImgBrush.ImageSize = FVector2D(90.f, 90.f);
			Img->SetBrush(ImgBrush);
		}
		Img->SetColorAndOpacity(Entry.bPeche ? FLinearColor::White : FLinearColor::Black);
		UVerticalBoxSlot* ImgSlot = VBox->AddChildToVerticalBox(Img);
		ImgSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		ImgSlot->SetHorizontalAlignment(HAlign_Center);
		ImgSlot->SetVerticalAlignment(VAlign_Center);

		// Numéro (position dans l'ordre original)
		int32 NumeroOriginal = EntreesOriginales.IndexOfByPredicate(
			[&](const FPokedexEntry& E){ return E.Poisson == Entry.Poisson; }) + 1;
		UTextBlock* TxtId = WidgetTree->ConstructWidget<UTextBlock>();
		TxtId->SetText(FText::FromString(FString::Printf(TEXT("#%03d"), NumeroOriginal)));
		UVerticalBoxSlot* IdSlot = VBox->AddChildToVerticalBox(TxtId);
		IdSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		IdSlot->SetHorizontalAlignment(HAlign_Center);

		// Mémoriser le premier bouton pour la navigation gamepad
		if (Index == 1)
			PremierBouton = Btn;

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
		SizeBox->SetHeightOverride(180.f);
		SizeBox->AddChild(Btn);

		UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(
			ListePoissons->AddChildToUniformGrid(SizeBox, Ligne, Colonne));
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}

		Colonne++;
		if (Colonne >= 4)
		{
			Colonne = 0;
			Ligne++;
		}
		Index++;
	}

	// Après un rebuild de grille (ex. changement de tri), redonner le focus au premier bouton
	// pour que le joystick continue de fonctionner.
	if (IsInViewport())
		FocuserPremierBouton();
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
			TSubclassOf<APokedexViewerActor> ClasseASpawner = ViewerActorClass
				? ViewerActorClass
				: TSubclassOf<APokedexViewerActor>(APokedexViewerActor::StaticClass());
			ViewerActor = GetWorld()->SpawnActor<APokedexViewerActor>(
				ClasseASpawner,
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
				Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
				Brush.OutlineSettings.CornerRadii = FVector4(RayonArrondissement, RayonArrondissement, RayonArrondissement, RayonArrondissement);
				Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
				IconePoisson->SetBrush(Brush);
			}
		}
	}

	// Retirer le curseur de navigation avant d'entrer dans le détail
	SurlignerFocusNavigation(nullptr);

	// Curseur de sélection : reset l'ancien, assombrit le nouveau
	SurlignerBouton(BoutonActif, false);
	BoutonActif = BoutonSource;
	SurlignerBouton(BoutonActif, true);

	SetDetailVisible(true);

	// Focus gamepad sur le bouton Retour pour permettre de revenir à la liste avec X
	if (BoutonRetour)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
			BoutonRetour->SetUserFocus(PC);
		else
			BoutonRetour->SetKeyboardFocus();
	}
}

void UPokedexWidget::SurlignerBouton(UButton* Bouton, bool bSurligne)
{
	if (!Bouton) return;

	for (UPokedexBoutonHelper* H : Helpers)
	{
		if (H && H->Bouton == Bouton && H->Poisson)
		{
			FLinearColor Couleur = CouleurRarete(H->Poisson->Rarete);
			if (bSurligne)
				Couleur = FLinearColor(Couleur.R * 0.5f, Couleur.G * 0.5f, Couleur.B * 0.5f, 1.0f);

			FSlateBrush B;
			B.DrawAs = ESlateBrushDrawType::RoundedBox;
			B.OutlineSettings.CornerRadii = FVector4(RayonCarte, RayonCarte, RayonCarte, RayonCarte);
			B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			B.OutlineSettings.Color = bSurligne ? FLinearColor(0.1f, 0.55f, 1.0f, 1.0f) : FLinearColor::Transparent;
			B.OutlineSettings.Width = bSurligne ? 4.f : 0.f;
			B.TintColor = FSlateColor(Couleur);
			FButtonStyle Style = Bouton->GetStyle();
			Style.Normal = B;
			Bouton->SetStyle(Style);
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
	// Remettre le focus gamepad sur le dernier bouton sélectionné (ou le premier si aucun)
	UButton* BoutonARefocus = BoutonActif ? BoutonActif : PremierBouton;
	if (BoutonARefocus)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
			BoutonARefocus->SetUserFocus(PC);
		else
			BoutonARefocus->SetKeyboardFocus();
	}

	SurlignerBouton(BoutonActif, false);
	BoutonActif = nullptr;
	SetDetailVisible(false);
}

void UPokedexWidget::InitialiserFocusGamepad_Implementation()
{
	FocuserPremierBouton();
}

void UPokedexWidget::NaviguerGauche_Implementation()
{
	if (bDetailVisible)
	{
		const float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
		RoterModele(-90.f * DeltaTime);
	}
	else
	{
		NaviguerTriGauche();
	}
}

void UPokedexWidget::NaviguerDroite_Implementation()
{
	if (bDetailVisible)
	{
		const float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
		RoterModele(90.f * DeltaTime);
	}
	else
	{
		NaviguerTriDroite();
	}
}

void UPokedexWidget::FocuserPremierBouton()
{
	if (!PremierBouton) return;

	APlayerController* PC = GetOwningPlayer();
	if (PC)
		PremierBouton->SetUserFocus(PC);
	else
		PremierBouton->SetKeyboardFocus();
}

void UPokedexWidget::NaviguerTriGauche()
{
	// Debounce : ignorer si le dernier changement est trop récent
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now - DernierChangementTri < 0.25f) return;
	DernierChangementTri = Now;

	// Numero ← Lieu ← Rarete ← Numero (cycle vers la gauche)
	switch (TriActuel)
	{
	case ETriPokedex::Numero: TrierParLieu();    break;
	case ETriPokedex::Rarete: TrierParNumero();  break;
	case ETriPokedex::Lieu:   TrierParRarete();  break;
	}
}

void UPokedexWidget::NaviguerTriDroite()
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now - DernierChangementTri < 0.25f) return;
	DernierChangementTri = Now;

	// Numero → Rarete → Lieu → Numero (cycle vers la droite)
	switch (TriActuel)
	{
	case ETriPokedex::Numero: TrierParRarete(); break;
	case ETriPokedex::Rarete: TrierParLieu();   break;
	case ETriPokedex::Lieu:   TrierParNumero(); break;
	}
}

void UPokedexWidget::RoterModele(float DeltaYaw)
{
	if (ViewerActor && bDetailVisible)
		ViewerActor->AjouterRotationYaw(DeltaYaw);
}

void UPokedexWidget::SurlignerFocusNavigation(UButton* NouveauFocus)
{
	// Retirer le highlight de l'ancien bouton en focus
	if (BoutonEnFocus && BoutonEnFocus != NouveauFocus)
	{
		for (UPokedexBoutonHelper* H : Helpers)
		{
			if (!H || H->Bouton != BoutonEnFocus || !H->Poisson) continue;
			FLinearColor C = CouleurRarete(H->Poisson->Rarete);
			FSlateBrush B;
			B.DrawAs = ESlateBrushDrawType::RoundedBox;
			B.OutlineSettings.CornerRadii = FVector4(RayonCarte, RayonCarte, RayonCarte, RayonCarte);
			B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			B.OutlineSettings.Color = FLinearColor::Transparent;
			B.OutlineSettings.Width = 0.f;
			B.TintColor = FSlateColor(C);
			FButtonStyle Style = BoutonEnFocus->GetStyle();
			Style.Normal = B;
			BoutonEnFocus->SetStyle(Style);
			break;
		}
	}

	BoutonEnFocus = NouveauFocus;

	// Appliquer le highlight bleu vif sur le nouveau bouton en focus
	if (BoutonEnFocus)
	{
		for (UPokedexBoutonHelper* H : Helpers)
		{
			if (!H || H->Bouton != BoutonEnFocus || !H->Poisson) continue;
			FLinearColor C = CouleurRarete(H->Poisson->Rarete);
			FSlateBrush B;
			B.DrawAs = ESlateBrushDrawType::RoundedBox;
			B.OutlineSettings.CornerRadii = FVector4(RayonCarte, RayonCarte, RayonCarte, RayonCarte);
			B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			B.OutlineSettings.Color = FLinearColor(0.1f, 0.65f, 1.0f, 1.0f);
			B.OutlineSettings.Width = 4.f;
			B.TintColor = FSlateColor(C);
			FButtonStyle Style = BoutonEnFocus->GetStyle();
			Style.Normal = B;
			BoutonEnFocus->SetStyle(Style);
			break;
		}
	}
}

void UPokedexWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bDetailVisible) return;

	// Trouver quel bouton a le focus gamepad cette frame
	UButton* NouveauFocus = nullptr;
	for (UPokedexBoutonHelper* H : Helpers)
	{
		if (H && H->Bouton && H->Bouton->HasAnyUserFocus())
		{
			NouveauFocus = H->Bouton;
			break;
		}
	}

	// Mettre à jour le highlight et le scroll seulement si le focus a changé
	if (NouveauFocus != BoutonEnFocus)
	{
		SurlignerFocusNavigation(NouveauFocus);
		if (ScrollListe && NouveauFocus)
			ScrollListe->ScrollWidgetIntoView(NouveauFocus, true);
	}
}

void UPokedexWidget::ScrollerVersFocus()
{
	// Géré par NativeTick — rien à faire ici
}

void UPokedexWidget::SetDetailVisible(bool bVisible)
{
	bDetailVisible = bVisible;
	ESlateVisibility DetailVis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (Bord)            Bord->SetVisibility(DetailVis);
	if (IconePoisson)    IconePoisson->SetVisibility(DetailVis);
	if (NomText)             NomText->SetVisibility(DetailVis);
	if (NomPoisson)          NomPoisson->SetVisibility(DetailVis);
	if (DescriptionText)     DescriptionText->SetVisibility(DetailVis);
	if (DescriptionPoisson)  DescriptionPoisson->SetVisibility(DetailVis);
	if (BoutonRetour)        BoutonRetour->SetVisibility(DetailVis);
	// ScrollListe reste toujours visible
}
