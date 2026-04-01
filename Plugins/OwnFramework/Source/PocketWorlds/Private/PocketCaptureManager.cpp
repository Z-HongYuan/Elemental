// Copyright © 2026 鸿源z. All Rights Reserved.


#include "PocketWorlds/Public/PocketCaptureManager.h"

#include "PocketWorlds/Public/PocketCapture.h"

UPocketCaptureManager::UPocketCaptureManager()
{
}

void UPocketCaptureManager::Initialize(FSubsystemCollectionBase& Collection)
{
	// 实现Tick函数
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));
}

void UPocketCaptureManager::Deinitialize()
{
	//移除Tick函数
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	for (const TWeakObjectPtr<UPocketCapture>& Capture : ThumbnailRenderers)
		if (Capture.Get()) Capture->Deinitialize();

	// for (int32 RendererIndex = 0; RendererIndex < ThumbnailRenderers.Num(); RendererIndex++)
	// 	if (UPocketCapture* Renderer = ThumbnailRenderers[RendererIndex].Get())
	// 		Renderer->Deinitialize();

	ThumbnailRenderers.Reset();
}

UPocketCapture* UPocketCaptureManager::CreateThumbnailRenderer(TSubclassOf<UPocketCapture> PocketCaptureClass)
{
	UPocketCapture* Renderer = NewObject<UPocketCapture>(this, PocketCaptureClass);

	//添加或重载缩略图渲染器
	int32 RendererEmptyIndex = ThumbnailRenderers.IndexOfByKey(nullptr);
	if (RendererEmptyIndex == INDEX_NONE)
	{
		RendererEmptyIndex = ThumbnailRenderers.Add(Renderer);
	}
	else
	{
		ThumbnailRenderers[RendererEmptyIndex] = Renderer;
	}

	Renderer->Initialize(GetWorld(), RendererEmptyIndex);

	return Renderer;
}

void UPocketCaptureManager::DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer)
{
	if (!ThumbnailRenderer) return;

	// 从缩略图渲染器列表中删除,并释放资源
	const int32 ThumbnailIndex = ThumbnailRenderers.IndexOfByKey(ThumbnailRenderer);
	if (ThumbnailIndex != INDEX_NONE)
	{
		ThumbnailRenderers[ThumbnailIndex] = nullptr;
		ThumbnailRenderer->Deinitialize();
	}
}

void UPocketCaptureManager::StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents)
{
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		PrimitiveComponent->bForceMipStreaming = true;
		StreamedLastFrameButNotNext.Remove(PrimitiveComponent);
	}

	StreamNextFrame.Append(PrimitiveComponents);
}

bool UPocketCaptureManager::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_URealTimeThumbnailSubsystem_Tick);

	for (TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponent : StreamedLastFrameButNotNext)
		if (PrimitiveComponent.IsValid())
			PrimitiveComponent->bForceMipStreaming = false;

	StreamedLastFrameButNotNext = MoveTemp(StreamNextFrame);

	return true;
}
