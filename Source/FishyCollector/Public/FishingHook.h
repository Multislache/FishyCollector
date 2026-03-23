#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Sound/SoundBase.h"
#include "FishingHook.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookLanded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookInvalidSurface);

UCLASS()
class FISHYCOLLECTOR_API AFishingHook : public AActor
{
	GENERATED_BODY()

public:
	AFishingHook();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing")
	USoundBase* WrongSurfaceSound;

	UPROPERTY(BlueprintAssignable)
	FOnHookLanded OnHookLanded;

	UPROPERTY(BlueprintAssignable)
	FOnHookInvalidSurface OnHookInvalidSurface;

	UFUNCTION()
	void OnHookOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* HookMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USphereComponent* HookCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UWidgetComponent* QTEWidgetComponent;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fishing")
	UStaticMeshComponent* GetHookMesh() const { return HookMesh; }

	UWidgetComponent* GetQTEWidgetComponent() const { return QTEWidgetComponent; }
};