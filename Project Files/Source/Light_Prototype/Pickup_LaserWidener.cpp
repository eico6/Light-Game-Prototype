// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen


#include "Pickup_LaserWidener.h"
#include "MyPlayer.h"

APickup_LaserWidener::APickup_LaserWidener()
{

}

void APickup_LaserWidener::BeginPlay()
{
	Super::BeginPlay();

	// Start checking for OverlapBegin events
	Collider->OnComponentBeginOverlap.AddDynamic(this, &APickup_LaserWidener::OnOverlapBegin);
}


void APickup_LaserWidener::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (Cast<AMyPlayer>(OtherActor))
	{
		Player = Cast <AMyPlayer>(OtherActor);
		Player->PickupEventBiggerLaser();
		this->Destroy();

		UE_LOG(LogTemp, Warning, TEXT("LaserWidener: OnOverlapBegin called"))

		// Functionalities for 'FlashlightWidener' is implemented in the BluePrint
	}
}