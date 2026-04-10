// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FishyBaseWidget.h"
#include "PokedexManager.h"
#include "PoissonTemplate.h"
#include "PokedexWidget.generated.h"

class UUniformGridPanel;
class UTextBlock;
class UImage;
class UButton;
class UPokedexWidget;
class APokedexViewerActor;
class UMaterialInterface;
class UDataTable;

UENUM(BlueprintType)
enum class ETriPokedex : uint8
{
	Numero    UMETA(DisplayName = "Numéro"),
	Rarete    UMETA(DisplayName = "Rareté"),
	Lieu      UMETA(DisplayName = "Lieu"),
};

UCLASS()
class UPokedexBoutonHelper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UPoissonTemplate* Poisson = nullptr;

	UPROPERTY()
	UPokedexWidget* Widget = nullptr;

	UPROPERTY()
	UButton* Bouton = nullptr;

	bool bPeche = false;

	UFUNCTION()
	void OnClique();
};

UCLASS()
class FISHYCOLLECTOR_API UPokedexWidget : public UFishyBaseWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void Rafraichir();

	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void AfficherDetail(UPoissonTemplate* Poisson, bool bPeche, UButton* BoutonSource = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void RetourListe();

	// Rotation du modèle 3D (appelé par la manette via L1/R1)
	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void RoterModele(float DeltaYaw);

	bool EstDetailVisible() const { return bDetailVisible; }

	virtual void InitialiserFocusGamepad_Implementation() override;
	virtual void NaviguerGauche_Implementation() override;
	virtual void NaviguerDroite_Implementation() override;

	void FocuserPremierBouton();

	// Navigation L1/R1 entre les onglets de tri (vue liste uniquement)
	void NaviguerTriGauche();
	void NaviguerTriDroite();

	// DataTable des lieux assigné dans le Blueprint (pour le tri par lieu)
	UPROPERTY(EditAnywhere, Category = "Pokedex")
	UDataTable* TableLieux;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Page Liste
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollListe;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* ListePoissons;

	// Boutons de tri
	UPROPERTY(meta = (BindWidget))
	UButton* BoutonTriNumero;

	UPROPERTY(meta = (BindWidget))
	UButton* BoutonTriRarete;

	UPROPERTY(meta = (BindWidget))
	UButton* BoutonTriLieu;

	// Page Détail
	UPROPERTY(meta = (BindWidget))
	UImage* IconePoisson;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NomText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NomPoisson;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionPoisson;

	UPROPERTY(meta = (BindWidget))
	UButton* BoutonRetour;

	// Matériau noir assigné dans le Blueprint pour les poissons non découverts
	UPROPERTY(EditAnywhere, Category = "Pokedex")
	UMaterialInterface* MateriauSilhouette;

private:
	UFUNCTION()
	void TrierParNumero();

	UFUNCTION()
	void TrierParRarete();

	UFUNCTION()
	void TrierParLieu();

	void RemplirGrille(const TArray<FPokedexEntry>& Entrees);
	void SetDetailVisible(bool bVisible);
	void SurlignerBouton(UButton* Bouton, bool bSurligne);
	void MettreAJourBoutonsTri();
	FName TrouverLieu(UPoissonTemplate* Poisson) const;

	// Entrées dans l'ordre original (numéro)
	UPROPERTY()
	TArray<FPokedexEntry> EntreesOriginales;

	UPROPERTY()
	TArray<UPokedexBoutonHelper*> Helpers;

	UPROPERTY()
	APokedexViewerActor* ViewerActor = nullptr;

	UPROPERTY()
	UButton* BoutonActif = nullptr;

	// Premier bouton de la grille – sert de point d'entrée pour la navigation gamepad
	UPROPERTY()
	UButton* PremierBouton = nullptr;

	ETriPokedex TriActuel = ETriPokedex::Numero;

	bool bDragging = false;
	bool bDetailVisible = false;

	// Debounce pour éviter de cycler trop vite avec L1/R1 (secondes)
	float DernierChangementTri = 0.f;

};
