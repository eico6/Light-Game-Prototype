// Written by Eivind Hobrad Naasen


#include "LaserBeamVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "MyPlayer.h"

// Sets default values
ALaserBeamVisual::ALaserBeamVisual()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TimePassed = 0.0f;
	NewLightBrightness = 0.0f;
	InitialLightBrightness = 200000.0f;

	// Laser Mesh
	LaserLightVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserLightVisual"));
	LaserLightVisual->SetupAttachment(GetRootComponent());
	LaserLightVisual->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LaserLightVisual->SetRelativeScale3D(FVector(0.83f, 0.83f, 26.25f));

	// OverLight
	OverLight = CreateDefaultSubobject<URectLightComponent>(TEXT("OverLight"));
	OverLight->SetupAttachment(LaserLightVisual);
	OverLight->SetIntensity(InitialLightBrightness);
	OverLight->SetAttenuationRadius(1400.0f);
	OverLight->SetSourceHeight(3300.0f);
	OverLight->SetRelativeLocation(FVector(2.0f, 0.0f, 50.0f));

	// UnderLight
	UnderLight = CreateDefaultSubobject<URectLightComponent>(TEXT("UnderLight"));
	UnderLight->SetupAttachment(LaserLightVisual);
	UnderLight->SetIntensity(InitialLightBrightness);
	UnderLight->SetAttenuationRadius(1400.0f);
	UnderLight->SetSourceHeight(3300.0f);
	UnderLight->SetRelativeLocation(FVector(2.0f, 0.0f, 50.0f));
}

// Called when the game starts or when spawned
void ALaserBeamVisual::BeginPlay()
{
	Super::BeginPlay();
	
	TimePassed = 0.0f;
	NewLightBrightness = 0.0f;

	PlayerRef = Cast<AMyPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (PlayerRef)
	{
		// If player picked up a LaserWidener, then increase scale of mesh
		if (PlayerRef->CurrentPowerUp == ECurrentPowerUp::ECP_Blue_PowerUp)
		{
			LaserLightVisual->SetRelativeScale3D(FVector(0.83f * PlayerRef->PowerUpLaserScale, 0.83f * PlayerRef->PowerUpLaserScale, 26.25f));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Missing reference to player from LaserBeamVisual.cpp ..."))
	}

	// Create a dynamic material instance to change opacity and brightness in runtime
	LaserMaterial = LaserLightVisual->CreateDynamicMaterialInstance(0);

	// Spawn particle system
	if (NormalLaser && LargeLaser)
	{
		if (PlayerRef->CurrentPowerUp == ECurrentPowerUp::ECP_Blue_PowerUp)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LargeLaser, (LaserLightVisual->GetComponentLocation() + LaserLightVisual->GetForwardVector()), GetActorRotation(), true);
		}
		else
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), NormalLaser, (LaserLightVisual->GetComponentLocation() + LaserLightVisual->GetForwardVector()), GetActorRotation(), true);
		}
	}
}

// Called every frame
void ALaserBeamVisual::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimePassed += DeltaTime;
	NewLightBrightness = InitialLightBrightness - (TimePassed * 800000.0f);

	// Quickly fade away laser light
	OverLight->SetIntensity(NewLightBrightness);
	UnderLight->SetIntensity(NewLightBrightness);

	// Quickly fade away laser material
	LaserMaterial->SetScalarParameterValue(FName("Brightness"), 30.0f - (TimePassed * 60.0f));
	if (TimePassed >= 0.10f) LaserMaterial->SetScalarParameterValue(FName("Opacity"), 0.1f - (TimePassed / 4.0f));


	if (NewLightBrightness <= 10.0f)
	{
		Destroy();
	}
}

