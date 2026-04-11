// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PokedexViewerActor.generated.h"

class UStaticMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UPointLightComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class FISHYCOLLECTOR_API APokedexViewerActor : public AActor
{
	GENERATED_BODY()

public:
	APokedexViewerActor();

	void SetMesh(UStaticMesh* InMesh, UMaterialInterface* InMaterial, bool bDecouvert);
	void AjouterRotationYaw(float DeltaYaw);
	UTextureRenderTarget2D* GetRenderTarget() const;

	// Material Unlit avec un Vector Parameter nommé "Color" — à créer dans le projet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewer")
	UMaterialInterface* MateriauFond = nullptr;

	// Couleur appliquée sur le MateriauFond
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewer")
	FLinearColor CouleurFond = FLinearColor::White;

private:
	UPROPERTY()
	UStaticMeshComponent* MeshComp;

	UPROPERTY()
	UStaticMeshComponent* FondComp;

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY()
	UPointLightComponent* LightComp;

	UPROPERTY()
	UPointLightComponent* FondLightComp;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY()
	UMaterialInstanceDynamic* MIDFond = nullptr;
};
