// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen


#include "Pickup_FasterReload.h"
#include "MyPlayer.h"

APickup_FasterReload::APickup_FasterReload()
{

}

void APickup_FasterReload::BeginPlay()
{
	Super::BeginPlay();

	// Start checking for OverlapBegin events
	Collider->OnComponentBeginOverlap.AddDynamic(this, &APickup_FasterReload::OnOverlapBegin);
}


void APickup_FasterReload::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
		
	if (Cast<AMyPlayer>(OtherActor))
	{
		Player = Cast <AMyPlayer>(OtherActor);
		Player->PickupEventFasterReload();
		this->Destroy();

		UE_LOG(LogTemp, Warning, TEXT("Faster Reload: OnOverlapBegin called"))

		// Functionalities for 'FlashlightWidener' is implemented in the BluePrint
	}
}