// Copyright Ali El Saleh, 2019

#include "FPCharacter.h"

#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/InputSettings.h"

AFPCharacter::AFPCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	CameraComponent->bUsePawnControlRotation = true;

	// Other settings
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->JumpZVelocity = 300.0f;
	GetCharacterMovement()->AirControl = 0.1f;
	GetCapsuleComponent()->bReturnMaterialOnMove = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;

	bCanUnCrouch = true;
}

void AFPCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Get access to the input settings
	Input = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

	// Movement setup
	GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = Movement.JumpVelocity;

	// Initialization
	OriginalCameraLocation = CameraComponent->GetRelativeLocation();
	OriginalCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	bCanUnCrouch = true;

	// Footstep setup
	LastLocation = GetActorLocation();
	LastFootstepLocation = GetActorLocation();
	TravelDistance = 0;

	// Input setup
	SetupInputBindings();
}

void AFPCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameraShake();

	UpdateCrouch(DeltaTime);
}

void AFPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis bindings
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AFPCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AFPCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &AFPCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &AFPCharacter::AddControllerPitchInput);

	// Action bindings
	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &AFPCharacter::Jump);
	PlayerInputComponent->BindAction(FName("Jump"), IE_Released, this, &AFPCharacter::StopJumping);
	PlayerInputComponent->BindAction(FName("Run"), IE_Pressed, this, &AFPCharacter::Run);
	PlayerInputComponent->BindAction(FName("Run"), IE_Released, this, &AFPCharacter::StopRunning);
	PlayerInputComponent->BindAction(FName("Crouch"), IE_Pressed, this, &AFPCharacter::StartCrouch);
	PlayerInputComponent->BindAction(FName("Crouch"), IE_Released, this, &AFPCharacter::StopCrouching);
	PlayerInputComponent->BindAction(FName("Interact"), IE_Pressed, this, &AFPCharacter::Interact);
	PlayerInputComponent->BindAction(FName("Escape"), IE_Pressed, this, &AFPCharacter::Quit);
}

void AFPCharacter::Jump()
{
	if (!bIsCrouching)
	{
		Super::Jump();

		// Play jump camera shake
		UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(CameraShakes.JumpShake);
	}
}

void AFPCharacter::Landed(const FHitResult& Hit)
{
	if (!bIsCrouching)
	{
		Super::Landed(Hit);

		// Play jump camera shake
		UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(CameraShakes.JumpShake, 3.0f);

		if (FootstepSettings.bEnableFootsteps)
			PlayFootstepSound();
	}
}

void AFPCharacter::StartCrouch()
{
	if (GetCharacterMovement()->IsMovingOnGround() && bCanUnCrouch)
	{
		bIsCrouching = !bIsCrouching;
		
		GetCharacterMovement()->MaxWalkSpeed = bIsCrouching ? Movement.CrouchSpeed :Movement.WalkSpeed;
		FootstepSettings.Stride = bIsCrouching ? 100.0f : 160.0f;
	}
}

void AFPCharacter::StopCrouching()
{
	// Reset stance when crouch is not in toggle mode
	if (!Movement.bToggleToCrouch && bCanUnCrouch)
	{
		GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
		
		bIsCrouching = false;
		FootstepSettings.Stride = 160.0f;
	}
}

void AFPCharacter::MoveForward(const float AxisValue)
{
	if (Controller)
	{
		FRotator ForwardRotation = Controller->GetControlRotation();

		// Limit pitch rotation
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
			ForwardRotation.Pitch = 0.0f;

		// Find out which way is forward
		const FVector Direction = FRotationMatrix(ForwardRotation).GetScaledAxis(EAxis::X);

		// Apply movement in the calculated direction
		AddMovementInput(Direction, AxisValue);

		if (FootstepSettings.bEnableFootsteps)
		{
			// Continously add to Travel Distance when moving
			if (GetCharacterMovement()->Velocity.Size() > 0.0f && GetCharacterMovement()->IsMovingOnGround())
			{
				TravelDistance += (GetActorLocation() - LastLocation).Size();
				LastLocation = GetActorLocation();
			}
			// Reset when not moving AND if we are falling
			else if (GetCharacterMovement()->IsFalling())
			{
				LastLocation = GetActorLocation();
				TravelDistance = 0.0f;
			}

			// Is it time to play a footstep sound?
			if (GetCharacterMovement()->IsMovingOnGround() && TravelDistance > FootstepSettings.Stride)
			{
				PlayFootstepSound();
				TravelDistance = 0;
			}
		}
	}
}

void AFPCharacter::MoveRight(const float AxisValue)
{
	if (Controller)
	{
		// Find out which way is right
		const FRotator RightRotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(RightRotation).GetScaledAxis(EAxis::Y);

		// Apply movement in the calculated direction
		AddMovementInput(Direction, AxisValue);
	}
}

void AFPCharacter::Run()
{
	if (!bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = Movement.RunSpeed;
		FootstepSettings.Stride = 200.0f;
	}
}

void AFPCharacter::StopRunning()
{
	if (!bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
		FootstepSettings.Stride = 160.0f;
	}
}

void AFPCharacter::UpdateCrouch(const float DeltaTime)
{
	if (bIsCrouching)
	{
		// Smoothly move camera to target location and smoothly decrease the capsule height to fit through small openings
		const FVector NewLocation = FMath::Lerp(CameraComponent->GetRelativeLocation(), FVector(0.0f, 0.0f, 30.0f), Movement.StandToCrouchTransitionSpeed * DeltaTime);
		const float NewHalfHeight = FMath::Lerp(GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), OriginalCapsuleHalfHeight/2.0f, Movement.StandToCrouchTransitionSpeed * DeltaTime);
		
		CameraComponent->SetRelativeLocation(NewLocation);
		GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);

		if (IsBlockedInCrouchStance())
			bCanUnCrouch = false;
		else
			bCanUnCrouch = true;
	}
	else
	{
		// Smoothly move camera back to original location and smoothly increase the capsule height to the original height
		const FVector NewLocation = FMath::Lerp(CameraComponent->GetRelativeLocation(), OriginalCameraLocation, Movement.StandToCrouchTransitionSpeed * DeltaTime);
		const float NewHalfHeight = FMath::Lerp(GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), OriginalCapsuleHalfHeight, Movement.StandToCrouchTransitionSpeed * DeltaTime);

		CameraComponent->SetRelativeLocation(NewLocation);
		GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);
	}
}

bool AFPCharacter::IsBlockedInCrouchStance()
{
	// Raycast up above character
	FHitResult HitResult;
	const FVector RayLength = FVector(0.0f, 0.0f, 130.0f);

	return GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), GetActorLocation() + RayLength, ECC_Visibility);
}

void AFPCharacter::UpdateCameraShake()
{
	if (UGameplayStatics::GetPlayerController(this, 0)->GetPawn()->IsA(AFPCharacter::StaticClass()))
	{
		// Shake camera (Walking shake)
		if (GetVelocity().Size() > 0 && CanJump())
			UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(CameraShakes.WalkShake, 2.0f);
		// Shake camera (breathing shake)
		else
			UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(CameraShakes.IdleShake, 1.0f);
		
		// Shake camera (Run shake)
		if (GetVelocity().Size() > 0 && GetCharacterMovement()->MaxWalkSpeed >= Movement.RunSpeed && CanJump())
			UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(CameraShakes.RunShake, 1.0f);
	}
}

void AFPCharacter::Quit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), Cast<APlayerController>(GetController()), EQuitPreference::Quit, true);
}

void AFPCharacter::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("No functionality, derive from this character and implement this event"))
}

void AFPCharacter::PlayFootstepSound()
{
	GetCharacterMovement()->FindFloor(GetCapsuleComponent()->GetComponentLocation(), FloorResult, false);

	if (FloorResult.bBlockingHit)
	{
		if (IsValid(GetFootstepSound(&FloorResult.HitResult.PhysMaterial)))
		{
			if (bIsCrouching)
				UGameplayStatics::PlaySoundAtLocation(this, GetFootstepSound(&FloorResult.HitResult.PhysMaterial), FloorResult.HitResult.Location, 0.35f);
			else
				UGameplayStatics::PlaySoundAtLocation(this, GetFootstepSound(&FloorResult.HitResult.PhysMaterial), FloorResult.HitResult.Location);
		}
		else
		{
			AActor* FloorActor = FloorResult.HitResult.GetActor();
			if (FloorActor)
				UE_LOG(LogTemp, Warning, TEXT("No physical material found for %s"), *FloorActor->GetName())
		}
	}

	LastFootstepLocation = FloorResult.HitResult.Location;
}

USoundBase* AFPCharacter::GetFootstepSound(TWeakObjectPtr<UPhysicalMaterial>* Surface)
{
	for (auto FootstepMapping : FootstepSettings.Mappings)
	{
		if (FootstepMapping && FootstepMapping->GetPhysicalMaterial() == Surface->Get())
			return FootstepMapping->GetFootstepSounds()[FMath::RandRange(0, FootstepMapping->GetFootstepSounds().Num() - 1)];
	}

	UE_LOG(LogTemp, Warning, TEXT("No footstep sound"))
	return nullptr;
}

void AFPCharacter::SetupInputBindings()
{
	ActionMappings = Input->GetActionMappings();
	AxisMappings = Input->GetAxisMappings();

	// Exit early if we already have inputs set
	if (ActionMappings.Num() > 0 || AxisMappings.Num() > 0)
	{
		if (!bUseCustomKeyMappings)
			ResetToDefaultInputBindings();

		return;
	}
	
	ResetToDefaultInputBindings();
}

void AFPCharacter::ResetInputBindings()
{
	for (const auto& Action : ActionMappings)
		Input->RemoveActionMapping(Action);

	for (const auto& Axis : AxisMappings)
		Input->RemoveAxisMapping(Axis);
}

void AFPCharacter::ResetToDefaultInputBindings()
{
	// Clear all the action and axis mappings
	ResetInputBindings();

	// Populate the map with action and axis mappings
	TArray<FName> ActionNames;
	TArray<FName> AxisNames;
	TArray<FKey> ActionKeys;
	TArray<FKey> AxisKeys;
	TMap<FKey, FName> ActionMap;
	TMap<FKey, FName> AxisMap;
	ActionMap.Add(EKeys::SpaceBar, FName("Jump"));
	ActionMap.Add(EKeys::F, FName("Interact"));
	ActionMap.Add(EKeys::Escape, FName("Escape"));
	ActionMap.Add(EKeys::LeftShift, FName("Run"));
	ActionMap.Add(EKeys::LeftControl, FName("Crouch"));
	ActionMap.Add(EKeys::C, FName("Crouch"));
	AxisMap.Add(EKeys::MouseX, FName("Turn"));
	AxisMap.Add(EKeys::MouseY, FName("LookUp"));
	AxisMap.Add(EKeys::W, FName("MoveForward"));
	AxisMap.Add(EKeys::S, FName("MoveForward"));
	AxisMap.Add(EKeys::A, FName("MoveRight"));
	AxisMap.Add(EKeys::D, FName("MoveRight"));

	// Lambda function to set the new action mappings
	const auto SetActionMapping = [&](const FName Name, const FKey Key)
	{
		FInputActionKeyMapping ActionMapping;
		ActionMapping.ActionName = Name;
		ActionMapping.Key = bUseCustomKeyMappings ? EKeys::NAME_KeyboardCategory : Key;
		Input->AddActionMapping(ActionMapping);
	};

	// Lambda function to set the new axis mappings
	const auto SetAxisMapping = [&](const FName Name, const FKey Key, const float Scale)
	{
		FInputAxisKeyMapping AxisMapping;
		AxisMapping.AxisName = Name;
		AxisMapping.Key = bUseCustomKeyMappings ? EKeys::NAME_KeyboardCategory : Key;
		AxisMapping.Scale = Scale;
		Input->AddAxisMapping(AxisMapping);
	};

	// Loop through the entire ActionMap and assign action inputs
	ActionMap.GenerateKeyArray(ActionKeys);
	ActionMap.GenerateValueArray(ActionNames);
	for (int32 i = 0; i < ActionMap.Num(); i++)
	{
		SetActionMapping(ActionNames[i], ActionKeys[i]);
	}

	// Loop through the entire AxisMap and assign axis inputs
	AxisMap.GenerateKeyArray(AxisKeys);
	AxisMap.GenerateValueArray(AxisNames);
	for (int32 i = 0; i < AxisMap.Num(); i++)
	{
		const bool bIsNegativeScale = AxisKeys[i] == EKeys::S || AxisKeys[i] == EKeys::A || AxisKeys[i] == EKeys::MouseY;

		SetAxisMapping(AxisNames[i], AxisKeys[i], bIsNegativeScale ? -1.0f : 1.0f);
	}

	// Save to input config file
	Input->SaveKeyMappings();

	// Update in Project Settings -> Engine -> Input
	Input->ForceRebuildKeymaps();
}
