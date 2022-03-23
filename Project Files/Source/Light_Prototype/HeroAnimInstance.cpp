// Written by Lars Joar Bj�rkeland


#include "HeroAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

void UHeroAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}
}

void UHeroAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		bisInAir = Pawn->GetMovementComponent()->IsFalling();
	}
}
