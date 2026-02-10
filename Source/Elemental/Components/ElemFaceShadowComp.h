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
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ELEMENTAL_API UElemFaceShadowComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UElemFaceShadowComp();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> ParentMeshComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FaceMaterialInstance;

private:

public:
	//脸部材质ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	int FaceID;

	//需要变化的材质属性名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FName FaceForwardValueName;

	//需要变化的材质属性名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FName FaceRightValueName;

	//骨骼插槽名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FName FaceForwardBoneName;

	//骨骼插槽名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FName FaceRightBoneName;

	//骨骼插槽名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FName HeadBoneName;

	//骨骼网格体组件拥有的Tag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FName SkeletalMeshComponentTag;

	//刷新面部材质实例,在更换角色的时候非常有用
	UFUNCTION(BlueprintCallable, Category = "SDF")
	void RefreshFaceMaterialInstance();
};
