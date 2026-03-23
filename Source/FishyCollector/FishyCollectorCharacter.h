// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FishingRodData.h"
#include "Logging/LogMacros.h"
#include "FishyCollectorCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class AFishingRod;
class UPokedexWidget;
class AFishingRodStorage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AFishyCollectorCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ClickAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PokedexAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ResetPokedexAction;

	// L1 / Left Shoulder – rotation modèle 3D pokédex
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PokedexRotateLeftAction;

	// R1 / Right Shoulder – rotation modèle 3D pokédex
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PokedexRotateRightAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* RetourAction;

	// I / bouton manette – ouvrir/fermer l'inventaire
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InventaireAction;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPokedexWidget> PokedexWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ShopWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventaireWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, Category="UI")
	UUserWidget* ShopWidget;

	UPROPERTY()
	UUserWidget* InventaireWidget;

	UPROPERTY()
	UPokedexWidget* PokedexWidget;

	// Popup ouvert par un widget enfant (ex : détail d'un slot d'inventaire).
	// Renseigné/effacé par Blueprint via SetPopupActif.
	UPROPERTY(BlueprintReadOnly, Category="UI")
	UFishyBaseWidget* PopupActif = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category="Fishing")
	AFishingRodStorage* NearbyStorage;

public:
	AFishyCollectorCharacter();	

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	UPROPERTY(BlueprintReadWrite, Category="Fishing")
	bool bIsInFishingZone = false;

	UPROPERTY(BlueprintReadWrite, Category="Fishing")
	bool bIsInShopZone = false;
	
	UPROPERTY(EditAnywhere, Category="Fishing")
	TSubclassOf<AFishingRod> FishingRodClass;

	UPROPERTY(BlueprintReadOnly, Category="Fishing")
	AFishingRod* FishingRod;

	UPROPERTY(BlueprintReadWrite, Category="Ressource")
	float Money;
	
public:
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	UFUNCTION(BlueprintCallable, Category="Fishing")
	void SetFishingZoneActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void SetShopZoneActive(bool bActive);
	
	UFUNCTION(BlueprintNativeEvent, Category="Fishing")
	void DoThrowLine();

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void ClickInteractionManager();

	FTimerHandle FishBiteTimerHandle;

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void SetNearbyStorage(AFishingRodStorage* Storage);

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void EquipRodFromData(UFishingRodData* RodData);
	
	UFUNCTION(BlueprintCallable, Category="Fishing")
	void UnequipRod();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fishing")
	TSubclassOf<AFishingRod> GetCurrentRodClass() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<UFishingRodData*> OwnedFishingRods;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	UFishingRodData* EquippedFishingRod;
	
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void Interact();
	void TogglePokedex();
	void ResetPokedex();
	void JumpSiPokedexFerme();
	void PokedexRoterGauche();
	void PokedexRoterDroite();
	void ToggleInventaire();
	void RetourGeneral();

public:
	void OuvrirWidget(UUserWidget* Widget, APlayerController* PC);
	void FermerWidget(APlayerController* PC);
private:
	class UFishyBaseWidget* GetWidgetOuvert() const;

	// Navigation joystick → grid (injection D-pad dans Slate)
	void NaviguerUI(float X, float Y);

	// Clic sur le bouton en focus (IA_InteractAction quand UI ouverte)
	void AccepterUI();

	// true quand un widget UI est visible – bloque mouvement, caméra et saut
	bool bUIWidgetOuvert = false;

	// Debounce pour la navigation joystick dans la grille
	float DernierNavigationUI = 0.f;

	UFUNCTION(BlueprintCallable)
	void ToggleShop();

	// Appelé par Blueprint (WBP_Slot) quand un popup s'ouvre/se ferme.
	// Passer nullptr pour effacer.
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetPopupActif(UFishyBaseWidget* Popup);
};

