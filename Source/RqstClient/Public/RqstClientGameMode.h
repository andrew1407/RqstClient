// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Clients/ClientLabels.h"
#include "Clients/Interfaces/ClientContainer.h"
#include "RqstClientGameMode.generated.h"

class UClientFactory;

UCLASS(minimalapi)
class ARqstClientGameMode : public AGameModeBase, public IClientContainer
{
	GENERATED_BODY()

public:
	ARqstClientGameMode();

	bool SetClient_Implementation(EClientLabels Type);
	FORCEINLINE EClientLabels GetClientType_Implementation() { return ClientType; }
	FORCEINLINE UObject* GetClient_Implementation() { return Client.Get(); }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "RqstClientGameMode|StrategyClient", meta = (AllowPrivateAccess = true))
	TSubclassOf<UClientFactory> FactoryClass;

	UPROPERTY(BlueprintReadOnly, Category = "RqstClientGameMode|StrategyClient", meta = (AllowPrivateAccess = true))
	TObjectPtr<UClientFactory> ClientFactory;

	UPROPERTY(BlueprintReadOnly, Category = "RqstClientGameMode|StrategyClient", meta = (AllowPrivateAccess = true))
	EClientLabels ClientType = EClientLabels::NONE;

	UPROPERTY(BlueprintReadOnly, Category = "RqstClientGameMode|StrategyClient", meta = (AllowPrivateAccess = true))
	TObjectPtr<UObject> Client;
};
