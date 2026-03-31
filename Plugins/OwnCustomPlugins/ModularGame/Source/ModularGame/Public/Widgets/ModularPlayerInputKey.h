// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ModularPlayerInputKey.generated.h"

#define UE_API MODULARGAME_API


enum class ECommonInputType : uint8;
class UModularLocalPlayer;

UENUM(BlueprintType)
enum class EModularKeybindForcedHoldStatus : uint8
{
	NoForcedHold, //无强制按下
	ForcedHold, //强制按下
	NeverShowHold //从不显示按下
};

USTRUCT()
struct FMeasuredText
{
	GENERATED_BODY()

public:
	FText GetText() const { return CachedText; }
	void SetText(const FText& InText);

	FVector2D GetTextSize() const { return CachedTextSize; }
	FVector2D UpdateTextSize(const FSlateFontInfo& InFontInfo, float FontScale = 1.0f) const;

private:
	FText CachedText;
	mutable FVector2D CachedTextSize;
	mutable bool bTextDirty = true;
};

/**
 * 
 */
UCLASS(MinimalAPI, Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class UModularPlayerInputKey : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UE_API UModularPlayerInputKey(const FObjectInitializer& ObjectInitializer);

	/** 根据我们当前的Boundaction更新密钥和相关显示 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void UpdateKeybindWidget();

	/** 为我们的keybind设置绑定密钥 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetBoundKey(FKey NewBoundAction);

	/**为我们的keybind设置绑定操作*/
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetBoundAction(FName NewBoundAction);

	/** 强制此键绑定为保持键绑定 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetForcedHoldKeybindStatus(EModularKeybindForcedHoldStatus InForcedHoldKeybindStatus);

	/** 强制此键绑定为保持键绑定 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetShowProgressCountDown(bool bShow);

	/** 设置此关键帧绑定的轴比例值 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetAxisScale(const float NewValue) { AxisScale = NewValue; }

	/** 为此键绑定设置预设名称覆盖值。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetPresetNameOverride(const FName NewValue) { PresetNameOverride = NewValue; }

	/**我们当前的边界行动 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FName BoundAction;

	/** 使用轴映射时读取的比例*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	float AxisScale;

	/** Key这个小部件绑定到蓝图中直接设置。当我们想引用特定的键而不是操作时使用。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKeyFallback;

	/** 允许我们为按键绑定控件显式设置输入类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	ECommonInputType InputTypeOverride;

	/** 允许我们明确地为按键绑定控件设置预设名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	FName PresetNameOverride;

	/** 设置可以将此按键绑定显示为按住状态，或者始终不显示为按住状态（即使它实际上是按住状态）。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	EModularKeybindForcedHoldStatus ForcedHoldKeybindStatus;

	/** 当我们开始保持进度时，通过代理调用*/
	UFUNCTION()
	UE_API void StartHoldProgress(FName HoldActionName, float HoldDuration);

	/** 当我们停止保持进度时，通过代理调用 */
	UFUNCTION()
	UE_API void StopHoldProgress(FName HoldActionName, bool bCompletedSuccessfully);

	/** 判断此按键绑定是否为按住操作。*/
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	bool IsHoldKeybind() const { return bIsHoldKeybind; }

	UFUNCTION()
	bool IsBoundKeyValid() const { return BoundKey.IsValid(); }

protected:
	UE_API virtual void NativePreConstruct() override;
	UE_API virtual void NativeConstruct() override;
	UE_API virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                                 bool bParentEnabled) const override;
	UE_API void RecalculateDesiredSize();

	/** 覆盖以销毁我们的 MID*/
	UE_API virtual void NativeDestruct() override;

	/**这个快捷键控件当前是否设置为按住快捷键*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Keybind Widget", meta=(ScriptName = "IsHoldKeybindValue"))
	bool bIsHoldKeybind;

	/**  */
	UPROPERTY(Transient)
	bool bShowKeybindBorder;

	UPROPERTY(Transient)
	FVector2D FrameSize;

	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	bool bShowTimeCountDown;

	/**此组件绑定的派生键*/
	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKey;

	/** 用于展示进度的材料 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush HoldProgressBrush;

	/** 按键绑定文本边框. */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush KeyBindTextBorder;

	/** 此快捷键绑定控件是否应显示其当前未绑定状态的信息？ */
	UPROPERTY(EditAnywhere, Category = "Keybind Widget")
	bool bShowUnboundStatus = false;

	/** 每个字号要应用的字体*/
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo KeyBindTextFont;

	/**每个字号要应用的字体 */
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

	/** HoldKeybindImage 中保持百分比的材质参数名称 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FName PercentageMaterialParameterName;

	/** 进度百分比的 MID*/
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressPercentageMID;

	UE_API virtual void NativeOnInitialized() override;

private:
	/**
	 * 将按住的进度与当前在
	 * 所属玩家控制器中设置的任何内容同步。
	 */
	UE_API void SyncHoldProgress();

	/** 需要更新按住按键时的 HoldKeybindImage。 */
	UE_API void UpdateHoldProgress();

	/** 当我们想将此按键绑定控件设置为按住按键绑定时调用。 */
	UE_API void SetupHoldKeybind();

	UE_API void ShowHoldBackPlate();

	UE_API void HandlePlayerControllerSet(UModularLocalPlayer* LocalPlayer, APlayerController* PlayerController);

	/** 我们开始使用长按快捷键的时间 */
	float HoldKeybindStartTime = 0;

	/** 我们将按住某个按键多久（以秒为单位）？ */
	float HoldKeybindDuration = 0;

	bool bDrawProgress = false;
	bool bDrawBrushForKey = false;
	bool bDrawCountdownText = false;
	bool bWaitingForPlayerController = false;

	UPROPERTY(Transient)
	FSlateBrush CachedKeyBrush;
};
#undef UE_API
