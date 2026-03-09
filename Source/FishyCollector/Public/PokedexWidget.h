// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PokedexWidget.generated.h"

class UScrollBox;

UCLASS()
class FISHYCOLLECTOR_API UPokedexWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Rafraichir();

protected:
	virtual void NativeConstruct() override;

	// ScrollBox à nommer "ListePoissons" dans le Blueprint
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ListePoissons;
};
