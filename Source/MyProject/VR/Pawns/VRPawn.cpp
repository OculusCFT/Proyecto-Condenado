#include "VRPawn.h"
#include "MyProject/VR/Components/VRTeleportComponent.h"
#include "MyProject/VR/Components/VRInteractionComponent.h"
#include "MyProject/VR/Components/VRHandAnimationComponent.h"
#include "MyProject/VR/Components/VRInputComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AVRPawn::AVRPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupComponentHierarchy();
}

void AVRPawn::SetupComponentHierarchy()
{
	// VR Origin
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VROrigin);

	// HMD Mesh
	HeadMountedDisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMountedDisplayMesh"));
	HeadMountedDisplayMesh->SetupAttachment(Camera);

	// Motion Controllers
	MotionControllerRightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRightAim"));
	MotionControllerRightAim->SetupAttachment(VROrigin);
	MotionControllerRightAim->SetTrackingSource(EControllerHand::Right);
	MotionControllerRightAim->SetTrackingMotionSource(FName("RightAim"));

	MotionControllerRightGrip = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRightGrip"));
	MotionControllerRightGrip->SetupAttachment(VROrigin);
	MotionControllerRightGrip->SetTrackingSource(EControllerHand::Right);
	MotionControllerRightGrip->SetTrackingMotionSource(FName("RightGrip"));

	MotionControllerLeftAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeftAim"));
	MotionControllerLeftAim->SetupAttachment(VROrigin);
	MotionControllerLeftAim->SetTrackingSource(EControllerHand::Left);
	MotionControllerLeftAim->SetTrackingMotionSource(FName("LeftAim"));

	MotionControllerLeftGrip = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeftGrip"));
	MotionControllerLeftGrip->SetupAttachment(VROrigin);
	MotionControllerLeftGrip->SetTrackingSource(EControllerHand::Left);
	MotionControllerLeftGrip->SetTrackingMotionSource(FName("LeftGrip"));

	// Hands
	HandRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandRight"));
	HandRight->SetupAttachment(MotionControllerRightGrip);

	HandLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandLeft"));
	HandLeft->SetupAttachment(MotionControllerLeftGrip);

	// Device Visualization
	XRDeviceVisualizationRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("XRDeviceVisualizationRight"));
	XRDeviceVisualizationRight->SetupAttachment(MotionControllerRightGrip);

	XRDeviceVisualizationLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("XRDeviceVisualizationLeft"));
	XRDeviceVisualizationLeft->SetupAttachment(MotionControllerLeftGrip);

	// Widget Interaction
	WidgetInteractionRight = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionRight"));
	WidgetInteractionRight->SetupAttachment(MotionControllerRightAim);

	WidgetInteractionLeft = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionLeft"));
	WidgetInteractionLeft->SetupAttachment(MotionControllerLeftAim);

	// Niagara System
	TeleportTraceNiagaraSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TeleportTraceNiagaraSystem"));
	TeleportTraceNiagaraSystem->SetupAttachment(VROrigin);

	// VR System Components
	TeleportComponent = CreateDefaultSubobject<UVRTeleportComponent>(TEXT("TeleportComponent"));
	InteractionComponent = CreateDefaultSubobject<UVRInteractionComponent>(TEXT("InteractionComponent"));
	HandAnimationComponent = CreateDefaultSubobject<UVRHandAnimationComponent>(TEXT("HandAnimationComponent"));
	VRInputComponent = CreateDefaultSubobject<UVRInputComponent>(TEXT("VRInputComponent"));
}

void AVRPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Stage);
		SetupInputMappingContexts();
		UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("vr.EnableMotionControllerLateUpdate 1"), nullptr);
		
		InitializeVRComponents();
		
		UE_LOG(LogTemp, Log, TEXT("VRPawn: VR initialization completed successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPawn: HMD not detected, VR features disabled"));
	}
}

void AVRPawn::InitializeVRComponents()
{
	// Initialize Teleport Component
	if (TeleportComponent)
	{
		TeleportComponent->SetTeleportNiagaraSystem(TeleportTraceNiagaraSystem);
	}

	// Initialize Hand Animation Component
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetHandMeshes(HandRight, HandLeft);
		
		// ✅ CRÍTICO: Llamar SetupHandAnimBP DESPUÉS de SetHandMeshes
		HandAnimationComponent->SetupHandAnimBP();
		
		UE_LOG(LogTemp, Warning, TEXT("VRPawn: Hand animation setup initiated"));
	}

	// Initialize Interaction Component
	if (InteractionComponent)
	{
		InteractionComponent->SetMotionControllers(MotionControllerRightGrip, MotionControllerLeftGrip);
	}

	// Initialize VR Input Component
	if (VRInputComponent)
	{
		VRInputComponent->Initialize(TeleportComponent, InteractionComponent, HandAnimationComponent, this);
	}
}

void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Delegate input setup to VRInputComponent
	if (VRInputComponent)
	{
		VRInputComponent->SetupInputBindings(PlayerInputComponent);
	}
}

void AVRPawn::SetupInputMappingContexts()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem())
	{
		if (DefaultMappingContext)
		{
			FModifyContextOptions Options;
			Options.bIgnoreAllPressedKeysUntilRelease = true;
			Subsystem->AddMappingContext(DefaultMappingContext, 0, Options);
			UE_LOG(LogTemp, Log, TEXT("VRPawn: Default mapping context added"));
		}

		if (HandsMappingContext)
		{
			FModifyContextOptions Options;
			Options.bIgnoreAllPressedKeysUntilRelease = true;
			Subsystem->AddMappingContext(HandsMappingContext, 0, Options);
			UE_LOG(LogTemp, Log, TEXT("VRPawn: Hands mapping context added"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VRPawn: Failed to get Enhanced Input Subsystem"));
	}
}

UEnhancedInputLocalPlayerSubsystem* AVRPawn::GetEnhancedInputSubsystem() const
{
	if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
	}
	return nullptr;
}

void AVRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Debug hand bones
	if (bDebugHandBones)
	{
		if (HandAnimationComponent)
		{
			HandAnimationComponent->DebugDrawHandBones(true);
			HandAnimationComponent->DebugDrawHandBones(false);
		}
	}
	
	// ✅ NUEVO: Visualizar radio de agarre
	if (InteractionComponent && GetWorld())
	{
		// Right hand grab sphere
		if (MotionControllerRightGrip)
		{
			FVector RightPos = MotionControllerRightGrip->GetComponentLocation();
			DrawDebugSphere(GetWorld(), RightPos, 30.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f);
			DrawDebugString(GetWorld(), RightPos + FVector(0, 0, 5), TEXT("R GRAB"), 
				nullptr, FColor::White, 0.0f, true, 1.0f);
		}
		
		// Left hand grab sphere
		if (MotionControllerLeftGrip)
		{
			FVector LeftPos = MotionControllerLeftGrip->GetComponentLocation();
			DrawDebugSphere(GetWorld(), LeftPos, 30.0f, 12, FColor::Blue, false, -1.0f, 0, 2.0f);
			DrawDebugString(GetWorld(), LeftPos + FVector(0, 0, 5), TEXT("L GRAB"), 
				nullptr, FColor::White, 0.0f, true, 1.0f);
		}
	}
}

void AVRPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Components will clean themselves up automatically
	Super::EndPlay(EndPlayReason);
}