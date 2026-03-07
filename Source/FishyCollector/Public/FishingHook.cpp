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
	HookMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}