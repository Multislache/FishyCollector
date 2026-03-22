#pragma once

#include "CoreMinimal.h"

class FISHYCOLLECTOR_API FishUtils
{
public:

    static FString FormatTaille(float TailleCm)
    {
        if (TailleCm < 1.0f)
        {
            return FString::Printf(TEXT("%.0f mm"), TailleCm * 10.f);
        }

        if (TailleCm < 100.f)
        {
            return FString::Printf(TEXT("%.1f cm"), TailleCm);
        }

        return FString::Printf(TEXT("%.2f m"), TailleCm / 100.f);
    }

    static FString FormatPoids(float PoidsKg)
    {
        if (PoidsKg < 1.f)
        {
            return FString::Printf(TEXT("%.0f g"), PoidsKg * 1000.f);
        }

        if (PoidsKg < 1000.f)
        {
            return FString::Printf(TEXT("%.2f kg"), PoidsKg);
        }

        return FString::Printf(TEXT("%.2f t"), PoidsKg / 1000.f);
    }
};