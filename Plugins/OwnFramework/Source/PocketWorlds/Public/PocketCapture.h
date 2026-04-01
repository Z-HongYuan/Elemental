// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "PocketCapture.generated.h"

#define UE_API POCKETWORLDS_API

class UPocketCaptureManager;

/*捕获图像的对象引用*/
UCLASS(MinimalAPI, Abstract, Within=PocketCaptureManager, BlueprintType, Blueprintable)
class UPocketCapture : public UObject
{
	GENERATED_BODY()

public:
	UE_API UPocketCapture();

	UE_API virtual void Initialize(UWorld* InWorld, int32 InRendererIndex);
	UE_API virtual void Deinitialize();

	UE_API virtual void BeginDestroy() override;

	//设置渲染目标大小
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API void SetRenderTargetSize(int32 Width, int32 Height);

	//获取或创建Diffuse渲染目标
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API UTextureRenderTarget2D* GetOrCreateDiffuseRenderTarget();

	//获取或创建AlphaMask渲染目标
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API UTextureRenderTarget2D* GetOrCreateAlphaMaskRenderTarget();

	//获取或创建Effects渲染目标
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API UTextureRenderTarget2D* GetOrCreateEffectsRenderTarget();

	//设置渲染目标
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API void SetCaptureTarget(AActor* InCaptureTarget);

	//设置遮罩对象
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API void SetAlphaMaskedActors(const TArray<AActor*>& InCaptureTargets);

	//捕获Diffuse
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API void CaptureDiffuse();

	//捕获AlphaMask
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API void CaptureAlphaMask();

	//捕获Effects
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API void CaptureEffects();

	//释放资源
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API virtual void ReleaseResources();

	//更新资源
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API virtual void ReclaimResources();

	//获取渲染器索引
	UFUNCTION(BlueprintCallable, Category = "PocketWorlds")
	UE_API int32 GetRendererIndex() const;

protected:
	AActor* GetCaptureTarget() const { return CaptureTargetPtr.Get(); }

	virtual void OnCaptureTargetChanged(AActor* InCaptureTarget)
	{
	}

	UE_API bool CaptureScene(UTextureRenderTarget2D* InRenderTarget, const TArray<AActor*>& InCaptureActors, ESceneCaptureSource InCaptureSource, UMaterialInterface* OverrideMaterial);

protected:
	UE_API TArray<UPrimitiveComponent*> GatherPrimitivesForCapture(const TArray<AActor*>& InCaptureActors) const;

	UE_API UPocketCaptureManager* GetThumbnailSystem() const;

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> AlphaMaskMaterial;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> EffectMaskMaterial;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UWorld> PrivateWorld;

	UPROPERTY(Transient)
	int32 RendererIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere)
	int32 SurfaceWidth = 1;

	UPROPERTY(VisibleAnywhere)
	int32 SurfaceHeight = 1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> DiffuseRT;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> AlphaMaskRT;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> EffectsRT;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<AActor> CaptureTargetPtr;

	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<AActor>> AlphaMaskActorPtrs;
};

#undef UE_API
