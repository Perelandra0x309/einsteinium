/* main.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "einsteinium_engine.h"


int main()
{
	einsteinium_engine myApp;//create application
	thread_info info;
	int32 cookie=0;
	while(get_next_thread_info(0, &cookie, &info) == B_OK){
		set_thread_priority(info.thread, B_LOW_PRIORITY);
	}
	myApp.Run();//run application
	return 0;
}
