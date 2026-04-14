// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularPlayerController.h"

#include "ModularLocalPlayer.h"

void AModularPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
	{
		LocalPlayer->GetOnPlayerControllerSetDelegate().Broadcast(LocalPlayer, this);
		if (PlayerState) LocalPlayer->GetOnPlayerStateSetDelegate().Broadcast(LocalPlayer, PlayerState);
	}
}

void AModularPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
		LocalPlayer->GetOnPlayerPawnSetDelegate().Broadcast(LocalPlayer, InPawn);
}

void AModularPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
		LocalPlayer->GetOnPlayerPawnSetDelegate().Broadcast(LocalPlayer, nullptr);
}

void AModularPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
		if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
			LocalPlayer->GetOnPlayerStateSetDelegate().Broadcast(LocalPlayer, PlayerState);
}

void AModularPlayerController::OnPossess(class APawn* APawn)
{
	Super::OnPossess(APawn);

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
		LocalPlayer->GetOnPlayerPawnSetDelegate().Broadcast(LocalPlayer, APawn);
}
