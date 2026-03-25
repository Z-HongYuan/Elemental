// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#if ONLINEUSER_OSSV1
/*v1版本的导入*/
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineError.h"
#else
#include "Online/OnlineAsyncOpHandle.h"
#endif

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OnlineUserTypes.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineUserSubsystem.generated.h"

#define UE_API ONLINEUSER_API

class FNativeGameplayTag;

/*在线用户子系统使用的标签列表,包括系统严重程度和报错信息等*/
struct FOnlineUserTags
{
	/*生命常规严重程度级别和特定系统消息*/
	static UE_API FNativeGameplayTag SystemMessage_Error; // 系统消息：错误
	static UE_API FNativeGameplayTag SystemMessage_Warning; // 系统消息：警告
	static UE_API FNativeGameplayTag SystemMessage_Display; // 系统消息：显示

	/*所有初始化玩家的尝试均失败，用户在重试前需要执行某些操作*/
	static UE_API FNativeGameplayTag SystemMessage_Error_InitializeLocalPlayerFailed; // 系统消息：错误：初始化本地玩家失败

	// 平台特性标签，预期游戏实例或其他系统会为适当的平台调用 SetTraitTags 并传入这些标签
	/*此标签表示这是一个主机平台，直接将控制器 ID 映射到不同的系统用户。如果为 false，则同一用户可以拥有多个控制器*/
	static UE_API FNativeGameplayTag Platform_Trait_RequiresStrictControllerMapping; // 平台特性：需要严格的控制器映射
	/*此标签表示该平台只有一个在线用户，所有玩家都使用索引 0*/
	static UE_API FNativeGameplayTag Platform_Trait_SingleOnlineUser; // 平台特性：单一在线用户
};

/*单个用户的逻辑表示，每个已初始化的本地玩家都会存在一个这样的对象*/
UCLASS(MinimalAPI, BlueprintType)
class UOnlineUserInfo : public UObject
{
	GENERATED_BODY()

public:
	/*该用户的主要控制器输入设备，他们可能还拥有额外的次要设备*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/*指定本地平台上的逻辑用户，客座用户将指向主用户*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/*如果此用户被分配了 LocalPlayer，一旦完全创建，这将匹配 GameInstance localplayers 数组中的索引*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	/*如果为 true，则允许此用户作为客座用户*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	/*如果为 true，则这是附加到主用户 0 的客座用户*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	/*用户初始化过程的整体状态*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	ECommonUserInitializationState InitializationState = ECommonUserInitializationState::Invalid;

	/*如果此用户已成功登录，则返回 true*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsLoggedIn() const;

	/*如果此用户正在登录过程中，则返回 true*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsDoingLogin() const;

	/*返回特定权限的最新查询结果，如果从未查询过则返回 unknown*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API ECommonUserPrivilegeResult GetCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/*询问功能的总体可用性，这将结合缓存结果与当前状态*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API ECommonUserAvailability GetPrivilegeAvailability(ECommonUserPrivilege Privilege) const;

	/*返回给定上下文的网络 ID*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FUniqueNetIdRepl GetNetId(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/*返回用户的可读昵称，这将返回在 UpdateCachedNetId 或 SetNickname 期间缓存的值*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetNickname(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/*修改用户的可读昵称，这可用于设置多个客座用户，但对于真实用户会被平台昵称覆盖*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API void SetNickname(const FString& NewNickname, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game);

	/*返回此玩家的内部调试字符串*/
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetDebugString() const;

	/*平台用户 ID 的访问器*/
	UE_API FPlatformUserId GetPlatformUserId() const;

	/*获取平台用户索引，用于期望整数参数的旧函数*/
	UE_API int32 GetPlatformUserIndex() const;

	// 内部数据，仅旨在由在线子系统访问

	/*每个在线系统的缓存数据*/
	struct FCachedData
	{
		/*每个系统缓存的网络 ID*/
		FUniqueNetIdRepl CachedNetId;

		/*缓存的昵称，每当网络 ID 可能更改时更新*/
		FString CachedNickname;

		/*各种用户权限的缓存值*/
		TMap<ECommonUserPrivilege, ECommonUserPrivilegeResult> CachedPrivileges;
	};

	/*每个上下文的缓存，游戏上下文始终存在，但其他上下文可能不存在*/
	TMap<ECommonUserOnlineContext, FCachedData> CachedDataMap;

	/*使用解析规则查找缓存数据*/
	UE_API FCachedData* GetCachedData(ECommonUserOnlineContext Context);
	UE_API const FCachedData* GetCachedData(ECommonUserOnlineContext Context) const;

	/*更新缓存的权限结果，如果需要会传播到游戏上下文*/
	UE_API void UpdateCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context);

	/*更新缓存的网络 ID，如果需要会传播到游戏上下文*/
	UE_API void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, ECommonUserOnlineContext Context);

	/*返回拥有此对象的子系统*/
	UE_API class UOnlineUserSubsystem* GetSubsystem() const;
};

/*当初始化过程成功或失败时触发的委托*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FCommonUserOnInitializeCompleteMulticast, const UOnlineUserInfo*, UserInfo, bool, bSuccess, FText, Error, ECommonUserPrivilege,
                                              RequestedPrivilege, ECommonUserOnlineContext, OnlineContext);

DECLARE_DYNAMIC_DELEGATE_FiveParams(FCommonUserOnInitializeComplete, const UOnlineUserInfo*, UserInfo, bool, bSuccess, FText, Error, ECommonUserPrivilege, RequestedPrivilege,
                                    ECommonUserOnlineContext, OnlineContext);

/*当发送系统错误消息时触发的委托，游戏可以选择使用类型标签将其显示给用户*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCommonUserHandleSystemMessageDelegate, FGameplayTag, MessageType, FText, TitleText, FText, BodyText);

/*当权限更改时触发的委托，可以绑定此委托以查看游戏期间在线状态等是否发生变化*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCommonUserAvailabilityChangedDelegate, const UOnlineUserInfo*, UserInfo, ECommonUserPrivilege, Privilege, ECommonUserAvailability,
                                              OldAvailability, ECommonUserAvailability, NewAvailability);


/*初始化函数的参数结构体，通常由异步节点等包装函数填充*/
USTRUCT(BlueprintType)
struct FCommonUserInitializeParams
{
	GENERATED_BODY()

	/*要使用的本地玩家索引，如果启用了创建玩家功能，可以指定当前索引之上的索引*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 LocalPlayerIndex = 0;

	/*选择平台用户和输入设备的已弃用方法*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 ControllerId = -1;

	/*该用户的主要控制器输入设备，他们可能还拥有额外的次要设备*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/*指定本地平台上的逻辑用户*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/*通常是 CanPlay 或 CanPlayOnline，指定所需的权限级别*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	ECommonUserPrivilege RequestedPrivilege = ECommonUserPrivilege::CanPlay;

	/*要登录的具体在线上下文，game 表示登录到所有相关的上下文*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	ECommonUserOnlineContext OnlineContext = ECommonUserOnlineContext::Game;

	/*如果允许为初始登录创建新的本地玩家，则为 true*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanCreateNewLocalPlayer = false;

	/*如果此玩家可以作为没有实际在线存在的客座用户，则为 true*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanUseGuestLogin = false;

	/*如果不应显示登录错误，则为 true，将由游戏负责显示它们*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bSuppressLoginErrors = false;

	/*如果已绑定，则在登录完成时调用此动态委托*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	FCommonUserOnInitializeComplete OnUserInitializeComplete;
};


/**
 * 处理用户身份和登录状态的查询及更改的游戏子系统。
 * 每个游戏实例都会创建一个子系统，可以从蓝图或 C++ 代码访问。
 * 如果存在特定于游戏的子类，则不会创建此基础子系统。
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UOnlineUserSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UOnlineUserSubsystem()
	{
	}

	//~ UGameInstanceSubsystem interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UGameInstanceSubsystem interface

	/** BP delegate called when any requested initialization request completes
	 * 当任何请求的初始化请求完成时调用的蓝图委托
	 */
	UPROPERTY(BlueprintAssignable, Category = CommonUser)
	FCommonUserOnInitializeCompleteMulticast OnUserInitializeComplete;

	/** BP delegate called when the system sends an error/warning message
	 * 当系统发送错误/警告消息时调用的蓝图委托
	 */
	UPROPERTY(BlueprintAssignable, Category = CommonUser)
	FCommonUserHandleSystemMessageDelegate OnHandleSystemMessage;

	/** BP delegate called when privilege availability changes for a user
	 * 当用户的权限可用性发生变化时调用的蓝图委托
	 */
	UPROPERTY(BlueprintAssignable, Category = CommonUser)
	FCommonUserAvailabilityChangedDelegate OnUserPrivilegeChanged;

	/** Send a system message via OnHandleSystemMessage
	 * 通过 OnHandleSystemMessage 发送系统消息
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual void SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText);

	/** Sets the maximum number of local players, will not destroy existing ones
	 * 设置本地玩家的最大数量，不会销毁现有的玩家
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual void SetMaxLocalPlayers(int32 InMaxLocalPlayers);

	/** Gets the maximum number of local players
	 * 获取本地玩家的最大数量
	 */
	UFUNCTION(BlueprintPure, Category = CommonUser)
	UE_API int32 GetMaxLocalPlayers() const;

	/** Gets the current number of local players, will always be at least 1
	 * 获取当前的本地玩家数量，将始终至少为 1
	 */
	UFUNCTION(BlueprintPure, Category = CommonUser)
	UE_API int32 GetNumLocalPlayers() const;

	/** Returns the state of initializing the specified local player
	 * 返回指定本地玩家的初始化状态
	 */
	UFUNCTION(BlueprintPure, Category = CommonUser)
	UE_API ECommonUserInitializationState GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const;

	/** Returns the user info for a given local player index in game instance, 0 is always valid in a running game
	 * 返回游戏实例中给定本地玩家索引的用户信息，在运行的游戏中 0 始终有效
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	UE_API const UOnlineUserInfo* GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const;

	/** Deprecated, use PlatformUserId when available
	 * 已弃用，如有可能请使用 PlatformUserId
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	UE_API const UOnlineUserInfo* GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const;

	/** Returns the primary user info for a given platform user index. Can return null
	 * 返回给定平台用户索引的主要用户信息。可能返回 null
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	UE_API const UOnlineUserInfo* GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const;

	/** Returns the user info for a unique net id. Can return null
	 * 返回唯一网络 ID 的用户信息。可能返回 null
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	UE_API const UOnlineUserInfo* GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const;

	/** Deprecated, use InputDeviceId when available
	 * 已弃用，如有可能请使用 InputDeviceId
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	UE_API const UOnlineUserInfo* GetUserInfoForControllerId(int32 ControllerId) const;

	/** Returns the user info for a given input device. Can return null
	 * 返回给定输入设备的用户信息。可能返回 null
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	UE_API const UOnlineUserInfo* GetUserInfoForInputDevice(FInputDeviceId InputDevice) const;

	/**
	 * Tries to start the process of creating or updating a local player, including logging in and creating a player controller.
	 * When the process has succeeded or failed, it will broadcast the OnUserInitializeComplete delegate.
	 *
	 * @param LocalPlayerIndex	Desired index of LocalPlayer in Game Instance, 0 will be primary player and 1+ for local multiplayer
	 * @param PrimaryInputDevice The physical controller that should be mapped to this user, will use the default device if invalid
	 * @param bCanUseGuestLogin	If true, this player can be a guest without a real Unique Net Id
	 *
	 * @returns true if the process was started, false if it failed before properly starting
	 * 尝试启动创建或更新本地玩家的过程，包括登录和创建玩家控制器。
	 * 当过程成功或失败时，它将广播 OnUserInitializeComplete 委托。
	 *
	 * @param LocalPlayerIndex	Game Instance 中 LocalPlayer 的理想索引，0 为主玩家，1+ 为本地多人游戏
	 * @param PrimaryInputDevice 应映射到此用户的物理控制器，如果无效则使用默认设备
	 * @param bCanUseGuestLogin	如果为 true，则此玩家可以作为没有真实唯一网络 ID 的客座用户
	 *
	 * @returns 如果过程已启动则返回 true，如果在正确启动之前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual bool TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * Starts the process of taking a locally logged in user and doing a full online login including account permission checks.
	 * When the process has succeeded or failed, it will broadcast the OnUserInitializeComplete delegate.
	 *
	 * @param LocalPlayerIndex	Index of existing LocalPlayer in Game Instance
	 *
	 * @returns true if the process was started, false if it failed before properly starting
	 * 启动将本地登录用户进行完整在线登录的过程，包括账户权限检查。
	 * 当过程成功或失败时，它将广播 OnUserInitializeComplete 委托。
	 *
	 * @param LocalPlayerIndex	Game Instance 中现有 LocalPlayer 的索引
	 *
	 * @returns 如果过程已启动则返回 true，如果在正确启动之前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual bool TryToLoginForOnlinePlay(int32 LocalPlayerIndex);

	/**
	 * Starts a general user login and initialization process, using the params structure to determine what to log in to.
	 * When the process has succeeded or failed, it will broadcast the OnUserInitializeComplete delegate.
	 * AsyncAction_CommonUserInitialize provides several wrapper functions for using this in an Event graph.
	 *
	 * @returns true if the process was started, false if it failed before properly starting
	 * 启动一般用户登录和初始化过程，使用 params 结构体确定要登录的内容。
	 * 当过程成功或失败时，它将广播 OnUserInitializeComplete 委托。
	 * AsyncAction_CommonUserInitialize 提供了几个包装函数以便在事件图中使用此功能。
	 *
	 * @returns 如果过程已启动则返回 true，如果在正确启动之前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual bool TryToInitializeUser(FCommonUserInitializeParams Params);

	/** 
	 * Starts the process of listening for user input for new and existing controllers and logging them.
	 * This will insert a key input handler on the active GameViewportClient and is turned off by calling again with empty key arrays.
	 *
	 * @param AnyUserKeys		Listen for these keys for any user, even the default user. Set this for an initial press start screen or empty to disable
	 * @param NewUserKeys		Listen for these keys for a new user without a player controller. Set this for splitscreen/local multiplayer or empty to disable
	 * @param Params			Params passed to TryToInitializeUser after detecting key input
	 * 启动监听新现有控制器的用户输入并记录它们的过程。
	 * 这将在活动的 GameViewportClient 上插入键输入处理程序，并通过再次调用空键数组来关闭。
	 *
	 * @param AnyUserKeys		监听任何用户（甚至是默认用户）的这些按键。设置为初始“按开始”屏幕或留空以禁用
	 * @param NewUserKeys		监听没有玩家控制器的新用户的这些按键。设置为分屏/本地多人游戏或留空以禁用
	 * @param Params			检测到按键输入后传递给 TryToInitializeUser 的参数
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual void ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FCommonUserInitializeParams Params);

	/** Attempts to cancel an in-progress initialization attempt, this may not work on all platforms but will disable callbacks
	 * 尝试取消进行中的初始化尝试，这可能不适用于所有平台，但将禁用回调
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual bool CancelUserInitialization(int32 LocalPlayerIndex);

	/** Logs a player out of any online systems, and optionally destroys the player entirely if it's not the first one
	 * 将玩家从任何在线系统中注销，如果不是第一个玩家，还可以选择完全销毁该玩家
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual bool TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer = false);

	/** Resets the login and initialization state when returning to the main menu after an error
	 * 当发生错误返回主菜单时重置登录和初始化状态
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	UE_API virtual void ResetUserState();

	/** Returns true if this this could be a real platform user with a valid identity (even if not currently logged in)
	 * 如果这可能是具有有效身份的真实平台用户（即使当前未登录），则返回 true
	 */
	UE_API virtual bool IsRealPlatformUserIndex(int32 PlatformUserIndex) const;

	/** Returns true if this this could be a real platform user with a valid identity (even if not currently logged in)
	 * 如果这可能是具有有效身份的真实平台用户（即使当前未登录），则返回 true
	 */
	UE_API virtual bool IsRealPlatformUser(FPlatformUserId PlatformUser) const;

	/** Converts index to id
	 * 将索引转换为 ID
	 */
	UE_API virtual FPlatformUserId GetPlatformUserIdForIndex(int32 PlatformUserIndex) const;

	/** Converts id to index
	 * 将 ID 转换为索引
	 */
	UE_API virtual int32 GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const;

	/** Gets the user for an input device
	 * 获取输入设备对应的用户
	 */
	UE_API virtual FPlatformUserId GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const;

	/** Gets a user's primary input device id
	 * 获取用户的主要输入设备 ID
	 */
	UE_API virtual FInputDeviceId GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const;

	/** Call from game code to set the cached trait tags when platform state or options changes
	 * 从游戏代码调用，以便在平台状态或选项更改时设置缓存的特性标签
	 */
	UE_API virtual void SetTraitTags(const FGameplayTagContainer& InTags);

	/** Gets the current tags that affect feature avialability
	 * 获取影响功能可用性的当前标签
	 */
	const FGameplayTagContainer& GetTraitTags() const { return CachedTraitTags; }

	/** Checks if a specific platform/feature tag is enabled
	 * 检查是否启用了特定的平台/功能标签
	 */
	UFUNCTION(BlueprintPure, Category=CommonUser)
	bool HasTraitTag(const FGameplayTag TraitTag) const { return CachedTraitTags.HasTag(TraitTag); }

	/** Checks to see if we should display a press start/input confirmation screen at startup. Games can call this or check the trait tags directly
	 * 检查是否应该在启动时显示“按开始”/输入确认屏幕。游戏可以调用此函数或直接检查特性标签
	 */
	UFUNCTION(BlueprintPure, BlueprintPure, Category=CommonUser)
	UE_API virtual bool ShouldWaitForStartInput() const;

	// Functions for accessing low-level online system information
	// 访问低级在线系统信息的函数

#if ONLINEUSER_OSSV1
	/** Returns OSS interface of specific type, will return null if there is no type
	 * 返回特定类型的 OSS 接口，如果没有该类型则返回 null
	 */
	UE_API IOnlineSubsystem* GetOnlineSubsystem(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns identity interface of specific type, will return null if there is no type
	 * 返回特定类型的身份接口，如果没有该类型则返回 null
	 */
	UE_API IOnlineIdentity* GetOnlineIdentity(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns human readable name of OSS system
	 * 返回 OSS 系统的人类可读名称
	 */
	UE_API FName GetOnlineSubsystemName(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the current online connection status
	 * 返回当前的在线连接状态
	 */
	UE_API EOnlineServerConnectionStatus::Type GetConnectionStatus(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;
#else
	/** Get the services provider type, or None if there isn't one.
	 * 获取服务提供商类型，如果没有则返回 None
	 */
	UE_API UE::Online::EOnlineServices GetOnlineServicesProvider(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns auth interface of specific type, will return null if there is no type
	 * 返回特定类型的认证接口，如果没有该类型则返回 null
	 */
	UE_API UE::Online::IAuthPtr GetOnlineAuth(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the current online connection status
	 * 返回当前的在线连接状态
	 */
	UE_API UE::Online::EOnlineServicesConnectionStatus GetConnectionStatus(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;
#endif

	/** Returns true if we are currently connected to backend servers
	 * 如果我们当前已连接到后端服务器，则返回 true
	 */
	UE_API bool HasOnlineConnection(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the current login status for a player on the specified online system, only works for real platform users
	 * 返回指定在线系统上玩家的当前登录状态，仅适用于真实平台用户
	 */
	UE_API ELoginStatusType GetLocalUserLoginStatus(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the unique net id for a local platform user
	 * 返回本地平台用户的唯一网络 ID
	 */
	UE_API FUniqueNetIdRepl GetLocalUserNetId(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the nickname for a local platform user, this is cached in common user Info
	 * 返回本地平台用户的昵称，此信息缓存在 common user Info 中
	 */
	UE_API FString GetLocalUserNickname(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Convert a user id to a debug string
	 * 将用户 ID 转换为调试字符串
	 */
	UE_API FString PlatformUserIdToString(FPlatformUserId UserId);

	/** Convert a context to a debug string
	 * 将上下文转换为调试字符串
	 */
	UE_API FString ECommonUserOnlineContextToString(ECommonUserOnlineContext Context);

	/** Returns human readable string for privilege checks
	 * 返回用于权限检查的人类可读字符串
	 */
	UE_API virtual FText GetPrivilegeDescription(ECommonUserPrivilege Privilege) const;
	UE_API virtual FText GetPrivilegeResultDescription(ECommonUserPrivilegeResult Result) const;

	/** 
	 * Starts the process of login for an existing local user, will return false if callback was not scheduled 
	 * This activates the low level state machine and does not modify the initialization state on user info
	 * 启动现有本地用户的登录过程，如果未安排回调则返回 false
	 * 这会激活低级状态机，并且不会修改用户信息上的初始化状态
	 */
	DECLARE_DELEGATE_FiveParams(FOnLocalUserLoginCompleteDelegate, const UOnlineUserInfo* /*UserInfo*/, ELoginStatusType /*NewStatus*/, FUniqueNetIdRepl /*NetId*/,
	                            const TOptional<FOnlineErrorType>& /*Error*/, ECommonUserOnlineContext /*Type*/);
	UE_API virtual bool LoginLocalUser(const UOnlineUserInfo* UserInfo, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext Context,
	                                   FOnLocalUserLoginCompleteDelegate OnComplete);

	/** Assign a local player to a specific local user and call callbacks as needed
	 * 将本地玩家分配给特定的本地用户并按需调用回调
	 */
	UE_API virtual void SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UOnlineUserInfo* UserInfo);

	/** Resolves a context that has default behavior into a specific context
	 * 将具有默认行为的上下文解析为特定上下文
	 */
	UE_API ECommonUserOnlineContext ResolveOnlineContext(ECommonUserOnlineContext Context) const;

	/** True if there is a separate platform and service interface
	 * 如果存在独立的平台和服务接口，则为 true
	 */
	UE_API bool HasSeparatePlatformContext() const;

protected:
	/** Internal structure that caches status and pointers for each online context
	 * 缓存每个在线上下文的状态和指针的内部结构
	 */
	struct FOnlineContextCache
	{
#if ONLINEUSER_OSSV1
		/** Pointer to base subsystem, will stay valid as long as game instance does
		 * 指向基础子系统的指针，只要游戏实例存在就保持有效
		 */
		IOnlineSubsystem* OnlineSubsystem = nullptr;

		/** Cached identity system, this will always be valid
		 * 缓存的身份系统，这将始终有效
		 */
		IOnlineIdentityPtr IdentityInterface;

		/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander
		 * 最后传递给 HandleNetworkConnectionStatusChanged 处理程序的连接状态
		 */
		EOnlineServerConnectionStatus::Type CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#else
		/** Online services, accessor to specific services
		 * 在线服务，特定服务的访问器
		 */
		UE::Online::IOnlineServicesPtr OnlineServices;
		/** Cached auth service
		 * 缓存的认证服务
		 */
		UE::Online::IAuthPtr AuthService;
		/** Login status changed event handle
		 * 登录状态更改事件句柄
		 */
		UE::Online::FOnlineEventDelegateHandle LoginStatusChangedHandle;
		/** Connection status changed event handle
		 * 连接状态更改事件句柄
		 */
		UE::Online::FOnlineEventDelegateHandle ConnectionStatusChangedHandle;
		/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander
		 * 最后传递给 HandleNetworkConnectionStatusChanged 处理程序的连接状态
		 */
		UE::Online::EOnlineServicesConnectionStatus CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif

		/** Resets state, important to clear all shared ptrs
		 * 重置状态，重要的是清除所有共享指针
		 */
		void Reset()
		{
#if ONLINEUSER_OSSV1
			OnlineSubsystem = nullptr;
			IdentityInterface.Reset();
			CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#else
			OnlineServices.Reset();
			AuthService.Reset();
			CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif
		}
	};

	/** Internal structure to represent an in-progress login request
	 * 表示进行中登录请求的内部结构
	 */
	struct FUserLoginRequest : public TSharedFromThis<FUserLoginRequest>
	{
		FUserLoginRequest(UOnlineUserInfo* InUserInfo, ECommonUserPrivilege InPrivilege, ECommonUserOnlineContext InContext, FOnLocalUserLoginCompleteDelegate&& InDelegate)
			: UserInfo(TWeakObjectPtr<UOnlineUserInfo>(InUserInfo))
			  , DesiredPrivilege(InPrivilege)
			  , DesiredContext(InContext)
			  , Delegate(MoveTemp(InDelegate))
		{
		}

		/** Which local user is trying to log on
		 * 哪个本地用户正在尝试登录
		 */
		TWeakObjectPtr<UOnlineUserInfo> UserInfo;

		/** Overall state of login request, could come from many sources
		 * 登录请求的整体状态，可能来自多个来源
		 */
		ECommonUserAsyncTaskState OverallLoginState = ECommonUserAsyncTaskState::NotStarted;

		/** State of attempt to use platform auth. When started, this immediately transitions to Failed for OSSv1, as we do not support platform auth there.
		 * 尝试使用平台认证的状态。启动时，对于 OSSv1 这会立即转变为失败，因为我们在那里不支持平台认证。
		 */
		ECommonUserAsyncTaskState TransferPlatformAuthState = ECommonUserAsyncTaskState::NotStarted;

		/** State of attempt to use AutoLogin
		 * 尝试使用自动登录的状态
		 */
		ECommonUserAsyncTaskState AutoLoginState = ECommonUserAsyncTaskState::NotStarted;

		/** State of attempt to use external login UI
		 * 尝试使用外部登录 UI 的状态
		 */
		ECommonUserAsyncTaskState LoginUIState = ECommonUserAsyncTaskState::NotStarted;

		/** Final privilege to that is requested
		 * 最终请求的权限
		 */
		ECommonUserPrivilege DesiredPrivilege = ECommonUserPrivilege::Invalid_Count;

		/** State of attempt to request the relevant privilege
		 * 尝试请求相关权限的状态
		 */
		ECommonUserAsyncTaskState PrivilegeCheckState = ECommonUserAsyncTaskState::NotStarted;

		/** The final context to log into
		 * 最终要登录的上下文
		 */
		ECommonUserOnlineContext DesiredContext = ECommonUserOnlineContext::Invalid;

		/** What online system we are currently logging into
		 * 我们当前正在登录的在线系统
		 */
		ECommonUserOnlineContext CurrentContext = ECommonUserOnlineContext::Invalid;

		/** User callback for completion
		 * 用户完成回调
		 */
		FOnLocalUserLoginCompleteDelegate Delegate;

		/** Most recent/relevant error to display to user
		 * 最近/最相关的错误显示给用户
		 */
		TOptional<FOnlineErrorType> Error;
	};


	/** Create a new user info object
	 * 创建新的用户信息对象
	 */
	UE_API virtual UOnlineUserInfo* CreateLocalUserInfo(int32 LocalPlayerIndex);

	/** Deconst wrapper for const getters
	 * const 获取器的去 const 包装器
	 */
	FORCEINLINE UOnlineUserInfo* ModifyInfo(const UOnlineUserInfo* Info) { return const_cast<UOnlineUserInfo*>(Info); }

	/** Refresh user info from OSS
	 * 从 OSS 刷新用户信息
	 */
	UE_API virtual void RefreshLocalUserInfo(UOnlineUserInfo* UserInfo);

	/** Possibly send privilege availability notification, compares current value to cached old value
	 * 可能发送权限可用性通知，将当前值与缓存的旧值进行比较
	 */
	UE_API virtual void HandleChangedAvailability(UOnlineUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability);

	/** Updates the cached privilege on a user and notifies delegate
	 * 更新用户上的缓存权限并通知委托
	 */
	UE_API virtual void UpdateUserPrivilegeResult(UOnlineUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context);

	/** Gets internal data for a type of online system, can return null for service
	 * 获取某种在线系统的内部数据，服务可能返回 null
	 */
	UE_API const FOnlineContextCache* GetContextCache(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;
	UE_API FOnlineContextCache* GetContextCache(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game);

	/** Create and set up system objects before delegates are bound
	 * 在绑定委托之前创建并设置系统对象
	 */
	UE_API virtual void CreateOnlineContexts();
	UE_API virtual void DestroyOnlineContexts();

	/** Bind online delegates
	 * 绑定在线委托
	 */
	UE_API virtual void BindOnlineDelegates();

	/** Forcibly logs out and deinitializes a single user
	 * 强制注销并反初始化单个用户
	 */
	UE_API virtual void LogOutLocalUser(FPlatformUserId PlatformUser);

	/** Performs the next step of a login request, which could include completing it. Returns true if it's done
	 * 执行登录请求的下一步，可能包括完成它。如果完成则返回 true
	 */
	UE_API virtual void ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request);

	/** Call login on OSS, with platform auth from the platform OSS. Return true if AutoLogin started
	 * 在 OSS 上调用登录，使用来自平台 OSS 的平台认证。如果启动了自动登录则返回 true
	 */
	UE_API virtual bool TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** Call AutoLogin on OSS. Return true if AutoLogin started.
	 * 在 OSS 上调用自动登录。如果启动了自动登录则返回 true。
	 */
	UE_API virtual bool AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** Call ShowLoginUI on OSS. Return true if ShowLoginUI started.
	 * 在 OSS 上调用 ShowLoginUI。如果启动了 ShowLoginUI 则返回 true。
	 */
	UE_API virtual bool ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** Call QueryUserPrivilege on OSS. Return true if QueryUserPrivilege started.
	 * 在 OSS 上调用 QueryUserPrivilege。如果启动了 QueryUserPrivilege 则返回 true。
	 */
	UE_API virtual bool QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** OSS-specific functions
	 * OSS 特定函数
	 */
#if ONLINEUSER_OSSV1
	UE_API virtual ECommonUserPrivilege ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const;
	UE_API virtual EUserPrivileges::Type ConvertOSSPrivilege(ECommonUserPrivilege Privilege) const;
	UE_API virtual ECommonUserPrivilegeResult ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const;

	UE_API void BindOnlineDelegatesOSSv1();
	UE_API bool AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
#else
	UE_API virtual ECommonUserPrivilege ConvertOnlineServicesPrivilege(UE::Online::EUserPrivileges Privilege) const;
	UE_API virtual UE::Online::EUserPrivileges ConvertOnlineServicesPrivilege(ECommonUserPrivilege Privilege) const;
	UE_API virtual ECommonUserPrivilegeResult ConvertOnlineServicesPrivilegeResult(UE::Online::EUserPrivileges Privilege, UE::Online::EPrivilegeResults Results) const;

	UE_API void BindOnlineDelegatesOSSv2();
	UE_API void CacheConnectionStatus(ECommonUserOnlineContext Context);
	UE_API bool TransferPlatformAuthOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool AutoLoginOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API TSharedPtr<UE::Online::FAccountInfo> GetOnlineServiceAccountInfo(UE::Online::IAuthPtr AuthService, FPlatformUserId InUserId) const;
#endif

	/** Callbacks for OSS functions
	 * OSS 函数的回调
	 */
#if ONLINEUSER_OSSV1
	UE_API virtual void HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId,
	                                                     ECommonUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& Error, ECommonUserOnlineContext Context);
	UE_API virtual void HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus,
	                                                         EOnlineServerConnectionStatus::Type ConnectionStatus, ECommonUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error,
	                                          ECommonUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, ECommonUserPrivilege RequestedPrivilege,
	                                                  TWeakObjectPtr<UOnlineUserInfo> CommonUserInfo, ECommonUserOnlineContext Context);
#else
	UE_API virtual void HandleAuthLoginStatusChanged(const UE::Online::FAuthLoginStatusChanged& EventParameters, ECommonUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompletedV2(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, FPlatformUserId PlatformUser, ECommonUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosedV2(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result, FPlatformUserId PlatformUser,
	                                            ECommonUserOnlineContext Context);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const UE::Online::FConnectionStatusChanged& EventParameters, ECommonUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserPrivilege>& Result, TWeakObjectPtr<UOnlineUserInfo> CommonUserInfo,
	                                                  UE::Online::EUserPrivileges DesiredPrivilege, ECommonUserOnlineContext Context);
#endif

	/**
	 * Callback for when an input device (i.e. a gamepad) has been connected or disconnected. 
	 * 当输入设备（例如游戏手柄）连接或断开连接时的回调。
	 */
	UE_API virtual void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	UE_API virtual void HandleLoginForUserInitialize(const UOnlineUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& Error,
	                                                 ECommonUserOnlineContext Context, FCommonUserInitializeParams Params);
	UE_API virtual void HandleUserInitializeFailed(FCommonUserInitializeParams Params, FText Error);
	UE_API virtual void HandleUserInitializeSucceeded(FCommonUserInitializeParams Params);

	/** Callback for handling press start/login logic
	 * 处理按开始/登录逻辑的回调
	 */
	UE_API virtual bool OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs);


	/** Previous override handler, will restore on cancel
	 * 之前的覆盖处理程序，取消时将恢复
	 */
	FOverrideInputKeyHandler WrappedInputKeyHandler;

	/** List of keys to listen for from any user
	 * 要监听任何用户的按键列表
	 */
	TArray<FKey> LoginKeysForAnyUser;

	/** List of keys to listen for a new unmapped user
	 * 要监听新的未映射用户的按键列表
	 */
	TArray<FKey> LoginKeysForNewUser;

	/** Params to use for a key-triggered login
	 * 用于按键触发登录的参数
	 */
	FCommonUserInitializeParams ParamsForLoginKey;

	/** Maximum number of local players
	 * 本地玩家的最大数量
	 */
	int32 MaxNumberOfLocalPlayers = 0;

	/** True if this is a dedicated server, which doesn't require a LocalPlayer
	 * 如果这是不需要 LocalPlayer 的专用服务器，则为 true
	 */
	bool bIsDedicatedServer = false;

	/** List of current in progress login requests
	 * 当前进行中的登录请求列表
	 */
	TArray<TSharedRef<FUserLoginRequest>> ActiveLoginRequests;

	/** Information about each local user, from local player index to user
	 * 关于每个本地用户的信息，从本地玩家索引到用户
	 */
	UPROPERTY()
	TMap<int32, TObjectPtr<UOnlineUserInfo>> LocalUserInfos;

	/** Cached platform/mode trait tags
	 * 缓存的平台/模式特性标签
	 */
	FGameplayTagContainer CachedTraitTags;

	/** Do not access this outside of initialization
	 * 不要在初始化之外访问此项
	 */
	FOnlineContextCache* DefaultContextInternal = nullptr;
	FOnlineContextCache* ServiceContextInternal = nullptr;
	FOnlineContextCache* PlatformContextInternal = nullptr;

	friend UOnlineUserInfo;
};

#undef UE_API
