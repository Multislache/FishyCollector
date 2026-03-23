// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishyBaseWidget.generated.h"

/**
 * Classe de base pour tous les widgets du jeu.
 * Expose InitialiserFocusGamepad() que chaque Blueprint implémente
 * pour placer le focus sur son premier élément navigable (grille, bouton…).
 */
UCLASS()
class FISHYCOLLECTOR_API UFishyBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Appelée une frame après AddToViewport (Slate est layout).
	 * Chaque widget Blueprint surcharge cet event pour appeler SetKeyboardFocus
	 * sur son premier bouton / slot de grille.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gamepad")
	void InitialiserFocusGamepad();
	virtual void InitialiserFocusGamepad_Implementation() {}

	/**
	 * Appelée quand le joueur appuie sur Retour (O/B) avec ce widget ouvert.
	 * Retourne true si le widget a géré l'action lui-même (ex : fermer un popup interne
	 * et remettre le focus sur la grille).  Retourne false pour que le Character
	 * ferme le widget normalement.
	 * Par défaut : false (le Character se charge de fermer).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gamepad")
	bool GererRetour();
	virtual bool GererRetour_Implementation() { return false; }

	/**
	 * Appelée quand L1 est pressé avec ce widget ouvert.
	 * Pokédex : rotation gauche (détail) ou tri précédent (liste).
	 * Autres widgets : navigation vers l'élément précédent dans une HBox/VBox.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gamepad")
	void NaviguerGauche();
	virtual void NaviguerGauche_Implementation() {}

	/**
	 * Appelée quand R1 est pressé avec ce widget ouvert.
	 * Pokédex : rotation droite (détail) ou tri suivant (liste).
	 * Autres widgets : navigation vers l'élément suivant dans une HBox/VBox.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gamepad")
	void NaviguerDroite();
	virtual void NaviguerDroite_Implementation() {}

	// Injectent une touche D-pad dans Slate — pour naviguer dans une VBox/HBox avec L1/R1
	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void InjecterToucheHaut();

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void InjecterToucheBas();

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void InjecterToucheGauche();

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void InjecterToucheDroite();

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void InjecterToucheAccepter();

protected:
	virtual void NativeConstruct() override;

private:
	// Wrapper UFUNCTION pour le timer (BlueprintNativeEvent ne peut pas être callback direct)
	UFUNCTION()
	void AppliquerFocusInitial() { InitialiserFocusGamepad(); }
};
