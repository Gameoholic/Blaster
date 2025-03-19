// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BLASTER_API Item
{
public:
	Item();
	~Item();

private:
	UPROPERTY(EditDefaultsOnly)
	FString Name;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Icon;

public:
	FORCEINLINE FString GetName() { return Name; }
	FORCEINLINE FSlateBrush GetIcon() { return Icon; }

};
