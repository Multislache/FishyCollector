#pragma once

#include "CoreMinimal.h"
#include "CableComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "FishingRod.generated.h"


UENUM(BlueprintType)
enum class EFishingRodState : uint8
{
	Repos       UMETA(DisplayName = "Repos"),
	Lance       UMETA(DisplayName = "Lancé"),
	Attente     UMETA(DisplayName = "Attente"),
	Morsure     UMETA(DisplayName = "Morsure"),
	Tirer       UMETA(DisplayName = "Tirer")
};

class AFishingHook;
class ACharacter;
class UPoissonTemplate;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFishBite);

UCLASS()
class FISHYCOLLECTOR_API AFishingRod : public AActor
{
	GENERATED_BODY()

public:
	AFishingRod();

	UFUNCTION()
	void OnHookLandedInWater();

	UFUNCTION()
	void OnHookInvalidSurface();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere,  BlueprintReadWrite, Category="Components")
	UStaticMeshComponent* RodMesh;

	UPROPERTY(EditAnywhere,  BlueprintReadWrite, Category="Components")
	USceneComponent* LineAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fishing")
	EFishingRodState CurrentState = EFishingRodState::Repos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCableComponent* FishingLine;

	UPROPERTY(EditAnywhere, Category="Fishing")
	TSubclassOf<AFishingHook> FishingHookClass;

	UPROPERTY(BlueprintReadOnly, Category="Fishing")
	AFishingHook* FishingHook;

	UPROPERTY(BlueprintReadOnly, Category="Fishing")
	ACharacter* OwnerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Fishing")
	UPoissonTemplate* CurrentFishBiting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> FishingWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UUserWidget* FishingWidgetInstance;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> FishNotifyWidgetClass;
	
public:
	UFUNCTION(BlueprintCallable, Category="Fishing")
	void AttachToCharacter(ACharacter* Character, FName SocketName = "hand_r");

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void DetachFromCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fishing")
	EFishingRodState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void SetState(EFishingRodState NewState, FVector LaunchDirection = FVector::ZeroVector);

	UFUNCTION(BlueprintNativeEvent, Category="Fishing")
	void OnStateChanged(EFishingRodState OldState, EFishingRodState NewState, FVector LaunchDirection);
	virtual void OnStateChanged_Implementation(EFishingRodState OldState, EFishingRodState NewState, FVector LaunchDirection);

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void HandleInput();

	UPROPERTY(BlueprintAssignable)
	FOnFishBite OnFishBite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* SplashSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* RodSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* FishBiteSound;

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void EndMiniGame(bool bSuccess, UPoissonTemplate* CaughtFish);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fishing")
	FText RodDisplayName = FText::FromString("Canne à pêche");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* WrongSurfaceSound;
	
private:
	bool CanTransitionTo(EFishingRodState NewState) const;
	
	FTimerHandle FishBiteTimerHandle;
	FTimerHandle RodTimerHandle;
	
	void StartWaitingForBite();
	void TriggerFishBite();
	void ShowHook();

	bool bHookLandedInWater = false;
	void OnHookTimeout();
};