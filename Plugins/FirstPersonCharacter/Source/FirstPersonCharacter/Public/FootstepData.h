// Copyright Ali El Saleh, 2019

#pragma once

#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "FootstepData.generated.h"

/**
 * Stores an array of sounds and a pointer to a PhysicalMaterial
 */
UCLASS()
class FIRSTPERSONCHARACTER_API UFootstepData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPhysicalMaterial* GetPhysicalMaterial() const {return PhysicalMaterial;}
	TArray<USoundBase*> GetFootstepSounds() const {return Sounds;}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Properties")
		UPhysicalMaterial* PhysicalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Properties")
		TArray<USoundBase*> Sounds;
	
};
