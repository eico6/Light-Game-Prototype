// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class LIGHT_PROTOTYPE_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Collider")
	class UBoxComponent* Collider;

	UPROPERTY(EditAnywhere, Category = "Particles")
	class UParticleSystem* ParticleOnPickup;

	UPROPERTY(EditAnywhere, Category = "Light")
	class UPointLightComponent* LightSource;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// For OnOverlap functions to work properly, you need to expose them to the reflection system ( UFUNTION() )
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float PitchValue;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float YawValue;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float RollValue;

};
