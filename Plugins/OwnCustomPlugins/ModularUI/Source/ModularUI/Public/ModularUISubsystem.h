// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "ModularUISubsystem.generated.h"

#define UE_API MODULARUI_API

struct FUIExtensionRequest;
class UModularUISubsystem;

// UI拓展点的匹配规则
UENUM(BlueprintType)
enum class EUIExtensionPointMatch : uint8
{
	// 完全匹配只会接收包含完全相同点的扩展
	// （例如，注册“A.B”会匹配 A.B 的广播，但不会匹配 A.B.C）
	ExactMatch,

	// 部分匹配将接收所有源自同一点的扩展
	// （例如，注册“A.B”将匹配 A.B 以及 A.B.C 的广播）
	PartialMatch
};

// UI拓展点的匹配规则
UENUM(BlueprintType)
enum class EUIExtensionAction : uint8
{
	Added,
	Removed
};

DECLARE_DELEGATE_TwoParams(FExtendExtensionPointDelegate, EUIExtensionAction Action, const FUIExtensionRequest& Request);

/*
 *
 */
struct FUIExtension : TSharedFromThis<FUIExtension>
{
public:
	/** 此扩展程序的目标扩展点。 */
	FGameplayTag ExtensionPointTag;
	int32 Priority = INDEX_NONE;
	TWeakObjectPtr<UObject> ContextObject;
	//由 UModularUISubsystem::AddReferencedObjects 保持存活
	TObjectPtr<UObject> Data = nullptr;
};

/**
 * 
 */
struct FUIExtensionPoint : TSharedFromThis<FUIExtensionPoint>
{
public:
	FGameplayTag ExtensionPointTag;
	TWeakObjectPtr<UObject> ContextObject;
	EUIExtensionPointMatch ExtensionPointTagMatchType = EUIExtensionPointMatch::ExactMatch;
	TArray<TObjectPtr<UClass>> AllowedDataClasses;
	FExtendExtensionPointDelegate Callback;

	// 测试扩展和扩展点是否匹配，如果匹配，则此扩展点应该了解
	// 关于此扩展的信息。
	bool DoesExtensionPassContract(const FUIExtension* Extension) const;
};

USTRUCT(BlueprintType)
struct FUIExtensionPointHandle
{
	GENERATED_BODY()

public:
	FUIExtensionPointHandle()
	{
	}

	UE_API void Unregister();

	bool IsValid() const { return DataPtr.IsValid(); }

	bool operator==(const FUIExtensionPointHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FUIExtensionPointHandle& Other) const { return !operator==(Other); }

	friend uint32 GetTypeHash(const FUIExtensionPointHandle& Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	}

private:
	TWeakObjectPtr<UModularUISubsystem> ExtensionSource;

	TSharedPtr<FUIExtensionPoint> DataPtr;

	friend UModularUISubsystem;

	FUIExtensionPointHandle(UModularUISubsystem* InExtensionSource, const TSharedPtr<FUIExtensionPoint>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr)
	{
	}
};

template <>
struct TStructOpsTypeTraits<FUIExtensionPointHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionPointHandle>
{
	enum
	{
		WithCopy = true, // 这确保不透明类型在蓝图中正确复制。
		WithIdenticalViaEquality = true,
	};
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FUIExtensionHandle
{
	GENERATED_BODY()

public:
	FUIExtensionHandle()
	{
	}

	UE_API void Unregister();

	bool IsValid() const { return DataPtr.IsValid(); }

	bool operator==(const FUIExtensionHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FUIExtensionHandle& Other) const { return !operator==(Other); }

	friend FORCEINLINE uint32 GetTypeHash(FUIExtensionHandle Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	}

private:
	TWeakObjectPtr<UModularUISubsystem> ExtensionSource;

	TSharedPtr<FUIExtension> DataPtr;

	friend UModularUISubsystem;

	FUIExtensionHandle(UModularUISubsystem* InExtensionSource, const TSharedPtr<FUIExtension>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr)
	{
	}
};

template <>
struct TStructOpsTypeTraits<FUIExtensionHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionHandle>
{
	enum
	{
		WithCopy = true, // 这确保不透明类型在蓝图中正确复制。
		WithIdenticalViaEquality = true,
	};
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FUIExtensionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FUIExtensionHandle ExtensionHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ExtensionPointTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> Data = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> ContextObject = nullptr;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FExtendExtensionPointDynamicDelegate, EUIExtensionAction, Action, const FUIExtensionRequest&, ExtensionRequest);


/**
 * 
 */
UCLASS(MinimalAPI)
class UModularUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API FUIExtensionPointHandle RegisterExtensionPoint(const FGameplayTag& ExtensionPointTag, EUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDelegate ExtensionCallback);
	UE_API FUIExtensionPointHandle RegisterExtensionPointForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, EUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses,
	                                                                FExtendExtensionPointDelegate ExtensionCallback);

	UE_API FUIExtensionHandle RegisterExtensionAsWidget(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	UE_API FUIExtensionHandle RegisterExtensionAsWidgetForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	UE_API FUIExtensionHandle RegisterExtensionAsData(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	UE_API void UnregisterExtension(const FUIExtensionHandle& ExtensionHandle);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	UE_API void UnregisterExtensionPoint(const FUIExtensionPointHandle& ExtensionPointHandle);

	static UE_API void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;

	UE_API void NotifyExtensionPointOfExtensions(TSharedPtr<FUIExtensionPoint>& ExtensionPoint);
	UE_API void NotifyExtensionPointsOfExtension(EUIExtensionAction Action, TSharedPtr<FUIExtension>& Extension);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension Point"))
	UE_API FUIExtensionPointHandle K2_RegisterExtensionPoint(FGameplayTag ExtensionPointTag, EUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses,
	                                                         FExtendExtensionPointDynamicDelegate ExtensionCallback);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority = -1);

	/**
	 * 将小部件（作为数据）注册到特定玩家。这意味着扩展点将收到一个 UIExtensionForPlayer 数据对象。
	 * 它们可以查看该对象以确定它是否属于它们所认定的玩家。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget For Context)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority = -1);

	/**
	 * 将该扩展程序注册为任何可以使用它的扩展点的数据。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority = -1);

	/**
	 *将该扩展程序注册为任何可以使用它的扩展点的数据。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data For Context)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority = -1);

	UE_API FUIExtensionRequest CreateExtensionRequest(const TSharedPtr<FUIExtension>& Extension);

private:
	typedef TArray<TSharedPtr<FUIExtensionPoint>> FExtensionPointList;
	TMap<FGameplayTag, FExtensionPointList> ExtensionPointMap;

	typedef TArray<TSharedPtr<FUIExtension>> FExtensionList;
	TMap<FGameplayTag, FExtensionList> ExtensionMap;
};
#undef UE_API
