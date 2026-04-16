// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Widgets/ModularPlayerInputKey.h"

#include "CommonInputSubsystem.h"

DECLARE_LOG_CATEGORY_EXTERN(LogModularPlayerInput, Log, All); //注册日志
DEFINE_LOG_CATEGORY(LogModularPlayerInput); //注册日志

//工具
struct FSlateDrawUtil
{
	static void DrawBrushCenterFit(
		FSlateWindowElementList& ElementList,
		uint32 InLayer,
		const FGeometry& InAllottedGeometry,
		const FSlateBrush* InBrush,
		const FLinearColor& InTint = FLinearColor::White)
	{
		DrawBrushCenterFitWithOffset
		(
			ElementList,
			InLayer,
			InAllottedGeometry,
			InBrush,
			InTint,
			FVector2D(0, 0)
		);
	}

	static void DrawBrushCenterFitWithOffset(
		FSlateWindowElementList& ElementList,
		uint32 InLayer,
		const FGeometry& InAllottedGeometry,
		const FSlateBrush* InBrush,
		const FLinearColor& InTint,
		const FVector2D InOffset)
	{
		if (!InBrush)
		{
			return;
		}

		const FVector2D AreaSize = InAllottedGeometry.GetLocalSize();
		const FVector2D ProgressSize = InBrush->GetImageSize();
		const float FitScale = FMath::Min(FMath::Min(AreaSize.X / ProgressSize.X, AreaSize.Y / ProgressSize.Y), 1.0f);
		const FVector2D FinalSize = FitScale * ProgressSize;

		const FVector2D Offset = (InAllottedGeometry.GetLocalSize() * 0.5f) - (FinalSize * 0.5f) + InOffset;

		FSlateDrawElement::MakeBox
		(
			ElementList,
			InLayer,
			InAllottedGeometry.ToPaintGeometry(FinalSize, FSlateLayoutTransform(Offset)),
			InBrush,
			ESlateDrawEffect::None,
			InTint
		);
	}
};


void UModularPlayerInputKey::UpdateKeybindWidget()
{
	//TODO 这里应该是转换为自定义的玩家控制器
	// if (!GetOwningPlayer<UCommonLocalPlayer>())
	if (!GetOwningPlayer())
	{
		bWaitingForPlayerController = true;
		return;
	}

	const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();

	//没有需要显示的按键提示,就直接折叠自身
	if (CommonInputSubsystem && !CommonInputSubsystem->ShouldShowInputKeys())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 检查输入类型
	const bool bIsUsingGamepad = (InputTypeOverride == ECommonInputType::Gamepad) || ((CommonInputSubsystem != nullptr) && (CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad));

	if (!BoundKey.IsValid())
	{
		BoundKey = BoundKeyFallback;
	}
	UE_LOG(LogModularPlayerInput, Verbose, TEXT("UModularPlayerInputKey::UpdateKeybindWidget: Action: %s Key: %s"), *(BoundAction.ToString()), *(BoundKey.ToString()));

	// 必须在更新前调用，因为创建了 ProgressPercentageMID 并将用于更新
	SetupHoldKeybind();

	bool NewDrawBrushForKey = false;
	bool NeedToRecalcSize = false;

	if (BoundKey.IsValid())
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);

		ShowHoldBackPlate();

		NeedToRecalcSize = true;
	}
	else
	{
		if (bShowUnboundStatus)
		{
			SetVisibility(ESlateVisibility::HitTestInvisible);
			NewDrawBrushForKey = false;

			KeybindText.SetText(NSLOCTEXT("ModularPlayerInputKey", "Unbound", "Unbound"));

			NeedToRecalcSize = true;
		}
		else
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (bDrawBrushForKey != NewDrawBrushForKey)
	{
		bDrawBrushForKey = NewDrawBrushForKey;
		Invalidate(EInvalidateWidget::Paint);
	}

	// 由于 RecalculateDesiredSize 依赖于 bDrawBrushForKey，
	// 因此在更新过程中，我们不应在该值最终确定之前调用它。
	if (NeedToRecalcSize)
	{
		RecalculateDesiredSize();
	}
}

void UModularPlayerInputKey::SetBoundKey(FKey NewBoundAction)
{
	if (NewBoundAction != BoundKey)
	{
		BoundKeyFallback = NewBoundAction;
		BoundAction = NAME_None;
		UpdateKeybindWidget();
	}
}

void UModularPlayerInputKey::SetBoundAction(FName NewBoundAction)
{
	bool bUpdateWidget = true;

	if (BoundAction != NewBoundAction) BoundAction = NewBoundAction;

	if (bUpdateWidget) UpdateKeybindWidget();
}

void UModularPlayerInputKey::SetForcedHoldKeybindStatus(ECommonKeybindForcedHoldStatus InForcedHoldKeybindStatus)
{
	ForcedHoldKeybindStatus = InForcedHoldKeybindStatus;

	UpdateKeybindWidget();
}

void UModularPlayerInputKey::SetShowProgressCountDown(bool bShow)
{
	bShowTimeCountDown = bShow;
}

void UModularPlayerInputKey::StartHoldProgress(FName HoldActionName, float HoldDuration)
{
	if (HoldActionName == BoundAction && ensureMsgf(HoldDuration > 0.0f, TEXT("Trying to perform hold action \"%s\" with no HoldDuration"), *BoundAction.ToString()))
	{
		HoldKeybindDuration = HoldDuration;
		HoldKeybindStartTime = GetWorld()->GetRealTimeSeconds();

		UpdateHoldProgress();
	}
}

void UModularPlayerInputKey::StopHoldProgress(FName HoldActionName, bool bCompletedSuccessfully)
{
	if (HoldActionName == BoundAction)
	{
		HoldKeybindStartTime = 0.f;
		HoldKeybindDuration = 0.f;

		if (ensure(ProgressPercentageMID))
		{
			ProgressPercentageMID->SetScalarParameterValue(PercentageMaterialParameterName, 0.f);
		}

		if (bDrawCountdownText)
		{
			bDrawCountdownText = false;
			Invalidate(EInvalidateWidget::Paint);
			RecalculateDesiredSize();
		}
	}
}

void UModularPlayerInputKey::NativePreConstruct()
{
	Super::NativePreConstruct();

	UpdateKeybindWidget();

	if (IsDesignTime())
	{
		ShowHoldBackPlate();
		RecalculateDesiredSize();
	}
}

int32 UModularPlayerInputKey::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                                          bool bParentEnabled) const
{
	int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (bDrawProgress)
	{
		FSlateDrawUtil::DrawBrushCenterFit
		(
			OutDrawElements,
			++MaxLayer,
			AllottedGeometry,
			&HoldProgressBrush,
			FLinearColor(InWidgetStyle.GetColorAndOpacityTint() * HoldProgressBrush.GetTint(InWidgetStyle))
		);
	}

	if (bDrawCountdownText)
	{
		const FVector2D CountdownTextOffset = (AllottedGeometry.GetLocalSize() - CountdownText.GetTextSize()) * 0.5f;

		FSlateDrawElement::MakeText
		(
			OutDrawElements,
			++MaxLayer,
			AllottedGeometry.ToOffsetPaintGeometry(CountdownTextOffset),
			CountdownText.GetText(),
			CountdownTextFont,
			ESlateDrawEffect::None,
			FLinearColor(InWidgetStyle.GetColorAndOpacityTint())
		);
	}
	else if (bDrawBrushForKey)
	{
		// Draw Shadow
		FSlateDrawUtil::DrawBrushCenterFitWithOffset
		(
			OutDrawElements,
			++MaxLayer,
			AllottedGeometry,
			&CachedKeyBrush,
			FLinearColor(InWidgetStyle.GetColorAndOpacityTint() * FLinearColor::Black),
			FVector2D(1, 1)
		);

		FSlateDrawUtil::DrawBrushCenterFit
		(
			OutDrawElements,
			++MaxLayer,
			AllottedGeometry,
			&CachedKeyBrush,
			FLinearColor(InWidgetStyle.GetColorAndOpacityTint() * CachedKeyBrush.GetTint(InWidgetStyle))
		);
	}
	else if (KeybindText.GetTextSize().X > 0)
	{
		const FVector2D FrameOffset = (AllottedGeometry.GetLocalSize() - FrameSize) * 0.5f;

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			++MaxLayer,
			AllottedGeometry.ToPaintGeometry(FrameSize, FSlateLayoutTransform(FrameOffset)),
			&KeyBindTextBorder,
			ESlateDrawEffect::None,
			FLinearColor(InWidgetStyle.GetColorAndOpacityTint() * KeyBindTextBorder.GetTint(InWidgetStyle))
		);

		const FVector2D ActionTextOffset = (AllottedGeometry.GetLocalSize() - KeybindText.GetTextSize()) * 0.5f;

		FSlateDrawElement::MakeText
		(
			OutDrawElements,
			++MaxLayer,
			AllottedGeometry.ToOffsetPaintGeometry(ActionTextOffset),
			KeybindText.GetText(),
			KeyBindTextFont,
			ESlateDrawEffect::None,
			FLinearColor(InWidgetStyle.GetColorAndOpacityTint())
		);
	}

	return MaxLayer;
}

void UModularPlayerInputKey::RecalculateDesiredSize()
{
	FVector2D MaximumDesiredSize(0, 0);
	float LayoutScale = 1;

	if (bDrawProgress)
	{
		MaximumDesiredSize = FVector2D::Max(MaximumDesiredSize, HoldProgressBrush.GetImageSize());
	}

	if (bDrawCountdownText)
	{
		MaximumDesiredSize = FVector2D::Max(MaximumDesiredSize, CountdownText.UpdateTextSize(CountdownTextFont, LayoutScale));
	}
	else if (bDrawBrushForKey)
	{
		MaximumDesiredSize = FVector2D::Max(MaximumDesiredSize, CachedKeyBrush.GetImageSize());
	}
	else
	{
		const FVector2D KeybindTextSize = KeybindText.UpdateTextSize(KeyBindTextFont, LayoutScale);
		FrameSize = FVector2D::Max(KeybindTextSize, KeybindFrameMinimumSize) + KeybindTextPadding.GetDesiredSize();
		MaximumDesiredSize = FVector2D::Max(MaximumDesiredSize, FrameSize);
	}

	SetMinimumDesiredSize(MaximumDesiredSize);
}

void UModularPlayerInputKey::NativeDestruct()
{
	if (ProgressPercentageMID)
	{
		// 在我们切断MID之前，需要恢复刷子上的材料。
		HoldProgressBrush.SetResourceObject(ProgressPercentageMID->GetMaterial());

		ProgressPercentageMID->MarkAsGarbage();
		ProgressPercentageMID = nullptr;
	}

	Super::NativeDestruct();
}

void UModularPlayerInputKey::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		//TODO LocalPlayer->OnPlayerControllerSet.AddUObject(this, &ThisClass::HandlePlayerControllerSet);
		LocalPlayer->OnPlayerControllerChanged().AddUObject(this, &ThisClass::HandlePlayerControllerSet);
	}
}

void UModularPlayerInputKey::SyncHoldProgress()
{
	// If we had an active hold action, stop it
	if (HoldKeybindStartTime > 0.f)
	{
		StopHoldProgress(BoundAction, false);
	}
}

void UModularPlayerInputKey::UpdateHoldProgress()
{
	if (HoldKeybindStartTime != 0.f && HoldKeybindDuration > 0.f)
	{
		const float CurrentTime = GetWorld()->GetRealTimeSeconds();
		const float ElapsedTime = FMath::Min(CurrentTime - HoldKeybindStartTime, HoldKeybindDuration);
		const float RemainingTime = FMath::Max(0.0f, HoldKeybindDuration - ElapsedTime);

		if (ElapsedTime < HoldKeybindDuration && ensure(ProgressPercentageMID))
		{
			const float HoldKeybindPercentage = ElapsedTime / HoldKeybindDuration;
			ProgressPercentageMID->SetScalarParameterValue(PercentageMaterialParameterName, HoldKeybindPercentage);

			// 自Tick
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateHoldProgress);
		}

		if (bShowTimeCountDown)
		{
			FNumberFormattingOptions Options;
			Options.MinimumFractionalDigits = 1;
			Options.MaximumFractionalDigits = 1;
			CountdownText.SetText(FText::AsNumber(RemainingTime, &Options));

			bDrawCountdownText = true;
			Invalidate(EInvalidateWidget::Paint);
			RecalculateDesiredSize();
		}
	}
}

void UModularPlayerInputKey::SetupHoldKeybind()
{
	APlayerController* OwningCommonPlayer = GetOwningPlayer();

	// Setup the hold
	if (ForcedHoldKeybindStatus == ECommonKeybindForcedHoldStatus::ForcedHold)
	{
		bIsHoldKeybind = true;
	}
	else if (ForcedHoldKeybindStatus == ECommonKeybindForcedHoldStatus::NeverShowHold)
	{
		bIsHoldKeybind = false;
	}

	if (ensure(OwningCommonPlayer))
	{
		if (bIsHoldKeybind)
		{
			// Setup the ProgressPercentageMID
			if (ProgressPercentageMID == nullptr)
			{
				if (UMaterialInterface* Material = Cast<UMaterialInterface>(HoldProgressBrush.GetResourceObject()))
				{
					ProgressPercentageMID = UMaterialInstanceDynamic::Create(Material, this);
					HoldProgressBrush.SetResourceObject(ProgressPercentageMID);
				}
			}
			SyncHoldProgress();
		}
	}
}

void UModularPlayerInputKey::ShowHoldBackPlate()
{
	bool bDirty = false;

	if (IsHoldKeybind())
	{
		float BrushSizeAsValue = 32.0f;

		float DesiredBoxSize = BrushSizeAsValue + 10.0f;
		if (!bDrawBrushForKey)
		{
			DesiredBoxSize += 14.0f;
		}

		const FVector2D NewDesiredBrushSize(DesiredBoxSize, DesiredBoxSize);
		if (HoldProgressBrush.GetImageSize() != NewDesiredBrushSize)
		{
			HoldProgressBrush.SetImageSize(NewDesiredBrushSize);
			bDirty = true;
		}

		if (!bDrawProgress)
		{
			bDrawProgress = true;
			bDirty = true;
		}

		static const FName BackAlphaName = TEXT("BackAlpha");
		static const FName OutlineAlphaName = TEXT("OutlineAlpha");

		if (ProgressPercentageMID)
		{
			ProgressPercentageMID->SetScalarParameterValue(BackAlphaName, 0.2f);
			ProgressPercentageMID->SetScalarParameterValue(OutlineAlphaName, 0.4f);
		}
	}
	else
	{
		if (bDrawProgress)
		{
			bDrawProgress = false;
			bDirty = true;
		}
	}

	if (bDirty)
	{
		Invalidate(EInvalidateWidget::Paint);
	}
}

void UModularPlayerInputKey::HandlePlayerControllerSet(APlayerController* NewPC)
{
	if (bWaitingForPlayerController && GetOwningPlayer())
	{
		UpdateKeybindWidget();
		bWaitingForPlayerController = false;
	}
}
