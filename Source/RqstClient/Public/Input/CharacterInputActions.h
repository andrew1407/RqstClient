#pragma once

#include "UObject/ObjectPtr.h"
#include "CharacterInputActions.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FCharacterInputActions
{
    GENERATED_USTRUCT_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> InteractAction;
};
