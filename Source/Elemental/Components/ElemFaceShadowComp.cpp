// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ElemFaceShadowComp.h"


UElemFaceShadowComp::UElemFaceShadowComp() :
	FaceForwardValueName("SDF_F"),
	FaceRightValueName("SDF_R"),
	FaceForwardBoneName("SDF_F"),
	FaceRightBoneName("SDF_R"),
	HeadBoneName("Head"),
	SkeletalMeshComponentTag("UseFaceShadow")
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(false);
}


void UElemFaceShadowComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//TODO 修改面部曲线状态,在光源在面部侧面时,过渡太快
	if (FaceMaterialInstance and ParentMeshComponent.IsValid())
	{
		const FVector Forward = ParentMeshComponent->GetSocketLocation(FaceForwardBoneName);
		const FVector Right = ParentMeshComponent->GetSocketLocation(FaceRightBoneName);
		const FVector Head = ParentMeshComponent->GetSocketLocation(HeadBoneName);

		FaceMaterialInstance->SetVectorParameterValue(FaceForwardValueName, (Forward - Head).GetSafeNormal());
		FaceMaterialInstance->SetVectorParameterValue(FaceRightValueName, (Right - Head).GetSafeNormal());
	}
}


void UElemFaceShadowComp::BeginPlay()
{
	Super::BeginPlay();

	RefreshFaceMaterialInstance();
}

void UElemFaceShadowComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FaceMaterialInstance != nullptr)
	{
		FaceMaterialInstance = nullptr;
		UE_LOG(LogTemp, Log, TEXT("UElemFaceShadowComp::EndPlay() FaceMaterialInstance is Set nullptr"));
	}
	ParentMeshComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UElemFaceShadowComp::RefreshFaceMaterialInstance()
{
	if (FaceMaterialInstance != nullptr)
	{
		FaceMaterialInstance = nullptr;
		UE_LOG(LogTemp, Log, TEXT("UElemFaceShadowComp::RefreshFaceMaterialInstance() FaceMaterialInstance is Set nullptr"));
	}

	ParentMeshComponent = Cast<USkeletalMeshComponent>(GetOwner()->FindComponentByTag(USkeletalMeshComponent::StaticClass(), SkeletalMeshComponentTag));

	if (ParentMeshComponent.IsValid())
	{
		//父类没有材质可供创建动态的时候,会崩溃
		FaceMaterialInstance = ParentMeshComponent->CreateDynamicMaterialInstance(FaceID, ParentMeshComponent->GetMaterial(FaceID), "FaceShadowMI");
		//log
		UE_LOG(LogTemp, Log, TEXT("UElemFaceShadowComp::RefreshFaceMaterialInstance() FaceMaterialInstance is Created"));
	}
}
