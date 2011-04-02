/* EngineSubscriber.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ENGINE_SUBSCRIBER_H
#define ENGINE_SUBSCRIBER_H

#include <Handler.h>
#include <Looper.h>
#include <Roster.h>
#include <stdio.h>
#include "string.h"

//Einsteinium_engine application signature
#define einsteinium_engine_sig "application/x-vnd.Einsteinium_Engine"

//Subscribe message names
#define E_SUBSCRIPTION_MESSENGER "messenger"
#define E_SUBSCRIPTION_UNIQUEID "uniqueID"
#define E_SUBSCRIPTION_COUNT "count"
#define E_SUBSCRIPTION_LAUNCH_SCALE "launch_scale"
#define E_SUBSCRIPTION_FIRST_SCALE "first_scale"
#define E_SUBSCRIPTION_LAST_SCALE "last_scale"
#define E_SUBSCRIPTION_INTERVAL_SCALE "interval_scale"
#define E_SUBSCRIPTION_RUNTIME_SCALE "runtime_scale"

//Messages
enum subscriber_messages
{	E_SUBSCRIBER_UPDATE_RANKED_APPS,
	E_SUBSCRIBE_RANKED_APPS,
	E_UNSUBSCRIBE_RANKED_APPS,
	E_SUBSCRIBE_CONFIRMED,
	E_SUBSCRIBE_FAILED,
	E_UNSUBSCRIBE_CONFIRMED,
	E_UNSUBSCRIBE_FAILED
};

class EngineSubscriber;

class SubscriberHandler : public BHandler {
public:
							SubscriberHandler(EngineSubscriber *owner);
	virtual void			MessageReceived(BMessage *message);

private:
	EngineSubscriber		*fOwner;
};

class EngineSubscriber {
public:
							EngineSubscriber();
							~EngineSubscriber();

protected:
	void					_SubscribeToEngine(int itemCount, int numberOfLaunchesScale, int firstLaunchScale,
							int latestLaunchScale, int lastIntervalScale, int totalRuntimeScale);
	void					_UnsubscribeFromEngine();
	bool					_IsEngineRunning();
	status_t				_LaunchEngine();

	// pure virtual functions- these need to be implemented by your class
	virtual void			_SubscribeFailed() = 0;
	virtual void			_SubscribeConfirmed() = 0;
	virtual void			_UpdateReceived(BMessage *message) = 0;

private:
	int32					fUniqueID;
	BLooper					*fLooper;
	SubscriberHandler		*fHandler;
	void					_ProcessEngineMessage(BMessage *message);

	friend SubscriberHandler;
};

#endif
