// Written by Lars Joar Bjørkeland and Eivind Hobrad Naasen


#include "FlashlightWidener.h"
#include "MyPlayer.h"


void AFlashlightWidener::BeginPlay()
{
	Super::BeginPlay();

	// Start checking for OverlapBegin events
	Collider->OnComponentBeginOverlap.AddDynamic(this, &AFlashlightWidener::OnOverlapBegin);
}


void AFlashlightWidener::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
									    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (Cast<AMyPlayer>(OtherActor))
	{
		Player = Cast <AMyPlayer>(OtherActor);
		Player->PickupEventBiggerFlashlight();
		this->Destroy();

		UE_LOG(LogTemp, Warning, TEXT("Flashlight Widener: OnOverlapBegin called"))

		// Functionalities for 'FlashlightWidener' is implemented in the BluePrint
	}

}