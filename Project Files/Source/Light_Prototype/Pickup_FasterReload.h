// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Pickup_FasterReload.generated.h"

/**
 *
 */
UCLASS()
class LIGHT_PROTOTYPE_API APickup_FasterReload : public APickup
{
	GENERATED_BODY()

public:
	APickup_FasterReload();

	UPROPERTY()
	class AMyPlayer* Player;//Player reference


	// DON'T USE UFUNCTION() FOR OVERRIDES 
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
