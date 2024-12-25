// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "LobbySettingsWidgetComponent.generated.h"

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
class BLASTER_API ULobbySettingsWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called as a result of LobbySettingsWidget BP events:
	UFUNCTION(BlueprintCallable)
	void SelectMap(FString SelectedMapPathString);

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void KickPlayer(APlayerState* PlayerToBeKicked);

	// Rep events:
	UFUNCTION()
	void OnRep_SelectedMapPath(FString LastMapPath);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReplicatedSelectedMapPath();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Category = "Settings", EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FMapType> Maps = { };

	UPROPERTY(ReplicatedUsing = OnRep_SelectedMapPath, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FString SelectedMapPath = TEXT("");

	class ALobbyGameMode* LobbyGameMode;
	ALobbyGameMode* GetLobbyGameMode();
};
