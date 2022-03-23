// Written by Eivind Hobrad Naasen

#include "EnemySpawner.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "EngineUtils.h"
#include "Herder.h"
#include "Ankelbiter.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StartPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartPoint"));
	SetRootComponent(StartPoint);
	StartPoint->bHiddenInGame = true;

	EndPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndPoint"));
	EndPoint->SetupAttachment(GetRootComponent());
	EndPoint->SetRelativeLocation(FVector(500.0f, 0.0f, 300.0f));
	EndPoint->bHiddenInGame = true;

	// Default 'none' value, as a safety if you forgot to assign it in the editor
	SpawnLabel = ESpawnLabel::ESL_None;

	CurrentIndex = 0;
	MaxIndex = 0;
	CurrentWave = 0;
	bEmptyWave = false;
	AmountOfWaves = sizeof(Waves) / sizeof(Waves[0]);

	// Assignes all elements of 'Waves' with the value 0.
	for (int32 i{ 0 }; i < AmountOfWaves; i++)
	{
		Waves[i] = 0;
	}
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	// Safety check
	CurrentWave = 0;

	// 'SpawnInt' and 'EnemyInt' are the enum constants of 'ESpawnLabel' and 'EEnemyLabel' converted
	// over to int32. The reason behind this conversion is explained further down in the if-statement.
	UWorld* CurrentLevel = GetWorld();
	int32 SpawnInt = (int32)SpawnLabel;
	int32 EnemyInt{};

	// Fill up 'Enemy' array with all instances of type 'AEnemy' class.
	// Could also use 'GetAllActorsOfClass' function. This for loop is basically
	// what is inside that function, but I want more control of each element.
	// StaticClass() is a member function in 'UObject' which returns an object of type AEnemy.
	// NOTICE: this is an expensive operation, don't do this every frame.
	for (TActorIterator<AEnemy> Enemy(CurrentLevel, AEnemy::StaticClass()); Enemy; ++Enemy)
	{
		AEnemy* EnemyPtr = *Enemy;
		if (EnemyPtr != NULL)
		{
			// Converting 'EnemyLabel' to int32 and storing it in 'EnemyInt'
			EnemyInt = (int32)EnemyPtr->EnemyLabel;

			// Checks if the enemy is supposed to be spawned at this spawner.
			// This is the reason why I converted the enum constants over to int32;
			// as both sides of the '==' have to be the same data type.
			if (SpawnInt == EnemyInt) Enemies.Add(EnemyPtr);
		}
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemySpawner::ShuffleArray(TArray<AEnemy*> Array)
{
	int32 Size{Array.Num()};
	int32 Index{0};
	AEnemy* Temp;

	// Simple shuffling where I switch spots with the first element and a random element.
	// Then do the same for the seconds element, third, fourth... until it has reached the end.
	while (Index < Size)
	{
		int32 RandomIndex{ UKismetMathLibrary::RandomInteger(Size) };
		Temp = Enemies[Index];
		Enemies[Index] = Enemies[RandomIndex];
		Enemies[RandomIndex] = Temp;
		Index++;
	}
}

void AEnemySpawner::SpawnNextWave()
{
	// Spawn Delay
	float RandomSpawnDelay{0.5f};
	RandomSpawnDelay *= UKismetMathLibrary::RandomFloat();

	// If there are more waves to spawn.
	// This is how the values look like if we are supposed to spawn the third wave:
	//       CurrentWave   = 2
	//       AmountOfWaves = 3
	if (CurrentWave < AmountOfWaves)
	{
		// Resets values for incoming wave
		CurrentIndex = 0;
		MaxIndex = Waves[CurrentWave];

		// Checks if level designer has decided to spawn enemies who don't exist.
		// If this for example is an EnemySpawner with ESpawnLabel = Label_Two, and there are
		// 7 enemies in total with Label_Two located in the spawn pool. 
		// If you then tell the spawner to spawn 8+ enemies, there will be enemies missing of that label.
		if (MaxIndex > Enemies.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("Not enough enemies in pool to spawn %d enemies!"), MaxIndex)
			MaxIndex = 0;
		}
		UE_LOG(LogTemp, Warning, TEXT("Spawns : %i"), MaxIndex)

		// If there are supposed to spawn enemies this wave (only this spawner)
		if (MaxIndex > 0)
		{
			// Shuffle the order of the enemies if there are supposed to spawn more than one enemy this wave
			if (MaxIndex != 1) ShuffleArray(Enemies);

			// Start spawning enemies
			bEmptyWave = false;
			GetWorld()->GetTimerManager().SetTimer(MyTimerHandle, this, &AEnemySpawner::SpawnEnemies, (2.0f + RandomSpawnDelay));
		}
		else
		{
			bEmptyWave = true;
		}
		CurrentWave++; 
	}
	else
	{
		bEmptyWave = true;
	}

}

// Recursive function
void AEnemySpawner::SpawnEnemies()
{
	float RandomSpawnDelay = 1.2f;
	float RandomLaunchForce{};

	UWorld* CurrentLevel = GetWorld();
	if (CurrentIndex < MaxIndex)
	{
		// Spawning enemies in order (starting at index 0) by changing their transform
		// (enemies are already instantiated at start of level, this is to increase performance)
		AEnemy* SpawnedEnemy = Enemies[CurrentIndex];
		if (SpawnedEnemy)
		{
			// Teleport enemy to "spawn location" and set its rotation to be equal to spawner's rotation.
			// "Spawn location" is this spawner's location + offset in in z-direction.
			// This is so that enemies actually spawn below the water, while still being able to keep track 
			// of where they will spawn, based on the location of this spawner's x- and y-values.
			SpawnedEnemy->SetActorLocation(StartPoint->GetComponentLocation() + FVector(0.0f, 0.0f, -150.0f));
			SpawnedEnemy->SetActorRotation(this->GetActorRotation());
			
			// Enables movement for spawned enemy (movement will be disabled while enemy is located in spawn pool)
			SpawnedEnemy->bDead = false;

			// Assign random values for both variables
			RandomSpawnDelay *= UKismetMathLibrary::RandomFloat(); // Will normalize the 'RandomSpawnDelay' randomly
			RandomLaunchForce = UKismetMathLibrary::RandomFloatInRange(0.95f, 1.05f);

			// 'LaunchVector' is calculated (through trial and error) to launch in the correct direction with the correct force
			UCharacterMovementComponent* EnemyMovementComponent;
			FVector LaunchDirection = this->GetActorForwardVector();
			FVector LaunchVector = FVector(28000.0f * LaunchDirection.X, 28000.0f * LaunchDirection.Y, 120000.0f) * RandomLaunchForce;

			// Cast from PawnMovementComponent to CharacterMovementComponent to get access to 'AddImpulse()'
			// I use 'AddImpulse()' to launch the enemy towards the platform at spawn time
			EnemyMovementComponent = Cast<UCharacterMovementComponent>(SpawnedEnemy->GetMovementComponent());
			EnemyMovementComponent->AddImpulse(LaunchVector, false);

			CurrentIndex++;
			// Add a little delay between each spawn with minimum delay being 0.57 seconds and max being 1.77 seconds
			CurrentLevel->GetTimerManager().SetTimer(MyTimerHandle, this, &AEnemySpawner::SpawnEnemies, (0.57f + RandomSpawnDelay));
		}
	}

}

void AEnemySpawner::ResetWaves()
{
	CurrentWave = 0;
	
	// Teleport/reset enemies by killing them
	for (int32 i{ 0 }; i < Enemies.Num(); i++)
	{
		if (Enemies[i]->bPreSpawnedEnemy == false)
		{
			Enemies[i]->bIsStunned = true;
			Enemies[i]->Die();
		}
	}
}