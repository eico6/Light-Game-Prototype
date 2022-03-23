// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"


// ENUM identical to the ENUM found in 'EnemySpawner.h'
// ENUM used to identify which 'EnemySpawner' instance this particular enemy is assigned.
UENUM(BlueprintType)
enum class EEnemyLabel : uint8
{
	ESL_None 	    UMETA(Displayname = "None"),
	ESL_Label_One	UMETA(Displayname = "Label_One"),
	ESL_LABEL_Two   UMETA(Displayname = "Label_Two"),
	ESL_Label_Three UMETA(Displayname = "Label_Three"),
	ESL_Label_Four  UMETA(Displayname = "Label_Four"),
	ESL_Max		    UMETA(Displayname = "DefaultMax")
};


UCLASS()
class LIGHT_PROTOTYPE_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	// Assign which 'EnemySpawner' this enemy should spawn at
	UPROPERTY(EditAnywhere, Category = "MyVariables | Enums")
	EEnemyLabel EnemyLabel;

	class UMaterialInstanceDynamic* EnemyMaterial;

	// ArenaDetector is communicating with its ArenaManager regarding OverlapEvents
	UPROPERTY(EditAnywhere, Category = "Collision")
	class UBoxComponent* ArenaDetector;

	UPROPERTY(EditAnywhere, Category = "Collision")
	class UBoxComponent* FlashlightDetector;

	UPROPERTY(EditAnywhere, Category = "Collision")
	class UBoxComponent* LaserDetector;

	UPROPERTY(EditAnywhere, Category = "Collision")
	class UBoxComponent* AttackRange;

	UPROPERTY(EditAnywhere, Category = "Collision")
	class UBoxComponent* StartAttackingRange;

	UPROPERTY(EditAnywhere, Category = "Collision")
	class USphereComponent* PlayerDetector;

	UPROPERTY()
	class AMyPlayer* Player;

	UPROPERTY()
	class ACharacter* CharacterCaster;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Pickups")
	TSubclassOf<class APickup> FlashlightWidener;

	UPROPERTY(EditAnywhere, Category = "Pickups")
	TSubclassOf<class APickup> FasterReload;

	UPROPERTY(EditAnywhere, Category = "Pickups")
	TSubclassOf<class APickup> LazerWidener;

	UPROPERTY(EditAnywhere, Category = "Pickups")
	TSubclassOf<class APickup> Health;

	UPROPERTY()
	bool bBeingStunned;//Check if enemy is standing in flashlight

	UPROPERTY(BlueprintReadOnly)
	bool bIsStunned;//Check if enemy is stunned 

	UPROPERTY(BlueprintReadOnly)
	bool bRotateTowardsPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	UPROPERTY(BlueprintReadOnly)
	bool bDead;

	UPROPERTY(BlueprintReadOnly)
	bool bDissolveEnemy;

	bool bWithinAttackRange;

	// If enemy is NOT initialized in spawn pool. If this enemy is placed in the game world by a level designer.
	UPROPERTY(EditAnywhere, Category = "MyVariables | PreSpawner")
	bool bPreSpawnedEnemy;

	// Used for pre spawned enemies. They will only move towards palyer if within range.
	bool bWithinRangeOfPlayer;

	UPROPERTY()
	float TimeStunned;//How long the enemy should be stunned in seconds

	UPROPERTY()//How long it takes to stun enemy in seconds
	float TimeUntilStunned;

	UPROPERTY()
	float TimeInFlashlight;//How long the enemy has been isnide flashlight

	UPROPERTY(BlueprintReadWrite)
	float HerderTopSpeed;// Herder max walk speed

	UPROPERTY(BlueprintReadWrite)
	float AnkelbiterTopSpeed;// Ankelbiter max walk speed

	UPROPERTY()
	float MoveAttackRange;

	UPROPERTY()
	float MovementSpeedReduction;//Decides how much speed enemy should lose by being inside flashlight

	// Time passed since dissolving of enemy started.
	float TimePassed;

	// Amount of seconds before enemy is teleported back to spawn pool. Starting from the time of death
	float TimerForTpEnemy;

	UPROPERTY()
	FVector SpawnPoolLocation;//Location of where non-used enemies will be

	// This is a struct!
	// Timer handle for "teleport back to spawn pool" delay
	FTimerHandle DeathTimer;

	int32 AmountOfMaterials;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Teleport enemy to spawn poll when dead
	void TpEnemyToPool();

	UFUNCTION()
	void ArenaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ArenaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void FlashLightBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void FlashLightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void LaserBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void AttackRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void StartAttackingRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void PlayerDetectorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void PlayerDetectorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void Stunning();

	UFUNCTION()
	void Rally();

	UFUNCTION()
	void Die();

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void EndAttack();

	UFUNCTION(BlueprintCallable)
	void DamagePlayer();

	UFUNCTION()
	void SpawnPowerUp(int32 PowerUpIndex);

	UFUNCTION()
	void SpawnHealth();

	UFUNCTION()
	void DestroyEnemy();

	// 0.0f = no dissolve
	// 1.0f = fullly dissolved
	float AmountToDissolve(float Amount);

};