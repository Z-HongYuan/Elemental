// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once


/**
 * 这是一个预加载界面,是实际Widget所在
 * 使用静态构造函数构造一个空的Border，并设置其背景为纯黑色
 * 使用Slate复合控件基类
 */
class SPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SPreLoadingScreenWidget)
		{
		}

	SLATE_END_ARGS()

	//Slate的构造函数
	void Construct(const FArguments& InArgs);

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject interface

private:
};
