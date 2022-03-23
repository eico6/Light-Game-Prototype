// Written by Eivind Hobrad Naasen


#include "Pickup_Health.h"
#include "Components/BoxComponent.h"
#include "MyPlayer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"

APickup_Health::APickup_Health()
{
	// Move collider up to center of pickup
	Collider->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	// Default values
	DeltaLevitate = FVector(0.0f, 0.0f, 0.0f);
	TimePassed = 0.0f;
	//DimmingTimer = 0.0f;
	//bDimLight = false;
}

void APickup_Health::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	Collider->OnComponentBeginOverlap.AddDynamic(this, &APickup_Health::OnOverlapBegin);
}

void APickup_Health::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimePassed += DeltaTime;

	// Make health pickup levitate
	DeltaLevitate = SpawnLocation + (FVector(0.0f, 0.0f, 16.0f) * UKismetMathLibrary::Cos(TimePassed * 3.8f));
	SetActorRelativeLocation(DeltaLevitate);


	// If LightSource should slowly dim away out of existence
	//if (bDimLight)
	//{
	//	DimmingTimer += DeltaTime;
	//	LightSource->SetIntensity(1200.0f * (1.0f / (DimmingTimer * 20.0f)));

	//		// If intensity is small enough, destroy actor
	//		if (LightSource->Intensity <= 50.0f)
	//		{
	//			bDimLight = false;
	//			LightSource->DestroyComponent();
	//			this->Destroy();
	//		}
	//}
}

void APickup_Health::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OtherActor->IsA<AMyPlayer>())
	{
		PlayerRef = Cast<AMyPlayer>(OtherActor);
		if (PlayerRef)
		{
			// Give player full health
			PlayerRef->PlayerHealth = 5;
			this->Destroy();
			//bDimLight = true;
		}
	}
	
}