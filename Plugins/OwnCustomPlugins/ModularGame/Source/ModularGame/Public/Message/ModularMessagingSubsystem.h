// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ModularMessagingSubsystem.generated.h"

#define UE_API MODULARGAME_API

/** 对话可能产生的结果*/
UENUM(BlueprintType)
enum class EModularMessagingResult : uint8
{
	/** “是”按钮被按下*/
	Confirmed,
	/** “否”按钮被按下。 */
	Declined,
	/** 按下了“忽略/取消”按钮。*/
	Cancelled,
	/** 对话框被显式终止（没有用户输入）。*/
	Killed,
	Unknown UMETA(Hidden)
};

DECLARE_DELEGATE_OneParam(FModularMessagingResultDelegate, EModularMessagingResult /* Result */);

/**
 * 
 */
UCLASS(MinimalAPI, config = Game)
class UModularMessagingSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UModularMessagingSubsystem()
	{
	}

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UE_API virtual void ShowConfirmation(class UModularGameDialogDescriptor* DialogDescriptor, FModularMessagingResultDelegate ResultCallback = FModularMessagingResultDelegate());
	UE_API virtual void ShowError(class UModularGameDialogDescriptor* DialogDescriptor, FModularMessagingResultDelegate ResultCallback = FModularMessagingResultDelegate());

private:
};
#undef UE_API
