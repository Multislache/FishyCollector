// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PokedexManager.h"
#include "PokedexWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UImage;
class UButton;
class UPokedexWidget;

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

	// Page Liste
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ListePoissons;

	// Page Détail
	UPROPERTY(meta = (BindWidget))
	UImage* IconePoisson;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NomPoisson;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionPoisson;

	UPROPERTY(meta = (BindWidget))
	UButton* BoutonRetour;

private:
	void SetDetailVisible(bool bVisible);
};
