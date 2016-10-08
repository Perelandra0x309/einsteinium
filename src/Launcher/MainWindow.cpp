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
	const int inset = 0;
	BRect frameRect(Bounds());
//	frameRect.InsetBy(-1,0);
	frameRect.bottom-=25;
	fView = new MainView(frameRect);
	fInfoView = new BStringView("InfoView","");
//	fInfoView->SetExplicitMaxSize(BSize(100, 20));
/*	fAboutTextView = new BTextView(BRect(-10,0,0,1), "About text", BRect(0,0,0,0), B_FOLLOW_NONE);
	fAboutTextView->SetText("");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
	fAboutTextView->MakeResizable(true);
	fAboutTextView->SetExplicitMaxSize(BSize(9999, 20));
//	fAboutTextView->SetViewColor(fView->ViewColor());
//	fAboutTextView->SetTextRect(BRect(0,0,999,40));*/


	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fView)
		.Add(fInfoView)
	//	.Add(fAboutTextView)
		.SetInsets(inset, inset, inset, inset)
	);
	/*
	BRect frameRect(Bounds());
	frameRect.InsetBy(-1,0);
	frameRect.bottom-=20;
	fView = new MainView(frameRect, settings);
	BRect newFrameRect(Bounds());
	newFrameRect.top = frameRect.bottom + 1;
//s	newFrameRect.left+=1;
	fInfoView = new BStringView(newFrameRect, "InfoView","", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
	AddChild(fView);
	AddChild(fInfoView);*/

	SetSizeLimits(fView->TabFrame(fView->CountTabs()-1).right + 2*inset, 9999, 200, 9999);
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
			fInfoView->TruncateString(&textStr, B_TRUNCATE_SMART, Bounds().Width());
			fInfoView->SetText(textStr);
		//	fAboutTextView->SetText(textStr);
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


void
MainWindow::FrameResized(float new_width, float new_height)
{
	BWindow::FrameResized(new_width, new_height);
	be_app->PostMessage(EL_WINDOW_MOVED);
}
*/

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
