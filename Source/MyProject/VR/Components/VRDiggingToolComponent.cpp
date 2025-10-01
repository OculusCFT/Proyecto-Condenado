#include "VRDiggingToolComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UVRDiggingToolComponent::UVRDiggingToolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVRDiggingToolComponent::BeginPlay()
{
	Super::BeginPlay();

	// Intentar encontrar el mesh automáticamente
	if (!ToolMesh)
	{
		ToolMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	}

	LastTipPosition = GetToolTipLocation();
	
	UE_LOG(LogTemp, Log, TEXT("DiggingTool: Initialized on %s"), *GetOwner()->GetName());
}

void UVRDiggingToolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDigging && ToolMesh)
	{
		UpdateVelocity(DeltaTime);

		// Debug: Visualizar la punta
		FVector TipLocation = GetToolTipLocation();
		DrawDebugSphere(GetWorld(), TipLocation, DigRadius, 8, FColor::Yellow, false, -1.0f, 0, 2.0f);

		if (ShouldDig())
		{
			TryDigAtLocation(TipLocation);
		}
	}
}

void UVRDiggingToolComponent::StartDigging()
{
	bIsDigging = true;
	SetComponentTickEnabled(true);
	LastTipPosition = GetToolTipLocation();
	CurrentTipVelocity = FVector::ZeroVector;
	
	UE_LOG(LogTemp, Log, TEXT("DiggingTool: Started digging"));
}

void UVRDiggingToolComponent::StopDigging()
{
	bIsDigging = false;
	SetComponentTickEnabled(false);
	
	UE_LOG(LogTemp, Log, TEXT("DiggingTool: Stopped digging"));
}

FVector UVRDiggingToolComponent::GetToolTipLocation() const
{
	if (!ToolMesh)
		return GetOwner()->GetActorLocation();

	// Intentar usar un socket si existe
	if (ToolMesh->DoesSocketExist(ToolTipSocketName))
	{
		return ToolMesh->GetSocketLocation(ToolTipSocketName);
	}

	// Si no hay socket, usar offset
	return ToolMesh->GetComponentLocation() + 
		   ToolMesh->GetComponentRotation().RotateVector(ToolTipOffset);
}

bool UVRDiggingToolComponent::TryDigAtLocation(const FVector& Location)
{
	if (!GetWorld())
		return false;

	// Sphere overlap en la punta
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(DigRadius),
		QueryParams
	);

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AActor* HitActor = Result.GetActor())
			{
				// Verificar si tiene el tag "Diggable"
				if (HitActor->Tags.Contains(FName("Diggable")))
				{
					// Llamar función OnDig en el actor
					if (UFunction* DigFunction = HitActor->GetClass()->FindFunctionByName(FName("OnDig")))
					{
						struct FDigParams
						{
							FVector Location;
							float Radius;
							float Depth;
							FVector ImpactNormal;
						};

						FDigParams Params;
						Params.Location = Location;
						Params.Radius = DigRadius;
						Params.Depth = DigDepth;
						Params.ImpactNormal = CurrentTipVelocity.GetSafeNormal();

						HitActor->ProcessEvent(DigFunction, &Params);

						// Feedback visual
						DrawDebugSphere(GetWorld(), Location, DigRadius, 12, FColor::Red, false, 1.0f, 0, 3.0f);

						UE_LOG(LogTemp, Log, TEXT("DiggingTool: Dug at %s with velocity %.2f"), 
							*Location.ToString(), CurrentTipVelocity.Size());

						return true;
					}
				}
			}
		}
	}

	return false;
}

void UVRDiggingToolComponent::UpdateVelocity(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
		return;

	FVector CurrentTipPosition = GetToolTipLocation();
	CurrentTipVelocity = (CurrentTipPosition - LastTipPosition) / DeltaTime;
	LastTipPosition = CurrentTipPosition;
}

bool UVRDiggingToolComponent::ShouldDig() const
{
	float SpeedCmPerSec = CurrentTipVelocity.Size();
	return SpeedCmPerSec >= MinVelocityToDigCm;
}