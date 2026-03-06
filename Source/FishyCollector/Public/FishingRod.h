#pragma once

#include "CoreMinimal.h"
#include "CableComponent.h"
#include "GameFramework/Actor.h"
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

UCLASS()
class FISHYCOLLECTOR_API AFishingRod : public AActor
{
	GENERATED_BODY()

public:
	AFishingRod();

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

public:
	UFUNCTION(BlueprintCallable, Category="Fishing")
	void AttachToCharacter(ACharacter* Character, FName SocketName = "hand_r");

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void DetachFromCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fishing")
	EFishingRodState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category="Fishing")
	void SetState(EFishingRodState NewState);

	UFUNCTION(BlueprintNativeEvent, Category="Fishing")
	void OnStateChanged(EFishingRodState OldState, EFishingRodState NewState);
	virtual void OnStateChanged_Implementation(EFishingRodState OldState, EFishingRodState NewState);

private:
	bool CanTransitionTo(EFishingRodState NewState) const;

};