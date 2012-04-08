/* MainWindow.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainWindow.h"

MainWindow::MainWindow(BRect size, AppSettings settings)
	:
	BWindow(size, "Einsteinium Launcher", settings.windowLook, B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE, B_ALL_WORKSPACES)
{
	Lock();
	const int inset = 2;
	fView = new MainView(Bounds(), settings);
	SetLayout(new BGroupLayout(B_VERTICAL));
//	AddChild(fView);
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fView)
		.SetInsets(inset, inset, inset, inset)
	);
/*	BView *containerView = new BView(Bounds(), "Services", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS);

	AddChild(containerView);
	containerView->SetLayout(new BGroupLayout(B_VERTICAL));
	containerView->AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fView)
		.SetInsets(3, 3, 3, 3)
	);*/
	SetSizeLimits(fView->TabFrame(0).Width() + 2*inset, 9999, 100, 9999);
	Unlock();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(EL_HIDE_APP);
	return false;
}


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case B_MOUSE_WHEEL_CHANGED:
		case EL_UPDATE_RECENT_DOCS: {
			fView->MessageReceived(msg);
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
MainWindow::FrameMoved(BPoint new_position)
{
	BWindow::FrameMoved(new_position);
	be_app->PostMessage(EL_WINDOW_MOVED);
}


void
MainWindow::FrameResized(float new_width, float new_height)
{
	BWindow::FrameResized(new_width, new_height);
	be_app->PostMessage(EL_WINDOW_MOVED);
}


void
MainWindow::SettingsChanged(uint32 what, AppSettings settings)
{
	fView->SettingsChanged(what, settings);
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
