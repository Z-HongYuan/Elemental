// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "ModularWithPlayerController.h"
#include "ModularPlayerController.generated.h"

#define UE_API MODULARGAMEHUB_API

/**
 * 
 */
UCLASS(MinimalAPI, Config=Game)
class AModularPlayerController : public AModularWithPlayerController
{
	GENERATED_BODY()

public:
	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void SetPawn(APawn* InPawn) override;
	UE_API virtual void OnRep_PlayerState() override;

protected:
	UE_API virtual void OnPossess(APawn* APawn) override;
	UE_API virtual void OnUnPossess() override;

private:
};

#undef UE_API
