// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Message/ModularGameDialogDescriptor.h"

#define LOCTEXT_NAMESPACE "Messaging"

UModularGameDialogDescriptor* UModularGameDialogDescriptor::CreateConfirmationOk(const FText& Header, const FText& Body)
{
	UModularGameDialogDescriptor* Descriptor = NewObject<UModularGameDialogDescriptor>();
	Descriptor->Header = Header;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = LOCTEXT("Ok", "Ok");

	Descriptor->ButtonActions.Add(ConfirmAction);

	return Descriptor;
}

UModularGameDialogDescriptor* UModularGameDialogDescriptor::CreateConfirmationOkCancel(const FText& Header, const FText& Body)
{
	UModularGameDialogDescriptor* Descriptor = NewObject<UModularGameDialogDescriptor>();
	Descriptor->Header = Header;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = LOCTEXT("Ok", "Ok");

	FConfirmationDialogAction CancelAction;
	CancelAction.Result = EModularMessagingResult::Cancelled;
	CancelAction.OptionalDisplayText = LOCTEXT("Cancel", "Cancel");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(CancelAction);

	return Descriptor;
}

UModularGameDialogDescriptor* UModularGameDialogDescriptor::CreateConfirmationYesNo(const FText& Header, const FText& Body)
{
	UModularGameDialogDescriptor* Descriptor = NewObject<UModularGameDialogDescriptor>();
	Descriptor->Header = Header;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = LOCTEXT("Yes", "Yes");

	FConfirmationDialogAction DeclineAction;
	DeclineAction.Result = EModularMessagingResult::Declined;
	DeclineAction.OptionalDisplayText = LOCTEXT("No", "No");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(DeclineAction);

	return Descriptor;
}

UModularGameDialogDescriptor* UModularGameDialogDescriptor::CreateConfirmationYesNoCancel(const FText& Header, const FText& Body)
{
	UModularGameDialogDescriptor* Descriptor = NewObject<UModularGameDialogDescriptor>();
	Descriptor->Header = Header;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.Result = EModularMessagingResult::Confirmed;
	ConfirmAction.OptionalDisplayText = LOCTEXT("Yes", "Yes");

	FConfirmationDialogAction DeclineAction;
	DeclineAction.Result = EModularMessagingResult::Declined;
	DeclineAction.OptionalDisplayText = LOCTEXT("No", "No");

	FConfirmationDialogAction CancelAction;
	CancelAction.Result = EModularMessagingResult::Cancelled;
	CancelAction.OptionalDisplayText = LOCTEXT("Cancel", "Cancel");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(DeclineAction);
	Descriptor->ButtonActions.Add(CancelAction);

	return Descriptor;
}

UModularGameDialog::UModularGameDialog()
{
}

void UModularGameDialog::SetupDialog(UModularGameDialogDescriptor* Descriptor, FModularMessagingResultDelegate ResultCallback)
{
}

void UModularGameDialog::KillDialog()
{
}

#undef LOCTEXT_NAMESPACE
