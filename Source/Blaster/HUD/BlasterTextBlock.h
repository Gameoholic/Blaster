// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "BlasterTextBlock.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterTextBlock : public UTextBlock
{
	GENERATED_BODY()
	

public:
	void SetWrappingPolicy(ETextWrappingPolicy WrappingPolicy);
};
