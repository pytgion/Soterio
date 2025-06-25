// Copyright Epic Games, Inc. All Rights Reserved.

#include "SoterioCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "BladesmithController.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASoterioCharacter

ASoterioCharacter::ASoterioCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 50.0f;
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, 0.0f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ASoterioCharacter::BeginPlay()
{
	Super::BeginPlay();

	AnvilMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Controls/IMC_Anvil.IMC_Anvil'"));
	ForgeMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Controls/IMC_Forge.IMC_Forge'"));
	GrindMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Controls/IMC_Grind.IMC_Grind'"));
}

FHitResult ASoterioCharacter::PerformRayFromCharacter()
{
	FVector WorldLocation, WorldDirection;
	WorldDirection = FVector(-5, 0, 0);
	float TraceDistance = 100.0f;
	FVector StartLocation = GetActorLocation() + FVector(0, 0, 15);
	FVector EndLocation = StartLocation + (GetActorForwardVector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		CollisionParams
	);


#if WITH_EDITOR
	if (bHit && HitResult.GetActor()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Ray Called! Hit Actor: %s"), *HitResult.GetActor()->GetFName().ToString());
		DrawDebugLine(
			GetWorld(),
			StartLocation,
			EndLocation,
			FColor::Green,
			false,
			1.0f
		);
	}
	else
	{
		DrawDebugLine(
			GetWorld(),
			StartLocation,
			EndLocation,
			FColor::Red,
			false,
			1.0f
		);
		UE_LOG(LogTemp, Warning, TEXT("Ray Called! No Actor Hit."));
	}
#endif

	return HitResult;
}


void ASoterioCharacter::GameModeSwitch()
{
	FHitResult HitResult = PerformRayFromCharacter();

	if (HitResult.bBlockingHit && HitResult.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Broadcasting NewGameMode for Actor: %s"), *HitResult.GetActor()->GetFName().ToString());
		SwitchGameControls(HitResult);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Actor hit! Cannot broadcast."));
	}
}

void ASoterioCharacter::SwitchGameControls(FHitResult Result)
{
	if (Result.GetActor()->Tags.Contains(FName("Anvil")))
	{
		OnGameModeChanged.Broadcast(ES_GameMode::Anvil);
	}
	else if (Result.GetActor()->Tags.Contains(FName("Forge")))
	{
		OnGameModeChanged.Broadcast(ES_GameMode::Forge);
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void ASoterioCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->ClearActionBindings();
		//// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASoterioCharacter::ResetDefaultControls);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoterioCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoterioCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASoterioCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASoterioCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASoterioCharacter::ClearAllControls()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->ClearActionBindings();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found on %s during ClearAllControls"), *GetName());
	}
}

void ASoterioCharacter::BindAnvilControls()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem)
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(AnvilMappingContext, 0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get EnhancedInputLocalPlayerSubsystem"));
		return;
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->ClearActionBindings();

		// Exit Anvil
		EnhancedInputComponent->BindAction(IA_Anvil_Exit, ETriggerEvent::Triggered, this, &ASoterioCharacter::GameModeSwitch);

		// Movement
		EnhancedInputComponent->BindAction(IA_Anvil_Hit, ETriggerEvent::Triggered, this, &ASoterioCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoterioCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No EnhancedInputComponent found in character '%s'"), *GetName());
	}

	CheckInputBindings(); // optional for debug
}


void ASoterioCharacter::CheckInputBindings()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputSubsystem is not valid."));
		return;
	}

	if (InputSubsystem->HasMappingContext(AnvilMappingContext))
	{
		UE_LOG(LogTemp, Warning, TEXT("ANVIL IMC FOUND"));
	}
}

void ASoterioCharacter::BindForgeControls()
{
}

void ASoterioCharacter::BindGrindControls()
{
}

void ASoterioCharacter::ResetDefaultControls()
{
	UE_LOG(LogTemp, Warning, TEXT("Shit Called"));
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	TObjectPtr<class UInputComponent> PlayerInputComponent = GetWorld()->GetFirstPlayerController()->InputComponent;

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->ClearActionBindings();
		//// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASoterioCharacter::GameModeSwitch);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoterioCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoterioCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}
