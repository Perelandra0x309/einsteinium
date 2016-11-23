/* SystemStatusBox.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "SystemStatusBox.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Status box"

SystemStatusBox::SystemStatusBox(const char *label, BEntry entry, const char * sig)
	:
	BBox(sig),
	fCurrentState(STATE_NONE),
	fStatusText(B_TRANSLATE_COMMENT("Status:", "Status text")),
	fRestartLabel(B_TRANSLATE_COMMENT("Restart", "Button label")),
	fStartLabel(B_TRANSLATE_COMMENT("Start", "Button label")),
	fStopLabel(B_TRANSLATE_COMMENT("Stop", "Button label")),
	fStartingErrorText(B_TRANSLATE_COMMENT("There was an error starting the service", "Status error message")),
	fStoppingErrorText(B_TRANSLATE_COMMENT("There was an error stopping the service", "Status error message")),
	fStatusUnknownText(B_TRANSLATE_COMMENT("Unknown", "Status 'unknown' text")),
	fStatusRunningText(B_TRANSLATE_COMMENT("Running", "Status 'Running' text")),
	fStatusStoppingText(B_TRANSLATE_COMMENT("Stopping...", "Status 'Stopping...' text")),
	fStatusStoppedText(B_TRANSLATE_COMMENT("Stopped", "Status 'Stopped' text")),
	fStatusStartingText(B_TRANSLATE_COMMENT("Starting...", "Status 'Starting...' text")),
	fStatusRestartingText(B_TRANSLATE_COMMENT("Restarting...", "Status 'Restarting...' text"))
{
	BPath path;
	entry.GetPath(&path);
	fName.SetTo(path.Leaf());
	SetLabel(label);
	fSignature.SetTo(sig);
	fStatusSV = new BStringView("Status String", fStatusText);
	fRestartB = new BButton(fRestartLabel, new BMessage(RESTART_SERVICE));
	fRestartB->SetEnabled(false);
	fStartstopB = new BButton(fStopLabel);
	fStartstopB->SetEnabled(false);
	
	BGroupLayout *layout = new BGroupLayout(B_HORIZONTAL, 10);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fStatusSV)
		.AddGlue()
		.Add(fRestartB)
		.Add(fStartstopB)
		.SetInsets(10, 20, 10, 10)
	;
}


/*SystemStatusBox::~SystemStatusBox()
{
}*/


void
SystemStatusBox::AttachedToWindow()
{
	BBox::AttachedToWindow();
	fRestartB->SetTarget(this);
	fStartstopB->SetTarget(this);
	fUsedWidth = fRestartB->PreferredSize().width + fStartstopB->PreferredSize().width
				+ (2 * 10) // space between each control
				+ (2 * 5) // insets
				+ 100 + 4*BORDER_SIZE // left selection view size
				+ 45; // extra padding
	_GetRunningState();
}


void
SystemStatusBox::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case B_SOME_APP_QUIT:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(fSignature.Compare(sig)==0)
			{
				if(fCurrentState == STATE_OK_RESTARTING)
				// In the middle of a restart- send message to start the service
				{
					BMessenger messenger(this);
					BMessage message(START_SERVICE);
					messenger.SendMessage(&message);
				}
				else
				// All other states
					_SetState(STATE_OK_STOPPED);
			}
			break;
		}
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(fSignature.Compare(sig)==0)
				_SetState(STATE_OK_RUNNING);
			break;
		}
		case RESTART_SERVICE:
		{
			_SetState(STATE_OK_RESTARTING);
			status_t rc = _StopService();
			if(rc != B_OK)
			{
				if(be_roster->IsRunning(fSignature.String()))
				{	_SetState(STATE_ERROR_STOPPING__IS_RUNNING); }
				else
				{
					_SetState(STATE_ERROR_STOPPING__IS_NOT_RUNNING);
					// Attempt to start the service
					BMessenger messenger(this);
					BMessage message(START_SERVICE);
					messenger.SendMessage(&message);
				}
			}
			break;
		}
		case START_SERVICE:
		{
			_SetState(STATE_OK_STARTING);
			status_t rc = _StartService();
			if(rc != B_OK)
			{
				if(be_roster->IsRunning(fSignature.String()))
					_SetState(STATE_ERROR_STARTING__IS_RUNNING);
				else
					_SetState(STATE_ERROR_STARTING__IS_NOT_RUNNING);
			}
			break;
		}
		case STOP_SERVICE:
		{
			_SetState(STATE_OK_STOPPING);
			status_t rc = _StopService();
			if(rc != B_OK)
			{
				if(be_roster->IsRunning(fSignature.String()))
					_SetState(STATE_ERROR_STOPPING__IS_RUNNING);
				else
					_SetState(STATE_ERROR_STOPPING__IS_NOT_RUNNING);
			}
			break;
		}
	}
}


status_t
SystemStatusBox::_StartService()
{
	status_t rc = be_roster->Launch(fSignature.String());
	return rc;
}


status_t
SystemStatusBox::_StopService()
{
	status_t rc = B_ERROR;
	BMessenger appMessenger(fSignature.String());
	if(!appMessenger.IsValid())
	{
		BString err(B_TRANSLATE_COMMENT("Error: Could not create messenger for service ", "Alert message"));
		err.Append(fName);
		(new BAlert("", err.String(),"OK"))->Go(NULL);
		return rc;
	}
	rc = appMessenger.SendMessage(B_QUIT_REQUESTED);
	return rc;
}


void
SystemStatusBox::_SetStatusText(const char* text)
{
	fStringViewText.SetTo(fStatusText);
	fStringViewText.Append(" ");
	fStringViewText.Append(text);
//	fStringViewText.Append(" (Really really long text used for testing purposes.)");
	ResizeStatusText();
}


void
SystemStatusBox::ResizeStatusText()
{
	// truncate text and create tool tip if message is too long
	BString newString(fStringViewText);
	float remainingWidth = Window()->Bounds().Width() - fUsedWidth;
	fStatusSV->TruncateString(&newString, B_TRUNCATE_END, remainingWidth);
	fStatusSV->SetText(newString.String());
	// TODO need to work on the tooltip- not working correctly
/*	if(newString.Compare(stringViewText)!=0)
	{
		fStatusSV->SetToolTip(stringViewText.String());
	}
	else
	{
		fStatusSV->SetToolTip("");
		fStatusSV->SetToolTip((BToolTip*)NULL);
	}*/
}


void
SystemStatusBox::_GetRunningState(){
	if(be_roster->IsRunning(fSignature.String()))
		_SetState(STATE_OK_RUNNING);
	else
		_SetState(STATE_OK_STOPPED);
}


void
SystemStatusBox::_SetState(int newstate)
{
	switch(newstate)
	{
		case STATE_OK_RUNNING:
		{
			_SetStatusText(fStatusRunningText);
			fRestartB->SetEnabled(true);
			fStartstopB->SetLabel(fStopLabel);
			fStartstopB->SetEnabled(true);
			fStartstopB->SetMessage(new BMessage(STOP_SERVICE));
			fCurrentState = STATE_OK_RUNNING;
			break;
		}
		case STATE_OK_STOPPING:
		{
			_SetStatusText(fStatusStoppingText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetEnabled(false);
			fCurrentState = STATE_OK_STOPPING;
			break;
		}
		case STATE_ERROR_STOPPING__IS_NOT_RUNNING:
		{
			_SetStatusText(fStoppingErrorText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetLabel(fStartLabel);
			fStartstopB->SetEnabled(true);
			fStartstopB->SetMessage(new BMessage(START_SERVICE));
			fCurrentState = STATE_ERROR_STOPPING__IS_NOT_RUNNING;
			break;
		}
		case STATE_ERROR_STOPPING__IS_RUNNING:
		{
			_SetStatusText(fStoppingErrorText);
			fRestartB->SetEnabled(true);
			fStartstopB->SetLabel(fStopLabel);
			fStartstopB->SetEnabled(true);
			fStartstopB->SetMessage(new BMessage(STOP_SERVICE));
			fCurrentState = STATE_ERROR_STOPPING__IS_RUNNING;
			break;
		}
		case STATE_OK_STOPPED:
		{
			_SetStatusText(fStatusStoppedText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetLabel(fStartLabel);
			fStartstopB->SetEnabled(true);
			fStartstopB->SetMessage(new BMessage(START_SERVICE));
			fCurrentState = STATE_OK_STOPPED;
			break;
		}
		case STATE_OK_STARTING:
		{
			_SetStatusText(fStatusStartingText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetEnabled(false);
			fCurrentState = STATE_OK_STARTING;
			break;
		}
		case STATE_ERROR_STARTING__IS_RUNNING:
		{
			_SetStatusText(fStartingErrorText);
			fRestartB->SetEnabled(true);
			fStartstopB->SetLabel(fStopLabel);
			fStartstopB->SetEnabled(true);
			fStartstopB->SetMessage(new BMessage(STOP_SERVICE));
			fCurrentState = STATE_ERROR_STARTING__IS_RUNNING;
		}
		case STATE_ERROR_STARTING__IS_NOT_RUNNING:
		{
			_SetStatusText(fStartingErrorText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetLabel(fStartLabel);
			fStartstopB->SetEnabled(true);
			fStartstopB->SetMessage(new BMessage(START_SERVICE));
			fCurrentState = STATE_ERROR_STARTING__IS_NOT_RUNNING;
			break;
		}
		case STATE_OK_RESTARTING:
		{
			_SetStatusText(fStatusRestartingText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetEnabled(false);
			fCurrentState = STATE_OK_RESTARTING;
			break;
		}
		default:
		{
			_SetStatusText(fStatusUnknownText);
			fRestartB->SetEnabled(false);
			fStartstopB->SetLabel(fStopLabel);
			fStartstopB->SetEnabled(false);
			fStartstopB->SetMessage(NULL);
			fCurrentState = STATE_NONE;
		}
	}
}
