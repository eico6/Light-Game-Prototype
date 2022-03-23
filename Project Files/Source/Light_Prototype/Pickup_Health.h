// Written by Eivind Hobrad Naasen

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Pickup_Health.generated.h"

/**
 * 
 */
UCLASS()
class LIGHT_PROTOTYPE_API APickup_Health : public APickup
{
	GENERATED_BODY()
	
public:
	APickup_Health();

	class AMyPlayer* PlayerRef;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	FVector DeltaLevitate;

	FVector SpawnLocation;

	float TimePassed;

	// Another TimePassed variable, put only used for dimming of LightSource
	//float DimmingTimer;

	//bool bDimLight;

};
