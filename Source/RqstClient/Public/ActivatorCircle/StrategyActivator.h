// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Clients/ClientLabels.h"
#include "StrategyActivator.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UStrategyActivator : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RQSTCLIENT_API IStrategyActivator
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IStrategyActivator")
	EClientLabels GetClientType();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IStrategyActivator")
	void SetClientType(EClientLabels Label);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IStrategyActivator")
	void SetShiningColor(const FLinearColor& Color);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IStrategyActivator")
	void SetTitleColor(const FLinearColor& Color);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IStrategyActivator")
	void SetTitleText(const FText& Text);
};
