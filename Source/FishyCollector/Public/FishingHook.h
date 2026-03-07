#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingHook.generated.h"

UCLASS()
class FISHYCOLLECTOR_API AFishingHook : public AActor
{
	GENERATED_BODY()

public:
	AFishingHook();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* HookMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USphereComponent* HookCollision;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fishing")
	UStaticMeshComponent* GetHookMesh() const { return HookMesh; }
};