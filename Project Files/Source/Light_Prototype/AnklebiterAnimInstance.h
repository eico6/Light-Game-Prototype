// Written by Lars Joar Bjørkeland

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AnklebiterAnimInstance.generated.h"

//Taken from the UDEMY course
/**
 *
 */
UCLASS()
class LIGHT_PROTOTYPE_API UAnklebiterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bisInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bEnemyDead;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class APawn* Pawn;

	class AEnemy* EnemyPtr;
};
