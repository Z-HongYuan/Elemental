// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_AsyncAction_ListenForModularMessage.generated.h"

#define UE_API MODULARMESSAGEROUTER_API

/**
 * 专门用于在蓝图中使用的异步节点,本质上是改变 Pin 和外观的 Node
 * 将返回的代理对象换为负载输出
 */
UCLASS()
class UK2Node_AsyncAction_ListenForModularMessage : public UK2Node_AsyncAction
{
	GENERATED_BODY()

public:
	//~UEdGraphNode interface
	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~End of UEdGraphNode interface

	//~UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	//~End of UK2Node interface

protected:
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext) override;

private:
	// 将GetPayload流添加到委托处理程序逻辑链的末尾
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	// 确保输出Payload通配符与输入PayloadType匹配
	void RefreshOutputPayloadType();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
	UEdGraphPin* GetOutputChannelPin() const;
};

#undef UE_API
