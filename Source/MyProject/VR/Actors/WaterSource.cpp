#include "WaterSource.h"
#include "Components/StaticMeshComponent.h"

AWaterSource::AWaterSource()
{
	PrimaryActorTick.bCanEverTick = false;

	// Mesh del agua
	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
	RootComponent = WaterMesh;
	WaterMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// TAG IMPORTANTE: Identifica como fuente de agua
	Tags.Add(FName("WaterSource"));
}

void AWaterSource::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("WaterSource: Active at %s"), 
		*GetActorLocation().ToString());
}