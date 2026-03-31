// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"

#include "ModularPlayerControllerBase.generated.h"

#define UE_API MODULARGAME_API
/**
 * 
 */
UCLASS(MinimalAPI, config=Game)
class AModularPlayerControllerBase : public AModularPlayerController
{
	GENERATED_BODY()

public:
	UE_API AModularPlayerControllerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void SetPawn(APawn* InPawn) override;
	UE_API virtual void OnPossess(class APawn* APawn) override;
	UE_API virtual void OnUnPossess() override;

protected:
	UE_API virtual void OnRep_PlayerState() override;
};
#undef UE_API
