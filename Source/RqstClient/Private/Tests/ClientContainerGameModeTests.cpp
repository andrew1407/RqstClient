// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS

#include "Tests/ClientContainerGameModeTests.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "Tests/TestUtils.h"

#include "RqstClientGameMode.h"
#include "Clients/Interfaces/ClientContainer.h"
#include "Clients/ClientLabels.h"

#include "Clients/Strategies/HttpClient.h"
#include "Clients/Strategies/WsClient.h"
#include "Clients/Strategies/UdpClient.h"
#include "Clients/Strategies/TcpClient.h"

using namespace TestUtils;

namespace
{
    constexpr char* TEST_MAP_PATH = "/Game/Core/Tests/Maps/GameModeTEstMap.GameModeTEstMap";
}

BEGIN_DEFINE_SPEC(FClientContainerGameMode, "RqstClient.ClientContainerGameMode",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

private:
    void CreateEnvironmentChecked(UWorld*& World, ARqstClientGameMode*& GameMode)
    {
        bool IsWorldLoaded = AutomationOpenMap(TEST_MAP_PATH);
        TestTrueExpr(IsWorldLoaded);
        World = GetGameWorld();
        TestNotNull("Current test world created", World);

        GameMode = World->GetAuthGameMode<ARqstClientGameMode>();
        TestNotNull("Current game mode created", GameMode);
        TestNotNull("Current game mode is client container", Cast<IClientContainer>(GameMode));
    }

END_DEFINE_SPEC(FClientContainerGameMode);

void FClientContainerGameMode::Define()
{
    Describe("HasValidLonatinerLogic", [this]
    {
        It("HasUninitializedDefaultValues", [this]
        {
            UWorld* World = nullptr;
            ARqstClientGameMode* GameMode = nullptr;
            CreateEnvironmentChecked(World, GameMode);

            TestTrueExpr(IClientContainer::Execute_GetClientType(GameMode) == EClientLabels::NONE);
            TestNull("Default cleint pointer must be null", IClientContainer::Execute_GetClient(GameMode));

            ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);
        });

        It("SetsClientsCorrectly", [this]
        {
            UWorld* World = nullptr;
            ARqstClientGameMode* GameMode = nullptr;
            CreateEnvironmentChecked(World, GameMode);

            const TMap<EClientLabels, const UClass*> ClientTable {
                { EClientLabels::HTTP, UHttpClient::StaticClass() },
                { EClientLabels::WS, UWsClient::StaticClass() },
                { EClientLabels::UDP, UUdpClient::StaticClass() },
                { EClientLabels::TCP, UTcpClient::StaticClass() },
            };

            for (const auto &[Label, Class] : ClientTable)
            {
                TestTrueExpr(IClientContainer::Execute_SetClient(GameMode, Label));
                const UObject* Client = IClientContainer::Execute_GetClient(GameMode);
                EClientLabels SetLabel = IClientContainer::Execute_GetClientType(GameMode);

                TestTrueExpr(Client->IsA(Class));
                TestTrueExpr(SetLabel == Label);
            }

            TestTrueExpr(IClientContainer::Execute_SetClient(GameMode, EClientLabels::NONE) == false);
            TestNull("None client type should set client as null", IClientContainer::Execute_GetClient(GameMode));
            TestTrueExpr(IClientContainer::Execute_GetClientType(GameMode) == EClientLabels::NONE);

            ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);
        });
    });
};

#endif
