#include "VRTeleportComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

UVRTeleportComponent::UVRTeleportComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ProjectedTeleportLocation = FVector::ZeroVector;
}

void UVRTeleportComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("VRTeleportComponent: Initialized"));
}

void UVRTeleportComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyTeleportVisualizer();
	Super::EndPlay(EndPlayReason);
}

void UVRTeleportComponent::StartTeleportTrace()
{
	DestroyTeleportVisualizer();

	bTeleportTraceActive = true;
	UpdateTeleportValidation(false);
	ProjectedTeleportLocation = FVector::ZeroVector;
	TeleportTracePathPositions.Empty();

	CreateTeleportVisualizer();

	if (TeleportTraceNiagaraSystem && IsValid(TeleportTraceNiagaraSystem))
	{
		TeleportTraceNiagaraSystem->SetVisibility(true);
		TeleportTraceNiagaraSystem->Activate();
	}

	UE_LOG(LogTemp, Log, TEXT("VRTeleportComponent: Teleport trace started"));
}

void UVRTeleportComponent::UpdateTeleportTrace(const FVector& StartPos, const FVector& ForwardVector)
{
	if (!bTeleportTraceActive || !GetWorld())
	{
		return;
	}

	TeleportTracePathPositions.Empty();

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.StartLocation = StartPos;
	PathParams.LaunchVelocity = ForwardVector * TeleportLaunchVelocity;
	PathParams.MaxSimTime = 5.0f;
	PathParams.OverrideGravityZ = -980.0f;
	PathParams.ProjectileRadius = TeleportProjectileRadius;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugType = EDrawDebugTrace::None;

	FPredictProjectilePathResult PathResult;
	const bool bHit = UGameplayStatics::PredictProjectilePath(
		this,
		PathParams,
		PathResult
	);

	bool bNewValidLocation = false;
	FVector NewProjectedLocation = FVector::ZeroVector;

	if (bHit && PathResult.HitResult.bBlockingHit)
	{
		for (const FPredictProjectilePathPointData& Point : PathResult.PathData)
		{
			TeleportTracePathPositions.Add(Point.Location);
		}

		const FVector HitLocation = PathResult.HitResult.Location;
		
		FVector ProjectedLocation;
		const bool bProjected = UNavigationSystemV1::K2_ProjectPointToNavigation(
			this,
			HitLocation,
			ProjectedLocation,
			nullptr,
			nullptr,
			TeleportProjectPointToNavigationQueryExtent
		);

		if (bProjected && IsValidTeleportLocationInternal(ProjectedLocation))
		{
			bNewValidLocation = true;
			NewProjectedLocation = ProjectedLocation;
		}
	}

	UpdateTeleportValidation(bNewValidLocation);
	ProjectedTeleportLocation = NewProjectedLocation;

	if (TeleportVisualizerReference && IsValid(TeleportVisualizerReference))
	{
		UpdateTeleportVisualization(TeleportTracePathPositions, bValidTeleportLocation, ProjectedTeleportLocation);
	}
}

void UVRTeleportComponent::EndTeleportTrace()
{
	bTeleportTraceActive = false;

	if (TeleportTraceNiagaraSystem && IsValid(TeleportTraceNiagaraSystem))
	{
		TeleportTraceNiagaraSystem->SetVisibility(false);
		TeleportTraceNiagaraSystem->Deactivate();
	}

	DestroyTeleportVisualizer();
	TeleportTracePathPositions.Empty();

	UE_LOG(LogTemp, Log, TEXT("VRTeleportComponent: Teleport trace ended"));
}

bool UVRTeleportComponent::TryExecuteTeleport()
{
	if (!bValidTeleportLocation || ProjectedTeleportLocation == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRTeleportComponent: Teleportation failed - invalid location"));
		return false;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("VRTeleportComponent: Owner is not a Pawn"));
		return false;
	}

	// Find camera component for HMD offset calculation
	UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		UE_LOG(LogTemp, Error, TEXT("VRTeleportComponent: Camera component not found"));
		return false;
	}

	// Calculate HMD offset
	FVector HMDOffset = Camera->GetComponentLocation() - OwnerPawn->GetActorLocation();
	HMDOffset.Z = 0.0f;

	const FVector FinalTeleportLocation = ProjectedTeleportLocation - HMDOffset;
	
	if (IsValidTeleportLocationInternal(FinalTeleportLocation))
	{
		bool bTeleportSuccess = OwnerPawn->K2_TeleportTo(FinalTeleportLocation, OwnerPawn->GetActorRotation());
		
		if (bTeleportSuccess)
		{
			UpdateTeleportValidation(false);
			ProjectedTeleportLocation = FVector::ZeroVector;

			OnTeleportComplete.Broadcast(FinalTeleportLocation);

			UE_LOG(LogTemp, Log, TEXT("VRTeleportComponent: Teleportation successful to location: %s"), 
				*FinalTeleportLocation.ToString());
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VRTeleportComponent: K2_TeleportTo failed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VRTeleportComponent: Final teleport location validation failed"));
	}

	return false;
}

void UVRTeleportComponent::SetTeleportNiagaraSystem(UNiagaraComponent* NiagaraComponent)
{
	TeleportTraceNiagaraSystem = NiagaraComponent;
}

bool UVRTeleportComponent::IsValidTeleportLocationInternal(const FVector& Location) const
{
	if (!GetOwner())
		return false;

	const float DistanceToLocation = FVector::Dist(GetOwner()->GetActorLocation(), Location);
	
	if (DistanceToLocation < MIN_TELEPORT_DISTANCE)
	{
		return false;
	}

	if (!IsValidSurfaceAngle(Location))
	{
		return false;
	}

	return true;
}

bool UVRTeleportComponent::IsValidSurfaceAngle(const FVector& Location) const
{
	if (!GetWorld())
	{
		return false;
	}

	FHitResult HitResult;
	FVector TraceStart = Location + FVector(0, 0, SURFACE_TRACE_DISTANCE);
	FVector TraceEnd = Location - FVector(0, 0, SURFACE_TRACE_DISTANCE);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		FVector SurfaceNormal = HitResult.Normal;
		float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
		
		if (SurfaceAngle > MAX_TELEPORT_SURFACE_ANGLE)
		{
			return false;
		}
	}
	
	return true;
}

void UVRTeleportComponent::CreateTeleportVisualizer()
{
	if (!TeleportVisualizerClass || !GetWorld())
	{
		return;
	}

	if (!TeleportVisualizerReference || !IsValid(TeleportVisualizerReference))
	{
		FTransform SpawnTransform = FTransform::Identity;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TeleportVisualizerReference = GetWorld()->SpawnActor<AActor>(
			TeleportVisualizerClass,
			SpawnTransform,
			SpawnParams
		);

		if (TeleportVisualizerReference)
		{
			UE_LOG(LogTemp, Log, TEXT("VRTeleportComponent: Teleport visualizer created"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VRTeleportComponent: Failed to create teleport visualizer"));
		}
	}
}

void UVRTeleportComponent::DestroyTeleportVisualizer()
{
	if (TeleportVisualizerReference && IsValid(TeleportVisualizerReference))
	{
		TeleportVisualizerReference->Destroy();
		TeleportVisualizerReference = nullptr;
		UE_LOG(LogTemp, Log, TEXT("VRTeleportComponent: Teleport visualizer destroyed"));
	}
}

void UVRTeleportComponent::UpdateTeleportValidation(bool bNewValidState)
{
	if (bValidTeleportLocation != bNewValidState)
	{
		bValidTeleportLocation = bNewValidState;
		OnTeleportValidationChanged.Broadcast(bValidTeleportLocation);
	}
}

void UVRTeleportComponent::UpdateTeleportVisualization(const TArray<FVector>& PathPositions, bool bIsValidLocation, const FVector& TargetLocation)
{
	if (!TeleportVisualizerReference || !IsValid(TeleportVisualizerReference))
	{
		return;
	}

	if (UFunction* UpdateVisualizationFunction = TeleportVisualizerReference->GetClass()->FindFunctionByName(FName("UpdateVisualization")))
	{
		struct FUpdateVisualizationParams
		{
			TArray<FVector> PathPositions;
			bool bIsValidLocation;
			FVector TargetLocation;
		};

		FUpdateVisualizationParams Params;
		Params.PathPositions = PathPositions;
		Params.bIsValidLocation = bIsValidLocation;
		Params.TargetLocation = TargetLocation;

		TeleportVisualizerReference->ProcessEvent(UpdateVisualizationFunction, &Params);
	}
}