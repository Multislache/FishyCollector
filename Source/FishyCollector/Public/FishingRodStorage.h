#pragma once

#include "CoreMinimal.h"
#include "FishingRod.h"
#include "GameFramework/Actor.h"
#include "FishingRodStorage.generated.h"

class AFishyCollectorCharacter;

UCLASS()
class FISHYCOLLECTOR_API AFishingRodStorage : public AActor
{
	GENERATED_BODY()

public:
	AFishingRodStorage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storage")
	TArray<TSubclassOf<AFishingRod>> OwnedRods;

	UFUNCTION(BlueprintCallable, Category="Storage")
	void OpenStorage(AFishyCollectorCharacter* PlayerCharacter);

	UFUNCTION(BlueprintCallable, Category="Storage")
	void CloseStorage();

protected:
	UPROPERTY(EditAnywhere, Category="Components")
	UStaticMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> StorageWidgetClass;

	UPROPERTY()
	UUserWidget* StorageWidgetInstance;

	UPROPERTY()
	AFishyCollectorCharacter* InteractingPlayer;
};