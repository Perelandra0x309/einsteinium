/* SubscriberExample.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <AppKit.h>
#include "EngineSubscriber.h"

class SubscriberApp : public BApplication, EngineSubscriber {
public:
					SubscriberApp();
					~SubscriberApp();
	virtual void	ArgvReceived(int32, char**);
	virtual void	ReadyToRun();
private:
	// virtual functions inherited from the EngineSubscriber class
	virtual void	_SubscribeFailed();
	virtual void	_SubscribeConfirmed();
	virtual void	_UpdateReceived(BMessage *message);
};

SubscriberApp::SubscriberApp()
: BApplication("application/x-vnd.Einsteinium_Subscriber_Example")
{ }

SubscriberApp::~SubscriberApp()
{
	printf("Unsubscribing from Engine...\n");
	_UnsubscribeFromEngine();
	printf("Quitting Einsteinium Subscriber Example.\n");
}

void SubscriberApp::ArgvReceived(int32 argc, char** argv)
{
	if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)//option to quit
		PostMessage(B_QUIT_REQUESTED);
}

void SubscriberApp::ReadyToRun()
{
	printf("Example of the Einsteinium Engine Subscriber\n");
	// Launch the Engine if it is not running
	if(!_IsEngineRunning())
		_LaunchEngine();

	// Subscribe to the Einsteinium Engine- simulate the Recent Applications list
	printf("Subscribing to Engine...\n");
	_ResetSubscriberValues();
	_SetCount(20);
	_SetTotalLaunchesScale(0);
	_SetFirstLaunchScale(0);
	_SetLastLaunchScale(1);
	_SetLastIntervalScale(0);
	_SetTotalRuntimeScale(0);
	_AddExclusion("application/x-vnd.Be-TSKB");
	_SubscribeToEngine();
}

void SubscriberApp::_SubscribeFailed()
{
	printf("Subscribe Failed.\n");
}

void SubscriberApp::_SubscribeConfirmed()
{
	printf("Subscribe Confirmed.\n");
}

void SubscriberApp::_UpdateReceived(BMessage *message)
{
	// Print the updated applications list
	int32 fSubscriptionRefCount = 0;
	if(message)
	{
		type_code typeFound;
		message->GetInfo("refs", &typeFound, &fSubscriptionRefCount);
		printf("Updated application list:\n");
		entry_ref newref;
		for(int i=0; i<fSubscriptionRefCount; i++)
		{
			message->FindRef("refs", i, &newref);
			printf("%i- %s\n", i+1, newref.name);
		}
	}
}

int main()
{
	SubscriberApp myApp;
	myApp.Run();

	return 0;
}
