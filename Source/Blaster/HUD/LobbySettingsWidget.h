// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbySettingsWidget.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMapType
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TAssetPtr<UWorld> Map;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UTexture2D* Image;
};

UCLASS()
class BLASTER_API ULobbySettingsWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable)
	void SelectMap(FString SelectedMapPathString);

	UFUNCTION(BlueprintCallable)
	void StartGame();
private:
	UPROPERTY(Category = "Settings", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FMapType> Maps = { };

	FString SelectedMapPath = TEXT("");
};
