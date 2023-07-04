// Copyright Epic Games, Inc. All Rights Reserved.

#include "RqstClientGameMode.h"
#include "RqstClientCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Clients/Interfaces/Client.h"
#include "Clients/ClientFactory.h"

ARqstClientGameMode::ARqstClientGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class) DefaultPawnClass = PlayerPawnBPClass.Class;

	FactoryClass = UClientFactory::StaticClass();
}

void ARqstClientGameMode::BeginPlay()
{
	Super::BeginPlay();

	ClientFactory = NewObject<UClientFactory>(this, FactoryClass ? FactoryClass : UClientFactory::StaticClass(), TEXT("ClientFactory"));
}

bool ARqstClientGameMode::SetClient_Implementation(EClientLabels Type)
{
	if (IsValid(ClientFactory)) Client = ClientFactory->CreateByTarget(Type, this, ClientType);
	else ClientType = EClientLabels::NONE;
	return ClientType != EClientLabels::NONE;
}
