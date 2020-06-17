// Copyright Ali El Saleh, 2019

#pragma once

#include "GameFramework/Character.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerInput.h"

#include "FPCharacter.generated.h"

USTRUCT()
struct FCameraShakes
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "A camera shake to play while in an idle state"), Category = "Shakes")
		TSubclassOf<class UCameraShake> IdleShake;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "A camera shake to play while walking"), Category = "Shakes")
		TSubclassOf<class UCameraShake> WalkShake;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "A camera shake to play while running"), Category = "Shakes")
		TSubclassOf<class UCameraShake> RunShake;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "A camera shake to play when player has jumped"), Category = "Shakes")
		TSubclassOf<class UCameraShake> JumpShake;
};

USTRUCT()
struct FFootstepSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Footstep", meta = (ToolTip = "Enable/Disable the ability to play footsteps?"))
		bool bEnableFootsteps = true;

	UPROPERTY(EditInstanceOnly, Category = "Footstep", meta = (EditCondition = "bEnableFootsteps", ToolTip = "How many units should the character travel unitl we play the next footstep sound? (Distance between footsteps) Lower=More Frequently, Higher=Less Frequently"))
		float Stride = 160.0f;

	UPROPERTY(EditInstanceOnly, Category = "Footstep", meta = (EditCondition = "bEnableFootsteps", ToolTip = "An array of footstep data assets to play depending on the material the character is moving on"))
		TArray<class UFootstepData*> Mappings;
};

USTRUCT()
struct FFirstPersonMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Movement", meta = (ClampMin=1.0f, ClampMax=10000.0f, ToolTip = "The normal movement speed"))
		float WalkSpeed = 300.0f;

	UPROPERTY(EditInstanceOnly, Category = "Movement", meta = (ClampMin=1.0f, ClampMax=10000.0f, ToolTip = "The movement speed while crouching"))
		float CrouchSpeed = 150.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Movement", meta = (ClampMin=1.0f, ClampMax=10000.0f, ToolTip = "The movement speed while running"))
		float RunSpeed = 500.0f;

	UPROPERTY(EditInstanceOnly, Category = "Movement", meta = (ClampMin=1.0f, ClampMax=10000.0f, ToolTip = "The intial jump velocity (vertical acceleration)"))
		float JumpVelocity = 300.0f;

	UPROPERTY(EditInstanceOnly, Category = "Movement", meta = (ClampMin=1.0f, ClampMax=1000.0f, ToolTip = "How long does it take to enter the crouch stance?"))
		float StandToCrouchTransitionSpeed = 10.0f;

	UPROPERTY(EditInstanceOnly, Category = "Movement", meta = (ToolTip = "Enable/Disable the ability to toggle crouch when the crouch key is pressed?"))
		bool bToggleToCrouch = false;
};

USTRUCT()
struct FFirstPersonCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Camera", DisplayName = "Sensitivity (Yaw)", meta = (ClampMin=0.0f, UIMax=100.0f, ToolTip = "The sensitivity of the horizontal camera rotation (Yaw). Lower values = Slower camera rotation. Higher values = Faster camera rotation"))
		float SensitivityX = 50.0f;

	UPROPERTY(EditInstanceOnly, Category = "Camera", DisplayName = "Sensitivity (Pitch)", meta = (ClampMin=0.0f, UIMax=100.0f, ToolTip = "The sensitivity of the vertical camera rotation (Pitch). Lower values = Slower camera rotation. Higher values = Faster camera rotation"))
        float SensitivityY = 50.0f;

	UPROPERTY(EditInstanceOnly, Category = "Camera", meta = (ClampMin="-360.0", ClampMax=360.0f, ToolTip = "The minimum view pitch, in degrees. Some examples are 300.0, 340.0, -90.0, 270.0 or 0.0"))
        float MinPitch = -90.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Camera", meta = (ClampMin="-360.0", ClampMax=360.0f, ToolTip = "The maximum view pitch, in degrees. Some examples are 20.0, 45.0, 90.0 or 0.0"))
        float MaxPitch = 90.0f;
};

UCLASS()
class FIRSTPERSONCHARACTER_API AFPCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPCharacter();

protected:
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Jump() override;
	void Landed(const FHitResult& Hit) override;
	void PossessedBy(AController* NewController) override;
	void StartCrouch();
	void StopCrouching();
	void SetupInputBindings();
	void ResetInputBindings();
	void ResetToDefaultInputBindings();

	void AddControllerYawInput(float Value) override;
	void AddControllerPitchInput(float Value) override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);

	virtual void Quit();

	void PlayFootstepSound();
	USoundBase* GetFootstepSound(TWeakObjectPtr<UPhysicalMaterial>* Surface);

	void UpdateCrouch(float DeltaTime);
	bool IsBlockedInCrouchStance();
	void UpdateCameraShake();

	UFUNCTION()
		virtual void Interact();
	UFUNCTION()
		void Run();
	UFUNCTION()
		void StopRunning();

	UPROPERTY()
		class UCameraComponent* CameraComponent;
	
	UPROPERTY(EditInstanceOnly, Category = "First Person Settings", meta = (ToolTip = "Enable this setting if you want to change the keys for specific action or axis mappings. Go to Project Settings -> Engine -> Input to update your inputs."))
		bool bUseCustomKeyMappings = false;

	UPROPERTY(EditInstanceOnly, Category = "First Person Settings", meta = (ToolTip = "Adjust these camera settings to your liking"))
		FFirstPersonCameraSettings Camera;
	
	UPROPERTY(EditInstanceOnly, Category = "First Person Settings", meta = (ToolTip = "Adjust these movement settings to your liking"))
		FFirstPersonMovementSettings Movement;

	UPROPERTY(EditInstanceOnly, Category = "First Person Settings", meta = (ToolTip = "Adjust these footstep settings to your liking"))
		FFootstepSettings FootstepSettings;

	UPROPERTY(EditInstanceOnly, Category = "First Person Settings", meta = (ToolTip = "Add one of your custom camera shakes to the corresponding slot"))
		FCameraShakes CameraShakes;

	class UInputSettings* Input{};

private:
	APlayerController* PlayerController;
	
	// Footstep variables
	FVector LastFootstepLocation;
	FVector LastLocation;
	FFindFloorResult FloorResult;
	float TravelDistance = 0.0f;

	float OriginalCapsuleHalfHeight{};
	FVector OriginalCameraLocation; // Relative

	bool bCanUnCrouch{};
	bool bIsCrouching{};

	TArray<FInputActionKeyMapping> ActionMappings;
	TArray<FInputAxisKeyMapping> AxisMappings;
};
