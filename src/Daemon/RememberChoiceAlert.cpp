/*RememberChoiceAlert.cpp

*/
#include "RememberChoiceAlert.h"

RememberChoiceAlert::RememberChoiceAlert(const char* title, const char* text,
								const char* button1, const char* button2 = NULL)
	:BAlert(title, text, button1, button2)
{
	pInvoker = NULL;
//	SetFlags(B_NOT_CLOSABLE | /*B_NOT_RESIZABLE | */B_ASYNCHRONOUS_CONTROLS);
	rememberCB = new BCheckBox("Save Settings", "Always use the following choice:", NULL);
	// Add the checkbox to the BAlert view
	BView *view = ChildAt(0);
	view->AddChild(rememberCB);
	// Find the first button and the text view
	BButton *buttonView = ButtonAt(0);
	BTextView *textView = TextView();
	// Move the checkbox to the correct spot and resize the window
	rememberCB->MoveTo(textView->Frame().left, buttonView->Frame().top);
	rememberCB->ResizeToPreferred();
	ResizeBy(0, 10 + rememberCB->Frame().Height());
}

void RememberChoiceAlert::MessageReceived(BMessage* msg)
{
	// Insert the check box value into the message
	if (msg->what == 'ALTB')
	{
		if (pInvoker)
		{
			BMessage* out = pInvoker->Message();
			bool value = rememberCB->Value()? true : false;
			out->AddBool(ED_ALERT_REMEMBER, value);
		}
	}
	BAlert::MessageReceived(msg);
}

status_t RememberChoiceAlert::Go(BInvoker *invoker)
{
	pInvoker = invoker;
	return BAlert::Go(invoker);
}
