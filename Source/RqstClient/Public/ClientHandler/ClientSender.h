// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClientHandler/Interactive.h"
#include "Clients/DataContainers/RequestData.h"
#include "ClientSender.generated.h"

struct FResponseData;

class UStaticMeshComponent;
class UBoxComponent;
class UTextRenderComponent;

class UClientCollection;
class UDefaultFieldsFilling;

UCLASS()
class RQSTCLIENT_API AClientSender : public AActor, public IInteractive
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AClientSender(const FObjectInitializer& ObjectInitializer);

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual void Interract_Implementation();

	UFUNCTION(BlueprintCallable)
	void SetDenominations(const TArray<int32>& Denominations);

	UFUNCTION(BlueprintCallable)
	void SetAmount(int32 Amount);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClientSender|Assets", meta = (AllowPrivateAccess = true))
	TObjectPtr<UClientCollection> ClientStyles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClientSender|Assets", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDefaultFieldsFilling> DefaultOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClientSender|Interaction", meta = (AllowPrivateAccess = true))
	bool IsActionTriggered = false;

	#pragma region SCENE_COMPONENTS

	UPROPERTY(VisibleAnywhere, Category = "ClientSender|Interaction", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "ClientSender|Interaction", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleAnywhere, Category = "ClientSender|Interaction", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTextRenderComponent> InterractionText;

	UPROPERTY(VisibleAnywhere, Category = "ClientSender|Text", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTextRenderComponent> DenominationsText;

	UPROPERTY(VisibleAnywhere, Category = "ClientSender|Text", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTextRenderComponent> AmountText;

	UPROPERTY(VisibleAnywhere, Category = "ClientSender|Text", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTextRenderComponent> ResultText;

	#pragma endregion

	FRequestData DataToSend; 

	UFUNCTION()
	void OnDataReceived(const FResponseData& ResponseData, bool bSuccess);

	void CreateTextBlock(TObjectPtr<UTextRenderComponent>& Component, const FName& Name, const FString& Entries, const FColor& Color, float Top);
	FColor GetClientColor();
};
