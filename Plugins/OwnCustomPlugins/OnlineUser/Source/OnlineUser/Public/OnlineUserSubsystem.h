// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if ONLINEUSER_OSSV1
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineError.h"
#endif

#if !ONLINEUSER_OSSV1
#include "Online/OnlineAsyncOpHandle.h"
#endif

#include "NativeGameplayTags.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineUserTypes.h"
#include "OnlineUserSubsystem.generated.h"


#define UE_API ONLINEUSER_API

class FNativeGameplayTag;

/*构建子系统使用的GameplayTag*/
namespace OnlineUserTags
{
	// 常规严重程度级别和特定系统消息
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnlineUser_SystemMessage_Error);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnlineUser_SystemMessage_Warning)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnlineUser_SystemMessage_Display)

	// 所有初始化玩家的尝试均失败，用户在重试前需要执行某些操作
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnlineUser_SystemMessage_Error_InitializeLocalPlayerFailed)


	// 平台特性标签，预期游戏实例或其他系统会为适当的平台调用 SetTraitTags 并传入这些标签

	/*此标签表示这是一个主机平台，直接将控制器 ID 映射到不同的系统用户。如果为 false，则同一用户可以拥有多个控制器*/
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnlineUser_Platform_Trait_RequiresStrictControllerMapping)

	/*此标签表示该平台只有一个在线用户，所有玩家都使用索引 0*/
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnlineUser_Platform_Trait_SingleOnlineUser)
}


/*用户的数据对象，每个已初始化的本地玩家都会存在一个这样的对象,封装和管理玩家的用户身份信息*/
UCLASS(MinimalAPI, BlueprintType)
class UOnlineUserInfo : public UObject
{
	GENERATED_BODY()

public:
	//表示主要控制器输入设备，他们可能还拥有额外的次要设备 游戏手柄、键盘等
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	//指定本地平台上的逻辑用户，游客用户将指向主用户
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	//此用户在 GameInstance 本地玩家数组中的索引
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	//如果为 true，则允许此用户作为游客用户
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	//如果为 true，则这是附加到主用户 0 的游客用户
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	//保存的用户初始化过程的整体状态
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	EOnlineUserInitializationState InitializationState = EOnlineUserInitializationState::Invalid;

	//查看本对象中保存的状态枚举 如果此用户已成功登录，则返回 true
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsLoggedIn() const;

	//查看本对象中保存的状态枚举 如果此用户正在登录过程中，则返回 true
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsDoingLogin() const;

	//返回特定权限的最新查询结果，如果从未查询过则返回 unknown
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EOnlineUserPrivilegeResult GetCachedPrivilegeResult(EOnlineUserPrivilege Privilege, EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const;

	//询问功能的总体可用性，这将结合缓存结果与当前状态,综合判断,使用了多方的信息参数
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EOnlineUserAvailability GetPrivilegeAvailability(EOnlineUserPrivilege Privilege) const;

	//获取网络ID,一般从缓存中获取
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FUniqueNetIdRepl GetNetId(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const;

	//返回用户的可读昵称，这将返回在 UpdateCachedNetId 或 SetNickname 期间缓存的值
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetNickname(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const;

	//修改用户的可读昵称，这可用于设置多个游客用户，但对于真实用户会被平台昵称覆盖
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API void SetNickname(const FString& NewNickname, EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game);

	//返回此玩家的内部调试字符串
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetDebugString() const;

	// PlatformUser 的Getter
	UE_API FPlatformUserId GetPlatformUserId() const;

	//获取平台用户索引，用于期望整数参数的旧函数
	UE_API int32 GetPlatformUserIndex() const;

	// 内部数据，仅旨在由在线子系统访问

	//每个在线系统的缓存数据
	struct FCachedData
	{
		//每个系统缓存的网络 ID
		FUniqueNetIdRepl CachedNetId;

		//缓存的昵称，每当网络 ID 可能更改时更新
		FString CachedNickname;

		//各种用户权限的缓存值
		TMap<EOnlineUserPrivilege, EOnlineUserPrivilegeResult> CachedPrivileges;
	};

	// 每个上下文的缓存，游戏上下文始终存在，但其他上下文可能不存在
	TMap<EOnlineUserOnlineContext, FCachedData> CachedDataMap;

	//使用解析规则查找缓存数据,要么从数据对象中查找,要么从Subsystem中查找
	//智能地获取正确上下文的缓存
	UE_API FCachedData* GetCachedData(EOnlineUserOnlineContext Context);
	UE_API const FCachedData* GetCachedData(EOnlineUserOnlineContext Context) const;

	//更新缓存的权限结果，如果需要会传播到游戏上下文
	UE_API void UpdateCachedPrivilegeResult(EOnlineUserPrivilege Privilege, EOnlineUserPrivilegeResult Result, EOnlineUserOnlineContext Context);

	//更新缓存的网络 ID，如果需要会传播到游戏上下文
	UE_API void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EOnlineUserOnlineContext Context);

	//从Outer中获取OnlineSubsystem
	UE_API class UOnlineUserSubsystem* GetSubsystem() const;
};

/*当初始化过程成功或失败时触发的委托*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineUserOnInitializeCompleteMulticast, const UOnlineUserInfo*, UserInfo, bool, bSuccess, FText, Error, EOnlineUserPrivilege,
                                              RequestedPrivilege, EOnlineUserOnlineContext, OnlineContext);

DECLARE_DYNAMIC_DELEGATE_FiveParams(FOnlineUserOnInitializeComplete, const UOnlineUserInfo*, UserInfo, bool, bSuccess, FText, Error, EOnlineUserPrivilege, RequestedPrivilege,
                                    EOnlineUserOnlineContext, OnlineContext);

/*当发送系统错误消息时触发的委托，游戏可以选择使用类型标签将其显示给用户*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineUserHandleSystemMessageDelegate, FGameplayTag, MessageType, FText, TitleText, FText, BodyText);

/*当权限更改时触发的委托，可以绑定此委托以查看游戏期间在线状态等是否发生变化*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineUserAvailabilityChangedDelegate, const UOnlineUserInfo*, UserInfo, EOnlineUserPrivilege, Privilege, EOnlineUserAvailability,
                                              OldAvailability, EOnlineUserAvailability, NewAvailability);

/* 初始化函数的参数结构体，通常由异步节点等包装函数填充,全是变量,没有函数
 * 用于初始化用户
 */
USTRUCT(BlueprintType)
struct FOnlineUserInitializeParams
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
	EOnlineUserPrivilege RequestedPrivilege = EOnlineUserPrivilege::CanPlay;

	/*要登录的具体在线上下文，game 表示登录到所有相关的上下文*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	EOnlineUserOnlineContext OnlineContext = EOnlineUserOnlineContext::Game;

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
	FOnlineUserOnInitializeComplete OnUserInitializeComplete;
};


/**
 * 处理用户身份和登录状态的查询及更改的游戏子系统。
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UOnlineUserSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UOnlineUserSubsystem()
	{
	}

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override; //保持继承链单例

	/*委托列表*/
	UPROPERTY(BlueprintAssignable, Category = OnlineUser)
	FOnlineUserOnInitializeCompleteMulticast OnUserInitializeComplete; //当任何请求的初始化请求完成时调用的蓝图委托
	UPROPERTY(BlueprintAssignable, Category = OnlineUser)
	FOnlineUserHandleSystemMessageDelegate OnHandleSystemMessage; //当系统发送错误/警告消息时调用的蓝图委托
	UPROPERTY(BlueprintAssignable, Category = OnlineUser)
	FOnlineUserAvailabilityChangedDelegate OnUserPrivilegeChanged; //当用户的权限可用性发生变化时调用的蓝图委托
	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual void SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText)
	{
		OnHandleSystemMessage.Broadcast(MessageType, TitleText, BodyText); //调用委托广播系统消息
	}

	/*
	 * 本地玩家数量的Getter和Setter
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual void SetMaxLocalPlayers(int32 InMaxLocalPlayers); //设置最大本地玩家数量
	UFUNCTION(BlueprintPure, Category = OnlineUser)
	UE_API int32 GetMaxLocalPlayers() const; //获取最大本地玩家数量(返回自身变量)
	UFUNCTION(BlueprintPure, Category = OnlineUser)
	UE_API int32 GetNumLocalPlayers() const; //获取当前本地玩家数量(从游戏实例),应该至少为1
	UFUNCTION(BlueprintPure, Category = OnlineUser)
	UE_API EOnlineUserInitializationState GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const; //获取指定本地玩家的初始化状态

	/* UOnlineUserInfo 的Getter*/
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = OnlineUser)
	UE_API const UOnlineUserInfo* GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const; //从本地玩家索引获取用户信息,在Runtime中0应该始终有效
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = OnlineUser)
	UE_API const UOnlineUserInfo* GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const; //已弃用，如有可能请使用 PlatformUserId
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = OnlineUser)
	UE_API const UOnlineUserInfo* GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const; //从新API中获取用户信息,从平台用户中获取
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = OnlineUser)
	UE_API const UOnlineUserInfo* GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const; //从网络ID中获取用户信息
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = OnlineUser)
	UE_API const UOnlineUserInfo* GetUserInfoForControllerId(int32 ControllerId) const; //已弃用，如有可能请使用 InputDeviceId
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = OnlineUser)
	UE_API const UOnlineUserInfo* GetUserInfoForInputDevice(FInputDeviceId InputDevice) const; //从输入设备中获取用户信息

	/*初始化流程*/
	/*
	 * 尝试启动创建或更新本地玩家的过程，包括登录和创建玩家控制器。
	 * 当过程成功或失败时，它将广播 OnUserInitializeComplete 委托。
	 * @param LocalPlayerIndex	GameInstance中LocalPlayer的理想索引，0为主玩家，1+为本地多人游戏
	 * @param PrimaryInputDevice 应映射到此用户的物理控制器，如果无效则使用默认设备
	 * @returns 如果过程已启动则返回 true，如果在正确启动之前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual bool TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/*
	 * 启动将本地登录用户进行完整在线登录的过程，包括账户权限检查。
	 * 当过程成功或失败时，它将广播 OnUserInitializeComplete 委托。
	 * @param LocalPlayerIndex GameInstance中现有LocalPlayer的索引
	 * @returns 如果过程已启动则返回 true，如果在正确启动之前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual bool TryToLoginForOnlinePlay(int32 LocalPlayerIndex);

	/*
	 * 启动一般用户登录和初始化过程，使用 params 结构体确定要登录的内容。
	 * 当过程成功或失败时，它将广播 OnUserInitializeComplete 委托。
	 * 异步蓝图节点 提供了几个包装函数以便在事件图中使用此功能。
	 * @returns 如果过程已启动则返回 true，如果在正确启动之前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual bool TryToInitializeUser(FOnlineUserInitializeParams Params);

	/*
	 * 启动监听新现有控制器的用户输入并记录它们的过程。
	 * 这将在活动的 GameViewportClient 上插入键输入处理程序，并通过再次调用空键数组来关闭。
	 * @param AnyUserKeys		监听任何用户（甚至是默认用户）的这些按键。设置为初始“按开始”屏幕或留空以禁用
	 * @param NewUserKeys		监听没有玩家控制器的新用户的这些按键。设置为分屏/本地多人游戏或留空以禁用
	 * @param Params			检测到按键输入后传递给 TryToInitializeUser 的参数
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual void ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FOnlineUserInitializeParams Params);

	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual bool CancelUserInitialization(int32 LocalPlayerIndex); //尝试取消进行中的初始化尝试，这可能不适用于所有平台，但将禁用回调

	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual bool TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer = false); //将玩家从任何在线系统中注销，如果不是第一个玩家，还可以选择完全销毁该玩家

	UFUNCTION(BlueprintCallable, Category = OnlineUser)
	UE_API virtual void ResetUserState(); //当发生错误返回主菜单时重置登录和初始化状态

	UE_API virtual bool IsRealPlatformUserIndex(int32 PlatformUserIndex) const; //如果这可能是具有有效身份的真实平台用户（即使当前未登录），则返回 true
	UE_API virtual bool IsRealPlatformUser(FPlatformUserId PlatformUser) const; //如果这可能是具有有效身份的真实平台用户（即使当前未登录），则返回 true

	UE_API virtual FPlatformUserId GetPlatformUserIdForIndex(int32 PlatformUserIndex) const; //从ID获取索引
	UE_API virtual int32 GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const; //从索引获取ID
	UE_API virtual FPlatformUserId GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const; //获取输入设备对应的用户
	UE_API virtual FInputDeviceId GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const; //获取用户的主要输入设备 ID
	UE_API virtual void SetTraitTags(const FGameplayTagContainer& InTags); //从游戏代码调用，以便在平台状态或选项更改时设置缓存的特性标签

	const FGameplayTagContainer& GetTraitTags() const { return CachedTraitTags; } //获取影响功能可用性的当前标签

	UFUNCTION(BlueprintPure, Category=OnlineUser)
	bool HasTraitTag(const FGameplayTag TraitTag) const { return CachedTraitTags.HasTag(TraitTag); } //检查是否启用了特定的平台/功能标签

	UFUNCTION(BlueprintPure, BlueprintPure, Category=OnlineUser)
	UE_API virtual bool ShouldWaitForStartInput() const; //检查是否应该在启动时显示“按开始”/输入确认屏幕。游戏可以调用此函数或直接检查特性标签


	// 访问低层级在线系统信息的函数
#if ONLINEUSER_OSSV1
	UE_API IOnlineSubsystem* GetOnlineSubsystem(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回特定类型的 OSS 接口，如果没有该类型则返回 null
	UE_API IOnlineIdentity* GetOnlineIdentity(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回特定类型的身份接口，如果没有该类型则返回 null
	UE_API FName GetOnlineSubsystemName(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回 OSS 系统的人类可读名称
	UE_API EOnlineServerConnectionStatus::Type GetConnectionStatus(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回当前的在线连接状态
#endif

#if !ONLINEUSER_OSSV1
	UE_API UE::Online::EOnlineServices GetOnlineServicesProvider(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //获取服务提供商类型，如果没有则返回 None
	UE_API UE::Online::IAuthPtr GetOnlineAuth(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回特定类型的认证接口，如果没有该类型则返回 null
	UE_API UE::Online::EOnlineServicesConnectionStatus GetConnectionStatus(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回当前的在线连接状态
#endif

	UE_API bool HasOnlineConnection(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //如果我们当前已连接到后端服务器，则返回 true

	UE_API ELoginStatusType GetLocalUserLoginStatus(FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const;
	//返回指定在线系统上玩家的当前登录状态，仅适用于真实平台用户

	UE_API FUniqueNetIdRepl GetLocalUserNetId(FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const; //返回本地平台用户的唯一网络 ID

	UE_API FString GetLocalUserNickname(FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const;
	//返回本地平台用户的昵称，此信息缓存在 Online user Info 中

	UE_API FString PlatformUserIdToString(FPlatformUserId UserId); //将用户 ID 转换为调试字符串

	UE_API FString EOnlineUserOnlineContextToString(EOnlineUserOnlineContext Context); //将上下文转换为调试字符串

	//返回用于权限检查的人类可读字符串
	UE_API virtual FText GetPrivilegeDescription(EOnlineUserPrivilege Privilege) const;
	UE_API virtual FText GetPrivilegeResultDescription(EOnlineUserPrivilegeResult Result) const;

	/** 
	 * 启动现有本地用户的登录过程，如果未安排回调则返回 false
	 * 这会激活低级状态机，并且不会修改用户信息上的初始化状态
	 */
	DECLARE_DELEGATE_FiveParams(FOnLocalUserLoginCompleteDelegate, const UOnlineUserInfo* /*UserInfo*/, ELoginStatusType /*NewStatus*/, FUniqueNetIdRepl /*NetId*/,
	                            const TOptional<FOnlineErrorType>& /*Error*/, EOnlineUserOnlineContext /*Type*/);
	UE_API virtual bool LoginLocalUser(const UOnlineUserInfo* UserInfo, EOnlineUserPrivilege RequestedPrivilege, EOnlineUserOnlineContext Context,
	                                   FOnLocalUserLoginCompleteDelegate OnComplete);

	UE_API virtual void SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UOnlineUserInfo* UserInfo); //将本地玩家分配给特定的本地用户并按需调用回调

	UE_API EOnlineUserOnlineContext ResolveOnlineContext(EOnlineUserOnlineContext Context) const; //将具有默认行为的上下文解析为特定上下文

	UE_API bool HasSeparatePlatformContext() const; //如果存在独立的平台和服务接口，则为 true

protected:
	//缓存每个在线上下文的状态和指针的内部结构
	struct FOnlineContextCache
	{
#if ONLINEUSER_OSSV1

		IOnlineSubsystem* OnlineSubsystem = nullptr; //指向基础子系统的指针，只要游戏实例存在就保持有效
		IOnlineIdentityPtr IdentityInterface; //缓存的身份系统，这将始终有效
		EOnlineServerConnectionStatus::Type CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal; //最后传递给 HandleNetworkConnectionStatusChanged 处理程序的连接状态
#endif

#if !ONLINEUSER_OSSV1

		UE::Online::IOnlineServicesPtr OnlineServices; //在线服务，特定服务的访问器

		UE::Online::IAuthPtr AuthService; //缓存的认证服务

		UE::Online::FOnlineEventDelegateHandle LoginStatusChangedHandle; //登录状态更改事件句柄

		UE::Online::FOnlineEventDelegateHandle ConnectionStatusChangedHandle; //连接状态更改事件句柄

		UE::Online::EOnlineServicesConnectionStatus CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
		//最后传递给 HandleNetworkConnectionStatusChanged 处理程序的连接状态
#endif


		void Reset() //重置状态，重要的是清除所有共享指针
		{
#if ONLINEUSER_OSSV1
			OnlineSubsystem = nullptr;
			IdentityInterface.Reset();
			CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#endif

#if !ONLINEUSER_OSSV1
			OnlineServices.Reset();
			AuthService.Reset();
			CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif
		}
	};

	//表示进行中登录请求的内部结构
	struct FUserLoginRequest : public TSharedFromThis<FUserLoginRequest>
	{
		FUserLoginRequest(UOnlineUserInfo* InUserInfo, EOnlineUserPrivilege InPrivilege, EOnlineUserOnlineContext InContext, FOnLocalUserLoginCompleteDelegate&& InDelegate)
			: UserInfo(TWeakObjectPtr<UOnlineUserInfo>(InUserInfo))
			  , DesiredPrivilege(InPrivilege)
			  , DesiredContext(InContext)
			  , Delegate(MoveTemp(InDelegate))
		{
		}

		TWeakObjectPtr<UOnlineUserInfo> UserInfo; //哪个本地用户正在尝试登录
		EOnlineUserAsyncTaskState OverallLoginState = EOnlineUserAsyncTaskState::NotStarted; //登录请求的整体状态，可能来自多个来源
		EOnlineUserAsyncTaskState TransferPlatformAuthState = EOnlineUserAsyncTaskState::NotStarted; //尝试使用平台认证的状态。启动时，对于 OSSv1 这会立即转变为失败，因为我们在那里不支持平台认证。
		EOnlineUserAsyncTaskState AutoLoginState = EOnlineUserAsyncTaskState::NotStarted; //尝试使用自动登录的状态
		EOnlineUserAsyncTaskState LoginUIState = EOnlineUserAsyncTaskState::NotStarted; //尝试使用外部登录 UI 的状态
		EOnlineUserPrivilege DesiredPrivilege = EOnlineUserPrivilege::Invalid_Count; //最终请求的权限
		EOnlineUserAsyncTaskState PrivilegeCheckState = EOnlineUserAsyncTaskState::NotStarted; //尝试请求相关权限的状态
		EOnlineUserOnlineContext DesiredContext = EOnlineUserOnlineContext::Invalid; //最终要登录的上下文
		EOnlineUserOnlineContext CurrentContext = EOnlineUserOnlineContext::Invalid; //我们当前正在登录的在线系统
		FOnLocalUserLoginCompleteDelegate Delegate; //用户完成回调
		TOptional<FOnlineErrorType> Error; //最近/最相关的错误显示给用户
	};

	UE_API virtual UOnlineUserInfo* CreateLocalUserInfo(int32 LocalPlayerIndex); //创建新的用户信息对象

	FORCEINLINE UOnlineUserInfo* ModifyInfo(const UOnlineUserInfo* Info) { return const_cast<UOnlineUserInfo*>(Info); } //const 获取器的去 const 包装器

	UE_API virtual void RefreshLocalUserInfo(UOnlineUserInfo* UserInfo); //从 OSS 刷新用户信息

	UE_API virtual void HandleChangedAvailability(UOnlineUserInfo* UserInfo, EOnlineUserPrivilege Privilege, EOnlineUserAvailability OldAvailability); //可能发送权限可用性通知，将当前值与缓存的旧值进行比较

	//更新用户上的缓存权限并通知委托
	UE_API virtual void UpdateUserPrivilegeResult(UOnlineUserInfo* UserInfo, EOnlineUserPrivilege Privilege, EOnlineUserPrivilegeResult Result, EOnlineUserOnlineContext Context);

	//获取某种在线系统的内部数据，服务可能返回 null
	UE_API const FOnlineContextCache* GetContextCache(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game) const;
	UE_API FOnlineContextCache* GetContextCache(EOnlineUserOnlineContext Context = EOnlineUserOnlineContext::Game);

	//在绑定委托之前创建并设置系统对象
	UE_API virtual void CreateOnlineContexts();
	UE_API virtual void DestroyOnlineContexts();

	UE_API virtual void BindOnlineDelegates(); //绑定在线委托

	UE_API virtual void LogOutLocalUser(FPlatformUserId PlatformUser); //强制注销并反初始化单个用户

	UE_API virtual void ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request); //执行登录请求的下一步，可能包括完成它。如果完成则返回 true

	//在 OSS 上调用登录，使用来自平台 OSS 的平台认证。如果启动了自动登录则返回 true
	UE_API virtual bool TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	UE_API virtual bool AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser); //在 OSS 上调用自动登录。如果启动了自动登录则返回 true。

	// 在 OSS 上调用 ShowLoginUI。如果启动了 ShowLoginUI 则返回 true。
	UE_API virtual bool ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	// 在 OSS 上调用 QueryUserPrivilege。如果启动了 QueryUserPrivilege 则返回 true。
	UE_API virtual bool QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/*OSS 特定函数*/
#if ONLINEUSER_OSSV1
	UE_API virtual EOnlineUserPrivilege ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const;
	UE_API virtual EUserPrivileges::Type ConvertOSSPrivilege(EOnlineUserPrivilege Privilege) const;
	UE_API virtual EOnlineUserPrivilegeResult ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const;

	UE_API void BindOnlineDelegatesOSSv1();
	UE_API bool AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
#endif

#if !ONLINEUSER_OSSV1
	UE_API virtual EOnlineUserPrivilege ConvertOnlineServicesPrivilege(UE::Online::EUserPrivileges Privilege) const;
	UE_API virtual UE::Online::EUserPrivileges ConvertOnlineServicesPrivilege(EOnlineUserPrivilege Privilege) const;
	UE_API virtual EOnlineUserPrivilegeResult ConvertOnlineServicesPrivilegeResult(UE::Online::EUserPrivileges Privilege, UE::Online::EPrivilegeResults Results) const;

	UE_API void BindOnlineDelegatesOSSv2();
	UE_API void CacheConnectionStatus(EOnlineUserOnlineContext Context);
	UE_API bool TransferPlatformAuthOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool AutoLoginOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API TSharedPtr<UE::Online::FAccountInfo> GetOnlineServiceAccountInfo(UE::Online::IAuthPtr AuthService, FPlatformUserId InUserId) const;
#endif

	/*OSS 函数的回调*/
#if ONLINEUSER_OSSV1
	UE_API virtual void HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId,
	                                                     EOnlineUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& ErrorString,
	                                             EOnlineUserOnlineContext Context);
	UE_API virtual void HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus,
	                                                         EOnlineServerConnectionStatus::Type ConnectionStatus, EOnlineUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error,
	                                          EOnlineUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type UserPrivilege, uint32 PrivilegeResults,
	                                                  EOnlineUserPrivilege RequestedPrivilege,
	                                                  TWeakObjectPtr<UOnlineUserInfo> OnlineUserInfo, EOnlineUserOnlineContext Context);
#endif

#if !ONLINEUSER_OSSV1
	UE_API virtual void HandleAuthLoginStatusChanged(const UE::Online::FAuthLoginStatusChanged& EventParameters, EOnlineUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompletedV2(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosedV2(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result, FPlatformUserId PlatformUser,
	                                            EOnlineUserOnlineContext Context);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const UE::Online::FConnectionStatusChanged& EventParameters, EOnlineUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserPrivilege>& Result, TWeakObjectPtr<UOnlineUserInfo> OnlineUserInfo,
	                                                  UE::Online::EUserPrivileges DesiredPrivilege, EOnlineUserOnlineContext Context);
#endif

	/*当输入设备（例如游戏手柄）连接或断开连接时的回调。*/
	UE_API virtual void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	UE_API virtual void HandleLoginForUserInitialize(const UOnlineUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId,
	                                                 const TOptional<FOnlineErrorType>& InError,
	                                                 EOnlineUserOnlineContext Context, FOnlineUserInitializeParams Params);
	UE_API virtual void HandleUserInitializeFailed(FOnlineUserInitializeParams Params, FText Error);
	UE_API virtual void HandleUserInitializeSucceeded(FOnlineUserInitializeParams Params);

	UE_API virtual bool OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs); //处理按开始/登录逻辑的回调

	FOverrideInputKeyHandler WrappedInputKeyHandler; //之前的覆盖处理程序，取消时将恢复

	TArray<FKey> LoginKeysForAnyUser; //要监听任何用户的按键列表

	TArray<FKey> LoginKeysForNewUser; //要监听新的未映射用户的按键列表

	FOnlineUserInitializeParams ParamsForLoginKey; //用于按键触发登录的参数

	int32 MaxNumberOfLocalPlayers = 0; //本地玩家的最大数量

	bool bIsDedicatedServer = false; //如果这是不需要 LocalPlayer 的专用服务器，则为 true

	TArray<TSharedRef<FUserLoginRequest>> ActiveLoginRequests; //当前进行中的登录请求列表

	UPROPERTY()
	TMap<int32, TObjectPtr<UOnlineUserInfo>> LocalUserInfos; //关于每个本地用户的信息，从本地玩家索引到用户

	FGameplayTagContainer CachedTraitTags; //缓存的平台/模式特性标签

	/*不要在初始化之外访问此项*/
	FOnlineContextCache* DefaultContextInternal = nullptr;
	FOnlineContextCache* ServiceContextInternal = nullptr;
	FOnlineContextCache* PlatformContextInternal = nullptr;

	friend UOnlineUserInfo;
};

#undef UE_API
