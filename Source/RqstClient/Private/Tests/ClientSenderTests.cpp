// Fill out your copyright notice in the Description page of Project Settings.

#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/ClientSenderTests.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "Engine/AssetManager.h"
#include "GameFramework/Character.h"

#include "Tests/TestUtils.h"
#include "DataAssets/ClientCollection.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/TextRenderComponent.h"

#include "Clients/Interfaces/ClientContainer.h"
#include "Clients/ClientLabels.h"
#include "Clients/ClientFactory.h"

#include "RqstClientGameMode.h"
#include "Clients/Interfaces/ClientContainer.h"
#include "Clients/Interfaces/Client.h"
#include "Clients/Strategies/Test/ClientTestData.h"

#include "RqstClientCharacter.h"
#include "Input/CharacterInputActions.h"
#include "ClientHandler/ClientSender.h"
#include "ClientHandler/Interactive.h"

#include "Clients/Strategies/Test/HttpTestClient.h"
#include "Clients/Strategies/Test/WsTestClient.h"
#include "Clients/Strategies/Test/UdpTestClient.h"
#include "Clients/Strategies/Test/TcpTestClient.h"

using namespace TestUtils;

namespace
{
    constexpr char* TEST_MAP_PATH = "/Game/Core/Tests/Maps/ActivatorsTestMap.ActivatorsTestMap";
    constexpr char* ACTIVATOR_BLUEPRINT_PATH = "/Script/Engine.Blueprint'/Game/Core/Blueprints/Gameplay/BP_StrategyActivator.BP_StrategyActivator'";
    constexpr char* SENDER_BLUEPRINT_PATH = "/Script/Engine.Blueprint'/Game/Core/Blueprints/Gameplay/BP_AClientSender.BP_AClientSender'";
    constexpr char* CLIENT_STYLES_PATH = "/ClientConnectionStrategies/DataAssets/DA_ClientCollection";

    constexpr float PAUSE_DURATION = 1;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FDelayedAction, float, Duration, TFunction<void()>, Action);
bool FDelayedAction::Update()
{
    bool IsCompleted = FPlatformTime::Seconds() - StartTime >= Duration;
    if (IsCompleted) Action();
    return IsCompleted;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitUntilAction, TFunction<bool()>, OnTick, TFunction<void()>, Action);
bool FWaitUntilAction::Update()
{
    bool IsCompleted = OnTick();
    if (IsCompleted) Action();
    return IsCompleted;
}

BEGIN_DEFINE_SPEC(FClientSenderTests, "RqstClient.ClientSender",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

private:
    void GetCreatedEnvChecked(UWorld*& World, ARqstClientGameMode*& GameMode, TSubclassOf<AActor>& ActivatorClass)
    {
        bool IsWorldLoaded = AutomationOpenMap(TEST_MAP_PATH);
        TestTrueExpr(IsWorldLoaded);
        World = GetGameWorld();
        TestNotNull("Current test world created", World);

        GameMode = World->GetAuthGameMode<ARqstClientGameMode>();
        TestNotNull("Current game mode created", GameMode);
        TestNotNull("Current game mode is client container", Cast<IClientContainer>(GameMode));

        const UBlueprint* ActivatorBlueprint = LoadObject<UBlueprint>(nullptr, *FString(ACTIVATOR_BLUEPRINT_PATH));
        TestNotNull("Activator blueprint class loaded", ActivatorBlueprint);
        ActivatorClass = Cast<UClass, AActor>(ActivatorBlueprint->GeneratedClass.Get());
        TestNotNull("Activator class loaded", ActivatorClass.Get());
    }

    void CheckMaterialColors(ACharacter* Character, const FLinearColor& ExpectedColor, const TArray<uint8>& MeshIndices)
    {
        USkeletalMeshComponent* Mesh = Character->GetMesh();
        for (const auto MeshId : MeshIndices)
        {
            UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(MeshId));
            TestNotNull(FString::Printf(TEXT("Character dynamic mes present id = %d"), MeshId), Material);
            FLinearColor ResultColor;
            bool bHasTint = Material->GetVectorParameterValue(TEXT("Tint"), ResultColor);
            bool bEqual = ResultColor.Equals(ExpectedColor);

            TestTrue(FString::Printf(TEXT("MAterial with id = %d has tint"), MeshId), bHasTint);
            TestTrue(FString::Printf(TEXT("MAterial with id = %d has valid color"), MeshId), bEqual);
        }
    }

    AClientSender* GenerateClientSender(UWorld* World)
    {
        const UBlueprint* SenderBlueprint = LoadObject<UBlueprint>(nullptr, *FString(SENDER_BLUEPRINT_PATH));
        TestNotNull("Sender blueprint class loaded", SenderBlueprint);
        TSubclassOf<AClientSender> SenderClass = Cast<UClass, AClientSender>(SenderBlueprint->GeneratedClass.Get());
        TestNotNull("Sender class loaded", SenderClass.Get());

        FTransform Transform;
        Transform.SetLocation(FVector(400, 0, 0));
        Transform.SetRotation(FRotator(0, 90, 0).Quaternion());
        auto ClientSender = World->SpawnActorDeferred<AClientSender>(SenderClass, Transform);
        TestNotNull("Sender actor spawned", ClientSender);
        ClientSender->SetActorLabel(TEXT("ClientSender"));
        ClientSender->FinishSpawning(Transform);
        return ClientSender;
    }

    void WaitThenMove(UEnhancedPlayerInput* PlayerInput, ARqstClientCharacter* Character, AActor* Target, float MinDistance, TFunction<void()> Callback)
    {
        auto MoveUntil = [PlayerInput, Character, Target, MinDistance] () -> bool
        {
            const FVector CharacterLocation = Character->GetActorLocation();
            const FVector TargetLocation = Target->GetActorLocation();
            const FCharacterInputActions& InputActions = Character->GetInputActions();
            const FInputActionValue MovenemtInput = GetMovementInput(Character, TargetLocation - CharacterLocation);
            PlayerInput->InjectInputForAction(InputActions.MoveAction, MovenemtInput);
            const FVector2D CharacterPosition { CharacterLocation.X, CharacterLocation.Y };
            const FVector2D TargetPosition { TargetLocation.X, TargetLocation.Y };
            return (TargetPosition - CharacterPosition).Size() <= MinDistance;
        };
        // Wait to start run
        ADD_LATENT_AUTOMATION_COMMAND(FDelayedAction(PAUSE_DURATION, [MoveUntil = MoveTemp(MoveUntil), Callback = MoveTemp(Callback)]
        {
            // Run to react destination
            ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilAction(MoveUntil, Callback));
        }));
    }

    void RunCheckLoop(ARqstClientCharacter* Character, UEnhancedPlayerInput* PlayerInput, TArray<AActor*> Activators, AClientSender* ClientSender, TFunction<void(uint8)> Check, uint8 Index = 0)
    {
        if (Index >= Activators.Num())
        {
            ADD_LATENT_AUTOMATION_COMMAND(FDelayedAction(PAUSE_DURATION, []
            {
                ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);
            }))
            return;
        }

        AActor* Activator = Activators[Index];
        auto AfterReached = [this, Character, PlayerInput, Activators, ClientSender, Index, Check = MoveTemp(Check)]
        {
            Check(Index);
            RunCheckLoop(Character, PlayerInput, Activators, ClientSender, Check, Index + 1);
        };
        auto AfterReachedDelayed = [AfterReached = MoveTemp(AfterReached)]
        {
            ADD_LATENT_AUTOMATION_COMMAND(FDelayedAction(PAUSE_DURATION, AfterReached));
        };
        auto MoveToSender = [this, PlayerInput, Character, ClientSender, AfterReachedDelayed = MoveTemp(AfterReachedDelayed)]
        {
            const float MinDistance = 150;
            WaitThenMove(PlayerInput, Character, ClientSender, MinDistance, AfterReachedDelayed);
        };
        const float MinDistance = 50;
        WaitThenMove(PlayerInput, Character, Activator, MinDistance, MoveToSender);
    }

    void SetStubData(EClientLabels Label, UObject* ClientRaw, bool bStatus, const FResponseData& ResponseData)
    {
        switch (Label)
        {
            case EClientLabels::HTTP:
            {
                auto Client = CastChecked<UHttpTestClient>(ClientRaw);
                Client->ResponseStatus = bStatus;
                Client->ResponseData = ResponseData;
                break;
            }
            case EClientLabels::WS:
            {
                auto Client = CastChecked<UWsTestClient>(ClientRaw);
                Client->ResponseStatus = bStatus;
                Client->ResponseData = ResponseData;
                break;
            }
            case EClientLabels::UDP:
            {
                auto Client = CastChecked<UUdpTestClient>(ClientRaw);
                Client->ResponseStatus = bStatus;
                Client->ResponseData = ResponseData;
                break;
            }
            case EClientLabels::TCP:
            {
                auto Client = CastChecked<UTcpTestClient>(ClientRaw);
                Client->ResponseStatus = bStatus;
                Client->ResponseData = ResponseData;
                break;
            }
        }
    }

END_DEFINE_SPEC(FClientSenderTests);

void FClientSenderTests::Define()
{
    It("UsesEachClientToReceiveData", [this]
    {
        const TArray<EClientLabels> PresentActivators = {
            EClientLabels::HTTP,
            EClientLabels::WS,
            EClientLabels::UDP,
            EClientLabels::TCP,
            EClientLabels::NONE,
        };

        const UClientCollection* ClientStyles = UAssetManager::GetStreamableManager().LoadSynchronous<UClientCollection>(FString(CLIENT_STYLES_PATH));

        UWorld* World = nullptr;
        ARqstClientGameMode* GameMode = nullptr;
        TSubclassOf<AActor> ActivatorClass = nullptr;
        GetCreatedEnvChecked(World, GameMode, ActivatorClass);

        UClientFactory* ClientFactory = GameMode->GetClientFactory();
        TestNotNull("Game mode client factory exists", ClientFactory);
        SetFactoryStubClasses(ClientFactory);

        TArray<AActor*> Activators = SpawnActivators(PresentActivators, World, ActivatorClass, ClientStyles);
        TestTrueExpr(PresentActivators.Num() == Activators.Num());
        for (const auto& Actor : Activators)
            TestNotNull(FString::Printf(TEXT("Activator \"%d\" swapned"), *Actor->GetActorLabel()), Actor);

        auto Character = Cast<ARqstClientCharacter>(World->GetFirstPlayerController()->GetCharacter());
        TestNotNull("Character exists", Character);

        const int8 ActivatorsCount = Activators.Num();
        TArray<FName> Names = GetOrderedNames(ClientStyles->ClientNames);

        APlayerController* PlayerControlelr = World->GetFirstPlayerController();
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerControlelr->GetLocalPlayer());
        UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();

        AClientSender* ClientSender = GenerateClientSender(World);
        auto ResultTextBlock = Cast<UTextRenderComponent>(ClientSender->GetDefaultSubobjectByName(TEXT("ResultText")));
        TestNotNull("Result Text exists", ResultTextBlock);

        auto OnCheck = [this, PlayerInput, Character, GameMode, ClientStyles, ResultTextBlock] (uint8 Index)
        {
            const EClientLabels Label = IClientContainer::Execute_GetClientType(GameMode);
            const bool IsNone = Label == EClientLabels::NONE;
            const FString Name = GetNameByLabel(ClientStyles->ClientNames, Label).ToString();
            
            FString ExpectedTextValue;
            FColor ExpectedColor;

            if (IsNone)
            {
                ExpectedTextValue = ResultTextBlock->Text.ToString();
                ExpectedColor = ResultTextBlock->TextRenderColor;
            }
            else
            {
                const int32 ExpectedResult = Index + 1;
                const bool ExpectedStatus = true;
                UObject* ClientRaw = IClientContainer::Execute_GetClient(GameMode);
                SetStubData(Label, ClientRaw, ExpectedStatus, FResponseData{ExpectedResult});
                ExpectedTextValue = FString::Printf(TEXT("Result: %d"), ExpectedResult);
                ExpectedColor = ClientStyles->ClientColors.Contains(Label) ? ClientStyles->ClientColors[Label] : ClientStyles->DefaultColor;
            }

            const FCharacterInputActions& InputActions = Character->GetInputActions();
            PlayerInput->InjectInputForAction(InputActions.InteractAction, FInputActionValue(true));
            
            ADD_LATENT_AUTOMATION_COMMAND(FDelayedAction(.5f, [this, Name, ResultTextBlock, ExpectedTextValue, ExpectedColor]
            {
                const FString ResultWhat = FString::Printf(TEXT("Client \"%s\" must have text result: \"%d\""), *Name, *ExpectedTextValue);
                const FString ResultTextValue = ResultTextBlock->Text.ToString();
                TestEqual(ResultWhat, ResultTextValue, ExpectedTextValue);

                const FString ColortWhat = FString::Printf(TEXT("Client \"%s\" must have text color: %s"), *Name, *ExpectedColor.ToString());
                const FColor ResultColor = ResultTextBlock->TextRenderColor;
                TestEqual(ColortWhat, ResultColor, ExpectedColor);
            }));
        };

        RunCheckLoop(Character, PlayerInput, MoveTemp(Activators), ClientSender, MoveTemp(OnCheck));
    });
}

#endif
