// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "PreLoadScreenBase.h"

/*
 * 这个模块是StartUpLoadingScreen模块
 * 其中实现了Engine未完全初始化时显示的加载界面
 * 目前只有一个黑屏界面
 * 后期可能会添加其他功能,例如在开始的时候播放一段开始动画
 */
class FCommonPreLoadScreen : public FPreLoadScreenBase
{
public:
	/*** IPreLoadScreen 实现 ***/
	//负责创建和设置加载界面组件
	virtual void Init() override;

	//标识这是一个引擎加载屏幕
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }

	//返回实际的 Slate 控件指针
	virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }

private:
	TSharedPtr<SWidget> EngineLoadingWidget;
};
