#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/TestUtils.h"

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

    bool AreLinearColorsEqual(const FLinearColor& ColorA, const FLinearColor& ColorB, float Tolerance)
    {
        return FMath::Abs(ColorA.R - ColorB.R) < Tolerance &&
            FMath::Abs(ColorA.G - ColorB.G) < Tolerance &&
            FMath::Abs(ColorA.B - ColorB.B) < Tolerance &&
            FMath::Abs(ColorA.A - ColorB.A) < Tolerance;
    }

} // namespace TestUtils

#endif
