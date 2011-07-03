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
								const char* button0Label, const char* button1Label = NULL,
								const char* button2Label = NULL,
								button_width widthStyle = B_WIDTH_AS_USUAL,
								alert_type alert = B_INFO_ALERT);
	virtual	void		MessageReceived(BMessage *msg);
	status_t			Go(BInvoker *invoker);
	int32				GetRememberValue() { return fRememberCB->Value(); }
private:
	BCheckBox			*fRememberCB;
	BInvoker			*fInvoker;
};

#endif
