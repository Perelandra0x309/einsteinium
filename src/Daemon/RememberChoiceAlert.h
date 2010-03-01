/*RememberChoiceAlert.h

*/
#ifndef ED_REMEMBERALERT_H
#define ED_REMEMBERALERT_H

#include <InterfaceKit.h>
#include "daemon_constants.h"


class RememberChoiceAlert : public BAlert
{public:
						RememberChoiceAlert(const char* title, const char* text,
								const char* button1, const char* button2 = NULL);
	virtual	void		MessageReceived(BMessage *an_event);
	status_t			Go(BInvoker *invoker);
	int32				GetRememberValue() { return rememberCB->Value(); }
private:
	BCheckBox			*rememberCB;
	BInvoker			*pInvoker;
};

#endif
