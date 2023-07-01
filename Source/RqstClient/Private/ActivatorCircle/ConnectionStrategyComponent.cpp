// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivatorCircle/ConnectionStrategyComponent.h"
#include "RqstClientCharacter.h"

#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

#include "DataAssets/ClientCollection.h"
#include "ActivatorCircle/StrategyActivator.h"

#include "Clients/Interfaces/ClientContainer.h"
#include "Clients/Interfaces/Connection.h"
#include "Clients/Interfaces/Client.h"

void UConnectionStrategyComponent::SetStrategyOptions(EClientLabels Label, const FString& Host, int32 Port)
{
	if (!IsValid(ClientCollection)) return;
	AActor* Owner = GetOwner();
	bool IsActivator = UKismetSystemLibrary::DoesImplementInterface(Owner, UStrategyActivator::StaticClass());
	if (!IsActivator) return;
	const FName LabelName = GetLabelName(Label);
	if (!LabelName.IsValid()) return;
	const FLinearColor Color = DefineClientColor(Label);
	const FString Title = Label == EClientLabels::NONE ? LabelName.ToString() : FString::Printf(TEXT("%s://%s:%d"), *LabelName.ToString(), *Host, Port);
	IStrategyActivator::Execute_SetTitleText(Owner, FText::FromString(Title));
	IStrategyActivator::Execute_SetTitleColor(Owner, Color);
	IStrategyActivator::Execute_SetShiningColor(Owner, Color);
}

void UConnectionStrategyComponent::SetStrateryClient(EClientLabels Label, const FString& Host, int32 Port, ARqstClientCharacter* Character)
{
	const TFunctionRef<void()> OnFailure = [&] { Character->SetMaterialColor(FLinearColor::Red); };
	if (!IsValid(ClientCollection)) return OnFailure();
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    bool IsClientContainer = UKismetSystemLibrary::DoesImplementInterface(GameMode, UClientContainer::StaticClass());
	if (!IsClientContainer) return OnFailure();

	EClientLabels CurrentLabel = IClientContainer::Execute_GetClientType(GameMode);
	if (Label == EClientLabels::NONE && CurrentLabel != EClientLabels::NONE)
	{
		IClientContainer::Execute_SetClient(GameMode, EClientLabels::NONE);
		return Character->SetMaterialColor(ClientCollection->DefaultColor.ReinterpretAsLinear());
	}

	if (CurrentLabel == Label) return;
	UObject* CurrentClient = IClientContainer::Execute_GetClient(GameMode);
	bool IsConnection = CheckStrategyConnection(CurrentClient) && IConnection::Execute_Connected(CurrentClient);
	
	TUniqueFunction<void()> DefineClient = [this, Host, Port, GameMode, Label, Character] { SetClient(Host, Port, GameMode, Label, Character); };
	if (!IsConnection) return DefineClient();

	FConnectionDelegate DisconnectionDelegate;
	DisonnectionEvent = [DefineClient = MoveTemp(DefineClient)] (EClientLabels Type, bool bSuccess) { DefineClient(); };
	DisconnectionDelegate.BindDynamic(this, &UConnectionStrategyComponent::OnClientDisconnect);
	IConnection::Execute_Disconnect(CurrentClient, DisconnectionDelegate);
}

void UConnectionStrategyComponent::SetClient(const FString& Host, int32 Port, AGameModeBase* GameMode, EClientLabels Label, ARqstClientCharacter* Character)
{
	const TUniqueFunction<void()> OnFailure = [&] { Character->SetMaterialColor(ClientCollection->ErrorColor.ReinterpretAsLinear()); };
	bool isSet = IClientContainer::Execute_SetClient(GameMode, Label);
	if (!isSet) return OnFailure(); 

	UObject* Client = IClientContainer::Execute_GetClient(GameMode);
	bool IsClient = UKismetSystemLibrary::DoesImplementInterface(Client, UClient::StaticClass());
	if (!IsClient) return OnFailure();
	
	IClient::Execute_SetAddress(Client, Host, Port);
	bool IsConnection = CheckStrategyConnection(Client);
	if (!IsConnection) return Character->SetMaterialColor(DefineClientColor(Label));
	FConnectionDelegate ConnectionDelegate;
	ConnectionEvent = [this, Character] (EClientLabels Type, bool bSuccess) -> void
	{
		const FLinearColor Color = bSuccess ? DefineClientColor(Type) : ClientCollection->ErrorColor.ReinterpretAsLinear();
		Character->SetMaterialColor(Color);
	};
	ConnectionDelegate.BindDynamic(this, &UConnectionStrategyComponent::OnClientConnect);
	IConnection::Execute_Connect(Client, ConnectionDelegate);
}

FName UConnectionStrategyComponent::GetLabelName(EClientLabels Label) const
{
	for (const auto& Pair : ClientCollection->ClientNames)
		if (Pair.Value == Label) return Pair.Key;
	return FName();
}

FLinearColor UConnectionStrategyComponent::DefineClientColor(EClientLabels Label) const
{
	const FColor* ColorPtr = ClientCollection->ClientColors.Find(Label);
	const FColor Color = ColorPtr ? *ColorPtr : ClientCollection->DefaultColor;
	return Color.ReinterpretAsLinear();
}

bool UConnectionStrategyComponent::CheckStrategyConnection(UObject* Client)
{
	return UKismetSystemLibrary::DoesImplementInterface(Client, UConnection::StaticClass());
}

void UConnectionStrategyComponent::OnClientConnect(EClientLabels Type, bool bSuccess)
{
	if (ConnectionEvent) ConnectionEvent(Type, bSuccess);
}

void UConnectionStrategyComponent::OnClientDisconnect(EClientLabels Type, bool bSuccess)
{
	if (DisonnectionEvent) DisonnectionEvent(Type, bSuccess);
}

