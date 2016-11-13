/* prefsWindow.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "prefsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main Window"

prefsWindow::prefsWindow(BRect size)
	:
	BWindow(size, B_TRANSLATE_COMMENT("Einsteinium Preferences", "Window title"), B_TITLED_WINDOW, B_NOT_ZOOMABLE)
{
	Lock();
	BRect viewRect(Bounds());
	fMainView = new BView(viewRect, "Background View", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fMainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fMainView);


	//Prefs list view
	viewRect.Set(BORDER_SIZE, BORDER_SIZE, BORDER_SIZE + 100, Bounds().bottom - BORDER_SIZE);
	fPrefsListView = new BListView(viewRect, "Preferences", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	fPrefsListView->SetSelectionMessage(new BMessage(PREFS_ITEM_CHANGED));
	//Daemon settings
	fDaemonBLI = new BitmapListItem(e_daemon_sig, B_TRANSLATE_COMMENT("Daemon", "List label"));
	BString appRelaunchLabel("    ");
	appRelaunchLabel.Append(B_TRANSLATE_COMMENT("App Relaunch", "List label"));
	fDAppLaunchSI = new BStringItem(appRelaunchLabel);
	//Engine settings
	fEngineBLI = new BitmapListItem(e_engine_sig, B_TRANSLATE_COMMENT("Engine", "List label"));
	BString maintenanceLabel("    ");
	maintenanceLabel.Append(B_TRANSLATE_COMMENT("Maintenance", "List label"));
	fEMaintSI = new BStringItem(maintenanceLabel);
	//Launcher settings
	fLauncherBLI = new BitmapListItem(e_launcher_sig, B_TRANSLATE_COMMENT("Launcher", "List label"));

	//Resize the list view
	BFont font;
	fPrefsListView->GetFont(&font);
	fPrefsListView->ResizeTo(font.StringWidth(fDAppLaunchSI->Text()) + 10, fPrefsListView->Frame().Height());
	fPrefsScrollView = new BScrollView("List Scrollview", fPrefsListView,
		B_FOLLOW_TOP_BOTTOM);
	fMainView->AddChild(fPrefsScrollView);
	viewRect.Set(fPrefsScrollView->Frame().right + BORDER_SIZE, BORDER_SIZE,
					Bounds().right - BORDER_SIZE, Bounds().bottom - BORDER_SIZE);

	//Main about view
	fEmptySettingsView = new BView(viewRect, "Empty SettingsView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fEmptySettingsView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fAboutBox = new BBox("About");
	fAboutBox->SetLabel(B_TRANSLATE_COMMENT("About Einsteinium Preferences", "Box label"));
	fAboutTextView = new BTextView("About text");
	fAboutTextView->SetText(B_TRANSLATE_COMMENT("Einsteinium provides smarter monitoring of applications and"
		" system services for Haiku.  Currently the two major functions implimented are"
		" automatically restarting applications and system services that quit or crash,"
		" and gathering statistics on application usage to provide customizable ranked"
		" lists of applications.\n\n"
		"This preferences application is used to set options for the Einsteinium Daemon"
		" and Engine.  See each application's main section to get more details.", "About text"));
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
	fAboutTextView->SetViewColor(fMainView->ViewColor());
	fCopyrightStringView = new BStringView("Copyright", B_TRANSLATE_COMMENT("Einsteinium Copyright 2013 by Brian Hill", "Copyright text"));
	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL, 5);
	fAboutBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
		.Add(fAboutTextView, 1)
		.SetInsets(10, 20, 10, 10)
	;
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL, 10);
	fEmptySettingsView->SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fAboutBox, 0)
		.Add(fCopyrightStringView, 0)
		.AddGlue()
	;
	fMainView->AddChild(fEmptySettingsView);
	fEmptySettingsView->Hide();

	//Settings views
	fDStatusView = new DaemonStatusView(viewRect);
	fDRelaunchView = new DaemonRelaunchView(viewRect);
	fEStatusView = new EngineStatusView(viewRect);
	fMaintenanceView = new EngineMaintenanceView(viewRect);
	fLAboutView = new LauncherAboutView(viewRect);

	//Add all the ListItems and settings views to the window
	_AddSettingsView(fDaemonBLI, fDStatusView);
	_AddSettingsView(fDAppLaunchSI, fDRelaunchView);
	_AddSettingsView(fEngineBLI, fEStatusView);
	_AddSettingsView(fEMaintSI, fMaintenanceView);
	_AddSettingsView(fLauncherBLI, fLAboutView);

	fCurrentView = fEmptySettingsView;
	fCurrentView->Show();

	// resize main window and set min size based on the min sizes of each view
	float minHeight=0, minWidth=0;
	BSize minSize = fDRelaunchView->GetMinSize();
	minHeight = minSize.height;
	minWidth = minSize.width;
	// Make height minimum of 500
	minHeight = max_c(minHeight, 500);
	float finalMinWidth = fPrefsListView->Frame().right + minWidth + 10;
	float finalMinHeight = minHeight + 10;
	SetSizeLimits(finalMinWidth, B_SIZE_UNLIMITED, finalMinHeight, B_SIZE_UNLIMITED);
	ResizeTo(finalMinWidth, finalMinHeight);

	//Settings
	_ReadAllSettings();

	//File Panel
	fAppFilter = new AppRefFilter();
	fAppsPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, fAppFilter);

	Unlock();
	Show();
}


prefsWindow::~prefsWindow()
{
	fSettingsViews.MakeEmpty();
	delete fAppsPanel;
	delete fAppFilter;
}


bool
prefsWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}


void
prefsWindow::FrameResized(float width, float height)
{
	fPrefsListView->Invalidate();
	fPrefsScrollView->Invalidate();
	fEmptySettingsView->Invalidate();
	fAboutBox->Invalidate();
	fCopyrightStringView->Invalidate();
	BWindow::FrameResized(width, height);
}


void
prefsWindow::MessageReceived(BMessage *msg)
{	switch(msg->what)
	{	case PREFS_ITEM_CHANGED: {
			Lock();
			fCurrentView->Hide();
			// Show view for selected item, or the fEmptySettingsView if nothing is selected
			int32 index = fPrefsListView->CurrentSelection();
			if(index < 0)
				fCurrentView = fEmptySettingsView;
			else
				fCurrentView = (BView*)fSettingsViews.ItemAt(index);
			fCurrentView->Show();
			Unlock();
			break; }
		case E_RESCAN_DATA: {
			BMessenger messenger(e_engine_sig);
			BMessage msg(E_RESCAN_DATA_FILES);
			messenger.SendMessage(&msg);
			break; }
		case ED_SETTINGS_FILE_CHANGED_EXTERNALLY: {
			fDRelaunchView->ReadSettings();
			break; }
		// Pass these messages to the views that need them
		case ED_RELAPP_SELECTION_CHANGED:
		case ED_ADD_APPITEM:
		case ED_ADD_APPITEM_REF:
		case ED_REMOVE_APPITEM:
		case ED_AUTO_RELAUNCH_CHANGED: {
			fDRelaunchView->MessageReceived(msg);
			break; }
		case OPEN_LAUNCHER_SETTINGS: {
			if(be_roster->IsRunning(e_launcher_sig))
			{
				status_t rc = B_ERROR;
				BMessenger appMessenger(e_launcher_sig, -1, &rc);
				if(!appMessenger.IsValid())
					break;
				appMessenger.SendMessage(EL_SHOW_SETTINGS);
			}
			else
			{
				BMessage goToMessage(EL_SHOW_SETTINGS);
				be_roster->Launch(e_launcher_sig, &goToMessage);
			}
			break;
		}
		default: {
			BWindow::MessageReceived(msg);
			break; }
	}
}


void
prefsWindow::_AddSettingsView(BListItem *item, BView *view)
{
	// Add list item
	fPrefsListView->AddItem(item);

	// Add view
	fMainView->AddChild(view);
	view->Hide();
	fSettingsViews.AddItem(view);
}


void
prefsWindow::_ReadAllSettings()
{
	Lock();
	_ReadDaemonSettings();
	Unlock();
}


void prefsWindow::_ReadDaemonSettings()
{
	fDRelaunchView->ReadSettings();
}
