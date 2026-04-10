// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexViewerActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"

APokedexViewerActor::APokedexViewerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root neutre : MeshComp tourne, SceneCapture et lumière restent fixes
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->CastShadow = false;

	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	LightComp->SetupAttachment(RootComponent);
	LightComp->SetRelativeLocation(FVector(100.f, -200.f, 200.f));
	LightComp->Intensity = 5000.f;
	LightComp->AttenuationRadius = 2000.f;

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->FOVAngle = 45.f;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

void APokedexViewerActor::SetMesh(UStaticMesh* InMesh, UMaterialInterface* InMaterial, bool bDecouvert)
{
	MeshComp->SetStaticMesh(InMesh);

	// Seul ce composant sera capturé
	SceneCapture->ShowOnlyComponents.Empty();
	SceneCapture->ShowOnlyComponents.Add(MeshComp);

	if (InMesh)
	{
		// Applique le matériau sur tous les slots (noir si non découvert, réel si découvert)
		if (InMaterial)
		{
			for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
				MeshComp->SetMaterial(i, InMaterial);
		}

		FBoxSphereBounds Bounds = InMesh->GetBounds();
		float Radius = Bounds.SphereRadius;
		float Distance = (Radius / FMath::Tan(FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f))) * 1.5f;
		SceneCapture->SetRelativeLocation(FVector(0.f, -Distance, Radius * 0.5f));
		SceneCapture->SetRelativeRotation(FRotator(-10.f, 90.f, 0.f));
		LightComp->SetRelativeLocation(FVector(Distance * 0.5f, -Distance, Radius));
	}

	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this);
		RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		// Fond légèrement visible pour que la silhouette noire soit perceptible
		RenderTarget->ClearColor = FLinearColor(0.15f, 0.15f, 0.15f);
		RenderTarget->InitAutoFormat(512, 512);
		RenderTarget->UpdateResourceImmediate(true);
		SceneCapture->TextureTarget = RenderTarget;
	}

	SceneCapture->CaptureScene();
}

void APokedexViewerActor::AjouterRotationYaw(float DeltaYaw)
{
	// Seul le mesh tourne, la caméra et la lumière restent fixes
	FRotator NewRot = MeshComp->GetRelativeRotation();
	NewRot.Yaw += DeltaYaw;
	MeshComp->SetRelativeRotation(NewRot);
	SceneCapture->CaptureScene();
}

UTextureRenderTarget2D* APokedexViewerActor::GetRenderTarget() const
{
	return RenderTarget;
}
