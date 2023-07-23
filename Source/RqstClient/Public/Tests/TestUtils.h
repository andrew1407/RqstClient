#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Clients/ClientLabels.h"

class UClientCollection;
class UClientFactory;

namespace TestUtils
{
    UWorld* GetGameWorld();

    TArray<FName> GetOrderedNames(const TMap<FName, EClientLabels>& Collection);
    FName GetNameByLabel(const TMap<FName, EClientLabels>& Collection, EClientLabels Label);
    TArray<AActor*> SpawnActivators(const TArray<EClientLabels>& PresentActivators, UWorld* World, TSubclassOf<AActor>& ActivatorClass, const UClientCollection* ClientStyles);
    void SetFactoryStubClasses(UClientFactory* ClientFactory);

    FVector2D GetMovementInput(ACharacter* Character, const FVector& DirectionToTarget);

} // namespace TestUtils

#endif
