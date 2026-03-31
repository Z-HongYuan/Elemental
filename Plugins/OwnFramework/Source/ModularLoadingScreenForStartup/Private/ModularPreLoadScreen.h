// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once
#include "PreLoadScreenBase.h"


/* 用于显示控件的一个预加载屏幕处理器 */
class FModularPreLoadScreen : public FPreLoadScreenBase
{
public:
	/*** IPreLoadScreen Implementation ***/
	virtual void Init() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
	virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }

private:
	TSharedPtr<SWidget> EngineLoadingWidget;
};
