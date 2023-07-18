// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Clients/ClientLabels.h"
#include "ConnectionStrategyComponent.generated.h"

class UClientCollection;
class UDefaultFieldsFilling;
class ARqstClientCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RQSTCLIENT_API UConnectionStrategyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "ConnectionStrategyComponent")
	void SetStrategyOptions(EClientLabels Label, const FString& Host, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "ConnectionStrategyComponent")
	void SetStrateryClient(EClientLabels Label, const FString& Host, int32 Port, ARqstClientCharacter* Character);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets", meta = (AllowPrivateAccess = true))
	TObjectPtr<UClientCollection> ClientCollection;

	UFUNCTION()
	void OnClientConnect(EClientLabels Type, bool bSuccess);
	
	UFUNCTION()
	void OnClientDisconnect(EClientLabels Type, bool bSuccess);

	TUniqueFunction<void(EClientLabels, bool)> ConnectionEvent;
	TUniqueFunction<void(EClientLabels, bool)> DisonnectionEvent;

	FORCEINLINE bool CheckStrategyConnection(UObject* Client);
	void SetClient(const FString& Host, int32 Port, AGameModeBase* GameMode, EClientLabels Label, ARqstClientCharacter* Character);
	FName GetLabelName(EClientLabels Label) const;
	FLinearColor DefineClientColor(EClientLabels Label) const;
};
