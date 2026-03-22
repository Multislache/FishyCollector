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
    
    UPROPERTY(BlueprintReadOnly)
    float Prix = 0.f;
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

    // =========================
    // POIDS
    // =========================
    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static FText GetPoidsTexte(const FFishCatchRecord& Record)
    {
        const float Poids = Record.Poids;

        // Fonction pour nettoyer les zéros inutiles
        auto FormatFloat = [](float Value) -> FString
            {
                FString Str = FString::Printf(TEXT("%.2f"), Value);

                // Supprime les zéros à la fin
                Str.RemoveFromEnd(TEXT("0"));
                Str.RemoveFromEnd(TEXT("0"));
                Str.RemoveFromEnd(TEXT("."));

                return Str;
            };

        if (Poids < 1.f)
        {
            return FText::FromString(FormatFloat(Poids * 1000.f) + TEXT(" g"));
        }

        if (Poids < 1000.f)
        {
            return FText::FromString(FormatFloat(Poids) + TEXT(" kg"));
        }

        return FText::FromString(FormatFloat(Poids / 1000.f) + TEXT(" t"));
    }

    // =========================
    // TAILLE
    // =========================
    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static FText GetTailleTexte(const FFishCatchRecord& Record)
    {
        const float Taille = Record.Taille;

        auto FormatFloat = [](float Value) -> FString
            {
                FString Str = FString::Printf(TEXT("%.2f"), Value);
                Str.RemoveFromEnd(TEXT("0"));
                Str.RemoveFromEnd(TEXT("0"));
                Str.RemoveFromEnd(TEXT("."));
                return Str;
            };

        if (Taille < 1.f)
        {
            return FText::FromString(FormatFloat(Taille * 10.f) + TEXT(" mm"));
        }

        if (Taille < 100.f)
        {
            return FText::FromString(FormatFloat(Taille) + TEXT(" cm"));
        }

        return FText::FromString(FormatFloat(Taille / 100.f) + TEXT(" m"));
    }

    // =========================
    // TEXTE COMPLET
    // =========================
    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static FText GetCaptureTexte(const FFishCatchRecord& Record)
    {
        FString PoidsStr = GetPoidsTexte(Record).ToString();
        FString TailleStr = GetTailleTexte(Record).ToString();

        return FText::FromString(PoidsStr + TEXT(" et ") + TailleStr);
    }

    // =========================
    // PRIX
    // =========================
    UFUNCTION(BlueprintPure, Category = "Inventaire")
    static float GetPrix(const FFishCatchRecord& Record)
    {
        return Record.Prix;
    }
};