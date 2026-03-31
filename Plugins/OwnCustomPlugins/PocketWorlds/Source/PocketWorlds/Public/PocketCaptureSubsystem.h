// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PocketCaptureSubsystem.generated.h"

#define UE_API POCKETWORLDS_API
class UPocketCapture;
/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType)
class UPocketCaptureSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API UPocketCaptureSubsystem();

	// Begin USubsystem
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	// End USubsystem

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "PocketCaptureClass"))
	UE_API UPocketCapture* CreateThumbnailRenderer(TSubclassOf<UPocketCapture> PocketCaptureClass);

	UFUNCTION(BlueprintCallable)
	UE_API void DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer);

	UE_API void StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents);

protected:
	UE_API bool Tick(float DeltaTime);

	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamNextFrame;
	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamedLastFrameButNotNext;

private:
	TArray<TWeakObjectPtr<UPocketCapture>> ThumbnailRenderers;

	FTSTicker::FDelegateHandle TickHandle;
};
#undef UE_API
