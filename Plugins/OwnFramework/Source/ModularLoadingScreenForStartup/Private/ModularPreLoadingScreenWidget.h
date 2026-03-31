// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

/*用于启动界面的控件*/
class SModularPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SModularPreLoadingScreenWidget)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject interface
};
