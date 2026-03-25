// Copyright © 2026 鸿源z. All Rights Reserved.


#include "OnlineUserTypes.h"

/*导入UBT生成的cpp文件*/
#include UE_INLINE_GENERATED_CPP_BY_NAME(OnlineUserTypes)

/*根据版本条件编译*/
#if ONLINEUSER_OSSV1
#include "OnlineError.h"
#else
#include "Online/OnlineErrorDefinitions.h"
#endif

/*通过不同底层API来构建项目自己的Error*/
void FOnlineResultInformation::FromOnlineError(const FOnlineErrorType& InOnlineError)
{
#if ONLINEUSER_OSSV1
	bWasSuccessful = InOnlineError.WasSuccessful(); //操作是否成功
	ErrorId = InOnlineError.GetErrorCode(); //获取错误代码
	ErrorText = InOnlineError.GetErrorMessage(); //错误描述文本
#else
	bWasSuccessful = InOnlineError != UE::Online::Errors::Success(); //与成功状态比较来判断是否失败
	ErrorId = InOnlineError.GetErrorId(); //获取错误 ID（FString）
	ErrorText = InOnlineError.GetText(); //获取错误文本（FText）
#endif
}
