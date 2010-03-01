/*SystemStatusBox.cpp

*/
#include "SystemStatusBox.h"


SystemStatusBox::SystemStatusBox(BEntry entry, const char * sig)
	:BBox(sig)
	,currentState(STATE_NONE)
{
	BPath path;
	entry.GetPath(&path);
	name.SetTo(path.Leaf());
	SetLabel(name.String());
	SetBorder(B_PLAIN_BORDER);
	SetResizingMode(B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
	signature.SetTo(sig);
	statusSV = new BStringView("Status String", "Status: ");
	restartB = new BButton("Restart", new BMessage(EP_RESTART_SERVICE));
	restartB->SetEnabled(false);
	startstopB = new BButton("Stop");
	startstopB->SetEnabled(false);

	AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 10)
		.Add(statusSV)
		.AddGlue()
		.Add(restartB)
		.Add(startstopB)
		.SetInsets(5, 5, 5, 5)
	);

	GetRunningState();
}

SystemStatusBox::~SystemStatusBox()
{
}

void SystemStatusBox::AttachedToWindow()
{
	BBox::AttachedToWindow();
	restartB->SetTarget(this);
	startstopB->SetTarget(this);
}

void SystemStatusBox::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case B_SOME_APP_QUIT:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(signature.Compare(sig)==0)
			{
				if(currentState == STATE_OK_RESTARTING)
				// In the middle of a restart- send message to start the service
				{
					BMessenger messenger(this);
					BMessage message(EP_START_SERVICE);
					messenger.SendMessage(&message);
				}
				else
				// All other states
				{
					SetState(STATE_OK_STOPPED);
				}
			}
			break;
		}
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(signature.Compare(sig)==0)
			{
				SetState(STATE_OK_RUNNING);
			}
			break;
		}
		case EP_RESTART_SERVICE:
		{
			SetState(STATE_OK_RESTARTING);
			status_t rc = StopService();
			if(rc != B_OK)
			{
				if(be_roster->IsRunning(signature.String()))
				{	SetState(STATE_ERROR_STOPPING__IS_RUNNING); }
				else
				{
					SetState(STATE_ERROR_STOPPING__IS_NOT_RUNNING);
					// Attempt to start the service
					BMessenger messenger(this);
					BMessage message(EP_START_SERVICE);
					messenger.SendMessage(&message);
				}
			}
			break;
		}
		case EP_START_SERVICE:
		{
			SetState(STATE_OK_STARTING);
			status_t rc = StartService();
			if(rc != B_OK)
			{
				if(be_roster->IsRunning(signature.String()))
				{	SetState(STATE_ERROR_STARTING__IS_RUNNING); }
				else
				{	SetState(STATE_ERROR_STARTING__IS_NOT_RUNNING); }
			}
			break;
		}
		case EP_STOP_SERVICE:
		{
			SetState(STATE_OK_STOPPING);
			status_t rc = StopService();
			if(rc != B_OK)
			{
				if(be_roster->IsRunning(signature.String()))
				{	SetState(STATE_ERROR_STOPPING__IS_RUNNING); }
				else
				{	SetState(STATE_ERROR_STOPPING__IS_NOT_RUNNING); }
			}
			break;
		}
	}
}

status_t SystemStatusBox::StartService()
{
	status_t rc = be_roster->Launch(signature.String());
	return rc;
}

status_t SystemStatusBox::StopService()
{
	status_t rc = B_ERROR;
	BMessenger appMessenger(signature.String());
	if(!appMessenger.IsValid())
	{
		BString err("Error: Could not create messenger for ");
		err.Append(name);
		(new BAlert("", err.String(),"OK"))->Go(NULL);
		return rc;
	}
	rc = appMessenger.SendMessage(B_QUIT_REQUESTED);
	return rc;
}

void SystemStatusBox::SetStatusText(const char* text)
{
	// TODO check size of string view, possibly create tool tip if message too long?
	BString label("Status: ");
	label.Append(text);
	statusSV->SetText(label.String());
}

void SystemStatusBox::GetRunningState(){
	if(be_roster->IsRunning(signature.String()))
	{	SetState(STATE_OK_RUNNING); }
	else
	{	SetState(STATE_OK_STOPPED); }
}

void SystemStatusBox::SetState(int newstate)
{
	switch(newstate)
	{
		case STATE_OK_RUNNING:
		{
			SetStatusText("Running");
			restartB->SetEnabled(true);
			startstopB->SetLabel("Stop");
			startstopB->SetEnabled(true);
			startstopB->SetMessage(new BMessage(EP_STOP_SERVICE));
			currentState = STATE_OK_RUNNING;
			break;
		}
		case STATE_OK_STOPPING:
		{
			SetStatusText("Stopping...");
			restartB->SetEnabled(false);
			//startstopB->SetLabel("Start");
			startstopB->SetEnabled(false);
			//startstopB->SetMessage(new BMessage(EP_START_SERVICE));
			currentState = STATE_OK_STOPPING;
			break;
		}
		case STATE_ERROR_STOPPING__IS_NOT_RUNNING:
		{
			SetStatusText("There was an error stopping the service");
			restartB->SetEnabled(false);
			startstopB->SetLabel("Start");
			startstopB->SetEnabled(true);
			startstopB->SetMessage(new BMessage(EP_START_SERVICE));
			currentState = STATE_ERROR_STOPPING__IS_NOT_RUNNING;
			break;
		}
		case STATE_ERROR_STOPPING__IS_RUNNING:
		{
			SetStatusText("There was an error stopping the service");
			restartB->SetEnabled(true);
			startstopB->SetLabel("Stop");
			startstopB->SetEnabled(true);
			startstopB->SetMessage(new BMessage(EP_STOP_SERVICE));
			currentState = STATE_ERROR_STOPPING__IS_RUNNING;
			break;
		}
		case STATE_OK_STOPPED:
		{
			SetStatusText("Stopped");
			restartB->SetEnabled(false);
			startstopB->SetLabel("Start");
			startstopB->SetEnabled(true);
			startstopB->SetMessage(new BMessage(EP_START_SERVICE));
			currentState = STATE_OK_STOPPED;
			break;
		}
		case STATE_OK_STARTING:
		{
			SetStatusText("Starting...");
			restartB->SetEnabled(false);
			//startstopB->SetLabel("Start");
			startstopB->SetEnabled(false);
			//startstopB->SetMessage(new BMessage(EP_START_SERVICE));
			currentState = STATE_OK_STARTING;
			break;
		}
		case STATE_ERROR_STARTING__IS_RUNNING:
		{
			SetStatusText("There was an error starting the service");
			restartB->SetEnabled(true);
			startstopB->SetLabel("Stop");
			startstopB->SetEnabled(true);
			startstopB->SetMessage(new BMessage(EP_STOP_SERVICE));
			currentState = STATE_ERROR_STARTING__IS_RUNNING;
		}
		case STATE_ERROR_STARTING__IS_NOT_RUNNING:
		{
			SetStatusText("There was an error starting the service");
			restartB->SetEnabled(false);
			startstopB->SetLabel("Start");
			startstopB->SetEnabled(true);
			startstopB->SetMessage(new BMessage(EP_START_SERVICE));
			currentState = STATE_ERROR_STARTING__IS_NOT_RUNNING;
			break;
		}
		case STATE_OK_RESTARTING:
		{
			SetStatusText("Restarting...");
			restartB->SetEnabled(false);
			//startstopB->SetLabel("Start");
			startstopB->SetEnabled(false);
			//startstopB->SetMessage(new BMessage(EP_START_SERVICE));
			currentState = STATE_OK_RESTARTING;
			break;
		}
		default:
		{
			SetStatusText("Unknown");
			restartB->SetEnabled(false);
			startstopB->SetLabel("Stop");
			startstopB->SetEnabled(false);
			startstopB->SetMessage(NULL);
			currentState = STATE_NONE;
		}
	}
}
