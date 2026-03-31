// Copyright © 2026 鸿源z. All Rights Reserved.

#include "ModularPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "FModularLoadingScreenForStartupModule"

void SModularPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	/*构建一个空白的Border,并且设置为黑色*/
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0)
	];
}

void SModularPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	//WidgetAssets.AddReferencedObjects(Collector);
}

FString SModularPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SCommonPreLoadingScreenWidget");
}

#undef LOCTEXT_NAMESPACE
