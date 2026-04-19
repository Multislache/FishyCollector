#include "FishingHook.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
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
	HookCollision->OnComponentBeginOverlap.AddDynamic(this, &AFishingHook::OnHookOverlap);

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMesh"));
	HookMesh->SetupAttachment(RootComponent);
	HookMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	QTEWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("QTEWidgetComponent"));
	QTEWidgetComponent->SetupAttachment(RootComponent);
    
	QTEWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	QTEWidgetComponent->SetDrawAtDesiredSize(true);
	QTEWidgetComponent->SetVisibility(false);
}

// Nouvelle fonction à ajouter :
void AFishingHook::OnHookOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

    if (OtherActor->ActorHasTag(FName("water")))
    {
        OnHookLanded.Broadcast();
    }
    else
    {
        if (WrongSurfaceSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, WrongSurfaceSound, GetActorLocation());
        }
        OnHookInvalidSurface.Broadcast();
    }
}
