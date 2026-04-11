// Fill out your copyright notice in the Description page of Project Settings.

#include "PokedexViewerActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APokedexViewerActor::APokedexViewerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->CastShadow = false;

	FondComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fond"));
	FondComp->SetupAttachment(RootComponent);
	FondComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FondComp->CastShadow = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
		FondComp->SetStaticMesh(PlaneMesh.Object);

	// Charger M_fond par défaut si présent dans le projet
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FondMat(TEXT("/Game/FishyCollector/UI/M_fond"));
	if (FondMat.Succeeded())
		MateriauFond = FondMat.Object;

	// Lumière principale pour le poisson (channel 0)
	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	LightComp->SetupAttachment(RootComponent);
	LightComp->SetRelativeLocation(FVector(100.f, -200.f, 200.f));
	LightComp->Intensity = 5000.f;
	LightComp->AttenuationRadius = 2000.f;

	// Lumière dédiée au fond (channel 1) — intensité massive pour blanc éclatant
	FondLightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("FondLight"));
	FondLightComp->SetupAttachment(RootComponent);
	FondLightComp->Intensity = 200000.f;
	FondLightComp->AttenuationRadius = 5000.f;
	FondLightComp->bUseInverseSquaredFalloff = false;
	FondLightComp->LightFalloffExponent = 1.f;
	FLightingChannels FondChannel;
	FondChannel.bChannel0 = false;
	FondChannel.bChannel1 = true;
	FondLightComp->LightingChannels = FondChannel;

	// FondComp reçoit uniquement la lumière du fond (channel 1)
	FLightingChannels FondMeshChannel;
	FondMeshChannel.bChannel0 = false;
	FondMeshChannel.bChannel1 = true;
	FondComp->SetLightingChannels(false, true, false);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->FOVAngle = 45.f;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->ShowFlags.SetSkyLighting(false);
	SceneCapture->ShowFlags.SetVolumetricFog(false);
}

void APokedexViewerActor::SetMesh(UStaticMesh* InMesh, UMaterialInterface* InMaterial, bool bDecouvert)
{
	MeshComp->SetStaticMesh(InMesh);

	if (InMesh && InMaterial)
	{
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
			MeshComp->SetMaterial(i, InMaterial);
	}

	// Créer ou mettre à jour le MID du fond
	if (MateriauFond)
	{
		if (!MIDFond || MIDFond->Parent != MateriauFond)
			MIDFond = UMaterialInstanceDynamic::Create(MateriauFond, this);
		MIDFond->SetVectorParameterValue(TEXT("Color"), CouleurFond);
		FondComp->SetMaterial(0, MIDFond);
	}

	SceneCapture->ShowOnlyComponents.Empty();
	SceneCapture->ShowOnlyComponents.Add(MeshComp);
	SceneCapture->ShowOnlyComponents.Add(FondComp);

	if (InMesh)
	{
		FBoxSphereBounds Bounds = InMesh->GetBounds();
		float Radius = Bounds.SphereRadius;
		float Distance = (Radius / FMath::Tan(FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f))) * 1.5f;

		SceneCapture->SetRelativeLocation(FVector(0.f, -Distance, Radius * 0.5f));
		SceneCapture->SetRelativeRotation(FRotator(-10.f, 90.f, 0.f));
		LightComp->SetRelativeLocation(FVector(Distance * 0.5f, -Distance, Radius));

		// Plan de fond : derrière le poisson, tourné face caméra, assez grand pour remplir le cadre
		float Scale = Distance / 50.f;
		FondComp->SetRelativeLocation(FVector(0.f, Distance * 0.4f, 0.f));
		FondComp->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));
		FondComp->SetRelativeScale3D(FVector(Scale, Scale, 1.f));

		// Lumière fond : juste devant le plan, côté caméra, pour l'écraser en blanc
		FondLightComp->SetRelativeLocation(FVector(0.f, Distance * 0.2f, 0.f));
		FondLightComp->AttenuationRadius = Distance * 2.f;
	}

	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this);
		RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		RenderTarget->ClearColor = FLinearColor::White;
		RenderTarget->InitAutoFormat(512, 512);
		RenderTarget->UpdateResourceImmediate(true);
		SceneCapture->TextureTarget = RenderTarget;
	}

	SceneCapture->CaptureScene();
}

void APokedexViewerActor::AjouterRotationYaw(float DeltaYaw)
{
	FRotator NewRot = MeshComp->GetRelativeRotation();
	NewRot.Yaw += DeltaYaw;
	MeshComp->SetRelativeRotation(NewRot);
	SceneCapture->CaptureScene();
}

UTextureRenderTarget2D* APokedexViewerActor::GetRenderTarget() const
{
	return RenderTarget;
}
