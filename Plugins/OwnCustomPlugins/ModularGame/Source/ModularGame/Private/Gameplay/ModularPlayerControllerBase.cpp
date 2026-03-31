// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Gameplay/ModularPlayerControllerBase.h"

#include "Gameplay/ModularLocalPlayer.h"

AModularPlayerControllerBase::AModularPlayerControllerBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AModularPlayerControllerBase::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerControllerSet.Broadcast(LocalPlayer, this);

		if (PlayerState)
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
	}
}

void AModularPlayerControllerBase::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, InPawn);
}

void AModularPlayerControllerBase::OnPossess(class APawn* APawn)
{
	Super::OnPossess(APawn);

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, APawn);
}

void AModularPlayerControllerBase::OnUnPossess()
{
	Super::OnUnPossess();

	if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, nullptr);
}

void AModularPlayerControllerBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
		if (UModularLocalPlayer* LocalPlayer = Cast<UModularLocalPlayer>(Player))
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
}
