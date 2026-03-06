#include "FishingHook.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

AFishingHook::AFishingHook()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	HookCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HookCollision"));
	HookCollision->SetupAttachment(RootComponent);
	HookCollision->SetSphereRadius(8.f);
	HookCollision->SetCollisionProfileName(TEXT("OverlapAll"));

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMesh"));
	HookMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		HookMesh->SetStaticMesh(SphereMesh.Object);
		HookMesh->SetRelativeScale3D(FVector(0.05f));
		HookMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}