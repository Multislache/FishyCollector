#pragma once

#include "CoreMinimal.h"
#include "PoissonTemplate.h"
#include "FishInventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FFishInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UPoissonTemplate* Fish = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;
};