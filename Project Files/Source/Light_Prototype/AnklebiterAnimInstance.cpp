// Written by Lars Joar Bjørkeland


#include "AnklebiterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy.h"

void UAnklebiterAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}
}

void UAnklebiterAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		bisInAir = Pawn->GetMovementComponent()->IsFalling();

		EnemyPtr = Cast<AEnemy>(Pawn);

		if (EnemyPtr->IsA<AEnemy>())
		{
			bEnemyDead = EnemyPtr->bDead;
		}
	}
}