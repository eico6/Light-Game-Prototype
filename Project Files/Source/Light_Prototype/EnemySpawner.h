// Written by Eivind Hobrad Naasen

#pragma once

#include "Enemy.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"


// ENUM identical to the ENUM in 'Enemy.h' where the these spawners will check if an enemy instance
// is assigned the same integer value (same label). If identical: the enemy should be spawned by this spawner.
UENUM(BlueprintType)
enum class ESpawnLabel : uint8
{
	ESL_None	    UMETA(Displayname = "None"),
	ESL_Label_One	UMETA(Displayname = "Label_One"),
	ESL_LABEL_Two   UMETA(Displayname = "Label_Two"),
	ESL_Label_Three UMETA(Displayname = "Label_Three"),
	ESL_Label_Four  UMETA(Displayname = "Label_Four"),
	ESL_Max		    UMETA(Displayname = "DefaultMax")
};


UCLASS()
class LIGHT_PROTOTYPE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemySpawner();

	UPROPERTY(EditAnywhere, Category = "Visual of Points")
	class UStaticMeshComponent* EndPoint;

	UPROPERTY(EditAnywhere, Category = "Visual of Points")
	class UStaticMeshComponent* StartPoint;

	// What group of enemies this spawner should spawn. Assignment is made in editor.
	UPROPERTY(EditAnywhere, Category = "MyVariables | Enums")
	ESpawnLabel SpawnLabel;

	// Array filled up with instances to spawn
	TArray<AEnemy*> Enemies;

	// This is a struct, not a class
	// TimerHandle to track time processes
	FTimerHandle MyTimerHandle;

	int32 MaxIndex;

	int32 CurrentWave;

	int32 CurrentIndex;

	int32 AmountOfWaves;

	// Number of waves can be changed by just increasing the number of elements.
	// Waves will continue to spawn as long as there are at least one enemy to spawn in CurrentWave,
	// so you can change the amount of waves to 3000, and it will still work.
	UPROPERTY(EditAnywhere, Category = "MyVariables | Waves")
	int32 Waves[5];

	bool bEmptyWave;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShuffleArray(TArray<AEnemy*> Array);

	// Spawn next enemy in line
	void SpawnEnemies();

	void SpawnNextWave();

	void ResetWaves();

};
