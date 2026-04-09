// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Sessions/ModularSession_SearchSessionRequest.h"

void UModularSession_SearchSessionRequest::NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage)
{
	OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
	K2_OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
}
