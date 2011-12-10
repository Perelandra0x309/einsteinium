/* EngineSubscriber.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EngineSubscriber.h"


SubscriberHandler::SubscriberHandler(EngineSubscriber *owner)
	:
	BHandler("ESM_Handler")
{
	fOwner = owner;
}

void
SubscriberHandler::MessageReceived(BMessage *message)
{
	//printf("ESM_Handler received message\n");
	fOwner->_ProcessEngineMessage(message);
}


EngineSubscriber::EngineSubscriber()
{
	fUniqueID = time(NULL);
	_ResetSubscriberValues();

	//Start the Looper running
	fHandler = new SubscriberHandler(this);
	fLooper = new BLooper("ESM_Looper");
	fLooper->AddHandler(fHandler);
	fLooper->Run();
}


EngineSubscriber::~EngineSubscriber()
{
	fLooper->Lock();
	fLooper->Quit();
}


void
EngineSubscriber::_ResetSubscriberValues()
{
	fCount=10;
	fLaunchesScale=0;
	fFirstLaunchScale=0;
	fLastLaunchScale=0;
	fLastIntervalScale=0;
	fTotalRuntimeScale=0;
	fExcludeList.MakeEmpty();
}


void
EngineSubscriber::_SetCount(int count)
{
	fCount = count;
}

void
EngineSubscriber::_SetTotalLaunchesScale(int value)
{
	fLaunchesScale = value;
}


void
EngineSubscriber::_SetFirstLaunchScale(int value)
{
	fFirstLaunchScale = value;
}


void
EngineSubscriber::_SetLastLaunchScale(int value)
{
	fLastLaunchScale = value;
}


void
EngineSubscriber::_SetLastIntervalScale(int value)
{
	fLastIntervalScale = value;
}


void
EngineSubscriber::_SetTotalRuntimeScale(int value)
{
	fTotalRuntimeScale = value;
}


void
EngineSubscriber::_SetExcludeList(BMessage *list = NULL)
{
	fExcludeList.MakeEmpty();
	if(list)
	{
		type_code typeFound;
		int32 countFound;
		status_t exStatus = list->GetInfo(E_SUBSCRIPTION_EXCLUSIONS, &typeFound, &countFound);
		if(exStatus==B_OK)
		{
			BString appSig;
			for(int i=0; i<countFound; i++)
			{
				list->FindString(E_SUBSCRIPTION_EXCLUSIONS, i, &appSig);
				fExcludeList.AddString(E_SUBSCRIPTION_EXCLUSIONS, appSig);
			}
		}
	}
}


void
EngineSubscriber::_AddExclusion(BString signature)
{
	fExcludeList.AddString(E_SUBSCRIPTION_EXCLUSIONS, signature);
}


void
EngineSubscriber::_SubscribeToEngine()
{
	// Subscribe with the Einsteinium Engine to receive updates
	BMessage subscribeMsg(E_SUBSCRIBE_RANKED_APPS);
	subscribeMsg.AddInt32(E_SUBSCRIPTION_UNIQUEID, fUniqueID);
	subscribeMsg.AddInt16(E_SUBSCRIPTION_COUNT, fCount);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_LAUNCH_SCALE, fLaunchesScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_FIRST_SCALE, fFirstLaunchScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_LAST_SCALE, fLastLaunchScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_INTERVAL_SCALE, fLastIntervalScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_RUNTIME_SCALE, fTotalRuntimeScale);

	type_code typeFound;
	int32 countFound;
	status_t exStatus = fExcludeList.GetInfo(E_SUBSCRIPTION_EXCLUSIONS, &typeFound, &countFound);
	if(exStatus==B_OK)
	{
		BString appSig;
		for(int i=0; i<countFound; i++)
		{
			fExcludeList.FindString(E_SUBSCRIPTION_EXCLUSIONS, i, &appSig);
			subscribeMsg.AddString(E_SUBSCRIPTION_EXCLUSIONS, appSig);
		}
	}

	status_t mErr;
	BMessenger messenger(fHandler, NULL, &mErr);
	if(!messenger.IsValid())
	{
		printf("ESM: Messenger is not valid, error=%s\n", strerror(mErr));
		return;
	}
	subscribeMsg.AddMessenger(E_SUBSCRIPTION_MESSENGER, messenger);
	BMessenger EsMessenger(einsteinium_engine_sig);
	EsMessenger.SendMessage(&subscribeMsg, fHandler);
	// TODO trying to get reply synchronously freezes
	//	BMessage reply;
	//	EsMessenger.SendMessage(&subscribeMsg, &reply);
}


void
EngineSubscriber::_UnsubscribeFromEngine()
{
	// Unsubscribe from the Einsteinium Engine
	if (be_roster->IsRunning(einsteinium_engine_sig))
	{
		BMessage unsubscribeMsg(E_UNSUBSCRIBE_RANKED_APPS);
		unsubscribeMsg.AddInt32(E_SUBSCRIPTION_UNIQUEID, fUniqueID);
		BMessenger EsMessenger(einsteinium_engine_sig);
		EsMessenger.SendMessage(&unsubscribeMsg, fHandler);
	}
}


bool
EngineSubscriber::_IsEngineRunning()
{
	return be_roster->IsRunning(einsteinium_engine_sig);
}


status_t
EngineSubscriber::_LaunchEngine()
{
	return be_roster->Launch(einsteinium_engine_sig);
}


status_t
EngineSubscriber::_QuitEngine()
{
	status_t rc = B_ERROR;
	BMessenger appMessenger(einsteinium_engine_sig, -1, &rc);
	if(!appMessenger.IsValid())
		return rc;
	rc = appMessenger.SendMessage(B_QUIT_REQUESTED);
	return rc;
}


void
EngineSubscriber::_ProcessEngineMessage(BMessage *message)
{
	switch(message->what)
	{
		case E_SUBSCRIBE_FAILED: {
		//	printf("ESM_Handler received \'Subscribe Failed\' message\n");
			_SubscribeFailed();
			break;
		}
		case E_SUBSCRIBE_CONFIRMED: {
		//	printf("ESM_Handler received \'Subscribe Confirmed\' message\n");
			_SubscribeConfirmed();
			break;
		}
		case E_SUBSCRIBER_UPDATE_RANKED_APPS: {
		//	printf("ESM_Handler received \'Update Ranked Apps\' message\n");
			_UpdateReceived(message);
			break;
		}
	}
}

