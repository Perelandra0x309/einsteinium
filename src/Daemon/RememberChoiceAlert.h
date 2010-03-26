/* RememberChoiceAlert.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ED_REMEMBERALERT_H
#define ED_REMEMBERALERT_H

#include <InterfaceKit.h>
#include "daemon_constants.h"


class RememberChoiceAlert : public BAlert {
public:
						RememberChoiceAlert(const char* title, const char* text,
								const char* button1, const char* button2 = NULL);
	virtual	void		MessageReceived(BMessage *msg);
	status_t			Go(BInvoker *invoker);
	int32				GetRememberValue() { return fRememberCB->Value(); }
private:
	BCheckBox			*fRememberCB;
	BInvoker			*fInvoker;
};

#endif
