// Written by Eivind Hobrad Naasen 

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "Ankelbiter.generated.h"

/**
 * 
 */
UCLASS()
class LIGHT_PROTOTYPE_API AAnkelbiter : public AEnemy
{
	GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
};
