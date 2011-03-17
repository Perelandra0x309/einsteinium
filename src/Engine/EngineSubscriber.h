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

#include "engine_constants.h"

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
