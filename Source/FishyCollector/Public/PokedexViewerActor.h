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

UCLASS()
class FISHYCOLLECTOR_API APokedexViewerActor : public AActor
{
	GENERATED_BODY()

public:
	APokedexViewerActor();

	void SetMesh(UStaticMesh* InMesh, UMaterialInterface* InMaterial, bool bDecouvert);
	void AjouterRotationYaw(float DeltaYaw);
	UTextureRenderTarget2D* GetRenderTarget() const;

private:
	UPROPERTY()
	UStaticMeshComponent* MeshComp;

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY()
	UPointLightComponent* LightComp;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;
};
