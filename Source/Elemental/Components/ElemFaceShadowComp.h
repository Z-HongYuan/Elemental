// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ElemFaceShadowComp.generated.h"

/*
 * 控制角色脸部SDF阴影的插件
 * 通过获取网格体中的位置信息,设置材质参数值
 * 需要设置全部的FName参数
 * 将在BeginPlay()中调用一次RefreshFaceMaterialInstance(),如果角色更换了模型,请调用此函数以刷新材质实例
 * 
 * 改用了自定义基元数据
 * 设定Mesh上的基元数据
 * 还是需要向网格体添加Tag用于识别
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ELEMENTAL_API UElemFaceShadowComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UElemFaceShadowComp();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> ParentMeshComponent;

public:
	//骨骼插槽名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="头部命名"))
	FName HeadBoneName;
	//骨骼插槽名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="SDF_F命名"))
	FName FaceForwardBoneName;
	//骨骼插槽名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="SDF_R命名"))
	FName FaceRightBoneName;

	//自定义基元数据偏移索引 SDF_F,前方参数的索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="SDF_F参数索引"))
	int32 FaceForwardOffsetIndex;
	//自定义基元数据偏移索引 SDF_R,右方参数的索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="SDF_R参数索引"))
	int32 FaceRightOffsetIndex;

	//骨骼网格体组件拥有的Tag,方便检索组件以获取引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="网格体所需Tag"))
	FName SkeletalMeshComponentTag;

	//更新Mesh引用
	UFUNCTION(BlueprintCallable, Category = "SDF")
	void RefreshMeshComponentRef();
};
