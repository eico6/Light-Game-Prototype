// Written by Eivind Hobrad Naasen 

#define COLLISION_AIAREA ECC_GameTraceChannel3

#include "ArenaManager.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnemySpawner.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyPlayer.h"

// Sets default values
AArenaManager::AArenaManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EmptyRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("EmptyRootComp"));
	RootComponent = EmptyRootComp;

	// ArenaVolume will keep track of: amount of enemies currently on level
	ArenaVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ArenaVolume"));
	ArenaVolume->SetBoxExtent(FVector(500.0f, 700.0f, 200.0f));
	SetRootComponent(GetRootComponent());

	// Make the collision only trigger OverlapEvents whenever the player collides
	EnterArenaCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("EnterArenaCollider"));
	EnterArenaCollider->SetupAttachment(GetRootComponent());
	EnterArenaCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnterArenaCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	EnterArenaCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EnterArenaCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Set position and mesh in blueprint (because it's different for each platform). Level designer's work
	EnterPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnterPlatform"));
	EnterPlatform->SetupAttachment(GetRootComponent());

	// Set position and mesh in blueprint (because it's different for each platform). Level designer's work
	ExitPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExitPlatform"));
	ExitPlatform->SetupAttachment(GetRootComponent());

	// Adjust size and mesh in blueprint.
	MainPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainPlatform"));
	MainPlatform->SetupAttachment(GetRootComponent());

	// Collision volume for Left/Right point's raycast to determine where Herders can walk.
	HerderNavigatableArea = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RaycastDetector"));
	HerderNavigatableArea->SetupAttachment(GetRootComponent());
	HerderNavigatableArea->SetRelativeScale3D(FVector(10.0f, 14.0f, 3.0f));
	HerderNavigatableArea->SetRelativeLocation(FVector(0.0f, 0.0f, -900.0f));
	HerderNavigatableArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HerderNavigatableArea->SetCollisionResponseToChannel(ECollisionChannel::COLLISION_AIAREA, ECollisionResponse::ECR_Block);
	HerderNavigatableArea->bHiddenInGame = true;


	// Assign default values for the encounter
	bEncounterComplete = false;
	bEncounterStarted = false;
	bLowerPlatforms = false;
	bRaisePlatforms = false;
	CalculatedEnter = 0.0f;
	CalculatedExit = 0.0f;
	AmountOfSpawners = 0;
	EnterAltitude = 3.0f;
	ExitAltitude = 3.0f;
	bJustReset = false;
	TimePassed = 0.0f;
	EnemiesLeft = 0;
}

// Called when the game starts or when spawned
void AArenaManager::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<AMyPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	for (int32 i{ 0 }; i < AmountOfSpawners; i++)
	{
		// This will most likely cause a crash though
		if (EnemySpawners[i] == NULL) UE_LOG(LogTemp, Error, TEXT("ArenaManager: can't find reference to spawner ..."));
	}

	// AmountOfSpawners will be equal to the number of spawners assigned to 'EnemySpawners' TArray
	AmountOfSpawners = EnemySpawners.Num();

	// Enabling detection
	EnterArenaCollider->OnComponentBeginOverlap.AddDynamic(this, &AArenaManager::OnOverlapBegin);
}

// Called every frame
void AArenaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If player died and encounter started, but did not complete it
	if ((PlayerRef->PlayerHealth <= 0 && bEncounterStarted == true && bEncounterComplete == false) && (bJustReset == false))
	{
		bJustReset = true;
		ResetEncounter();
	}
	else
	{
		bJustReset = false;
	}

	if (bLowerPlatforms == true)
	{
		// Lower platforms
		// The altitude of Enter/Exit platforms are set in editor by level designers
		TimePassed += DeltaTime;
		CalculatedEnter = (-EnterAltitude) * UKismetMathLibrary::Cos(TimePassed - (UKismetMathLibrary::GetPI() / 2));
		CalculatedExit = (-ExitAltitude) * UKismetMathLibrary::Cos(TimePassed - (UKismetMathLibrary::GetPI() / 2));
		EnterPlatform->SetWorldLocation(EnterPlatform->GetComponentLocation() + FVector(0.0f, 0.0f, CalculatedEnter*DeltaTime*60.0f));
		ExitPlatform->SetWorldLocation(ExitPlatform->GetComponentLocation() + FVector(0.0f, 0.0f, CalculatedExit*DeltaTime * 60.0f));
		if (TimePassed >= (UKismetMathLibrary::GetPI() / 2)) bLowerPlatforms = false;
	}

	if (bRaisePlatforms == true)
	{
		// Raise platforms
		// The altitude of Enter/Exit platforms are set in editor by level designers
		TimePassed += DeltaTime;
		CalculatedEnter = (-EnterAltitude) * UKismetMathLibrary::Cos(TimePassed + (UKismetMathLibrary::GetPI()));
		CalculatedExit = (-ExitAltitude) * UKismetMathLibrary::Cos(TimePassed + (UKismetMathLibrary::GetPI()));
		EnterPlatform->SetWorldLocation(EnterPlatform->GetComponentLocation() + FVector(0.0f, 0.0f, CalculatedEnter*DeltaTime * 60.0f));
		ExitPlatform->SetWorldLocation(ExitPlatform->GetComponentLocation() + FVector(0.0f, 0.0f, CalculatedExit*DeltaTime * 60.0f));
		if (TimePassed >= (UKismetMathLibrary::GetPI() / 2)) bRaisePlatforms = false;
	}
}

// Will be called by enemies
void AArenaManager::IncrementEnemies()
{
	EnemiesLeft++;
}

// Will be called by enemies
void AArenaManager::DecrementEnemies()
{
	EnemiesLeft--;
	UE_LOG(LogTemp, Warning, TEXT("Enemies Left: %i"), EnemiesLeft)

	if (EnemiesLeft <= 0 && bEncounterStarted)
	{
		int32 Counter{0};

		// Spawn next wave for each spawner
		for (int32 i{ 0 }; i < AmountOfSpawners; i++)
		{
			EnemySpawners[i]->SpawnNextWave();

			// If the spawner had no one to spawn this wave
			if (EnemySpawners[i]->bEmptyWave == true) Counter++;
		}

		// If all of the spawners had an empty wave (no one to spawn), then consider encounter complete.
		if ((Counter == AmountOfSpawners && !bEncounterComplete) && bEncounterStarted)
		{
			UE_LOG(LogTemp, Warning, TEXT("Encounter COMPLETE"))
			PlayerRef->LastCheckpoint = MainPlatform->GetComponentLocation() + FVector(0.0f, 0.0f, 600.0f);
			TimePassed = 0.0f;
			bEncounterComplete = true;
			bRaisePlatforms = true;
			bLowerPlatforms = false; // Make it possible to exit encounter area
			EnterArenaCollider->DestroyComponent();

			PlayerRef->bIsInCombat = false;
		}
	}

}

void AArenaManager::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bEncounterStarted == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting encounter ..."))

		// Start spawning enemies
		for (int32 i{ 0 }; i < AmountOfSpawners; i++)
		{
			EnemySpawners[i]->SpawnNextWave();
		}

		// Force player to fight enemies by lowering platforms
		TimePassed = 0.0f;
		bRaisePlatforms = false;
		bLowerPlatforms = true;
		bEncounterStarted = true;

		PlayerRef->bIsInCombat = true;
	}
}

void AArenaManager::ResetEncounter()
{
	bEncounterStarted = false;
	bEncounterComplete = false;
	bEncounterStarted = false;
	bLowerPlatforms = false;
	bRaisePlatforms = true;

	PlayerRef->bIsInCombat = false;

	for (int32 i{ 0 }; i < AmountOfSpawners; i++)
	{
		EnemySpawners[i]->ResetWaves();
	}

	TimePassed = 0.0f;
	EnemiesLeft = 0;
}