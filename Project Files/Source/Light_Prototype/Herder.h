// Written by Eivind Hobrad Naasen

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "Herder.generated.h"

/**
 * Derived class from AEnemy.
 * Will contain spesific functionalities for Herders.
 * Functionalities for its behaviour is done in the AIController.
 */
UCLASS()
class LIGHT_PROTOTYPE_API AHerder : public AEnemy
{
	GENERATED_BODY()
	
public:
    AHerder();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPrioritizeLeft;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStageOneComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPrioritizePlayer;
};
