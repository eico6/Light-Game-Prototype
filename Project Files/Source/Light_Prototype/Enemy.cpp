// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen


#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Pickup_LaserWidener.h"
#include "Pickup_FasterReload.h"
#include "FlashlightWidener.h"
#include "Pickup_Health.h"
#include "Engine/World.h"
#include "ArenaManager.h"
#include "TimerManager.h"
#include "Ankelbiter.h"
#include "MyPlayer.h"
#include "Herder.h"
#include "Pickup.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create component for Arena detector
	ArenaDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("ArenaDetector"));
	ArenaDetector->SetupAttachment(GetRootComponent());

	//Create component for Flashlight Detector
	FlashlightDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("FlashlightDetector"));
	FlashlightDetector->SetupAttachment(GetRootComponent());

	//Create component for Laser Detector
	LaserDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("LaserDetector"));
	LaserDetector->SetupAttachment(GetRootComponent());

	// Create component for Attack Range Collider
	// Attributes will be different for each enemy, so box extent etc. will be modified in BP
	AttackRange = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackRange"));
	AttackRange->SetupAttachment(GetRootComponent());
	AttackRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttackRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create component for Start Attacking Range Collider
	// Attributes will be different for each enemy, so box extent etc. will be modified in BP
	StartAttackingRange = CreateDefaultSubobject<UBoxComponent>(TEXT("StartAttackingRange"));
	StartAttackingRange->SetupAttachment(GetRootComponent());
	StartAttackingRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	StartAttackingRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Collider only used by pre spawned enemies
	PlayerDetector = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetector"));
	PlayerDetector->SetupAttachment(GetRootComponent());
	PlayerDetector->SetSphereRadius(1150.0f);
	PlayerDetector->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PlayerDetector->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Default 'none' value, as a safety if you forgot to assign it in the editor
	EnemyLabel = EEnemyLabel::ESL_None;

	TimeStunned = 3;
	TimePassed = 0.0f;
	TimeUntilStunned = 2;
	TimeInFlashlight = 0;
	AmountOfMaterials = 2;
	HerderTopSpeed = 350.0f;
	TimerForTpEnemy = 0.930f;
	AnkelbiterTopSpeed = 250.0f;
	MovementSpeedReduction = 60;
	bWithinRangeOfPlayer = true;
	bRotateTowardsPlayer = true;
	bWithinAttackRange = false;
	bPreSpawnedEnemy = false;
	bDissolveEnemy = false;
	bAttacking = false;
	bDead = true;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Start looking for OverlapEvents
	ArenaDetector->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::ArenaBeginOverlap);
	ArenaDetector->OnComponentEndOverlap.AddDynamic(this, &AEnemy::ArenaEndOverlap);

	FlashlightDetector->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::FlashLightBeginOverlap);
	FlashlightDetector->OnComponentEndOverlap.AddDynamic(this, &AEnemy::FlashLightEndOverlap);

	LaserDetector->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::LaserBeginOverlap);
	
	AttackRange->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AttackRangeEndOverlap);

	StartAttackingRange->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::StartAttackingRangeBeginOverlap);

	if (bPreSpawnedEnemy == false)
	{
		SpawnPoolLocation = GetActorLocation();
	}
	else
	{
		PlayerDetector->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::PlayerDetectorBeginOverlap);
		PlayerDetector->OnComponentEndOverlap.AddDynamic(this, &AEnemy::PlayerDetectorEndOverlap);
		bWithinRangeOfPlayer = false;
		bDead = false;
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bBeingStunned)
	{
		if (TimeInFlashlight < TimeUntilStunned)
		{
			TimeInFlashlight += DeltaTime;
			Stunning();
		}
		else
		{
			Stunning();
		}
	}
	else
	{
		TimeInFlashlight -= DeltaTime;

		if (TimeInFlashlight <= 0)
		{
			TimeInFlashlight = 0;
			Rally();
		}
	}

	// Adding gradient effect to material for feedback to player, regarding weakening progression. Also for the dissolve effect.
	// VERY IMPORTANT: when the real materials are imported and used, change the material from inside the node in 'MEnemyDefault'. 
	// Don't swap the whole material class itself. This is to still keep the blueprint functionalities within the material.
	for (int32 i{ 0 }; i < AmountOfMaterials; i++) 
	{
		// If there are materials to dissolve/weaken
		if ((GetMesh()->GetNumMaterials() - 1) >= i)
		{
			EnemyMaterial = GetMesh()->CreateDynamicMaterialInstance(i);

			// Weakening gradient effect and dissolve effect
			EnemyMaterial->SetScalarParameterValue(FName("WeakenedAmount"), (TimeInFlashlight / TimeUntilStunned));
			if (bDissolveEnemy)
			{
				EnemyMaterial->SetScalarParameterValue(FName("DissolveAmount"), AmountToDissolve(TimePassed * (1 / TimerForTpEnemy)));
				TimePassed += DeltaTime;
			}
		}
		else
		{
			break;
		}
	}

	// If enemy is attacking, the enemy should not be able to move
	// Or if a pre spawned enemy is not within range of player
	if (bAttacking || !bWithinRangeOfPlayer) GetCharacterMovement()->MaxWalkSpeed = 0;
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AEnemy::ArenaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<AArenaManager>())
	{
		AArenaManager* CurrentArenaManager = Cast<AArenaManager>(OtherActor);
		CurrentArenaManager->IncrementEnemies();
	}
}


void AEnemy::ArenaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA<AArenaManager>())
	{
		AArenaManager* CurrentArenaManager = Cast<AArenaManager>(OtherActor);
		CurrentArenaManager->DecrementEnemies();
	}
}

void AEnemy::FlashLightBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->IsA<UStaticMeshComponent>())
	{
		if (OtherComp->ComponentHasTag("Flashlight"))//Is it overlapping with flashlight?
		{
			Player = Cast <AMyPlayer>(OtherComp->GetOwner());//Set Player reference to be equal to the player

			if (Player->bJustShot == false)//Check if player just fired a shot
			{
				bBeingStunned = true;
			}

		}
	}
}

void AEnemy::FlashLightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp->IsA<UStaticMeshComponent>())
	{
		if (OtherComp->ComponentHasTag("Flashlight"))//Was it the flashlight that stopped overlapping ?
		{
			bBeingStunned = false;
		}
	}
}

void AEnemy::LaserBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->IsA<UBoxComponent>())
	{
		if (OtherComp->ComponentHasTag("Laser"))//Is it overlapping with flashlight?
		{
			Die();
			Rally();
		}
	}
}


void AEnemy::AttackRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyPlayer>(OtherActor))
	{
		bWithinAttackRange = false;
	}
}

void AEnemy::StartAttackingRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AMyPlayer>(OtherActor))
	{
		bWithinAttackRange = true;
		if (!bIsStunned)
		{
			Attack();
		}
	}
}

void AEnemy::PlayerDetectorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AMyPlayer>(OtherActor))
	{
		bWithinRangeOfPlayer = true;

		if (Cast<AHerder>(this))
		{
			GetCharacterMovement()->MaxWalkSpeed = HerderTopSpeed;
		}
		else if (Cast<AAnkelbiter>(this))
		{
			GetCharacterMovement()->MaxWalkSpeed = AnkelbiterTopSpeed;
		}
	}
}

void AEnemy::PlayerDetectorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyPlayer>(OtherActor))
	{
		bWithinRangeOfPlayer = false;
	}
}

void AEnemy::Stunning()
{
	if (TimeInFlashlight >= TimeUntilStunned)
	{
		bIsStunned = true;
		GetCharacterMovement()->MaxWalkSpeed = 0;
	}
	else if (!bIsStunned)
	{
		if (Cast<AHerder>(this))
		{
			GetCharacterMovement()->MaxWalkSpeed = HerderTopSpeed - (TimeInFlashlight * MovementSpeedReduction);
		}
		else if (Cast<AAnkelbiter>(this))
		{
			GetCharacterMovement()->MaxWalkSpeed = AnkelbiterTopSpeed - (TimeInFlashlight * MovementSpeedReduction);
		}
	}
}

void AEnemy::Rally()
{
	bIsStunned = false;
	if (!bDead)
	{
		if (Cast<AHerder>(this))
		{
			GetCharacterMovement()->MaxWalkSpeed = HerderTopSpeed;
		}
		else if (Cast<AAnkelbiter>(this))
		{
			GetCharacterMovement()->MaxWalkSpeed = AnkelbiterTopSpeed;
		}
	}
}

void AEnemy::Die()
{
	if (bIsStunned)
	{
		int32 PowerUpIndex{};

		// Make enemy drop PowerUp number 'PowerUpIndex'
		PowerUpIndex = UKismetMathLibrary::RandomIntegerInRange(1, 3);
		SpawnPowerUp(PowerUpIndex);

		bDead = true;
		bDissolveEnemy = true;
		bRotateTowardsPlayer = false;
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;

		// If enemy is pre spawned, then destroy it. Else, teleport it back to spawn pool
		if (bPreSpawnedEnemy)
		{
			GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, TimerForTpEnemy);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AEnemy::TpEnemyToPool, TimerForTpEnemy);
		}
	}
}

void AEnemy::TpEnemyToPool()
{
	SetActorLocation(SpawnPoolLocation);
	bDissolveEnemy = false;
	TimeInFlashlight = 0;
	bRotateTowardsPlayer = true;
	TimePassed = 0.0f;

	// Reset dissolve effect back to normal
	for (int32 i{ 0 }; i < AmountOfMaterials; i++)
	{
		if ((GetMesh()->GetNumMaterials() - 1) >= i)
		{
			EnemyMaterial = GetMesh()->CreateDynamicMaterialInstance(i);
			EnemyMaterial->SetScalarParameterValue(FName("DissolveAmount"), AmountToDissolve(0.0f));
		}
		else
		{
			break;
		}
	}
}

void AEnemy::Attack()
{
	bAttacking = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(FName("Attack_1"), AttackMontage);
	}
}

void AEnemy::EndAttack()
{
	bAttacking = false;

	// If still within attack range when attack ends, continue attacking
	if (bWithinAttackRange)
	{
		Attack();
	}
}

void AEnemy::DamagePlayer()
{
	if (bWithinAttackRange)
	{
		AMyPlayer* PlayerRef = Cast<AMyPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		PlayerRef->PlayerHealth -= 1;
	}
}

void AEnemy::SpawnPowerUp(int32 PowerUpIndex)
{
	TSubclassOf<APickup> PowerUpToSpawn;
	float Random{};

	if (FasterReload && FlashlightWidener && LazerWidener)
	{
		// This switch will choose a powerup to spawn
		switch (PowerUpIndex)
		{
		case 1:
			PowerUpToSpawn = FasterReload;
			UE_LOG(LogTemp, Warning, TEXT("FasterReload selected"))
			break;
		case 2:
			PowerUpToSpawn = FlashlightWidener;
			UE_LOG(LogTemp, Warning, TEXT("FlashlightWidener selected"))
			break;
		case 3:
			PowerUpToSpawn = LazerWidener;
			UE_LOG(LogTemp, Warning, TEXT("LazerWidener selected"))
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("PowerUpIndex's range exceeds the amount of existing PowerUps"))
			// Quit game function
			break;
		}

		Random = UKismetMathLibrary::RandomFloat();

		// 10% chance of dropping powerup
		if (Random <= 0.10f)
		{
			FActorSpawnParameters SpawnParams;
			FVector Location = GetActorLocation() + FVector(0.0f, 0.0f, 20.0f);
			FRotator Rotation(0.0f);

			// Spawn the 'PowerUpToSpawn' when enemy dies
			APickup* PowerUpRef = GetWorld()->SpawnActor<APickup>(PowerUpToSpawn, Location, Rotation, SpawnParams);
		}
		else
		{
			// No powerup was dropped, so check if health pickup should be spawned
			SpawnHealth();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Enemies are missing PowerUp! Assign the corresponding PowerUp in Herder/Ankelbiter BP ..."))
	}


}

void AEnemy::SpawnHealth()
{
	if (Health)
	{
		float Random{};
		FActorSpawnParameters SpawnParams;
		FVector Location = GetActorLocation() + FVector(0.0f, 0.0f, -5.0f);
		FRotator Rotation(0.0f);

		Random = UKismetMathLibrary::RandomFloat();

		// 4% chance of dropping health pickup
		if (Random <= 0.04f)
		{
			APickup* HealthRef = GetWorld()->SpawnActor<APickup>(Health, Location, Rotation, SpawnParams);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Health pickup missing in enemy's BP!!"))
	}
}

void AEnemy::DestroyEnemy()
{
	this->Destroy();
}

// Since the 'DissolveAmount' in enemy's material has inputs as (-5.0f = no dissolve) and (8.0f = fully dissolved),
// I created this function to normalize these values. So (0.0f = no dissolve) and (1.0f = fully dissolved).
float AEnemy::AmountToDissolve(float Amount)
{
	if (Amount > 1.0f || Amount < 0.0f) FMath::Clamp(Amount, 0.0f, 1.0f);

	return (15.0f * Amount - 2.0f);
}