#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"

namespace TestUtils
{
    UWorld* GetGameWorld();
    bool AreLinearColorsEqual(const FLinearColor& ColorA, const FLinearColor& ColorB, float Tolerance = KINDA_SMALL_NUMBER);

} // namespace TestUtils

#endif
