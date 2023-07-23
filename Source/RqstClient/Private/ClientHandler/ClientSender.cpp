// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientHandler/ClientSender.h"
#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"

#include "Clients/ClientLabels.h"

#include "Clients/Interfaces/Client.h"
#include "Clients/Interfaces/Connection.h"
#include "Clients/Interfaces/ClientContainer.h"

#include "Clients/DataContainers/ResponseData.h"

#include "DataAssets/ClientCollection.h"
#include "DataAssets/DefaultFieldsFilling.h"

AClientSender::AClientSender(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	if (StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh")))
		StaticMesh->SetupAttachment(RootComponent);

	if (BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision")))
		BoxCollision->SetupAttachment(RootComponent);

	const FColor InputColor { 255, 55, 0 };
	CreateTextBlock(InterractionText, GET_MEMBER_NAME_CHECKED(AClientSender, InterractionText), TEXT("E (B)"), FColor(255, 0, 72), 210);
	CreateTextBlock(DenominationsText, GET_MEMBER_NAME_CHECKED(AClientSender, DenominationsText), TEXT("Denominations: ..."), InputColor, 380);
	CreateTextBlock(AmountText, GET_MEMBER_NAME_CHECKED(AClientSender, AmountText), TEXT("Amount: ..."), InputColor, 320);
	CreateTextBlock(ResultText, GET_MEMBER_NAME_CHECKED(AClientSender, ResultText), TEXT("Result: NaN"), FColor::White, 260);

	if (InterractionText) InterractionText->SetVisibility(false);
}

void AClientSender::CreateTextBlock(TObjectPtr<UTextRenderComponent>& Component, const FName& Name, const FString& Entries, const FColor& Color, float Top)
{
	const uint16 TextSize = 50;
	if (Component = CreateDefaultSubobject<UTextRenderComponent>(Name))
	{
		Component->SetupAttachment(RootComponent);
		Component->SetText(FText::FromString(Entries));
		Component->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
		Component->SetTextRenderColor(Color);
		Component->SetWorldSize(TextSize);
		Component->SetRelativeLocationAndRotation({ 0, 0, Top }, { 0, 90, 0 });
	}
}

void AClientSender::Interract_Implementation()
{
	if (IsActionTriggered) return;
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    bool IsClientContainer = UKismetSystemLibrary::DoesImplementInterface(GameMode, UClientContainer::StaticClass());
	if (!IsClientContainer) return;
	EClientLabels Label = IClientContainer::Execute_GetClientType(GameMode);
	if (Label == EClientLabels::NONE) return;
	UObject* Client = IClientContainer::Execute_GetClient(GameMode);
	bool IsAbleToSend = IsValid(Client) && UKismetSystemLibrary::DoesImplementInterface(Client, UClient::StaticClass());
	bool IsConnection = UKismetSystemLibrary::DoesImplementInterface(Client, UConnection::StaticClass());
	if (IsConnection && IsAbleToSend) IsAbleToSend = IConnection::Execute_Connected(Client);
	if (!IsAbleToSend) return;
	IsActionTriggered = true;
	FResponseDelegate ResponseDelegate;
	ResponseDelegate.BindDynamic(this, &AClientSender::OnDataReceived);
	IClient::Execute_SetResponseDelegate(Client, ResponseDelegate);
	IClient::Execute_Send(Client, DataToSend);
}

void AClientSender::SetDenominations(const TArray<int32>& Denominations)
{
	DataToSend.Denominations = Denominations;
	const auto& NumStringifier = [] (const int32& Number) { return FString::Printf(TEXT("%d"), Number); };
    FString Stringified = FString::JoinBy(Denominations, TEXT(", "), NumStringifier);
	const FString Title = FString::Printf(TEXT("Denominations: %s"), *Stringified);
	DenominationsText->SetText(FText::FromString(Title));
}

void AClientSender::SetAmount(int32 Amount)
{
	DataToSend.Amount = Amount;
	const FString Title = FString::Printf(TEXT("Amount: %d"), Amount);
	AmountText->SetText(FText::FromString(Title));
}

void AClientSender::OnDataReceived(const FResponseData& ResponseData, bool bSuccess)
{
    bool IsStylesSet = ClientStyles && ClientStyles->IsValidLowLevel();
	if (bSuccess)
	{
		const FString Result = FString::Printf(TEXT("Result: %d"), ResponseData.Result);
		ResultText->SetText(FText::FromString(Result));
		ResultText->SetTextRenderColor(IsStylesSet ? GetClientColor() : FColor::White);
	}
	else
	{
		ResultText->SetText(FText::FromString(TEXT("Result: NaN")));
		ResultText->SetTextRenderColor(IsStylesSet ? ClientStyles->ErrorColor : FColor::Red);
	}
	IsActionTriggered = false;
}

FColor AClientSender::GetClientColor()
{
	const FColor DefaultColor = FColor::White;
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    bool IsClientContainer = UKismetSystemLibrary::DoesImplementInterface(GameMode, UClientContainer::StaticClass());
	if (!IsClientContainer) return DefaultColor;
	EClientLabels Label = IClientContainer::Execute_GetClientType(GameMode);
	const FColor* ColorPtr = ClientStyles->ClientColors.Find(Label);
	return ColorPtr ? *ColorPtr : DefaultColor;
}

// Called when the game starts or when spawned
void AClientSender::BeginPlay()
{
	Super::BeginPlay();
	
	if (DefaultOptions && DefaultOptions->IsValidLowLevel())
	{
		SetAmount(DefaultOptions->Amount);
		SetDenominations(DefaultOptions->Denominations);
	}
}

void AClientSender::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (auto Character = Cast<ACharacter>(OtherActor)) InterractionText->SetVisibility(true);
}

void AClientSender::NotifyActorEndOverlap(AActor* OtherActor)
{
	if (auto Character = Cast<ACharacter>(OtherActor)) InterractionText->SetVisibility(false);
}
