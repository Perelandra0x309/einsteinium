/* MainWindow.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainWindow.h"

MainWindow::MainWindow(BRect size, window_look look)
	:
	BWindow(size, "Einsteinium Launcher", look, B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE, B_ALL_WORKSPACES)
{
	Lock();
	BRect frameRect(Bounds());
	fView = new MainView(frameRect);
	fInfoView = new BStringView("InfoView","");

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		// Get rid of pixels at edges of list view
		.SetInsets(-3, 0, -3, 0)
		.Add(fView)
		.AddGroup(B_VERTICAL)
			// align text vertically centered
			.SetInsets(5, 2, 0, 2)
			.Add(fInfoView);

	SetSizeLimits(fView->TabFrame(fView->CountTabs()-1).right + 5, 9999, 200, 9999);
	Unlock();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(EL_HIDE_APP);
	return false			;
}


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case B_MOUSE_WHEEL_CHANGED:
		case EL_UPDATE_RECENT_LISTS:
		case EL_UPDATE_RECENT_LISTS_FORCE:
		case EL_EXCLUSIONS_CHANGED:
		{
			fView->MessageReceived(msg);
			break;
		}
		case EL_UPDATE_INFOVIEW:
		{
			BString textStr;
			msg->FindString(EL_INFO_STRING, &textStr);
			fInfoView->TruncateString(&textStr, B_TRUNCATE_SMART, Bounds().Width()-2);
			fInfoView->SetText(textStr);
			fInfoView->Invalidate();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}

/*
void
MainWindow::FrameMoved(BPoint new_position)
{
	BWindow::FrameMoved(new_position);
	be_app->PostMessage(EL_WINDOW_MOVED);
}
*/

void
MainWindow::FrameResized(float new_width, float new_height)
{
	BWindow::FrameResized(new_width, new_height);
	fView->UpdateInfoView();
}


void
MainWindow::SettingsChanged(uint32 what)
{
	fView->SettingsChanged(what);
}


void
MainWindow::SelectDefaultTab()
{
	Lock();
	fView->SelectDefaultTab();
	Unlock();
}


void
MainWindow::BuildAppsListView(BMessage *msg)
{
	fView->BuildAppsListView(msg);
}
