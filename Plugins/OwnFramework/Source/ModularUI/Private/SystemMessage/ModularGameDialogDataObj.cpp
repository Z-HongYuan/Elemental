// Copyright © 2026 鸿源z. All Rights Reserved.


#include "SystemMessage/ModularGameDialogDataObj.h"


UModularGameDialogDataObj* UModularGameDialogDataObj::CreateConfirmationOk(const FText& Title, const FText& Content)
{
	UModularGameDialogDataObj* Descriptor = NewObject<UModularGameDialogDataObj>();
	Descriptor->Title = Title;
	Descriptor->Content = Content;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("Messaging", "Ok", "好的");

	Descriptor->ButtonActions.Add(ConfirmAction);

	return Descriptor;
}

UModularGameDialogDataObj* UModularGameDialogDataObj::CreateConfirmationOkCancel(const FText& Title, const FText& Content)
{
	UModularGameDialogDataObj* Descriptor = NewObject<UModularGameDialogDataObj>();
	Descriptor->Title = Title;
	Descriptor->Content = Content;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("Messaging", "Ok", "好的");

	FConfirmationDialogAction CancelAction;
	CancelAction.Result = EModularMessagingResult::Cancelled;
	CancelAction.OptionalDisplayText = NSLOCTEXT("Messaging", "Cancel", "取消");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(CancelAction);

	return Descriptor;
}

UModularGameDialogDataObj* UModularGameDialogDataObj::CreateConfirmationYesNo(const FText& Title, const FText& Content)
{
	UModularGameDialogDataObj* Descriptor = NewObject<UModularGameDialogDataObj>();
	Descriptor->Title = Title;
	Descriptor->Content = Content;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("Messaging", "Yes", "是");

	FConfirmationDialogAction DeclineAction;
	DeclineAction.Result = EModularMessagingResult::Declined;
	DeclineAction.OptionalDisplayText = NSLOCTEXT("Messaging", "No", "不");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(DeclineAction);

	return Descriptor;
}

UModularGameDialogDataObj* UModularGameDialogDataObj::CreateConfirmationYesNoCancel(const FText& Title, const FText& Content)
{
	UModularGameDialogDataObj* Descriptor = NewObject<UModularGameDialogDataObj>();
	Descriptor->Title = Title;
	Descriptor->Content = Content;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("Messaging", "Yes", "是");

	FConfirmationDialogAction DeclineAction;
	DeclineAction.Result = EModularMessagingResult::Declined;
	DeclineAction.OptionalDisplayText = NSLOCTEXT("Messaging", "No", "不");

	FConfirmationDialogAction CancelAction;
	CancelAction.Result = EModularMessagingResult::Cancelled;
	CancelAction.OptionalDisplayText = NSLOCTEXT("Messaging", "Cancel", "取消");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(DeclineAction);
	Descriptor->ButtonActions.Add(CancelAction);

	return Descriptor;
}
