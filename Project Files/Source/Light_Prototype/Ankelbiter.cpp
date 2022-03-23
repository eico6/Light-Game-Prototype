// Written by Eivind Hobrad Naasen 


#include "Ankelbiter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


void AAnkelbiter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// OLD FIX FOR WEAKENING VISUAL (turned this into a for-lopp in "Enemy.cpp")
	//UMaterialInstanceDynamic* TempMaterial = Super::GetMesh()->CreateDynamicMaterialInstance(1);
	//if (TempMaterial /*&& !bIsStunned*/)
	//{
	//	TempMaterial->SetScalarParameterValue(FName("WeakenedAmount"), (TimeInFlashlight / TimeUntilStunned));
	//}

	GetCharacterMovement()->bOrientRotationToMovement = (bRotateTowardsPlayer) ? true : false;
}