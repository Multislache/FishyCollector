// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PokedexManager.h"
#include "PokedexWidget.generated.h"

class UUniformGridPanel;
class UTextBlock;
class UImage;
class UButton;
class UPokedexWidget;
class APokedexViewerActor;
class UMaterialInterface;

UCLASS()
class UPokedexBoutonHelper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UPoissonTemplate* Poisson = nullptr;

	UPROPERTY()
	UPokedexWidget* Widget = nullptr;

	bool bPeche = false;

	UFUNCTION()
	void OnClique();
};

UCLASS()
class FISHYCOLLECTOR_API UPokedexWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void Rafraichir();

	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void AfficherDetail(UPoissonTemplate* Poisson, bool bPeche);

	UFUNCTION(BlueprintCallable, Category = "Pokedex")
	void RetourListe();

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

	// Page Détail
	UPROPERTY(meta = (BindWidget))
	UImage* IconePoisson;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NomPoisson;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionPoisson;

	UPROPERTY(meta = (BindWidget))
	UButton* BoutonRetour;

	// Matériau noir assigné dans le Blueprint pour les poissons non découverts
	UPROPERTY(EditAnywhere, Category = "Pokedex")
	UMaterialInterface* MateriauSilhouette;

private:
	void SetDetailVisible(bool bVisible);

	UPROPERTY()
	TArray<UPokedexBoutonHelper*> Helpers;

	UPROPERTY()
	APokedexViewerActor* ViewerActor = nullptr;

	bool bDragging = false;

};
