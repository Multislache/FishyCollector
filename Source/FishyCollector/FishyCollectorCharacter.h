// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
	UInputAction* PokedexAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ResetPokedexAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ShopAction;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPokedexWidget> PokedexWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ShopWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, Category="UI")
	UUserWidget* ShopWidget;

	UPROPERTY()
	UPokedexWidget* PokedexWidget;
	

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

	FTimerHandle FishBiteTimerHandle;

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void SetNearbyStorage(AFishingRodStorage* Storage);

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void EquipRod(TSubclassOf<AFishingRod> NewRodClass);

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void UnequipRod();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fishing")
	TSubclassOf<AFishingRod> GetCurrentRodClass() const;
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void Interact();
	void TogglePokedex();
	void ResetPokedex();
	
	void ToggleShop();
};

