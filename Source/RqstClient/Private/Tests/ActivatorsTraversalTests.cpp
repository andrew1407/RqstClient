#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/ActivatorsTraversalTests.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "Engine/AssetManager.h"
#include "GameFramework/Character.h"

#include "Tests/TestUtils.h"
#include "Kismet/GameplayStatics.h"

#include "Clients/Interfaces/ClientContainer.h"
#include "Clients/ClientLabels.h"
#include "Clients/ClientFactory.h"

#include "Clients/Strategies/Test/HttpTestClient.h"
#include "Clients/Strategies/Test/WsTestClient.h"
#include "Clients/Strategies/Test/UdpTestClient.h"
#include "Clients/Strategies/Test/TcpTestClient.h"

#include "DataAssets/ClientCollection.h"

#include "RqstClientGameMode.h"
#include "ActivatorCircle/StrategyActivator.h"

using namespace TestUtils;

namespace
{
    constexpr char* TEST_MAP_PATH = "/Game/Core/Tests/Maps/ActivatorsTestMap.ActivatorsTestMap";
    constexpr char* ACTIVATOR_BLUEPRINT_PATH = "/Script/Engine.Blueprint'/Game/Core/Blueprints/Gameplay/BP_StrategyActivator.BP_StrategyActivator'";
    constexpr char* CLIENT_STYLES_PATH = "/ClientConnectionStrategies/DataAssets/DA_ClientCollection";

    TArray<FName> GetOrderedNames(const TMap<FName, EClientLabels>& Collection)
    {
        TSet<FName> NamesSet;
        Collection.GetKeys(NamesSet);
        TArray<FName> Names = NamesSet.Array();
        int32 NoneIndex = Names.IndexOfByPredicate([] (const FName& Name) { return Name.IsEqual(TEXT("NONE")); });
        if (NoneIndex != INDEX_NONE)
        {
            const FName NoneName = Names[NoneIndex];
            Names.RemoveAt(NoneIndex);
            Names.Add(NoneName);
        }
        return MoveTemp(Names);
    }

    FName GetNameByLabel(const TMap<FName, EClientLabels>& Collection, EClientLabels Label)
    {
        for (const auto& Pair : Collection)
            if (Pair.Value == Label) return Pair.Key;
        return NAME_None;
    }

    TArray<AActor*> SpawnActors(const TArray<EClientLabels>& PresentActivators, UWorld* World, TSubclassOf<AActor>& ActivatorClass, const UClientCollection* ClientStyles)
    {
        TArray<AActor*> Actors;
        const FName ActivatorsContainer = "Activators";
        FTransform Transform;
        Transform.SetLocation(FVector(180, -730, 0));
        const auto Distance = FVector(0, 400, 0);
        for (auto Label : PresentActivators)
        {
            FTransform SpawnTransform = Transform;
            auto Activator = World->SpawnActorDeferred<AActor>(ActivatorClass, Transform);
            const FName Name = GetNameByLabel(ClientStyles->ClientNames, Label);
            const FString LabelName = FString::Printf(TEXT("Activator_%s"), *Name.ToString());

            Activator->SetActorLabel(LabelName);
            Activator->SetFolderPath(ActivatorsContainer);
            IStrategyActivator::Execute_SetClientType(Activator, Label);
            Activator->FinishSpawning(SpawnTransform);

            Transform.SetLocation(Transform.GetLocation() + Distance);
            Actors.Add(Activator);
        }
        return MoveTemp(Actors);
    }

    void SetFactoryStubClasses(UClientFactory* ClientFactory)
    {
        ClientFactory->SetHttpInstance(UHttpTestClient::StaticClass());
        ClientFactory->SetWsInstance(UWsTestClient::StaticClass());
        ClientFactory->SetUdpInstance(UUdpTestClient::StaticClass());
        ClientFactory->SetTcpInstance(UTcpTestClient::StaticClass());
    }
}

BEGIN_DEFINE_SPEC(FActivatorsTraversal, "RqstClient.ActivatorsTraversal",
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
            bool bEqual = AreLinearColorsEqual(ResultColor, ExpectedColor);

            TestTrue(FString::Printf(TEXT("MAterial with id = %d has tint"), MeshId), bHasTint);
            TestTrue(FString::Printf(TEXT("MAterial with id = %d has valid color"), MeshId), bEqual);
        }
    }

END_DEFINE_SPEC(FActivatorsTraversal);

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FAvtivatorTimeoutInterval, float, Duration, int8, Index, int8, Count, TFunction<void(int8)>, OnCheck);
bool FAvtivatorTimeoutInterval::Update()
{
    if (FPlatformTime::Seconds() - StartTime < Duration) return false;
    if (Index < Count)
    {
        OnCheck(Index);
        ADD_LATENT_AUTOMATION_COMMAND(FAvtivatorTimeoutInterval(Duration, Index + 1, Count, OnCheck));
    }
    else
    {
        ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);
    }
    return true;
}

void FActivatorsTraversal::Define()
{
    It("SetsCorrentClientTypeToContainer", [this]
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

        TArray<AActor*> Activators = SpawnActors(PresentActivators, World, ActivatorClass, ClientStyles);
        TestTrueExpr(PresentActivators.Num() == Activators.Num());
        for (const auto& Actor : Activators)
            TestNotNull(FString::Printf(TEXT("Activator \"%d\" swapned"), *Actor->GetActorLabel()), Actor);

        ACharacter* Character = World->GetFirstPlayerController()->GetCharacter();
        TestNotNull("Character exists", Character);

        const int8 DurationPeriod = 1;
        const int8 ActivatorsCount = Activators.Num();
        TArray<FName> Names = GetOrderedNames(ClientStyles->ClientNames);

        const auto OnCheck = [this, ClientStyles, Character, GameMode, Activators = MoveTemp(Activators), Names = MoveTemp(Names)] (int8 Index) -> void
        {
            AActor* Activator = Activators[Index];
            const EClientLabels ExpentedLabel = IStrategyActivator::Execute_GetClientType(Activator);
            const FName Name = GetNameByLabel(ClientStyles->ClientNames, ExpentedLabel);
            
            const FString NameWhat = FString::Printf(TEXT("\"%s\" activator has name \"%s\""), *Activator->GetActorLabel(), *Name.ToString());
            bool NameValid = ExpentedLabel == EClientLabels::NONE ? !Name.IsNone() : Name.IsNone();
            TestFalse(NameWhat, NameValid);

            Character->SetActorLocation(Activator->GetActorLocation());
            const EClientLabels ResultLabel = IClientContainer::Execute_GetClientType(GameMode);
            const FString LabelWhat = FString::Printf(TEXT("\"%d\" activator with type \"%d\" set state type \"%d\""), *Activator->GetActorLabel(), ResultLabel, ExpentedLabel);
            TestEqual(LabelWhat, ResultLabel, ExpentedLabel);

            const FColor ExpectedColor = ClientStyles->ClientColors.Contains(ExpentedLabel) ? ClientStyles->ClientColors[ExpentedLabel] : ClientStyles->DefaultColor;
        	const TArray<uint8> MeshIndices = { 0, 1 };
            CheckMaterialColors(Character, ExpectedColor.ReinterpretAsLinear(), MeshIndices);
        };

        ADD_LATENT_AUTOMATION_COMMAND(FAvtivatorTimeoutInterval(DurationPeriod, 0, ActivatorsCount, OnCheck));
    });
}

#endif
