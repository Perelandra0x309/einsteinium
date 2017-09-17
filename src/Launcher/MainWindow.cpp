/* MainWindow.cpp
 * Copyright 2013-2017 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main Window"

MainWindow::MainWindow(BRect size, window_look look)
	:
	BWindow(size, "Einsteinium Launcher", look, B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE, B_ALL_WORKSPACES),
	fInfoString("")
{
	Lock();
	BRect frameRect(Bounds());
	fView = new MainView(frameRect);
	fInfoView = new BStringView("InfoView","");
	fMenuBar = new BMenuBar("MenuBar");
	// File menu
	fFileMenu = new BMenu("File");
	BMenuItem *menuItem1 = new BMenuItem(B_TRANSLATE_COMMENT("Hide window", "File menu"),
		new BMessage(EL_HIDE_APP), 'W');
	BMenuItem *menuItem2 = new BMenuItem(B_TRANSLATE_COMMENT("Quit", "File menu"),
		new BMessage(EL_QUIT_FROM_MENUBAR), 'Q');
	fFileMenu->AddItem(menuItem1);
	fFileMenu->AddItem(menuItem2);
	fMenuBar->AddItem(fFileMenu);
	// Settings menu
	fSettingsMenu = new BMenu("Settings");
	BMenuItem *menuItem3 = new BMenuItem(B_TRANSLATE_COMMENT("Layout"B_UTF8_ELLIPSIS, "Settings menu"),
		new BMessage(EL_SHOW_SETTINGS_LAYOUT));
	BMenuItem *menuItem4 = new BMenuItem(B_TRANSLATE_COMMENT("App Ranking"B_UTF8_ELLIPSIS, "Settings menu"),
		new BMessage(EL_SHOW_SETTINGS_RANKINGS));
	BMenuItem *menuItem5 = new BMenuItem(B_TRANSLATE_COMMENT("App Exclusions"B_UTF8_ELLIPSIS, "Settings menu"),
		new BMessage(EL_SHOW_SETTINGS_EXCLUSIONS));
	fSettingsMenu->AddItem(menuItem3);
	fSettingsMenu->AddItem(menuItem4);
	fSettingsMenu->AddItem(menuItem5);
	fMenuBar->AddItem(fSettingsMenu);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.Add(fView)
		.AddGroup(B_VERTICAL)
			// align text vertically centered
			.SetInsets(5, 2, 0, 2)
			.Add(fInfoView)
		.End()
	.End();

	SetSizeLimits(400, B_SIZE_UNLIMITED, 300, B_SIZE_UNLIMITED);
	Unlock();
	SelectDefaultView();
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
			status_t result = msg->FindString(EL_INFO_STRING, &textStr);
			if (result == B_OK) {
				fInfoString = textStr;
				_RedrawInfoText();
			}
			break;
		}
		case EL_LISTVIEW_CHANGED:
			fView->SelectedListViewChanged();
			break;
		case EL_HIDE_APP:
			be_app->PostMessage(EL_HIDE_APP);
			break;
		case EL_QUIT_FROM_MENUBAR:
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		case EL_SHOW_SETTINGS_LAYOUT:
		case EL_SHOW_SETTINGS_RANKINGS:
		case EL_SHOW_SETTINGS_EXCLUSIONS:
			be_app->PostMessage(msg);
			break;
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
	_RedrawInfoText();
		// Text may need to be retruncated
}


void
MainWindow::SettingsChanged(uint32 what)
{
	fView->SettingsChanged(what);
}


void
MainWindow::SelectDefaultView()
{
	Lock();
	fView->SelectDefaultView();
	Unlock();
}


void
MainWindow::BuildAppsListView(BMessage *msg)
{
	fView->BuildAppsListView(msg);
}


void
MainWindow::_RedrawInfoText()
{
	BString textStr(fInfoString);
	fInfoView->TruncateString(&textStr, B_TRUNCATE_SMART, Bounds().Width() - 5);
	fInfoView->SetText(textStr);
	fInfoView->Invalidate();
}
