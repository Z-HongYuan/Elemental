// Copyright © 2026 鸿源z. All Rights Reserved.


#include "SPreLoadingScreenWidget.h"

void SPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	//在构造函数中添加一个空的Border,创建一个纯黑色的矩形背景，用于覆盖整个屏幕
	//ChildSlot代表唯一子控件槽位
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0)
	];
}

void SPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	//WidgetAssets.AddReferencedObjects(Collector);这里是添加GC跟踪的引用
}

FString SPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SPreLoadingScreenWidget");
}
