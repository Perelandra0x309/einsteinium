/* RememberChoiceAlert.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "RememberChoiceAlert.h"

RememberChoiceAlert::RememberChoiceAlert(const char* title, const char* text,
								const char* button0Label, const char* button1Label = NULL,
								const char* button2Label = NULL,
								button_width widthStyle = B_WIDTH_AS_USUAL,
								alert_type type = B_INFO_ALERT)
	:
	BAlert(title, text, button0Label, button1Label, button2Label, widthStyle, type)
{
	fInvoker = NULL;
//	SetFlags(B_NOT_CLOSABLE | /*B_NOT_RESIZABLE | */B_ASYNCHRONOUS_CONTROLS);
	fRememberCB = new BCheckBox("Save Settings", "Always use the following choice:", NULL);
	// Add the checkbox to the BAlert view
	BView *view = ChildAt(0);
	view->AddChild(fRememberCB);
	// Find the first button and the text view
	BButton *buttonView = ButtonAt(0);
	BTextView *textView = TextView();
	// Move the checkbox to the correct spot and resize the window
	fRememberCB->MoveTo(textView->Frame().left, buttonView->Frame().top);
	fRememberCB->ResizeToPreferred();
	ResizeBy(0, 10 + fRememberCB->Frame().Height());
}


void
RememberChoiceAlert::MessageReceived(BMessage* msg)
{
	// Insert the check box value into the message
	if (msg->what == 'ALTB')
	{
		if (fInvoker)
		{
			BMessage* out = fInvoker->Message();
			bool value = fRememberCB->Value()? true : false;
			out->AddBool(ED_ALERT_REMEMBER, value);
		}
	}
	BAlert::MessageReceived(msg);
}


status_t
RememberChoiceAlert::Go(BInvoker *invoker)
{
	fInvoker = invoker;
	return BAlert::Go(invoker);
}
