// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CommonInputTypeEnum.h"
#include "CommonUserWidget.h"
#include "Fonts/FontMeasure.h"
#include "ModularPlayerInputKey.generated.h"

#define UE_API MODULARUI_API

enum class ECommonInputType : uint8;

/*按键提示的三个状态*/
UENUM(BlueprintType)
enum class ECommonKeybindForcedHoldStatus : uint8
{
	NoForcedHold, //当做普通按钮
	ForcedHold, //显示长按提示
	NeverShowHold //从不显示长按提示
};

/* 带缓存的文本测量工具，用于优化 UI 中文本尺寸计算的performance。它是 Slate UI 系统中常见的性能优化模式
 * 使用脏标记模式来更新
 */
USTRUCT()
struct FMeasuredText
{
	GENERATED_BODY()

public:
	FText GetText() const { return CachedText; }

	void SetText(const FText& InText)
	{
		CachedText = InText;
		bTextDirty = true;
	};

	FVector2D GetTextSize() const { return CachedTextSize; }

	FVector2D UpdateTextSize(const FSlateFontInfo& InFontInfo, float FontScale = 1.0f) const
	{
		if (bTextDirty)
		{
			bTextDirty = false;
			CachedTextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(CachedText, InFontInfo, FontScale);
		}
		return CachedTextSize;
	}

private:
	FText CachedText;
	mutable FVector2D CachedTextSize;
	mutable bool bTextDirty = true;
};

/*提供展示长按操作的控件*/
UCLASS(MinimalAPI, Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class UModularPlayerInputKey : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 根据我们当前的 Boundaction 更新按键和相关显示 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void UpdateKeybindWidget();

	/** keybind 设置绑定按键 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetBoundKey(FKey NewBoundAction);

	/** keybind 设置绑定操作 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetBoundAction(FName NewBoundAction);

	/** 强制此按键绑定为按住式绑定 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetForcedHoldKeybindStatus(ECommonKeybindForcedHoldStatus InForcedHoldKeybindStatus);

	/** 强制此按键绑定为按住式绑定 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetShowProgressCountDown(bool bShow);

	/** 为此按键绑定设置轴刻度值 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetAxisScale(const float NewValue) { AxisScale = NewValue; }

	/** 为此按键绑定设置预设名称覆盖值。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetPresetNameOverride(const FName NewValue) { PresetNameOverride = NewValue; }

	/** 我们当前的 BoundAction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FName BoundAction;

	/** 使用轴映射时读取的刻度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	float AxisScale;

	/** 此控件绑定的按键直接在蓝图中设置。当需要引用特定按键而非动作时使用。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKeyFallback = EKeys::Invalid;

	/** 允许我们为按键绑定控件显式设置输入类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	ECommonInputType InputTypeOverride = ECommonInputType::Count;

	/** 允许我们为按键绑定控件显式设置预设名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	FName PresetNameOverride;

	/** 一项设置，可将此按键绑定显示为“按住”操作，或始终不将其显示为“按住”（即使实际操作确实是按住）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	ECommonKeybindForcedHoldStatus ForcedHoldKeybindStatus;

	/** 当开始保持进度时，通过委托进行调用。 */
	UFUNCTION()
	UE_API void StartHoldProgress(FName HoldActionName, float HoldDuration);

	/** 当我们停止保持进度时，通过委托进行调用。 */
	UFUNCTION()
	UE_API void StopHoldProgress(FName HoldActionName, bool bCompletedSuccessfully);

	/** 获取此按键绑定是否为按住操作。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	bool IsHoldKeybind() const { return bIsHoldKeybind; }

	UFUNCTION()
	bool IsBoundKeyValid() const { return BoundKey.IsValid(); }

protected:
	UE_API virtual void NativePreConstruct() override;
	UE_API virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                                 bool bParentEnabled) const override;
	UE_API void RecalculateDesiredSize();

	/** 覆盖以销毁我们的 MID */
	UE_API virtual void NativeDestruct() override;

	/** 此按键绑定控件当前是否设置为“按住”绑定 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Keybind Widget", meta=(ScriptName = "IsHoldKeybindValue"))
	bool bIsHoldKeybind;

	/**  */
	UPROPERTY(Transient)
	bool bShowKeybindBorder;

	UPROPERTY(Transient)
	FVector2D FrameSize = FVector2D(0, 0);

	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	bool bShowTimeCountDown;

	/** 此组件绑定的派生键 */
	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKey;

	/** 用于展示进度的素材 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush HoldProgressBrush;

	/** 按键绑定文本边框 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush KeyBindTextBorder;

	/** 此按键绑定控件是否应显示当前未绑定的信息？ */
	UPROPERTY(EditAnywhere, Category = "Keybind Widget")
	bool bShowUnboundStatus = false;

	/** 应用于各尺寸的字体 */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo KeyBindTextFont;

	/** 应用于各尺寸的字体 */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo CountdownTextFont;

	UPROPERTY(Transient)
	FMeasuredText CountdownText;

	UPROPERTY(Transient)
	FMeasuredText KeybindText;

	UPROPERTY(Transient)
	FMargin KeybindTextPadding;

	UPROPERTY(Transient)
	FVector2D KeybindFrameMinimumSize;

	/** HoldKeybindImage 中“按住百分比”对应的材质参数名称 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FName PercentageMaterialParameterName;

	/** 进度百分比的 MID */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressPercentageMID;

	UE_API virtual void NativeOnInitialized() override;

private:
	//将保持进度同步至当前所属玩家控制器中设置的值。
	UE_API void SyncHoldProgress();

	/** 在执行按键绑定（Hold Keybind）期间，调用了 HoldKeybindImage 的更新。 */
	UE_API void UpdateHoldProgress();

	//长按类型的设置函数
	UE_API void SetupHoldKeybind();

	UE_API void ShowHoldBackPlate();

	UE_API void HandlePlayerControllerSet(APlayerController* NewPC);

	//长按开始时间
	float HoldKeybindStartTime = 0;

	//长按持续时间
	float HoldKeybindDuration = 0;

	bool bDrawProgress = false;
	bool bDrawBrushForKey = false;
	bool bDrawCountdownText = false;
	bool bWaitingForPlayerController = false;

	UPROPERTY(Transient)
	FSlateBrush CachedKeyBrush;
};

#undef UE_API
