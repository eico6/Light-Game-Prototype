// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen

#define COLLISION_AIAREA ECC_GameTraceChannel3

#include "MyPlayer.h"
#include "Herder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/TransformNonVectorized.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "LaserBeamVisual.h"

// Sets default values
AMyPlayer::AMyPlayer()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Initialize Object Components

	// HerderColliders is used as an empty component just used as a parent of behind/left/right colliders
	HerderAI = CreateDefaultSubobject<USceneComponent>(TEXT("HerderAI"));
	HerderAI->SetupAttachment(GetRootComponent());

	// Used in the same manner as HerderAI. Just used for arranging the hierarchy
	ParentOfPoints = CreateDefaultSubobject<USceneComponent>(TEXT("ParentOfPoints"));
	ParentOfPoints->SetupAttachment(HerderAI);

	FlashLightCollider = CreateDefaultSubobject <UStaticMeshComponent> (TEXT("FlashLightCollider"));
	FlashLightPivot = CreateDefaultSubobject<USphereComponent>(TEXT("FlashLightColliderPivot"));

	LaserCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("LaserCollider"));
	LaserCollider->SetRelativeScale3D(FVector(90.f, 1.5f, 1.5f));
	LaserCollider->SetRelativeLocation(FVector(2920.f, 0.f, 0.f));

	LaserPivot = CreateDefaultSubobject<USphereComponent>(TEXT("LaserColliderPivot"));
	LaserPivot->SetRelativeScale3D(FVector(.25f, .25f, .25f));
	LaserPivot->SetRelativeLocation(FVector(40, 0.f, 0.f));

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Flashlight->SetupAttachment(GetRootComponent());
	Flashlight->SetIntensity(500000.0f);
	Flashlight->SetAttenuationRadius(600.0f);
	Flashlight->SetOuterConeAngle(40.0f);

	// Colliders for Herder AI Behvaiour
	BehindCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BehindCollider"));
	BehindCollider->SetupAttachment(HerderAI);
	BehindCollider->SetRelativeLocation(FVector(-750.0f, 0.0f, 40.0f));
	BehindCollider->SetBoxExtent(FVector(750.0f, 700.0f, 80.0f));

	LeftCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("LetCollider"));
	LeftCollider->SetupAttachment(HerderAI);
	LeftCollider->SetRelativeLocation(FVector(0.0f, -800.0f, 40.0f));
	LeftCollider->SetBoxExtent(FVector(1500.0f, 550.0f, 80.0f));

	RightCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("RightCollider"));
	RightCollider->SetupAttachment(HerderAI);
	RightCollider->SetRelativeLocation(FVector(0.0f, 800.0f, 40.0f));
	RightCollider->SetBoxExtent(FVector(1500.0f, 550.0f, 80.0f));

	MidCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("MidCollider"));
	MidCollider->SetupAttachment(HerderAI);
	MidCollider->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	MidCollider->SetBoxExtent(FVector(175.0f, 175.0f, 80.0f));

	// Left point
	LeftPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPoint"));
	LeftPoint->SetupAttachment(ParentOfPoints);
	LeftPoint->SetRelativeRotation(FRotator(0.0f, 245.0f, 0.0f));
	LeftPoint->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Right point
	RightPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPoint"));
	RightPoint->SetupAttachment(ParentOfPoints);
	RightPoint->SetRelativeRotation(FRotator(0.0f, -245.0f, 0.0f));
	RightPoint->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Distance variables for Left/Right point
	LeftDistance = 700.0f;
	RightDistance = 700.0f;

	// Assigning variables which are connected to Left/Right point
	LengthOfTrace = 700.0f;
	DirectionOfTrace = FVector(0.0f, 0.0f, -1.0f);
	TraceParams = new FCollisionQueryParams();
	HitResult = new FHitResult();
	bRightValid = true;
	bLeftValid = true;
	bPrioritizeReady = true;
	bShouldFlicker = false;
	bJustFlicked = false;

	FlashLightCollider->SetupAttachment(FlashLightPivot);
	LaserCollider->SetupAttachment(LaserPivot);
	LaserPivot->SetupAttachment(GetRootComponent());
	FlashLightPivot->SetupAttachment(GetRootComponent());

	ColliderLocationOffset = 9000.0f;//Used for hiding flashlight cone and laser

	FlashLightScaleModifier = 1.1f;
	LightReduceScaleMod = 1.01f;//Used to make the scale of the flashlight go down when you charge up laser
	LightReturnSpeed = 1.0f; //Determines how fast you flashlight returns after shooting your laser

	//Initialize Laser Collider Transforms
	LaserLocationDefault = LaserPivot->GetRelativeLocation();
	LaserLocationDefault = FVector(LaserLocationDefault.X, LaserLocationDefault.Y, LaserLocationDefault.Z - ColliderLocationOffset);
	LaserScaleDefault = LaserPivot->GetRelativeScale3D();
	LaserRotationDefault = this->GetActorRotation();

	//Initialize PowerUp Variables
	PowerUpTime = 20.0f;
	PowerUpTimeLeft = 0;
	ReloadSpeedDefault = 2.0f;
	ReloadSpeedCurrent = 2.0f;
	ReloadSpeedUpgraded = 4.0f;
	PowerUpLightScale = 2.0f;
	PowerUpLaserScale = 4.0f;
	YScaler = 4.2f;

	LightRotationDefault = this->GetActorRotation();

	//Combat values
	PlayerHealth = 5;
	DamageTaken = 1;
	ShootingTime = 0.03f;
	CurrentPowerUp = ECurrentPowerUp::ECP_None;

}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Set first checkpoint as the location at the start of the game
	LastCheckpoint = GetActorLocation();

	//Initialize Flashlight collider Transforms
	LightLocationDefault = FlashLightPivot->GetRelativeLocation();
	LightScaleDefault = FlashLightPivot->GetRelativeScale3D();


	LightLocationCurrent = LightLocationDefault;
	LightScaleCurrent = LightScaleDefault;
	LightRotationCurrent = LightRotationDefault;

	LightLocationPoweredUp = LightLocationDefault;
	LightScalePoweredUp = FVector(LightScaleDefault.X * PowerUpLightScale, LightScaleDefault.Y * PowerUpLightScale * YScaler, LightScaleDefault.Z * PowerUpLightScale);
	LightRotationPoweredUp = LightRotationDefault;


	//Initialize Laser Collider Transforms
	LaserLocationCurrent = LaserLocationDefault;
	LaserScaleCurrent = LaserScaleDefault;
	LaserRotationCurrent = LaserRotationDefault;

	LaserLocationPoweredUp = LaserLocationDefault;
	LaserScalePoweredUp = FVector(LaserScaleDefault.X , LaserScaleDefault.Y * PowerUpLaserScale, LaserScaleDefault.Z * PowerUpLaserScale);
	LaserRotationPoweredUp = LaserRotationDefault;

	LaserPivot->SetRelativeLocation(LaserLocationDefault);
	LaserPivot->SetRelativeRotation(LaserRotationDefault);
	FlashLightPivot->SetRelativeLocation(LightLocationDefault);
	FlashLightPivot->SetRelativeRotation(LightRotationDefault);

	// Points behind player spawned at different locations (affected by intial this->ActorRotation)
	// Because I set their location further down as 'SetRelativeLocation', they spawned at twice the
	// rotation of GetActorRotation(). To fix this, I made the parent of these points half the rotation
	// of GetActorRotation(). This will spawn them at the intended location.
	float Yaw{GetActorRotation().Yaw / 2};
	ParentOfPoints->SetWorldRotation(FRotator(0.0f, Yaw, 0.0f));

	// Forward vectors of left/right point
	LeftForward = LeftPoint->GetForwardVector();
	RightForward = RightPoint->GetForwardVector();

	// Set left/right point at correct distance/locations
	LeftPoint->SetRelativeLocation(LeftForward * LeftDistance);
	RightPoint->SetRelativeLocation(RightForward * RightDistance);

	// Start looking for OverlapEvents in the following colliders:
	BehindCollider->OnComponentEndOverlap.AddDynamic(this, &AMyPlayer::BehindOverlapEnd);
	LeftCollider->OnComponentBeginOverlap.AddDynamic(this, &AMyPlayer::LeftOverlapBegin);
	RightCollider->OnComponentBeginOverlap.AddDynamic(this, &AMyPlayer::RightOverlapBegin);
	MidCollider->OnComponentBeginOverlap.AddDynamic(this, &AMyPlayer::MidOverlapBegin);
	MidCollider->OnComponentEndOverlap.AddDynamic(this, &AMyPlayer::MidOverlapEnd);

}

// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Time = DeltaTime;

	if (PlayerHealth <= 0) SpawnAtLastCheckpoint();

	FlashLightPivot->SetRelativeScale3D(FVector(LightScaleCurrent.X, (LightScaleCurrent.Y * (LightReduceScaleMod - LaserCharger)), LightScaleCurrent.Z));


	LightBehaviour();
	if (bHasPowerUp == true)
	{
		PowerUpTimeLeft = PowerUpTimeLeft - Time;
		if (PowerUpTimeLeft <= 0)
		{
			LosePowerup();
		}
	}
	if (bJustTookDamage == true)
	{
		TimeRecovering = TimeRecovering + Time;
		if (TimeRecovering >= TimeToRecover)
		{
			bJustTookDamage = false;
			//End damagefeedback here....
			TimeRecovering = 0;
		}

}

	// Always set the points to be the furthest distance possible (closest to 700)
	// This distance will be reduced to a valid distance in the function ReduceDistance();
	LeftDistance = 700.0f;
	RightDistance = 700.0f;
	LeftPoint->SetRelativeLocation(LeftForward * LeftDistance);
	RightPoint->SetRelativeLocation(RightForward * RightDistance);

	// Reduce the distance from Player to Left/Right Point to a valid distance
	ReduceDistance();

	// Update these values which will be sent to the AI for its behaviour to work
	LeftPointLocation = LeftPoint->GetComponentLocation();
	RightPointLocation = RightPoint->GetComponentLocation();

	// Go to PrioritizationTrue() definition for explenation.
	// This will basically generate an OverlapEvent for Left/Right colliders if AI
	// is already located inside one of the colliders.
	switch (bShouldFlicker)
	{
	case true:
		LeftCollider->SetRelativeLocation(LeftCollider->GetRelativeLocation() + 1000.0f);
		RightCollider->SetRelativeLocation(RightCollider->GetRelativeLocation() + 1000.0f);
		bShouldFlicker = false;
		bJustFlicked = true;
		break;

	case false:
		if (bJustFlicked)
		{
			LeftCollider->SetRelativeLocation(LeftCollider->GetRelativeLocation() - 1000.0f);
			RightCollider->SetRelativeLocation(RightCollider->GetRelativeLocation() - 1000.0f);
			bJustFlicked = false;
		}
		break;
	}
}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyPlayer::ChargeUp);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AMyPlayer::StopCharging);
}

void AMyPlayer::ChargeUp()//Hold button to charge 
{
	if (bJustShot == false)
	{
		bIsCharging = true; 
	}
}

void AMyPlayer::Shoot()//Shoot if your laser is fully charged
{
	if (bJustShot == false)
	{
		if (LaserCharger >= LaserFullyCharged)
		{
			LaserPivot->SetRelativeLocation(FVector(LaserLocationDefault.X, LaserLocationDefault.Y, LaserLocationDefault.Z + ColliderLocationOffset));	//Set location of Laser hitbox ahead of the player 

			if (LaserVisual)
			{
				FActorSpawnParameters SpawnParams;
				FVector Location = GetMesh()->GetSocketLocation(FName("joint39")) + FVector(0.0f, 0.0f, 30.0f);
				FRotator Rotation = GetActorRotation();

				// Spawn laser mesh for feedback to player
				ALaserBeamVisual* VisualRef = GetWorld()->SpawnActor<ALaserBeamVisual>(LaserVisual, Location + (GetActorForwardVector() * 45.0f), Rotation, SpawnParams);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Missing 'LaserVisual' from MyPlayer_BP ..."))
			}

			GetWorld()->GetTimerManager().SetTimer(MyTimerHandle, this, &AMyPlayer::CooledDown, (ShootingTime));
		}
	}
}

void AMyPlayer::StopCharging()
{
	bIsCharging = false;
	FlashLightPivot->SetRelativeLocation(LightLocationDefault);
}

void AMyPlayer::PickupEventBiggerFlashlight()//Increase size of flashlight
{
	LosePowerup();
	LightLocationCurrent = LightLocationPoweredUp;
	LightScaleCurrent = LightScalePoweredUp;
	LightRotationCurrent = LightRotationPoweredUp;
	FlashLightPivot->SetRelativeScale3D(LightScaleCurrent);

	bHasPowerUp = true;
	CurrentPowerUp = ECurrentPowerUp::ECP_Green_PowerUp;
	PowerUpTimeLeft = PowerUpTime;

}

void AMyPlayer::PickupEventFasterReload()//Increase fire rate of your finishing move
{
	LosePowerup();
	ReloadSpeedCurrent = ReloadSpeedUpgraded;
	bHasPowerUp = true;
	CurrentPowerUp = ECurrentPowerUp::ECP_Red_PowerUp;
	PowerUpTimeLeft = PowerUpTime;
}

void AMyPlayer::PickupEventBiggerLaser()//Increase the size of the finishing move
{
	LosePowerup();
	LaserLocationCurrent = LaserLocationPoweredUp;
	LaserScaleCurrent = LaserScalePoweredUp;
	LaserRotationCurrent = LaserRotationPoweredUp;
	LaserPivot->SetRelativeScale3D(LaserScaleCurrent);

	bHasPowerUp = true;
	CurrentPowerUp = ECurrentPowerUp::ECP_Blue_PowerUp;
	PowerUpTimeLeft = PowerUpTime;
}

void AMyPlayer::LosePowerup()//Remove the power you have and reset the timer(Not finished!!!)
{
	LightLocationCurrent = LightLocationDefault;
	LightScaleCurrent = LightScaleDefault;
	LightRotationCurrent = LightRotationDefault;

	FlashLightPivot->SetRelativeLocation(LightLocationDefault);
	FlashLightPivot->SetRelativeRotation(LightRotationDefault);
	FlashLightPivot->SetRelativeScale3D(LightScaleDefault);


	LaserScaleCurrent = LaserScaleDefault;
	LaserRotationCurrent = LaserRotationDefault;

	LaserPivot->SetRelativeRotation(LaserRotationDefault);
	LaserPivot->SetRelativeScale3D(LaserScaleDefault);

	ReloadSpeedCurrent = ReloadSpeedDefault;

	bHasPowerUp = false;
	CurrentPowerUp = ECurrentPowerUp::ECP_None;

}

void AMyPlayer::TakeDamageTho()
{
	PlayerHealth = PlayerHealth - DamageTaken;

	if (PlayerHealth <= 0)
	{
		this->Destroy();
	}
	else
	{
		//Damage feedback and invinsibility frames
	}
}

void AMyPlayer::LightBehaviour()
{
	if (bIsCharging == true)
	{
		if (LaserCharger >= LaserFullyCharged) //Are you fully charged ?
		{
			FlashLightPivot->SetRelativeLocation(FVector(LightLocationDefault.X, LightLocationDefault.Y, LightLocationDefault.Z - ColliderLocationOffset));//HideFlashlight
			//Make the laser shoot for one frame
			if(bIsShooting == true)
			{
				StopCharging();
				bIsShooting = false;
			}
			else
			{
				Shoot();
				bIsShooting = true;
			}


		}
		if (LaserCharger < LaserFullyCharged)
		{
			LaserCharger = LaserCharger + (ReloadSpeedCurrent * Time);
		}
		FlashLightPivot->SetRelativeScale3D(FVector(LightScaleCurrent.X, (LightScaleCurrent.Y * (LightReduceScaleMod - LaserCharger)), LightScaleCurrent.Z));
	}
	else
	{

		if (bJustShot == true)
		{
			LaserCharger = LaserCharger - (LightReturnSpeed * (ReloadSpeedCurrent * Time));

			if (LaserCharger <= 0)
			{
				bJustShot = false;
				//FlashLightPivot->SetHiddenInGame(false);
				//Set location of Finishing move to hidden
				LaserPivot->SetRelativeLocation(LaserLocationDefault);
				//Set location of flashlight hitbox in front of player
				FlashLightPivot->SetRelativeLocation(LightLocationDefault);
				//Change scale of Flashlight hitbox light
				FlashLightPivot->SetRelativeScale3D(FVector(LightScaleCurrent.X, (LightScaleCurrent.Y * (LightReduceScaleMod - LaserCharger)), LightScaleCurrent.Z));

				//Recheck overlap events
				FlashLightPivot->SetRelativeLocation(LightLocationDefault - ColliderLocationOffset);
				FlashLightPivot->SetRelativeLocation(LightLocationDefault);
			}
			/*else
			{
				//Flicker flashlight
				if (FlashLightPivot->bHiddenInGame == true)
				{
					FlashLightPivot->SetHiddenInGame(false);
				}
				else if (FlashLightPivot->bHiddenInGame == false)
				{
					FlashLightPivot->SetHiddenInGame(true);
				}

				FlashLightPivot->SetRelativeScale3D(FVector(LightScaleCurrent.X, (LightScaleCurrent.Y * (LightReduceScaleMod - LaserCharger)), LightScaleCurrent.Z));
			}
			*/
		}
		else
		{
			if (LaserCharger > 0)
			{
				LaserCharger = LaserCharger - (LightReturnSpeed * (ReloadSpeedCurrent * Time));
				FlashLightPivot->SetRelativeScale3D(FVector(LightScaleCurrent.X, (LightScaleCurrent.Y * (LightReduceScaleMod - LaserCharger)), LightScaleCurrent.Z));
			}
		}
	}

}

void AMyPlayer::LookAtMouse()
{

}

void AMyPlayer::CooledDown()
{
	LaserPivot->SetRelativeLocation(LaserLocationDefault);//Set location of Laser hitbox in hide location

	FlashLightPivot->SetRelativeLocation(LightLocationDefault);//Set location of Flashlight hitbox ahead of the player
	bIsCharging = false;
	bJustShot = true;
}


// Left Box Collider for Herder AI Behaviour begin overlap
void AMyPlayer::LeftOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AHerder* Herder;
		Herder = Cast<AHerder>(OtherActor);

		if (Herder && bPrioritizeReady == true && (Herder->bPrioritizeLeft == false))
		{
			// 'bPrioritizeReady' together with the 'AITimerHandle' makes sure that the AI can't re-prioritize 
			// a new location before it has gone 2.0 seconds.
			// This is to prevent the AI from walking left-right-left-right-left... in quick successions.
			bPrioritizeReady = false;
			GetWorld()->GetTimerManager().SetTimer(AITimerHandle, this, &AMyPlayer::PrioritizationTrue, 1.5f);
			Herder->bPrioritizeLeft = true;
		}
	}
}

// Right Box Collider for Herder AI Behaviour begin overlap
void AMyPlayer::RightOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AHerder* Herder;
		Herder = Cast<AHerder>(OtherActor);

		if (Herder && bPrioritizeReady == true && (Herder->bPrioritizeLeft == true))
		{
			// 'bPrioritizeReady' together with the 'AITimerHandle' makes sure that the AI can't re-prioritize 
			// a new location before it has gone 2.0 seconds.
			// This is to prevent the AI from walking left-right-left-right-left... in quick successions.
			bPrioritizeReady = false;
			GetWorld()->GetTimerManager().SetTimer(AITimerHandle, this, &AMyPlayer::PrioritizationTrue, 1.5f);
			Herder->bPrioritizeLeft = false;
		}
	}
}

// Mid Box Collider for Herder AI Behaviour begin overlap
void AMyPlayer::MidOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AHerder* Herder;
		Herder = Cast<AHerder>(OtherActor);

		if (Herder)
		{
			Herder->bPrioritizePlayer = true;
		}
	}
}

// Mid Box Collider for Herder AI Behaviour end overlap
void AMyPlayer::MidOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AHerder* Herder;
		Herder = Cast<AHerder>(OtherActor);

		if (Herder)
		{
			Herder->bPrioritizePlayer = false;
			Herder->bStageOneComplete = false;
		}
	}
}

// Behind Box Collider for Herder AI Behaviour end overlap
void AMyPlayer::BehindOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AHerder* Herder;
		Herder = Cast<AHerder>(OtherActor);

		if (Herder)
		{
			if (Herder->bStageOneComplete)
			{
				Herder->bStageOneComplete = false;
			}
		}
	}
}

void AMyPlayer::ReduceDistance()
{
	CheckLeftValid();
	CheckRightValid();

	while (!bLeftValid)
	{
		// Smaller decrements will reduce performance drastically, but will increase accuracy.
		// If you set the points to be visible in-game, you can see what I mean.
		// If the points were supposed to be visible to the player, I would've figured out
		// a more efficient, smoother solution.
		LeftDistance -= 40.0f;
		LeftPoint->SetRelativeLocation(LeftForward * LeftDistance);
		CheckLeftValid();
		if (LeftDistance <= 50.0f) break;
	}

	while (!bRightValid)
	{
		// Smaller decrements will reduce performance drastically, but will increase accuracy.
		// If you set the points to be visible in-game, you can see what I mean.
		// If the points were supposed to be visible to the player, I would've figured out
		// a more efficient, smoother solution.
		RightDistance -= 40.0f;
		RightPoint->SetRelativeLocation(RightForward * RightDistance);
		CheckRightValid();
		if (RightDistance <= 50.0f) break;
	}
}

void AMyPlayer::CheckLeftValid()
{
	FVector LeftStart = LeftPoint->GetComponentLocation();
	FVector LeftEnd = LeftStart + (LengthOfTrace * DirectionOfTrace);

	// Line trace from right point, straight downwards.
	// Return true/valid if trace hit an actor with collision channel: ECC_GameTraceChannel3, set to block.
	// Else, return false/invalid (ECC_GameTraceChannel3 = my custom trace channel, called "AIArea").
	bLeftValid = (GetWorld()->LineTraceSingleByChannel(*HitResult, LeftStart, LeftEnd, ECollisionChannel::COLLISION_AIAREA, *TraceParams) ? true : false);
}

void AMyPlayer::CheckRightValid()
{
	FVector RightStart = RightPoint->GetComponentLocation();
	FVector RightEnd = RightStart + (LengthOfTrace * DirectionOfTrace);

	// Line trace from right point, straight downwards.
	// Return true/valid if trace hit an actor with collision channel: ECC_GameTraceChannel3, set to block.
	// Else, return false/invalid (ECC_GameTraceChannel3 = my custom trace channel, called "AIArea").
	bRightValid = (GetWorld()->LineTraceSingleByChannel(*HitResult, RightStart, RightEnd, ECollisionChannel::COLLISION_AIAREA, *TraceParams) ? true : false);

}


// Function is called by 'AITimerHandle' to turn 'bPrioritizeReady' and 'bShouldFlicker' back to true.
// Left and Right box colliders should flicker (teleport far away and back to its position in the span of 2 frames)
// This is to be able to generate an OverlapEvent whenever AI is already inside one of the box colliders
// when 'bPrioritizeReady' turns back to true. Just a solution to how Unreal detect OverlapEvents.
void AMyPlayer::PrioritizationTrue()
{
	bPrioritizeReady = true;
	bShouldFlicker = true;
}

void AMyPlayer::SpawnAtLastCheckpoint()
{
	UE_LOG(LogTemp, Warning, (TEXT("Respawning...")))
	LosePowerup();
	SetActorLocation(LastCheckpoint);
	PlayerHealth = 5;
}