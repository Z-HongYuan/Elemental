// Copyright © 2026 鸿源z. All Rights Reserved.


#include "AsyncAction_OnlineUserInitialize.h"

UAsyncAction_OnlineUserInitialize* UAsyncAction_OnlineUserInitialize::InitializeForLocalPlay(UOnlineUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	// 设置为默认设备
	if (!PrimaryInputDevice.IsValid())
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();


	UAsyncAction_OnlineUserInitialize* Action = NewObject<UAsyncAction_OnlineUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;

		Action->Params.RequestedPrivilege = EOnlineUserPrivilege::CanPlay;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.PrimaryInputDevice = PrimaryInputDevice;
		Action->Params.bCanUseGuestLogin = bCanUseGuestLogin;
		Action->Params.bCanCreateNewLocalPlayer = true;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

UAsyncAction_OnlineUserInitialize* UAsyncAction_OnlineUserInitialize::LoginForOnlinePlay(UOnlineUserSubsystem* Target, int32 LocalPlayerIndex)
{
	UAsyncAction_OnlineUserInitialize* Action = NewObject<UAsyncAction_OnlineUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;

		Action->Params.RequestedPrivilege = EOnlineUserPrivilege::CanPlayOnline;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.bCanCreateNewLocalPlayer = false;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_OnlineUserInitialize::HandleFailure()
{
	const UOnlineUserInfo* UserInfo = nullptr;
	if (Subsystem.IsValid())
		UserInfo = Subsystem->GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex);

	HandleInitializationComplete(UserInfo, false, NSLOCTEXT("OnlineUser", "LoginFailedEarly", "Unable to start login process"), Params.RequestedPrivilege, Params.OnlineContext);
}

void UAsyncAction_OnlineUserInitialize::HandleInitializationComplete(const UOnlineUserInfo* UserInfo, bool bSuccess, FText Error, EOnlineUserPrivilege RequestedPrivilege, EOnlineUserOnlineContext OnlineContext)
{
	if (ShouldBroadcastDelegates())
		OnInitializationComplete.Broadcast(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

	SetReadyToDestroy();
}

void UAsyncAction_OnlineUserInitialize::Activate()
{
	if (Subsystem.IsValid())
	{
		Params.OnUserInitializeComplete.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UAsyncAction_OnlineUserInitialize, HandleInitializationComplete));
		bool bSuccess = Subsystem->TryToInitializeUser(Params);

		if (!bSuccess)
		{
			// Call failure next frame
			FTimerManager* TimerManager = GetTimerManager();

			if (TimerManager)
			{
				TimerManager->SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::HandleFailure));
			}
		}
	}
	else
	{
		SetReadyToDestroy();
	}
}
