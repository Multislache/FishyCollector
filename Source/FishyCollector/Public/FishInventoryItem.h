#pragma once

#include "CoreMinimal.h"
#include "PoissonTemplate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FishInventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FFishCatchRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float Poids = 0.f;

    UPROPERTY(BlueprintReadOnly)
    float Taille = 0.f;
};

USTRUCT(BlueprintType)
struct FFishInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    UPoissonTemplate* Fish = nullptr;

    UPROPERTY(BlueprintReadOnly)
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadOnly)
    FFishCatchRecord Capture;
};

UCLASS()
class FISHYCOLLECTOR_API UFishCatchRecordLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static FText GetPoidsTexte(const FFishCatchRecord& Record)
    {
        return FText::FromString(FString::Printf(TEXT("%.1f kg"), Record.Poids));
    }

    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static FText GetTailleTexte(const FFishCatchRecord& Record)
    {
        return FText::FromString(FString::Printf(TEXT("%.1f cm"), Record.Taille));
    }

    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static FText GetCaptureTexte(const FFishCatchRecord& Record)
    {
        return FText::FromString(FString::Printf(TEXT("%.1f kg et %.1f cm"), Record.Poids, Record.Taille));
    }
};