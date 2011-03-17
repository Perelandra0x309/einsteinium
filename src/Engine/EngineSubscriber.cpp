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
	printf("ESM_Handler received message\n");
	fOwner->_ProcessEngineMessage(message);
}


EngineSubscriber::EngineSubscriber()
{
	fUniqueID = time(NULL);

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
EngineSubscriber::_SubscribeToEngine(int itemCount, int numberOfLaunchesScale, int firstLaunchScale,
							int latestLaunchScale, int lastIntervalScale, int totalRuntimeScale)
{
	// Subscribe with the Einsteinium Engine to receive updates
	BMessage subscribeMsg(E_SUBSCRIBE_RANKED_APPS);
	subscribeMsg.AddInt32("uniqueID", fUniqueID);
	subscribeMsg.AddInt16("count", itemCount);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_LAUNCH_SCALE, numberOfLaunchesScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_FIRST_SCALE, firstLaunchScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_LAST_SCALE, latestLaunchScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_INTERVAL_SCALE, lastIntervalScale);
	subscribeMsg.AddInt8(E_SUBSCRIPTION_RUNTIME_SCALE, totalRuntimeScale);
	status_t mErr;
	BMessenger messenger(fHandler, NULL, &mErr);
	if(!messenger.IsValid())
	{
		printf("ESM: Messenger is not valid, error=%i\n", mErr);
		return;
	}
	subscribeMsg.AddMessenger("messenger", messenger);
	BMessenger EsMessenger(e_engine_sig);
	EsMessenger.SendMessage(&subscribeMsg, fHandler);
	// TODO trying to get reply synchronously freezes
	//	BMessage reply;
	//	EsMessenger.SendMessage(&subscribeMsg, &reply);
}


void
EngineSubscriber::_UnsubscribeFromEngine()
{
	// Unsubscribe from the Einsteinium Engine
	if (be_roster->IsRunning(e_engine_sig))
	{
		BMessage unsubscribeMsg(E_UNSUBSCRIBE_RANKED_APPS);
		unsubscribeMsg.AddInt32("uniqueID", fUniqueID);
		BMessenger EsMessenger(e_engine_sig);
		EsMessenger.SendMessage(&unsubscribeMsg, fHandler);
	}
}


void
EngineSubscriber::_ProcessEngineMessage(BMessage *message)
{
	switch(message->what)
	{
		case E_SUBSCRIBE_FAILED: {
			printf("ESM_Handler received \'Subscribe Failed\' message\n");
			_SubscribeFailed();
			break;
		}
		case E_SUBSCRIBE_CONFIRMED: {
			printf("ESM_Handler received \'Subscribe Confirmed\' message\n");
			_SubscribeConfirmed();
			break;
		}
		case E_SUBSCRIBER_UPDATE_RANKED_APPS: {
			printf("ESM_Handler received \'Update Ranked Apps\' message\n");
			_UpdateReceived(message);
			break;
		}
		// TODO Add a message for when the engine quits
	}
}

