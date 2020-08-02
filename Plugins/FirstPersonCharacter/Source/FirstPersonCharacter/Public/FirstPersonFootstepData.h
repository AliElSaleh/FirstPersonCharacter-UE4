// Copyright Ali El Saleh, 2020

#pragma once

#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "FirstPersonFootstepData.generated.h"

/**
 * Stores an array of sounds and a reference to a PhysicalMaterial
 */
UCLASS(BlueprintType)
class FIRSTPERSONCHARACTER_API UFirstPersonFootstepData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Footstep Data")
	UPhysicalMaterial* GetPhysicalMaterial() const { return PhysicalMaterial; }

	UFUNCTION(BlueprintPure, Category = "Footstep Data")
	TArray<USoundBase*> GetFootstepSounds() const { return Sounds; }
	
	UFUNCTION(BlueprintPure, Category = "Footstep Data")
	float GetFootstepStride_Walk() const { return WalkStride; }
	
	UFUNCTION(BlueprintPure, Category = "Footstep Data")
	float GetFootstepStride_Run() const { return RunStride; }
	
	UFUNCTION(BlueprintPure, Category = "Footstep Data")
	float GetFootstepStride_Crouch() const { return CrouchStride; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Properties")
	UPhysicalMaterial* PhysicalMaterial;

	// Walk stride setting. How many units should the character travel unitl we play the next footstep sound? (Distance between footsteps) Lower=More Frequently, Higher=Less Frequently
	UPROPERTY(EditInstanceOnly, Category = "Footstep", meta = (EditCondition = "bEnableFootsteps"))
	float WalkStride = 160.0f;

	// Crouch stride setting. How many units should the character travel unitl we play the next footstep sound? (Distance between footsteps) Lower=More Frequently, Higher=Less Frequently
	UPROPERTY(EditInstanceOnly, Category = "Footstep", meta = (EditCondition = "bEnableFootsteps"))
	float CrouchStride = 120.0f;
	
	// Run stride setting. How many units should the character travel unitl we play the next footstep sound? (Distance between footsteps) Lower=More Frequently, Higher=Less Frequently
	UPROPERTY(EditInstanceOnly, Category = "Footstep", meta = (EditCondition = "bEnableFootsteps"))
	float RunStride = 90.0f;
		
	UPROPERTY(EditDefaultsOnly, Category = "Properties")
	TArray<USoundBase*> Sounds;
};
