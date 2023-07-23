#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/TestUtils.h"

#include "GameFramework/Character.h"
#include "ActivatorCircle/StrategyActivator.h"
#include "DataAssets/ClientCollection.h"

#include "Clients/ClientFactory.h"
#include "Clients/Strategies/Test/HttpTestClient.h"
#include "Clients/Strategies/Test/WsTestClient.h"
#include "Clients/Strategies/Test/UdpTestClient.h"
#include "Clients/Strategies/Test/TcpTestClient.h"

namespace TestUtils
{
    UWorld* GetGameWorld()
    {
        const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
        for (const auto& Context : WorldContexts)
            if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
                return Context.World();
        return nullptr;
    }

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

    TArray<AActor*> SpawnActivators(const TArray<EClientLabels>& PresentActivators, UWorld* World, TSubclassOf<AActor>& ActivatorClass, const UClientCollection* ClientStyles)
    {
        TArray<AActor*> Actors;
        const FName ActivatorsContainer = "Activators";
        FTransform Transform;
        Transform.SetLocation(FVector(-50, -800, 0));
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

    FVector2D GetMovementInput(ACharacter* Character, const FVector& DirectionToTarget)
    {
        // Calculate the forward and right vectors from the direction
        const FRotator Rotation = Character->GetController()->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Calculate the dot product of the direction to the target and the forward vector
        float ForwardInput = FVector::DotProduct(DirectionToTarget, ForwardDirection);

        // Calculate the dot product of the direction to the target and the right vector
        float RightInput = FVector::DotProduct(DirectionToTarget, RightDirection);

		return { RightInput, ForwardInput };
    }
} // namespace TestUtils

#endif
