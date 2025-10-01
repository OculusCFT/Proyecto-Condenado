#include "DiggableTerrainActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADiggableTerrainActor::ADiggableTerrainActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Terrain mesh
    TerrainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerrainMesh"));
    RootComponent = TerrainMesh;
    TerrainMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // Mesh para hoyos (si usas deformación)
    HolesMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HolesMesh"));
    HolesMesh->SetupAttachment(TerrainMesh);
    HolesMesh->SetVisibility(false);

    // Tag para identificación
    Tags.Add(FName("Diggable"));
}

void ADiggableTerrainActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("DiggableTerrain: %s initialized"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  - Use Decals: %s"), bUseDecals ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("  - Deform Mesh: %s"), bDeformMesh ? TEXT("YES") : TEXT("NO"));
}

void ADiggableTerrainActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADiggableTerrainActor::OnDig(FVector Location, float Radius, float Depth, FVector ImpactNormal)
{
    // Verificar si ya hay un hoyo muy cerca
    if (IsLocationAlreadyDug(Location, Radius * 0.5f))
    {
        UE_LOG(LogTemp, Verbose, TEXT("DiggableTerrain: Location already dug, skipping"));
        return;
    }

    // Guardar hoyo
    DigHoles.Add(FDigHole(Location, Radius, Depth));

    // Limitar número de hoyos
    if (DigHoles.Num() > MaxHoles)
    {
        DigHoles.RemoveAt(0);
        
        // Remover decal más antiguo
        if (bUseDecals && DecalComponents.Num() > 0)
        {
            if (UDecalComponent* OldestDecal = DecalComponents[0])
            {
                OldestDecal->DestroyComponent();
            }
            DecalComponents.RemoveAt(0);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DiggableTerrain: Dug hole at %s (Total: %d)"), 
        *Location.ToString(), DigHoles.Num());

    // Crear visualización
    if (bUseDecals)
    {
        CreateHoleDecal(Location, Radius, ImpactNormal);
    }

    if (bDeformMesh)
    {
        DeformMeshAtLocation(Location, Radius, Depth);
    }

    // Debug visual
    DrawDebugSphere(GetWorld(), Location, Radius, 12, FColor::Orange, false, 3.0f, 0, 2.0f);
}

void ADiggableTerrainActor::CreateHoleDecal(const FVector& Location, float Radius, const FVector& Normal)
{
    if (!HoleMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("DiggableTerrain: No hole material assigned"));
        return;
    }

    // Crear decal component
    UDecalComponent* Decal = NewObject<UDecalComponent>(this);
    if (!Decal)
    {
        return;
    }

    Decal->RegisterComponent();
    Decal->AttachToComponent(TerrainMesh, FAttachmentTransformRules::KeepWorldTransform);
    
    // Configurar decal
    Decal->SetWorldLocation(Location);
    Decal->SetWorldRotation(Normal.Rotation());
    Decal->DecalSize = FVector(Radius, Radius, Radius);
    Decal->SetDecalMaterial(HoleMaterial);
    Decal->SetFadeScreenSize(0.001f); // No fade

    DecalComponents.Add(Decal);

    UE_LOG(LogTemp, Log, TEXT("DiggableTerrain: Created decal at %s"), *Location.ToString());
}

void ADiggableTerrainActor::DeformMeshAtLocation(const FVector& Location, float Radius, float Depth)
{
    // Esta función requiere acceso a los vértices del mesh
    // Es más complejo y requiere ProceduralMeshComponent o RuntimeMeshComponent
    
    UE_LOG(LogTemp, Warning, TEXT("DiggableTerrain: Mesh deformation not yet implemented"));
    UE_LOG(LogTemp, Warning, TEXT("  Consider using ProceduralMeshComponent for runtime mesh modification"));
}

bool ADiggableTerrainActor::IsLocationAlreadyDug(const FVector& Location, float MinDistance) const
{
    for (const FDigHole& Hole : DigHoles)
    {
        float Distance = FVector::Dist(Location, Hole.Location);
        if (Distance < MinDistance)
        {
            return true;
        }
    }
    return false;
}