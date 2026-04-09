// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ModularUserTypes.h"
#include "ModularUserInfo.h"

#if MODULARUSER_OSSV1
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineError.h"
#endif
#if !MODULARUSER_OSSV1
#include "Online/OnlineAsyncOpHandle.h"
#endif

#include "ModularUserManager.generated.h"

#define UE_API MODULARUSERMANAGEMENT_API

/**
 * 处理用户身份和登录状态查询和更改的管理器
 */
UCLASS(MinimalAPI)
class UModularUserManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ~ UGameInstanceSubsystem Interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// ~ UGameInstanceSubsystem Interface

	/** 请求的初始化请求完成时调用的蓝图委托 */
	UPROPERTY(BlueprintAssignable, Category = ModularUser)
	FModularUserOnInitializeCompleteMulticast OnUserInitializeComplete;

	/** 系统发送错误/警告消息时调用的蓝图委托 */
	UPROPERTY(BlueprintAssignable, Category = ModularUser)
	FModularUserHandleSystemMessageDelegate OnHandleSystemMessage;

	/** 用户权限可用性更改时调用的蓝图委托  */
	UPROPERTY(BlueprintAssignable, Category = ModularUser)
	FModularUserAvailabilityChangedDelegate OnUserPrivilegeChanged;

	/** 通过 OnHandleSystemMessage 发送系统消息 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual void SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText);

	/** 设置本地玩家的最大数量，不会销毁现有的玩家 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual void SetMaxLocalPlayers(int32 InMaxLocalPLayers);

	/** 获取本地玩家的最大数量 */
	UFUNCTION(BlueprintPure, Category = ModularUser)
	UE_API int32 GetMaxLocalPlayers() const;

	/** 获取本地玩家的当前数量，至少为 1 */
	UFUNCTION(BlueprintPure, Category = ModularUser)
	UE_API int32 GetNumLocalPlayers() const;

	/**返回指定本地玩家的初始化状态 */
	UFUNCTION(BlueprintPure, Category = ModularUser)
	UE_API EModularUserInitializationState GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const;

	/**返回游戏实例中给定本地玩家索引的用户信息，0 在运行的游戏中始终有效*/
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = ModularUser)
	UE_API const UModularUserInfo* GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const;

	/** 已弃用，请使用 PlatformUserId */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = ModularUser)
	UE_API const UModularUserInfo* GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const;

	/** 返回给定平台用户索引的主要用户信息。可能返回 null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = ModularUser)
	UE_API const UModularUserInfo* GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const;

	/** 返回唯一网络 ID 的用户信息。可能返回 null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = ModularUser)
	UE_API const UModularUserInfo* GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const;

	/** 已弃用，请使用 InputDeviceId  */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = ModularUser)
	UE_API const UModularUserInfo* GetUserInfoForControllerId(int32 ControllerId) const;

	/** 返回给定输入设备的用户信息。可能返回 null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = ModularUser)
	UE_API const UModularUserInfo* GetUserInfoForInputDevice(FInputDeviceId InputDevice) const;

	/**
	 * 尝试启动创建或更新本地玩家的过程，包括登录和创建玩家控制器。
	 * 当过程成功或失败时，将广播 OnUserInitializeComplete 委托。
	 *
	 * @param LocalPlayerIndex		LocalPlayer 在游戏实例中的期望索引，0 为主要玩家，1+ 为本地多人游戏
	 * @param PrimaryInputDevice 应映射到此用户的物理控制器，如果无效则使用默认设备
	 * @param bCanUseGuestLogin	如果为 true，此玩家可以是没有真实唯一网络 ID 的访客
	 *
	 * @returns 如果过程已启动则返回 true，如果在正式启动前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual bool TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * 启动本地登录用户执行完整在线登录（包括账户权限检查）的过程。
	 * 当过程成功或失败时，将广播 OnUserInitializeComplete 委托。
	 *
	 * @param LocalPlayerIndex	游戏实例中现有 LocalPlayer 的索引
	 *
	 * @returns 如果过程已启动则返回 true，如果在正式启动前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual bool TryToLoginForOnlinePlay(int32 LocalPlayerIndex);

	/**
	 * 启动一般用户登录和初始化过程，使用参数结构确定登录目标。
	 * 当过程成功或失败时，将广播 OnUserInitializeComplete 委托。
	 * AsyncAction_ModularUserInitialize 提供了几个包装函数，用于在事件图中使用此功能。
	 *
	 * @returns 如果过程已启动则返回 true，如果在正式启动前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual bool TryToInitializeUser(FModularUserInitializeParams Params);

	/** 
	 * 启动监听新用户输入和现有控制器用户输入并登录的过程。
	 * 这将在活动的 GameViewportClient 上插入按键输入处理器，并通过使用空按键数组再次调用来关闭。
	 *
	 * @param AnyUserKeys		监听任何用户的这些按键，即使是默认用户。设置为初始按下开始屏幕或留空以禁用
	 * @param NewUserKeys		监听没有玩家控制器的新用户的这些按键。设置为分屏/本地多人游戏或留空以禁用
	 * @param Params			检测到按键输入后传递给 TryToInitializeUser 的参数
	 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual void ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FModularUserInitializeParams Params);

	/** 尝试取消正在进行的初始化尝试，这可能不适用于所有平台，但将禁用回调 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual bool CancelUserInitialization(int32 LocalPlayerIndex);

	/** 将玩家从任何在线系统中登出，如果不是第一个玩家，还可以选择完全销毁玩家 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual bool TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer = false);

	/** 在出错后返回主菜单时重置登录和初始化状态 */
	UFUNCTION(BlueprintCallable, Category = ModularUser)
	UE_API virtual void ResetUserState();

	/** 如果这可能是具有有效身份的真实平台用户（即使当前未登录），则返回 true  */
	UE_API virtual bool IsRealPlatformUserIndex(int32 PlatformUserIndex) const;

	/** 如果这可能是具有有效身份的真实平台用户（即使当前未登录），则返回 true */
	UE_API virtual bool IsRealPlatformUser(FPlatformUserId PlatformUser) const;

	/** 将索引转换为 ID */
	UE_API virtual FPlatformUserId GetPlatformUserIdForIndex(int32 PlatformUserIndex) const;

	/** 将 ID 转换为索引  */
	UE_API virtual int32 GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const;

	/** 获取输入设备的用户 */
	UE_API virtual FPlatformUserId GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const;

	/** 获取用户的 primary 输入设备 ID */
	UE_API virtual FInputDeviceId GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const;

	/** 从游戏代码调用以在平台状态或选项更改时设置缓存的特征标签 */
	UE_API virtual void SetTraitTags(const FGameplayTagContainer& InTags);

	/** 获取影响功能可用性的当前标签 */
	const FGameplayTagContainer& GetTraitTags() const { return CachedTraitTags; }

	/** 检查是否启用了特定的平台/功能标签 */
	UFUNCTION(BlueprintPure, Category=ModularUser)
	bool HasTraitTag(const FGameplayTag TraitTag) const { return CachedTraitTags.HasTag(TraitTag); }

	/** 检查我们是否应该在启动时显示按下开始/输入确认屏幕。游戏可以调用此函数或直接检查特征标签 */
	UFUNCTION(BlueprintPure, BlueprintPure, Category=ModularUser)
	UE_API virtual bool ShouldWaitForStartInput() const;


	// 访问底层在线系统信息的函数

#if MODULARUSER_OSSV1
	/** 返回特定类型的 OSS 接口，如果没有该类型则返回 null  */
	UE_API IOnlineSubsystem* GetOnlineSubsystem(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回特定类型的 identity 接口，如果没有该类型则返回 null  */
	UE_API IOnlineIdentity* GetOnlineIdentity(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回 OSS 系统的人类可读名称 */
	UE_API FName GetOnlineSubsystemName(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回当前在线连接状态 */
	UE_API EOnlineServerConnectionStatus::Type GetConnectionStatus(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;
#endif
#if !MODULARUSER_OSSV1
	/** 获取服务提供者类型，如果没有则为 None. */
	UE_API UE::Online::EOnlineServices GetOnlineServicesProvider(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回特定类型的 auth 接口，如果没有该类型则返回 null */
	UE_API UE::Online::IAuthPtr GetOnlineAuth(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回当前在线连接状态 */
	UE_API UE::Online::EOnlineServicesConnectionStatus GetConnectionStatus(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;
#endif

	/** 如果我们当前连接到后端服务器，则返回 true */
	UE_API bool HasOnlineConnection(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回指定在线系统中玩家的当前登录状态，仅适用于真实平台用户  */
	UE_API ELoginStatusType GetLocalUserLoginStatus(FPlatformUserId PlatformUser, EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回本地平台用户的唯一网络 ID */
	UE_API FUniqueNetIdRepl GetLocalUserNetId(FPlatformUserId PlatformUser, EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回本地平台用户的昵称，这在 ModularUserInfo 中缓存 */
	UE_API FString GetLocalUserNickname(FPlatformUserId PlatformUser, EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 将用户 ID 转换为调试字符串 */
	UE_API FString PlatformUserIdToString(FPlatformUserId UserId);

	/** 将上下文转换为调试字符串 */
	UE_API FString EModularUserOnlineContextToString(EModularUserOnlineContext Context);

	/** 返回权限检查的人类可读字符串 */
	UE_API virtual FText GetPrivilegeDescription(EModularUserPrivilege Privilege) const;
	UE_API virtual FText GetPrivilegeResultDescription(EModularUserPrivilegeResult Result) const;

	/** 
	 * 启动现有本地用户的登录过程，如果未调度回调则返回 false 
	 * 这会激活底层状态机，不会修改用户信息上的初始化状态
	 */
	DECLARE_DELEGATE_FiveParams(FOnLocalUserLoginCompleteDelegate, const UModularUserInfo* /*UserInfo*/, ELoginStatusType /*NewStatus*/, FUniqueNetIdRepl /*NetId*/, const TOptional<FOnlineErrorType>& /*Error*/,
	                            EModularUserOnlineContext /*Type*/);
	UE_API virtual bool LoginLocalUser(const UModularUserInfo* UserInfo, EModularUserPrivilege RequestedPrivilege, EModularUserOnlineContext Context, FOnLocalUserLoginCompleteDelegate OnComplete);

	/** 将本地玩家分配给特定的本地用户并根据需要调用回调 */
	UE_API virtual void SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UModularUserInfo* UserInfo);

	/** 将具有默认行为的上下文解析为特定上下文 */
	UE_API EModularUserOnlineContext ResolveOnlineContext(EModularUserOnlineContext Context) const;

	/** 如果有单独的平台和服务接口，则为 tru */
	UE_API bool HasSeparatePlatformContext() const;

protected:
	/** 缓存每个在线上下文的状态和指针的内部结构 */
	struct FOnlineContextCache
	{
#if MODULARUSER_OSSV1
		/** 指向基础子系统的指针，只要游戏实例存在就保持有效 */
		IOnlineSubsystem* OnlineSubsystem = nullptr;

		/**缓存的 identity 系统，这将始终有效  */
		IOnlineIdentityPtr IdentityInterface;

		/** 传递给 HandleNetworkConnectionStatusChanged 处理程序的最后一个连接状态*/
		EOnlineServerConnectionStatus::Type CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#endif
#if !MODULARUSER_OSSV1
		/** 在线服务，特定服务的访问器 */
		UE::Online::IOnlineServicesPtr OnlineServices;
		/** 缓存的 auth 服务 */
		UE::Online::IAuthPtr AuthService;
		/** 登录状态更改事件句柄 */
		UE::Online::FOnlineEventDelegateHandle LoginStatusChangedHandle;
		/** 连接状态更改事件句柄  */
		UE::Online::FOnlineEventDelegateHandle ConnectionStatusChangedHandle;
		/** 传递给 HandleNetworkConnectionStatusChanged 处理程序的最后一个连接状态 */
		UE::Online::EOnlineServicesConnectionStatus CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif

		/** 重置状态，重要的是清除所有共享指针 */
		void Reset()
		{
#if MODULARUSER_OSSV1
			OnlineSubsystem = nullptr;
			IdentityInterface.Reset();
			CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#endif
#if !MODULARUSER_OSSV1
			OnlineServices.Reset();
			AuthService.Reset();
			CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif
		}
	};

	/** 表示进行中登录请求的内部结构 */
	struct FUserLoginRequest : public TSharedFromThis<FUserLoginRequest>
	{
		FUserLoginRequest(UModularUserInfo* InUserInfo, EModularUserPrivilege InPrivilege, EModularUserOnlineContext InContext, FOnLocalUserLoginCompleteDelegate&& InDelegate)
			: UserInfo(TWeakObjectPtr<UModularUserInfo>(InUserInfo))
			  , DesiredPrivilege(InPrivilege)
			  , DesiredContext(InContext)
			  , Delegate(MoveTemp(InDelegate))
		{
		}

		/**哪个本地用户尝试登录*/
		TWeakObjectPtr<UModularUserInfo> UserInfo;

		/**登录请求的整体状态，可能来自多个来源 */
		EModularUserAsyncTaskState OverallLoginState = EModularUserAsyncTaskState::NotStarted;

		/** 使用平台认证的尝试状态。启动时，对于 OSSv1，这会立即转换为 Failed，因为我们在那里不支持平台认证  */
		EModularUserAsyncTaskState TransferPlatformAuthState = EModularUserAsyncTaskState::NotStarted;

		/** 使用 AutoLogin 的尝试状态 */
		EModularUserAsyncTaskState AutoLoginState = EModularUserAsyncTaskState::NotStarted;

		/** 使用外部登录 UI 的尝试状态 */
		EModularUserAsyncTaskState LoginUIState = EModularUserAsyncTaskState::NotStarted;

		/** 请求的最终权限 */
		EModularUserPrivilege DesiredPrivilege = EModularUserPrivilege::Invalid_Count;

		/** 请求相关权限的尝试状态 */
		EModularUserAsyncTaskState PrivilegeCheckState = EModularUserAsyncTaskState::NotStarted;

		/** 最终要登录的上下文 */
		EModularUserOnlineContext DesiredContext = EModularUserOnlineContext::Invalid;

		/** 当前要登录的在线系统 */
		EModularUserOnlineContext CurrentContext = EModularUserOnlineContext::Invalid;

		/** 完成时的用户回调 */
		FOnLocalUserLoginCompleteDelegate Delegate;

		/** 向用户显示的最新/最相关的错误 */
		TOptional<FOnlineErrorType> Error;
	};


	/** 创建新的用户信息对象 */
	UE_API virtual UModularUserInfo* CreateLocalUserInfo(int32 LocalPlayerIndex);

	/** const 获取器的非 const 包装器 */
	FORCEINLINE UModularUserInfo* ModifyInfo(const UModularUserInfo* Info) { return const_cast<UModularUserInfo*>(Info); }

	/**从 OSS 刷新用户信息 */
	UE_API virtual void RefreshLocalUserInfo(UModularUserInfo* UserInfo);

	/** 可能发送权限可用性通知，将当前值与缓存的旧值进行比较 */
	UE_API virtual void HandleChangedAvailability(UModularUserInfo* UserInfo, EModularUserPrivilege Privilege, EModularUserAvailability OldAvailability);

	/** 更新用户上的缓存权限并通知委托 */
	UE_API virtual void UpdateUserPrivilegeResult(UModularUserInfo* UserInfo, EModularUserPrivilege Privilege, EModularUserPrivilegeResult Result, EModularUserOnlineContext Context);

	/** 获取在线系统类型的内部数据，服务可能返回 null  */
	UE_API const FOnlineContextCache* GetContextCache(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;
	UE_API FOnlineContextCache* GetContextCache(EModularUserOnlineContext Context = EModularUserOnlineContext::Game);

	/** 在绑定委托之前创建和设置系统对象 */
	UE_API virtual void CreateOnlineContexts();
	UE_API virtual void DestroyOnlineContexts();

	/** 绑定在线委托 */
	UE_API virtual void BindOnlineDelegates();

	/** 强制登出并反初始化单个用户 */
	UE_API virtual void LogOutLocalUser(FPlatformUserId PlatformUser);

	/** 执行登录请求的下一步，可能包括完成它。如果完成则返回 true */
	UE_API virtual void ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request);

	/** 在 OSS 上调用 login，使用来自平台 OSS 的平台认证。如果 AutoLogin 启动则返回 true */
	UE_API virtual bool TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** 在 OSS 上调用 AutoLogin。如果 AutoLogin 启动则返回 true */
	UE_API virtual bool AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/**在 OSS 上调用 ShowLoginUI。如果 ShowLoginUI 启动则返回 true. */
	UE_API virtual bool ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** 在 OSS 上调用 QueryUserPrivilege。如果 QueryUserPrivilege 启动则返回 true . */
	UE_API virtual bool QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** OSS 特定的函数 */
#if MODULARUSER_OSSV1
	UE_API virtual EModularUserPrivilege ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const;
	UE_API virtual EUserPrivileges::Type ConvertOSSPrivilege(EModularUserPrivilege Privilege) const;
	UE_API virtual EModularUserPrivilegeResult ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const;

	UE_API void BindOnlineDelegatesOSSv1();
	UE_API bool AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
#endif
#if !MODULARUSER_OSSV1
	UE_API virtual EModularUserPrivilege ConvertOnlineServicesPrivilege(UE::Online::EUserPrivileges Privilege) const;
	UE_API virtual UE::Online::EUserPrivileges ConvertOnlineServicesPrivilege(EModularUserPrivilege Privilege) const;
	UE_API virtual EModularUserPrivilegeResult ConvertOnlineServicesPrivilegeResult(UE::Online::EUserPrivileges Privilege, UE::Online::EPrivilegeResults Results) const;

	UE_API void BindOnlineDelegatesOSSv2();
	UE_API void CacheConnectionStatus(EModularUserOnlineContext Context);
	UE_API bool TransferPlatformAuthOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool AutoLoginOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API TSharedPtr<UE::Online::FAccountInfo> GetOnlineServiceAccountInfo(UE::Online::IAuthPtr AuthService, FPlatformUserId InUserId) const;
#endif

	/** OSS 函数的回调  */
#if MODULARUSER_OSSV1
	UE_API virtual void HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId, EModularUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& Error, EModularUserOnlineContext Context);
	UE_API virtual void HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus, EModularUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error, EModularUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, EModularUserPrivilege RequestedPrivilege, TWeakObjectPtr<UModularUserInfo> ModularUserInfo,
	                                                  EModularUserOnlineContext Context);
#endif
#if !MODULARUSER_OSSV1
	UE_API virtual void HandleAuthLoginStatusChanged(const UE::Online::FAuthLoginStatusChanged& EventParameters, EModularUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompletedV2(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, FPlatformUserId PlatformUser, EModularUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosedV2(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result, FPlatformUserId PlatformUser, EModularUserOnlineContext Context);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const UE::Online::FConnectionStatusChanged& EventParameters, EModularUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserPrivilege>& Result, TWeakObjectPtr<UModularUserInfo> ModularUserInfo, UE::Online::EUserPrivileges DesiredPrivilege,
	                                                  EModularUserOnlineContext Context);
#endif

	/**
	 * 输入设备（即游戏手柄）连接或断开连接时的回调
	 */
	UE_API virtual void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	UE_API virtual void HandleLoginForUserInitialize(const UModularUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& Error, EModularUserOnlineContext Context,
	                                                 FModularUserInitializeParams Params);
	UE_API virtual void HandleUserInitializeFailed(FModularUserInitializeParams Params, FText Error);
	UE_API virtual void HandleUserInitializeSucceeded(FModularUserInitializeParams Params);

	/** 处理按下开始/登录逻辑的回调 */
	UE_API virtual bool OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs);


	/** 之前的覆盖处理器，将在取消时恢复  */
	FOverrideInputKeyHandler WrappedInputKeyHandler;

	/** 任何用户监听的按键列表 */
	TArray<FKey> LoginKeysForAnyUser;

	/** 新的未映射用户监听的按键列表 */
	TArray<FKey> LoginKeysForNewUser;

	/** 用于按键触发登录的参数 */
	FModularUserInitializeParams ParamsForLoginKey;

	/**本地玩家的最大数量  */
	int32 MaxNumberOfLocalPlayers = 0;

	/** 如果这是不需要 LocalPlayer 的专用服务器，则为 true */
	bool bIsDedicatedServer = false;

	/** 当前进行中的登录请求列表  */
	TArray<TSharedRef<FUserLoginRequest>> ActiveLoginRequests;

	/**每个本地用户的信息，从本地玩家索引到用户 */
	UPROPERTY()
	TMap<int32, TObjectPtr<UModularUserInfo>> LocalUserInfos;

	/** 缓存的平台/模式特征标签 */
	FGameplayTagContainer CachedTraitTags;

	/**不要在初始化之外访问此项 */
	FOnlineContextCache* DefaultContextInternal = nullptr;
	FOnlineContextCache* ServiceContextInternal = nullptr;
	FOnlineContextCache* PlatformContextInternal = nullptr;

	friend UModularUserInfo;
};

#undef UE_API
