// Written by Eivind Hobrad Naasen 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaManager.generated.h"


UCLASS()
class LIGHT_PROTOTYPE_API AArenaManager : public AActor
{
	GENERATED_BODY()
	
	// ArenaManager is used per enemy encounter platform.
	// Will keep count of amount of enemies left and other important values.

public:	
	// Sets default values for this actor's properties
	AArenaManager();

	// Simple root component
	class USceneComponent* EmptyRootComp;

	// Volume to keep count of enemies
	UPROPERTY(EditAnywhere, Category = "Arena")
	class UBoxComponent* ArenaVolume;

	// Collider to detect when player enters the encounter platform
	UPROPERTY(EditAnywhere, Category = "Arena")
	class UBoxComponent* EnterArenaCollider;

	// Enter/Exit Platforms should be lowered/raised when entering the arena
	UPROPERTY(EditAnywhere, Category = "Arena")
	class UStaticMeshComponent* EnterPlatform;
	UPROPERTY(EditAnywhere, Category = "Arena")
	class UStaticMeshComponent* ExitPlatform;
	UPROPERTY(EditAnywhere, Category = "Arena")
	class UStaticMeshComponent* MainPlatform;

	// Collision volume used for Left/Right point's raycast "goal". Basically a raycast detector.
	// Decided to make it as a mesh, and not a BoxCollider. Because if the MainPlatform's mesh is of a more complex
	// geometry, you can assign the same mesh for 'RaycastDetector'. It will then "block" out the encounter automatically,
	// for a more accurate calculation.
	// Why I'm not just using 'MainPlatform' as the raycast detector? Because this gives me more control of where the
	// line traces should be able to detect the volume.
	UPROPERTY(EditAnywhere, Category = "Arena")
	class UStaticMeshComponent* HerderNavigatableArea;

	// Fill up array with the spawners which should be affected by this manager
	UPROPERTY(EditAnywhere, Category = "My Variables")
	TArray<class AEnemySpawner*> EnemySpawners;

	// Player reference
	class AMyPlayer* PlayerRef;

	int32 EnemiesLeft;
	int32 AmountOfSpawners;
	float TimePassed;

	// Distance EnterPlatform needs to travel when raising/lowering. Reasonable values range: 2.0 - 6.0
	UPROPERTY(EditAnywhere, Category = "My Variables")
	float EnterAltitude;

	// Distance ExitPlatform needs to travel when raising/lowering. Reasonable values range: 2.0 - 6.0
	UPROPERTY(EditAnywhere, Category = "My Variables")
	float ExitAltitude;

	// Calculated Z-values for lowering and raising enter/exit platforms
	float CalculatedEnter, CalculatedExit;

	bool bLowerPlatforms;
	bool bRaisePlatforms;
	bool bJustReset;
	UPROPERTY(BlueprintReadOnly)
	bool bEncounterStarted;
	UPROPERTY(BlueprintReadOnly)
	bool bEncounterComplete;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function will be called whenever an enemy begins collision with 'ArenaVolume'
	void IncrementEnemies();

	// Function will be called whenever an enemy ends collision with 'ArenaVolume'
	void DecrementEnemies();

	// If player dies, encounter should start over when player returns
	void ResetEncounter();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
