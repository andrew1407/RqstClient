// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ClientFormSwitch.h"
#include "FormTransitionWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class RQSTCLIENT_API UFormTransitionWidget : public UClientFormSwitch
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category="FormTransitionWidget|UI", meta = (BindWidget))
	TObjectPtr<UButton> SceneTransitionBtn;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FormTransitionWidget", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UWorld> LevelToOpen;

	UFUNCTION()
	void GoToScene();

	void SetupTransitionBtnProperties();
};
