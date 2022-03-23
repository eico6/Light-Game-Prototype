// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen


#include "Pickup.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PointLightComponent.h"
#include "MyPlayer.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a default root component (null object)
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create a mesh for the pickup
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	// Create a hitbox for the pickup
	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	Collider->SetupAttachment(GetRootComponent());

	// Decides which type of objects it will react with, and if it should overlap or collide.
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collider->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Mesh got its own collision, we don't want to control any functionalities through that collision
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Color of light will be assign in BP
	LightSource = CreateDefaultSubobject<UPointLightComponent>(TEXT("LightSource"));
	LightSource->SetupAttachment(GetRootComponent());
	LightSource->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
	LightSource->SetIntensity(1000.0f);
	LightSource->AttenuationRadius = 230.0f;

	//Set rotation speed of pickup
	PitchValue = 0.0f;
	YawValue = 1.0f;
	RollValue = 0.0f;

}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Set rotation of mushrooms every frame to make them spin
	//Got help from this page: https://unrealcpp.com/rotating-actor/
	FRotator NewRotation = FRotator(PitchValue, YawValue, RollValue);
	FQuat QuatRotation = FQuat(NewRotation);
	AddActorWorldRotation(QuatRotation, false, 0, ETeleportType::None);

}

void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("PARENT: OnOverlapBegin called"))

	if (OtherActor->IsA<AMyPlayer>())
	{
		if (ParticleOnPickup)
		{
			// Spawn particle system if assigned in BP
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleOnPickup, (GetActorLocation() + FVector(0.0f, 0.0f, 50.0f)), FRotator(0.0f), true);
		}
	}
}