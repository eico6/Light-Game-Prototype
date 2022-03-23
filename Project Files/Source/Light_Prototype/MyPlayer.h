// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "MyPlayer.generated.h"


UENUM(BlueprintType)
enum class ECurrentPowerUp : uint8
{
	ECP_None 		    UMETA(Displayname = "None"),
	ECP_Red_PowerUp		UMETA(Displayname = "Red_PowerUp"),
	ECP_Green_PowerUp	UMETA(Displayname = "Green_PowerUp"),
	ECP_Blue_PowerUp	UMETA(Displayname = "Blue_PowerUp"),

	ECP_Max		        UMETA(Displayname = "DefaultMax")
};


UCLASS()
class LIGHT_PROTOTYPE_API AMyPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PowerUp")
	ECurrentPowerUp CurrentPowerUp;

	UPROPERTY(BlueprintReadWrite)
	FVector LastCheckpoint;

	// Behind/Left/Right/Mid colliders are used for the behvaiour of Herder enemies
	UPROPERTY(EditAnywhere, Category = "AI")
	class USceneComponent* HerderAI; // Only used for sorting the components in the hierarchy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBoxComponent* BehindCollider;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBoxComponent* LeftCollider;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBoxComponent* RightCollider;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBoxComponent* MidCollider;

	// Left and Right points behind player's back.
	UPROPERTY(EditAnywhere, Category = "AI")
	class USceneComponent* ParentOfPoints;
	UPROPERTY(EditAnywhere, Category = "AI")
	class UStaticMeshComponent* LeftPoint;
	UPROPERTY(EditAnywhere, Category = "AI")
	class UStaticMeshComponent* RightPoint;

	// Left/Right point's distance from player
	UPROPERTY(EditAnywhere, Category = "AI")
	float LeftDistance;
	UPROPERTY(EditAnywhere, Category = "AI")
	float RightDistance;

	// Vectors used by Herder AI as destinations for Stage One
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector LeftPointLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector RightPointLocation;

	// Variables for Herder AI behaviour
	float LengthOfTrace;
	bool bRightValid;
	bool bLeftValid;
	bool bPrioritizeReady;
	bool bShouldFlicker;
	bool bJustFlicked;
	UPROPERTY(BlueprintReadOnly)
	bool bIsInCombat;
	FVector LeftForward;
	FVector RightForward;
	FVector DirectionOfTrace;
	FHitResult* HitResult;
	FCollisionQueryParams* TraceParams;

	//Making DeltaTime Global
	UPROPERTY()
	float Time;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* FlashLightCollider;
	UPROPERTY(EditAnywhere, Category = "Collider")
	class USphereComponent* FlashLightPivot;

	UPROPERTY(EditAnywhere, Category = "Collider")
	class UBoxComponent* LaserCollider;
	UPROPERTY(EditAnywhere, Category = "Collider")
	class USphereComponent* LaserPivot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	class USpotLightComponent* Flashlight;

	FTimerHandle MyTimerHandle;

	// TimerHandle for Herder AI Behaviour
	FTimerHandle AITimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	bool bIsCharging;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	bool bHasPowerUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	bool bJustShot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	bool bIsShooting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageFeedback")
	bool bJustTookDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
	int32 PlayerHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageFeedback")
	int32 DamageTaken;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	float FlashLightScaleModifier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	float ShootingTime;


	//PowerUp Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float PowerUpTime;//How long a power-up should last
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float PowerUpTimeLeft;//How much time there is left of a power-up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float ReloadSpeedDefault;//How fast you reload without any upgrades
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float ReloadSpeedCurrent;//How fast you are currently reloading
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float ReloadSpeedUpgraded;//How fast you reload when you have the upgraded reload pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float PowerUpLightScale;//How much the "increase light size" pickup should increace it by. 2 = double
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float PowerUpLaserScale;//How much the "increase finishing move size" pickup should increace it by. 2 = double
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float YScaler;//Extra scaler for y-direction to make fit for lighting

	//Values for charging up your finishingmove
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	float LightReduceScaleMod;//Helps make the flashlight scale down while charing the laser
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	float LightReturnSpeed;//Affects how fast your light returns if you let go before fully charged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	float LaserCharger;//Charges up finishing move
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	float LaserFullyCharged;//When your finishing move is fully charged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserCharge")
	float ColliderLocationOffset;//When your finishing move is fully charged

	//Values for damage invincibility after taking damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageFeedback")
	float TimeRecovering;//How long you have been in "recovery state"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageFeedback")
	float TimeToRecover;//How long it will take you to lose invincibility from taking damage

	//Transforms for the light cone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FVector LightLocationDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FVector LightScaleDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FRotator LightRotationDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FVector LightLocationCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FVector LightScaleCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FRotator LightRotationCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FVector LightLocationPoweredUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FVector LightScalePoweredUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightTransforms")
	FRotator LightRotationPoweredUp;

	//Transforms for the finishing move
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FVector LaserLocationDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FVector LaserScaleDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FRotator LaserRotationDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FVector LaserLocationCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FVector LaserScaleCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FRotator LaserRotationCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FVector LaserLocationPoweredUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FVector LaserScalePoweredUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserTransforms")
	FRotator LaserRotationPoweredUp;

	UPROPERTY(EditAnywhere, Category = "Light")
	TSubclassOf<class ALaserBeamVisual> LaserVisual;

	void PickupEventBiggerFlashlight();
	void PickupEventBiggerLaser();
	void PickupEventFasterReload();
	void LosePowerup();
	void LightBehaviour();
	void TakeDamageTho();
	void Shoot();
	void ChargeUp();
	void LookAtMouse();
	void CooledDown();
	void StopCharging();

	UFUNCTION(BlueprintCallable)
	void SpawnAtLastCheckpoint();

	// Functions for Left/Right points (connected to Herder AI behaviour)
	void ReduceDistance();
	void CheckLeftValid();
	void CheckRightValid();
	void PrioritizationTrue();


	UFUNCTION()
	void LeftOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void RightOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void MidOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void MidOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void BehindOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


};
