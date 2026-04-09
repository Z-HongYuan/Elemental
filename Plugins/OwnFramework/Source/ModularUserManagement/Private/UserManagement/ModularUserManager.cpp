// Copyright © 2026 鸿源z. All Rights Reserved.


#include "UserManagement/ModularUserManager.h"

#include "ModularUserGameplayTags.h"

#if MODULARUSER_OSSV1
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemUtils.h"
#endif

#if !MODULARUSER_OSSV1
#include "Online/Auth.h"
#include "Online/ExternalUI.h"
#include "Online/OnlineResult.h"
#include "Online/OnlineServices.h"
#include "Online/OnlineServicesEngineUtils.h"
#include "Online/Privileges.h"

using namespace UE::Online;
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogModularUser, Log, All); //创建日志分类
DEFINE_LOG_CATEGORY(LogModularUser); //创建日志分类

void UModularUserManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建我们的OSS包装器
	CreateOnlineContexts();

	BindOnlineDelegates();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);

	// 匹配引擎默认值
	SetMaxLocalPlayers(4);

	ResetUserState();

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UModularUserManager::Deinitialize()
{
	DestroyOnlineContexts();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);

	LocalUserInfos.Reset();
	ActiveLoginRequests.Reset();

	Super::Deinitialize();
}

bool UModularUserManager::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// Only create an instance if there is not a game-specific subclass
	return ChildClasses.Num() == 0;
}

void UModularUserManager::SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText)
{
}

void UModularUserManager::SetMaxLocalPlayers(int32 InMaxLocalPLayers)
{
}

int32 UModularUserManager::GetMaxLocalPlayers() const
{
	return 0;
}

int32 UModularUserManager::GetNumLocalPlayers() const
{
	return 0;
}

EModularUserInitializationState UModularUserManager::GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const
{
	return {};
}

const UModularUserInfo* UModularUserManager::GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const
{
	return nullptr;
}

const UModularUserInfo* UModularUserManager::GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const
{
	return nullptr;
}

const UModularUserInfo* UModularUserManager::GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const
{
	return nullptr;
}

const UModularUserInfo* UModularUserManager::GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const
{
	return nullptr;
}

const UModularUserInfo* UModularUserManager::GetUserInfoForControllerId(int32 ControllerId) const
{
	return nullptr;
}

const UModularUserInfo* UModularUserManager::GetUserInfoForInputDevice(FInputDeviceId InputDevice) const
{
	return nullptr;
}

bool UModularUserManager::TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	return false;
}

bool UModularUserManager::TryToLoginForOnlinePlay(int32 LocalPlayerIndex)
{
	return false;
}

bool UModularUserManager::TryToInitializeUser(FModularUserInitializeParams Params)
{
	return false;
}

void UModularUserManager::ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FModularUserInitializeParams Params)
{
}

bool UModularUserManager::CancelUserInitialization(int32 LocalPlayerIndex)
{
	return false;
}

bool UModularUserManager::TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer)
{
	return false;
}

void UModularUserManager::ResetUserState()
{
}

bool UModularUserManager::IsRealPlatformUserIndex(int32 PlatformUserIndex) const
{
	return false;
}

bool UModularUserManager::IsRealPlatformUser(FPlatformUserId PlatformUser) const
{
	return false;
}

FPlatformUserId UModularUserManager::GetPlatformUserIdForIndex(int32 PlatformUserIndex) const
{
	return {};
}

int32 UModularUserManager::GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const
{
	return 0;
}

FPlatformUserId UModularUserManager::GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const
{
	return {};
}

FInputDeviceId UModularUserManager::GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const
{
	return {};
}

void UModularUserManager::SetTraitTags(const FGameplayTagContainer& InTags)
{
}

bool UModularUserManager::ShouldWaitForStartInput() const
{
	return false;
}

IOnlineSubsystem* UModularUserManager::GetOnlineSubsystem(EModularUserOnlineContext Context) const
{
	return nullptr;
}

IOnlineIdentity* UModularUserManager::GetOnlineIdentity(EModularUserOnlineContext Context) const
{
	return nullptr;
}

FName UModularUserManager::GetOnlineSubsystemName(EModularUserOnlineContext Context) const
{
	return {};
}

EOnlineServerConnectionStatus::Type UModularUserManager::GetConnectionStatus(EModularUserOnlineContext Context) const
{
	return {};
}

bool UModularUserManager::HasOnlineConnection(EModularUserOnlineContext Context) const
{
	return false;
}

ELoginStatusType UModularUserManager::GetLocalUserLoginStatus(FPlatformUserId PlatformUser, EModularUserOnlineContext Context) const
{
	return {};
}

FUniqueNetIdRepl UModularUserManager::GetLocalUserNetId(FPlatformUserId PlatformUser, EModularUserOnlineContext Context) const
{
	return {};
}

FString UModularUserManager::GetLocalUserNickname(FPlatformUserId PlatformUser, EModularUserOnlineContext Context) const
{
	return {};
}

FString UModularUserManager::PlatformUserIdToString(FPlatformUserId UserId)
{
	return {};
}

FString UModularUserManager::EModularUserOnlineContextToString(EModularUserOnlineContext Context)
{
	return {};
}

FText UModularUserManager::GetPrivilegeDescription(EModularUserPrivilege Privilege) const
{
	return {};
}

FText UModularUserManager::GetPrivilegeResultDescription(EModularUserPrivilegeResult Result) const
{
	return {};
}

bool UModularUserManager::LoginLocalUser(const UModularUserInfo* UserInfo, EModularUserPrivilege RequestedPrivilege, EModularUserOnlineContext Context, FOnLocalUserLoginCompleteDelegate OnComplete)
{
	return false;
}

void UModularUserManager::SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UModularUserInfo* UserInfo)
{
}

EModularUserOnlineContext UModularUserManager::ResolveOnlineContext(EModularUserOnlineContext Context) const
{
	return {};
}

bool UModularUserManager::HasSeparatePlatformContext() const
{
	return false;
}

UModularUserInfo* UModularUserManager::CreateLocalUserInfo(int32 LocalPlayerIndex)
{
	UModularUserInfo* NewUser = nullptr;
	if (ensure(!LocalUserInfos.Contains(LocalPlayerIndex)))
	{
		NewUser = NewObject<UModularUserInfo>(this);
		NewUser->LocalPlayerIndex = LocalPlayerIndex;
		NewUser->InitializationState = EModularUserInitializationState::Unknown;

		// Always create game and default cache
		NewUser->CachedDataMap.Add(EModularUserOnlineContext::Game, UModularUserInfo::FCachedData());
		NewUser->CachedDataMap.Add(EModularUserOnlineContext::Default, UModularUserInfo::FCachedData());

		// Add platform if needed
		if (HasSeparatePlatformContext())
		{
			NewUser->CachedDataMap.Add(EModularUserOnlineContext::Platform, UModularUserInfo::FCachedData());
		}

		LocalUserInfos.Add(LocalPlayerIndex, NewUser);
	}
	return NewUser;
}

void UModularUserManager::RefreshLocalUserInfo(UModularUserInfo* UserInfo)
{
}

void UModularUserManager::HandleChangedAvailability(UModularUserInfo* UserInfo, EModularUserPrivilege Privilege, EModularUserAvailability OldAvailability)
{
}

void UModularUserManager::UpdateUserPrivilegeResult(UModularUserInfo* UserInfo, EModularUserPrivilege Privilege, EModularUserPrivilegeResult Result, EModularUserOnlineContext Context)
{
}

const UModularUserManager::FOnlineContextCache* UModularUserManager::GetContextCache(EModularUserOnlineContext Context) const
{
	return nullptr;
}

UModularUserManager::FOnlineContextCache* UModularUserManager::GetContextCache(EModularUserOnlineContext Context)
{
	return nullptr;
}

void UModularUserManager::CreateOnlineContexts()
{
	// First initialize default
	DefaultContextInternal = new FOnlineContextCache();
#if MODULARUSER_OSSV1
	DefaultContextInternal->OnlineSubsystem = Online::GetSubsystem(GetWorld());
	check(DefaultContextInternal->OnlineSubsystem);
	DefaultContextInternal->IdentityInterface = DefaultContextInternal->OnlineSubsystem->GetIdentityInterface();
	check(DefaultContextInternal->IdentityInterface.IsValid());

	IOnlineSubsystem* PlatformSub = IOnlineSubsystem::GetByPlatform();

	if (PlatformSub && DefaultContextInternal->OnlineSubsystem != PlatformSub)
	{
		// Set up the optional platform service if it exists
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineSubsystem = PlatformSub;
		PlatformContextInternal->IdentityInterface = PlatformSub->GetIdentityInterface();
		check(PlatformContextInternal->IdentityInterface.IsValid());
	}
#endif

#if !MODULARUSER_OSSV1
	DefaultContextInternal->OnlineServices = GetServices(GetWorld(), EOnlineServices::Default);
	check(DefaultContextInternal->OnlineServices);
	DefaultContextInternal->AuthService = DefaultContextInternal->OnlineServices->GetAuthInterface();
	check(DefaultContextInternal->AuthService);

	UE::Online::IOnlineServicesPtr PlatformServices = GetServices(GetWorld(), EOnlineServices::Platform);
	if (PlatformServices && DefaultContextInternal->OnlineServices != PlatformServices)
	{
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineServices = PlatformServices;
		PlatformContextInternal->AuthService = PlatformContextInternal->OnlineServices->GetAuthInterface();
		check(PlatformContextInternal->AuthService);
	}
#endif

	// Explicit external services can be set up after if needed
}

void UModularUserManager::DestroyOnlineContexts()
{
	// All cached shared ptrs must be cleared here
	if (ServiceContextInternal && ServiceContextInternal != DefaultContextInternal)
	{
		delete ServiceContextInternal;
	}
	if (PlatformContextInternal && PlatformContextInternal != DefaultContextInternal)
	{
		delete PlatformContextInternal;
	}
	if (DefaultContextInternal)
	{
		delete DefaultContextInternal;
	}

	ServiceContextInternal = PlatformContextInternal = DefaultContextInternal = nullptr;
}

void UModularUserManager::BindOnlineDelegates()
{
#if MODULARUSER_OSSV1
	return BindOnlineDelegatesOSSv1();
#endif

#if !MODULARUSER_OSSV1
	return BindOnlineDelegatesOSSv2();
#endif
}

void UModularUserManager::LogOutLocalUser(FPlatformUserId PlatformUser)
{
	UModularUserInfo* UserInfo = ModifyInfo(GetUserInfoForPlatformUser(PlatformUser));

	// Don't need to do anything if the user has never logged in fully or is in the process of logging in
	if (UserInfo && (UserInfo->InitializationState == EModularUserInitializationState::LoggedInLocalOnly || UserInfo->InitializationState == EModularUserInitializationState::LoggedInOnline))
	{
		EModularUserAvailability OldAvailablity = UserInfo->GetPrivilegeAvailability(EModularUserPrivilege::CanPlay);

		UserInfo->InitializationState = EModularUserInitializationState::FailedtoLogin;

		// This will broadcast the game delegate
		HandleChangedAvailability(UserInfo, EModularUserPrivilege::CanPlay, OldAvailablity);
	}
}

void UModularUserManager::ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request)
{
}

bool UModularUserManager::TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
#if MODULARUSER_OSSV1
	// Not supported in V1 path
	return false;
#endif

#if !MODULARUSER_OSSV1
	return TransferPlatformAuthOSSv2(System, Request, PlatformUser);
#endif
}

bool UModularUserManager::AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogModularUser, Log, TEXT("Player AutoLogin requested - UserIdx:%d, Privilege:%d, Context:%d"),
	       PlatformUser.GetInternalId(),
	       (int32)Request->DesiredPrivilege,
	       (int32)Request->DesiredContext);

#if MODULARUSER_OSSV1
	return AutoLoginOSSv1(System, Request, PlatformUser);
#endif

#if !MODULARUSER_OSSV1
	return AutoLoginOSSv2(System, Request, PlatformUser);
#endif
}

bool UModularUserManager::ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogModularUser, Log, TEXT("Player LoginUI requested - UserIdx:%d, Privilege:%d, Context:%d"),
	       PlatformUser.GetInternalId(),
	       (int32)Request->DesiredPrivilege,
	       (int32)Request->DesiredContext);

#if MODULARUSER_OSSV1
	return ShowLoginUIOSSv1(System, Request, PlatformUser);
#endif

#if !MODULARUSER_OSSV1
	return ShowLoginUIOSSv2(System, Request, PlatformUser);
#endif
}

bool UModularUserManager::QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
#if MODULARUSER_OSSV1
	return QueryUserPrivilegeOSSv1(System, Request, PlatformUser);
#endif

#if !MODULARUSER_OSSV1
	return QueryUserPrivilegeOSSv2(System, Request, PlatformUser);
#endif
}

EModularUserPrivilege UModularUserManager::ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const
{
	switch (Privilege)
	{
	case EUserPrivileges::CanPlay:
		return EModularUserPrivilege::CanPlay;
	case EUserPrivileges::CanPlayOnline:
		return EModularUserPrivilege::CanPlayOnline;
	case EUserPrivileges::CanCommunicateOnline:
		return EModularUserPrivilege::CanCommunicateViaTextOnline; // No good thing to do here, just mapping to text.
	case EUserPrivileges::CanUseUserGeneratedContent:
		return EModularUserPrivilege::CanUseUserGeneratedContent;
	case EUserPrivileges::CanUserCrossPlay:
		return EModularUserPrivilege::CanUseCrossPlay;
	default:
		return EModularUserPrivilege::Invalid_Count;
	}
}

EUserPrivileges::Type UModularUserManager::ConvertOSSPrivilege(EModularUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case EModularUserPrivilege::CanPlay:
		return EUserPrivileges::CanPlay;
	case EModularUserPrivilege::CanPlayOnline:
		return EUserPrivileges::CanPlayOnline;
	case EModularUserPrivilege::CanCommunicateViaTextOnline:
	case EModularUserPrivilege::CanCommunicateViaVoiceOnline:
		return EUserPrivileges::CanCommunicateOnline;
	case EModularUserPrivilege::CanUseUserGeneratedContent:
		return EUserPrivileges::CanUseUserGeneratedContent;
	case EModularUserPrivilege::CanUseCrossPlay:
		return EUserPrivileges::CanUserCrossPlay;
	default:
		// No failure type, return CanPlay
		return EUserPrivileges::CanPlay;
	}
}

EModularUserPrivilegeResult UModularUserManager::ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const
{
	// The V1 results enum is a bitfield where each platform behaves a bit differently
	if (Results == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		return EModularUserPrivilegeResult::Available;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotLoggedIn))
	{
		return EModularUserPrivilegeResult::UserNotLoggedIn;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate))
	{
		return EModularUserPrivilegeResult::VersionOutdated;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure)
	{
		return EModularUserPrivilegeResult::AgeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure)
	{
		return EModularUserPrivilegeResult::AccountTypeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::NetworkConnectionUnavailable)
	{
		return EModularUserPrivilegeResult::NetworkConnectionUnavailable;
	}

	// Bucket other account failures together
	uint32 AccountUseFailures = (uint32)IOnlineIdentity::EPrivilegeResults::OnlinePlayRestricted
		| (uint32)IOnlineIdentity::EPrivilegeResults::UGCRestriction
		| (uint32)IOnlineIdentity::EPrivilegeResults::ChatRestriction;

	if (Results & AccountUseFailures)
	{
		return EModularUserPrivilegeResult::AccountUseRestricted;
	}

	// If you can't play at all, this is a license failure
	if (Privilege == EUserPrivileges::CanPlay)
	{
		return EModularUserPrivilegeResult::LicenseInvalid;
	}

	// Unknown reason
	return EModularUserPrivilegeResult::PlatformFailure;
}

void UModularUserManager::BindOnlineDelegatesOSSv1()
{
	EModularUserOnlineContext ServiceType = ResolveOnlineContext(EModularUserOnlineContext::ServiceOrDefault);
	EModularUserOnlineContext PlatformType = ResolveOnlineContext(EModularUserOnlineContext::PlatformOrDefault);
	FOnlineContextCache* ServiceContext = GetContextCache(ServiceType);
	FOnlineContextCache* PlatformContext = GetContextCache(PlatformType);
	check(ServiceContext && ServiceContext->OnlineSubsystem && PlatformContext && PlatformContext->OnlineSubsystem);
	// Connection delegates need to listen for both systems

	ServiceContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, ServiceType));
	ServiceContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

	for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
	{
		ServiceContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, ServiceType));
		ServiceContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, ServiceType));
	}

	if (ServiceType != PlatformType)
	{
		PlatformContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, PlatformType));
		PlatformContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

		for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
		{
			PlatformContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, PlatformType));
			PlatformContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, PlatformType));
		}
	}

	// Hardware change delegates only listen to platform
	PlatformContext->IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &ThisClass::HandleControllerPairingChanged));
}

bool UModularUserManager::AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	return System->IdentityInterface->AutoLogin(GetPlatformUserIndexForId(PlatformUser));
}

bool UModularUserManager::ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IOnlineExternalUIPtr ExternalUI = System->OnlineSubsystem->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		// TODO Unclear which flags should be set
		return ExternalUI->ShowLoginUI(GetPlatformUserIndexForId(PlatformUser), false, false, FOnLoginUIClosedDelegate::CreateUObject(this, &ThisClass::HandleOnLoginUIClosed, Request->CurrentContext));
	}
	return false;
}

bool UModularUserManager::QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	// Start query on unknown or failure
	EUserPrivileges::Type OSSPrivilege = ConvertOSSPrivilege(Request->DesiredPrivilege);

	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	check(CurrentId.IsValid());
	IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate Delegate = IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &UModularUserManager::HandleCheckPrivilegesComplete, Request->DesiredPrivilege,
	                                                                                                                                    Request->UserInfo, Request->CurrentContext);
	System->IdentityInterface->GetUserPrivilege(*CurrentId, OSSPrivilege, Delegate);

	// This may immediately succeed and reenter this function, so we have to return
	return true;
}

void UModularUserManager::HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId, EModularUserOnlineContext Context)
{
	UE_LOG(LogModularUser, Log, TEXT("Player login status changed - System:%s, UserIdx:%d, OldStatus:%s, NewStatus:%s, NewId:%s"),
	       *GetOnlineSubsystemName(Context).ToString(),
	       PlatformUserIndex,
	       ELoginStatus::ToString(OldStatus),
	       ELoginStatus::ToString(NewStatus),
	       *NewId.ToString());

	if (NewStatus == ELoginStatus::NotLoggedIn && OldStatus != ELoginStatus::NotLoggedIn)
	{
		FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
		LogOutLocalUser(PlatformUser);
	}
}

void UModularUserManager::HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& Error, EModularUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(PlatformUser, Context);
	FUniqueNetIdRepl NewId = FUniqueNetIdRepl(NetId);
	UE_LOG(LogModularUser, Log, TEXT("Player login Completed - System:%s, UserIdx:%d, Successful:%d, NewStatus:%s, NewId:%s, ErrorIfAny:%s"),
	       *GetOnlineSubsystemName(Context).ToString(),
	       PlatformUserIndex,
	       (int32)bWasSuccessful,
	       ELoginStatus::ToString(NewStatus),
	       *NewId.ToString(),
	       *Error);

	// Update any waiting login requests
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UModularUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// User is gone, just delete this request
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		if (UserInfo->PlatformUser == PlatformUser && Request->CurrentContext == Context)
		{
			// On some platforms this gets called from the login UI with a failure
			if (Request->AutoLoginState == EModularUserAsyncTaskState::InProgress)
			{
				Request->AutoLoginState = bWasSuccessful ? EModularUserAsyncTaskState::Done : EModularUserAsyncTaskState::Failed;
			}

			if (!bWasSuccessful)
			{
				Request->Error = FOnlineError(FText::FromString(Error));
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UModularUserManager::HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser)
{
	UE_LOG(LogModularUser, Log, TEXT("Player controller pairing changed - UserIdx:%d, PreviousUser:%s, NewUser:%s"),
	       PlatformUserIndex,
	       *ToDebugString(PreviousUser),
	       *ToDebugString(NewUser));

	UGameInstance* GameInstance = GetGameInstance();
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ULocalPlayer* ControlledLocalPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(PlatformUser);
	ULocalPlayer* NewLocalPlayer = GameInstance->FindLocalPlayerFromUniqueNetId(NewUser.User);
	const UModularUserInfo* NewUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(NewUser.User));
	const UModularUserInfo* PreviousUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(PreviousUser.User));

	// See if we think this is already bound to an existing player	
	if (PreviousUser.ControllersRemaining == 0 && PreviousUserInfo && PreviousUserInfo != NewUserInfo)
	{
		// This means that the user deliberately logged out using a platform interface
		if (IsRealPlatformUser(PlatformUser))
		{
			LogOutLocalUser(PlatformUser);
		}
	}

	if (ControlledLocalPlayer && ControlledLocalPlayer != NewLocalPlayer)
	{
		// TODO Currently the platforms that call this delegate do not really handle swapping controller IDs
		// SetLocalPlayerUserIndex(ControlledLocalPlayer, -1);
	}
}

void UModularUserManager::HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus, EModularUserOnlineContext Context)
{
	UE_LOG(LogModularUser, Log, TEXT("HandleNetworkConnectionStatusChanged(ServiceName: %s, LastStatus: %s, ConnectionStatus: %s)"),
	       *ServiceName,
	       EOnlineServerConnectionStatus::ToString(LastConnectionStatus),
	       EOnlineServerConnectionStatus::ToString(ConnectionStatus));

	// Cache old availablity for current users
	TMap<UModularUserInfo*, EModularUserAvailability> AvailabilityMap;

	for (TPair<int32, UModularUserInfo*> Pair : LocalUserInfos)
	{
		AvailabilityMap.Add(Pair.Value, Pair.Value->GetPrivilegeAvailability(EModularUserPrivilege::CanPlayOnline));
	}

	FOnlineContextCache* System = GetContextCache(Context);
	if (ensure(System))
	{
		// Service name is normally the same as the OSS name, but not necessarily on all platforms
		System->CurrentConnectionStatus = ConnectionStatus;
	}

	for (TPair<UModularUserInfo*, EModularUserAvailability> Pair : AvailabilityMap)
	{
		// Notify other systems when someone goes online/offline
		HandleChangedAvailability(Pair.Key, EModularUserPrivilege::CanPlayOnline, Pair.Value);
	}
}

void UModularUserManager::HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error, EModularUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);

	// Update any waiting login requests
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UModularUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// User is gone, just delete this request
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		// Look for first user trying to log in on this context
		if (Request->CurrentContext == Context && Request->LoginUIState == EModularUserAsyncTaskState::InProgress)
		{
			if (LoggedInNetId.IsValid() && LoggedInNetId->IsValid() && Error.WasSuccessful())
			{
				// The platform user id that actually logged in may not be the same one who requested the UI,
				// so swap it if the returned id is actually valid
				if (UserInfo->PlatformUser != PlatformUser && PlatformUser != PLATFORMUSERID_NONE)
				{
					UserInfo->PlatformUser = PlatformUser;
				}

				Request->LoginUIState = EModularUserAsyncTaskState::Done;
				Request->Error.Reset();
			}
			else
			{
				Request->LoginUIState = EModularUserAsyncTaskState::Failed;
				Request->Error = Error;
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UModularUserManager::HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, EModularUserPrivilege RequestedPrivilege, TWeakObjectPtr<UModularUserInfo> ModularUserInfo,
                                                        EModularUserOnlineContext Context)
{
	// Only handle if user still exists
	UModularUserInfo* UserInfo = ModularUserInfo.Get();

	if (!UserInfo)
	{
		return;
	}

	EModularUserPrivilegeResult UserResult = ConvertOSSPrivilegeResult(Privilege, PrivilegeResults);

	// Update the user cached value
	UpdateUserPrivilegeResult(UserInfo, RequestedPrivilege, UserResult, Context);

	FOnlineContextCache* ContextCache = GetContextCache(Context);
	check(ContextCache);

	// If this returns disconnected, update the connection status
	if (UserResult == EModularUserPrivilegeResult::NetworkConnectionUnavailable)
	{
		ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::NoNetworkConnection;
	}
	else if (UserResult == EModularUserPrivilegeResult::Available && RequestedPrivilege == EModularUserPrivilege::CanPlayOnline)
	{
		if (ContextCache->CurrentConnectionStatus == EOnlineServerConnectionStatus::NoNetworkConnection)
		{
			ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
		}
	}

	// See if a login request is waiting on this
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.Get() == UserInfo && Request->CurrentContext == Context && Request->DesiredPrivilege == RequestedPrivilege && Request->PrivilegeCheckState == EModularUserAsyncTaskState::InProgress)
		{
			if (UserResult == EModularUserPrivilegeResult::Available)
			{
				Request->PrivilegeCheckState = EModularUserAsyncTaskState::Done;
			}
			else
			{
				Request->PrivilegeCheckState = EModularUserAsyncTaskState::Failed;

				// Forms strings in english like "(The user is not allowed) to (play the game)"
				Request->Error = FOnlineError(FText::Format(NSLOCTEXT("CommonUser", "PrivilegeFailureFormat", "{0} to {1}"), GetPrivilegeResultDescription(UserResult), GetPrivilegeDescription(RequestedPrivilege)));
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UModularUserManager::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	FString InputDeviceIDString = FString::Printf(TEXT("%d"), InputDeviceId.GetId());
	const bool bIsConnected = NewConnectionState == EInputDeviceConnectionState::Connected;
	UE_LOG(LogModularUser, Log, TEXT("Controller connection changed - UserIdx:%s, UserID:%s, Connected:%d"), *InputDeviceIDString, *PlatformUserIdToString(PlatformUserId), bIsConnected ? 1 : 0);

	// TODO Implement for platforms that support this
}

static inline FText GetErrorText(const FOnlineErrorType& InOnlineError)
{
#if MODULARUSER_OSSV1
	return InOnlineError.GetErrorMessage();
#endif
#if !MODULARUSER_OSSV1
	return InOnlineError.GetText();
#endif
}

void UModularUserManager::HandleLoginForUserInitialize(const UModularUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& InError, EModularUserOnlineContext Context,
                                                       FModularUserInitializeParams Params)
{
	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);
	FTimerManager& TimerManager = GameInstance->GetTimerManager();
	TOptional<FOnlineErrorType> Error = InError; // Copy so we can reset on handled errors

	UModularUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	UModularUserInfo* FirstUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(0));

	if (!ensure(LocalUserInfo && FirstUserInfo))
	{
		return;
	}

	// Check the hard platform/service ids
	RefreshLocalUserInfo(LocalUserInfo);

	FUniqueNetIdRepl FirstPlayerId = FirstUserInfo->GetNetId(EModularUserOnlineContext::PlatformOrDefault);

	// Check to see if we should make a guest after a login failure. Some platforms return success but reuse the first player's id, count this as a failure
	if (LocalUserInfo != FirstUserInfo && LocalUserInfo->bCanBeGuest && (NewStatus == ELoginStatusType::NotLoggedIn || NetId == FirstPlayerId))
	{
#if MODULARUSER_OSSV1
		NetId = (FUniqueNetIdRef)FUniqueNetIdString::Create(FString::Printf(TEXT("GuestPlayer%d"), LocalUserInfo->LocalPlayerIndex), NULL_SUBSYSTEM);
#endif
#if !MODULARUSER_OSSV1
		// TODO:  OSSv2 FUniqueNetIdRepl wrapping FAccountId is in progress
		// TODO:  OSSv2 - How to handle guest accounts?
#endif
		LocalUserInfo->bIsGuest = true;
		NewStatus = ELoginStatusType::UsingLocalProfile;
		Error.Reset();
		UE_LOG(LogModularUser, Log, TEXT("HandleLoginForUserInitialize created guest id %s for local player %d"), *NetId.ToString(), LocalUserInfo->LocalPlayerIndex);
	}
	else
	{
		LocalUserInfo->bIsGuest = false;
	}

	ensure(LocalUserInfo->IsDoingLogin());

	if (Error.IsSet())
	{
		FText ErrorText = GetErrorText(Error.GetValue());
		TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UModularUserManager::HandleUserInitializeFailed, Params, ErrorText));
		return;
	}

	if (Context == EModularUserOnlineContext::Game)
	{
		LocalUserInfo->UpdateCachedNetId(NetId, EModularUserOnlineContext::Game);
	}

	ULocalPlayer* CurrentPlayer = GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex);
	if (!CurrentPlayer && Params.bCanCreateNewLocalPlayer)
	{
		FString ErrorString;
		CurrentPlayer = GameInstance->CreateLocalPlayer(LocalUserInfo->PlatformUser, ErrorString, true);

		if (!CurrentPlayer)
		{
			TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UModularUserManager::HandleUserInitializeFailed, Params, FText::AsCultureInvariant(ErrorString)));
			return;
		}
		ensure(GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex) == CurrentPlayer);
	}

	// Updates controller and net id if needed
	SetLocalPlayerUserInfo(CurrentPlayer, LocalUserInfo);

	// Set a delayed callback
	TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UModularUserManager::HandleUserInitializeSucceeded, Params));
}

void UModularUserManager::HandleUserInitializeFailed(FModularUserInitializeParams Params, FText Error)
{
	UModularUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// The user info was reset since this was scheduled
		return;
	}

	UE_LOG(LogModularUser, Warning, TEXT("TryToInitializeUser %d failed with error %s"), LocalUserInfo->LocalPlayerIndex, *Error.ToString());

	// If state is wrong, abort as we might have gotten canceled
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// If initial login failed or we ended up totally logged out, set to complete failure
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(Params.PlatformUser, Params.OnlineContext);
	if (NewStatus == ELoginStatusType::NotLoggedIn || LocalUserInfo->InitializationState == EModularUserInitializationState::DoingInitialLogin)
	{
		LocalUserInfo->InitializationState = EModularUserInitializationState::FailedtoLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = EModularUserInitializationState::LoggedInLocalOnly;
	}

	FText TitleText = NSLOCTEXT("ModularUser", "LoginFailedTitle", "Login Failure");

	if (!Params.bSuppressLoginErrors)
	{
		SendSystemMessage(ModularUserManagementTags::ModularUser_SystemMessage_Error_InitializeLocalPlayerFailed, TitleText, Error);
	}

	// Call callbacks
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
}

void UModularUserManager::HandleUserInitializeSucceeded(FModularUserInitializeParams Params)
{
	UModularUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// The user info was reset since this was scheduled
		return;
	}

	// If state is wrong, abort as we might have gotten cancelled
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// Fix up state
	if (Params.RequestedPrivilege == EModularUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = EModularUserInitializationState::LoggedInOnline;
	}
	else
	{
		LocalUserInfo->InitializationState = EModularUserInitializationState::LoggedInLocalOnly;
	}

	ensure(LocalUserInfo->GetPrivilegeAvailability(Params.RequestedPrivilege) == EModularUserAvailability::NowAvailable);

	// Call callbacks
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
}

bool UModularUserManager::OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs)
{
	int32 NextLocalPlayerIndex = INDEX_NONE;

	const UModularUserInfo* MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);
	if (EventArgs.Event == IE_Pressed)
	{
		if (MappedUser == nullptr || !MappedUser->IsLoggedIn())
		{
			if (MappedUser)
			{
				NextLocalPlayerIndex = MappedUser->LocalPlayerIndex;
			}
			else
			{
				// Find next player
				for (int32 i = 0; i < MaxNumberOfLocalPlayers; i++)
				{
					if (GetLocalPlayerInitializationState(i) == EModularUserInitializationState::Unknown)
					{
						NextLocalPlayerIndex = i;
						break;
					}
				}
			}

			if (NextLocalPlayerIndex != INDEX_NONE)
			{
				if (LoginKeysForAnyUser.Contains(EventArgs.Key))
				{
					// If we're in the middle of logging in just return true to ignore platform-specific input
					if (MappedUser && MappedUser->IsDoingLogin())
					{
						return true;
					}

					// Press start screen
					FModularUserInitializeParams NewParams = ParamsForLoginKey;
					NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
					NewParams.PrimaryInputDevice = EventArgs.InputDevice;

					return TryToInitializeUser(NewParams);
				}

				// See if this controller id is mapped
				MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);

				if (!MappedUser || MappedUser->LocalPlayerIndex == INDEX_NONE)
				{
					if (LoginKeysForNewUser.Contains(EventArgs.Key))
					{
						// If we're in the middle of logging in just return true to ignore platform-specific input
						if (MappedUser && MappedUser->IsDoingLogin())
						{
							return true;
						}

						// Local multiplayer
						FModularUserInitializeParams NewParams = ParamsForLoginKey;
						NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
						NewParams.PrimaryInputDevice = EventArgs.InputDevice;

						return TryToInitializeUser(NewParams);
					}
				}
			}
		}
	}

	if (WrappedInputKeyHandler.IsBound())
	{
		return WrappedInputKeyHandler.Execute(EventArgs);
	}

	return false;
}
