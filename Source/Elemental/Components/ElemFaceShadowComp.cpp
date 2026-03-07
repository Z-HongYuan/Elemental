// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ElemFaceShadowComp.h"


UElemFaceShadowComp::UElemFaceShadowComp() :
	HeadBoneName("Head"),
	FaceForwardBoneName("SDF_F"),
	FaceRightBoneName("SDF_R"),
	FaceForwardOffsetIndex(0),
	FaceRightOffsetIndex(3),
	SkeletalMeshComponentTag("UseFaceShadow")
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UElemFaceShadowComp::BeginPlay()
{
	Super::BeginPlay();
	RefreshMeshComponentRef();
}


void UElemFaceShadowComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ParentMeshComponent.IsValid())
	{
		const FVector Forward = ParentMeshComponent->GetSocketLocation(FaceForwardBoneName);
		const FVector Right = ParentMeshComponent->GetSocketLocation(FaceRightBoneName);
		const FVector Head = ParentMeshComponent->GetSocketLocation(HeadBoneName);

		ParentMeshComponent->SetCustomPrimitiveDataVector3(FaceForwardOffsetIndex, (Forward - Head).GetSafeNormal());
		ParentMeshComponent->SetCustomPrimitiveDataVector3(FaceRightOffsetIndex, (Right - Head).GetSafeNormal());
	}
}

void UElemFaceShadowComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ParentMeshComponent = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UElemFaceShadowComp::RefreshMeshComponentRef()
{
	ParentMeshComponent = Cast<USkeletalMeshComponent>(GetOwner()->FindComponentByTag(USkeletalMeshComponent::StaticClass(), SkeletalMeshComponentTag));
}
