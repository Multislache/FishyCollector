// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexWidget.h"
#include "PokedexManager.h"
#include "PoissonTemplate.h"
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
#include "Engine/GameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"

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
	Helpers.Empty();

	int32 Index = 1;
	for (const FPokedexEntry& Entry : Manager->GetToutesLesEntrees())
	{
		if (!Entry.Poisson) continue;

		// Bouton carré
		UButton* Btn = WidgetTree->ConstructWidget<UButton>();

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

		// Numéro en dessous
		UTextBlock* TxtId = WidgetTree->ConstructWidget<UTextBlock>();
		TxtId->SetText(FText::FromString(FString::Printf(TEXT("#%03d"), Index++)));
		UVerticalBoxSlot* IdSlot = VBox->AddChildToVerticalBox(TxtId);
		IdSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		IdSlot->SetHorizontalAlignment(HAlign_Center);

		// Helper pour le clic
		UPokedexBoutonHelper* Helper = NewObject<UPokedexBoutonHelper>(this);
		Helper->Poisson = Entry.Poisson;
		Helper->Widget  = this;
		Helper->bPeche  = Entry.bPeche;
		Helpers.Add(Helper);
		Btn->OnClicked.AddDynamic(Helper, &UPokedexBoutonHelper::OnClique);

		// Taille fixe via SizeBox
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>();
		SizeBox->SetWidthOverride(100.f);
		SizeBox->SetHeightOverride(140.f);
		SizeBox->AddChild(Btn);

		int32 Position = Index - 2; // Index a déjà été incrémenté
		UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(ListePoissons->AddChildToUniformGrid(SizeBox, Position / 4, Position % 4));
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}
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
			// Toujours charger le mesh — matériau noir si non découvert
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

	SetDetailVisible(true);
}

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
	if (ScrollListe)         ScrollListe->SetVisibility(ListeVis);
}
