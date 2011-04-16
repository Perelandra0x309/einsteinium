/* prefsWindow.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "prefsWindow.h"

prefsWindow::prefsWindow(BRect size)
	:
	BWindow(size, "Einsteinium Preferences", B_TITLED_WINDOW, B_NOT_ZOOMABLE),
	fLauncherSettings(NULL)
{
	//Defaults
	fScales.launches_scale = DEFAULT_VALUE_LAUNCH_SCALE;
	fScales.first_launch_scale = DEFAULT_VALUE_FIRST_SCALE;
	fScales.last_launch_scale = DEFAULT_VALUE_LAST_SCALE;
	fScales.interval_scale = DEFAULT_VALUE_INTERVAL_SCALE;
	fScales.total_run_time_scale = DEFAULT_VALUE_RUNTIME_SCALE;

	Lock();
	BRect viewRect(Bounds());
	fMainView = new BView(viewRect, "Background View", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fMainView->SetViewColor(bg_color);
	AddChild(fMainView);


	//Prefs list view
	viewRect.Set(BORDER_SIZE, BORDER_SIZE, BORDER_SIZE + 100, Bounds().bottom - BORDER_SIZE);
	fPrefsListView = new BListView(viewRect, "Preferences", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	fPrefsListView->SetSelectionMessage(new BMessage(PREFS_ITEM_CHANGED));
	//Daemon settings
	fDaemonBLI = new BitmapListItem(e_daemon_sig, "Daemon");
	fDAppLaunchSI = new BStringItem("    App Relaunch");
	//Engine settings
	fEngineBLI = new BitmapListItem(e_engine_sig, "Engine");
	fEMaintSI = new BStringItem("    Maintenance");
	//Launcher settings
	fLauncherBLI = new BitmapListItem(e_launcher_sig, "Launcher");
	fLDeskbarSI = new BStringItem("    Deskbar Menu");
	fLRankSI = new BStringItem("    App Rankings");
	fLExclusionsSI = new BStringItem("    App Exclusions");

	//Resize the list view
	BFont font;
	fPrefsListView->GetFont(&font);
	fPrefsListView->ResizeTo(font.StringWidth(fLDeskbarSI->Text()) + 10, fPrefsListView->Frame().Height());
	fPrefsScrollView = new BScrollView("List Scrollview", fPrefsListView,
		B_FOLLOW_TOP_BOTTOM);
	fMainView->AddChild(fPrefsScrollView);
	viewRect.Set(fPrefsScrollView->Frame().right + BORDER_SIZE, BORDER_SIZE,
					Bounds().right - BORDER_SIZE, Bounds().bottom - BORDER_SIZE);

	//Main about view
	BRect aboutViewRect(0,0,viewRect.Width(), viewRect.Height());
	fEmptySettingsView = new BView(viewRect, "Empty SettingsView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fEmptySettingsView->SetViewColor(bg_color);
//	fAboutBox = new BBox("Empty Box");
	fAboutBox = new BBox(aboutViewRect, "About", B_FOLLOW_LEFT_RIGHT);
	fAboutBox->SetLabel("About Einsteinium Preferences");
//	fAboutTextView = new BTextView("About text");
	aboutViewRect.InsetBy(10, 5);
	aboutViewRect.top += 15;
	BRect textRect(0,0,aboutViewRect.Width(), aboutViewRect.Height());
	fAboutTextView = new BTextView(aboutViewRect, "About text", textRect, B_FOLLOW_ALL);
	fAboutTextView->SetText("Einsteinium provides smarter monitoring of applications and"
		" system services for Haiku.  Currently the two major functions implimented are"
		" automatically restarting applications and system services that quit or crash,"
		" and gathering statistics on application usage to provide customizable ranked"
		" lists of applications.\n\n"
		"This preferences application is used to set options for the Einsteinium Engine,"
		" Daemon and Launcher.  See each application's main section to get more details.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
	fAboutTextView->SetViewColor(fMainView->ViewColor());
	fCopyrightStringView = new BStringView("Copyright", "Einsteinium Copyright 2011 by Brian Hill");
	fCopyrightStringView->ResizeToPreferred();
/*	fAboutBox->AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 5)
		.Add(fAboutTextView)
		.SetInsets(5, 5, 5, 5)
	);
	fEmptySettingsView->SetLayout(new BGroupLayout(B_HORIZONTAL));
	fEmptySettingsView->AddChild(BGroupLayoutBuilder(B_VERTICAL, 10).Add(fAboutBox).AddGlue());*/
	fAboutBox->AddChild(fAboutTextView);
	fAboutBox->AddChild(fCopyrightStringView);
	fEmptySettingsView->AddChild(fAboutBox);
	fMainView->AddChild(fEmptySettingsView);
	fEmptySettingsView->Hide();

	//Settings views
	fDStatusView = new DaemonStatusView(viewRect);
	fDRelaunchView = new DaemonRelaunchView(viewRect);
	fEStatusView = new EngineStatusView(viewRect);
	fMaintenanceView = new EngineMaintenanceView(viewRect);
	fLAboutView = new LauncherAboutView(viewRect);
	fLDeskbarView = new LauncherDeskbarView(viewRect);
	fLRankingsView = new LauncherRankingsView(viewRect);
	fLExclusionsView = new LauncherExclusionsView(viewRect);

	//Add all the ListItems and settings views to the window
	_AddSettingsView(fDaemonBLI, fDStatusView);
	_AddSettingsView(fDAppLaunchSI, fDRelaunchView);
	_AddSettingsView(fEngineBLI, fEStatusView);
	_AddSettingsView(fEMaintSI, fMaintenanceView);
	_AddSettingsView(fLauncherBLI, fLAboutView);
	_AddSettingsView(fLDeskbarSI, fLDeskbarView);
	_AddSettingsView(fLRankSI, fLRankingsView);
	// TODO impliment exclusions
	_AddSettingsView(fLExclusionsSI, fLExclusionsView);
//	fMainView->AddChild(fLExclusionsView);
//	fLExclusionsView->Hide();

	fCurrentView = fEmptySettingsView;
	FrameResized(0,0);
	fCurrentView->Show();

	// resize main window and set min size based on the min sizes of each view
	float minHeight=0, minWidth=0;
	BSize minSize = fDRelaunchView->GetMinSize();
	minHeight = minSize.height;
	minWidth = minSize.width;
	minSize = fLRankingsView->GetMinSize();
	minHeight = max_c(minHeight, minSize.height);
	minWidth = max_c(minWidth, minSize.width);
	minSize = fLExclusionsView->GetMinSize();
	minHeight = max_c(minHeight, minSize.height);
	minWidth = max_c(minWidth, minSize.width);
	float finalMinWidth = fPrefsListView->Frame().right + minWidth + 10;
	float finalMinHeight = minHeight + 10;
	SetSizeLimits(finalMinWidth, B_SIZE_UNLIMITED, finalMinHeight, B_SIZE_UNLIMITED);
	ResizeTo(finalMinWidth, finalMinHeight);

	//Settings
	fLauncherSettings = new LauncherSettingsFile(this);
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
	delete fLauncherSettings;
	delete fAppsPanel;
	delete fAppFilter;
}


bool
prefsWindow::QuitRequested()
{
//	_StoreSettings();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}


void
prefsWindow::FrameResized(float width, float height)
{
	BRect textRect = fAboutTextView->Bounds();
	fAboutTextView->SetTextRect(textRect);
	textRect.bottom = fAboutTextView->TextHeight(0, fAboutTextView->TextLength());
	fAboutTextView->SetTextRect(textRect);
	fAboutTextView->ResizeTo(fAboutTextView->Bounds().Width(), textRect.Height());
	fCopyrightStringView->MoveTo((textRect.Width() - fCopyrightStringView->Frame().Width())/2.0,
									fAboutTextView->Frame().bottom + 30);
	fAboutBox->ResizeTo(fAboutBox->Frame().Width(), fCopyrightStringView->Frame().bottom + 25);
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
		// Deskbar settings changed
		case EL_DESKBAR_CHANGED: {
			_WriteLauncherDeskbarSettings();
			break;
		}
		// Save rank slider settings and recalculate scores
		case EL_SAVE_RANKING: {
			_WriteLauncherScaleSettings();
			fLRankingsView->MessageReceived(msg);
			break; }
		// Save exclusion settings
/*		case EL_LIST_INCLUSION_CHANGED: {
			_WriteLauncherListInclusionSetting();
			break; }*/
		// Add exclusion to list
		case EL_ADD_EXCLUSION: {
			BMessage addmsg(EL_ADD_EXCLUSION_REF);
			fAppsPanel->SetMessage(&addmsg);
			fAppsPanel->SetTarget(this);
			fAppsPanel->Show();
			break; }
		case EL_ADD_EXCLUSION_REF: {
			bool success = fLExclusionsView->AddExclusion(msg);
			if(success)
				_WriteLauncherExclusions();
			break; }
		case EL_REMOVE_EXCLUSION: {
			bool success = fLExclusionsView->RemoveSelectedExclusion();
			if(success)
				_WriteLauncherExclusions();
			break; }
		case EL_EXCLUSION_SELECTION_CHANGED: {
			fLExclusionsView->UpdateSelectedItem();
			break; }
		// Settings file was updated by an external application
		case EL_SETTINGS_FILE_CHANGED_EXTERNALLY: {
			_ReadLauncherSettings();
			break; }
		case EL_GOTO_LAUNCHER_SETTINGS: {
			fPrefsListView->Select(fPrefsListView->IndexOf(fLauncherBLI));
			break; }
		case ED_RELAPP_SELECTION_CHANGED:
		case ED_ADD_APPITEM:
		case ED_ADD_APPITEM_REF:
		case ED_REMOVE_APPITEM:
		case ED_AUTO_RELAUNCH_CHANGED: {
			fDRelaunchView->MessageReceived(msg);
			break; }
		case EL_LAUNCHES_SL_CHANGED:
		case EL_FIRST_SL_CHANGED:
		case EL_LAST_SL_CHANGED:
		case EL_INTERVAL_SL_CHANGED:
		case EL_RUNTIME_SL_CHANGED: {
			fLRankingsView->MessageReceived(msg);
			break; }
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

/*
void
prefsWindow::_StoreSettings()
{
//	_WriteDaemonSettings();
}*/


void
prefsWindow::_ReadAllSettings()
{
	Lock();
//	_ReadDaemonSettings();
	_ReadLauncherSettings();
	Unlock();
}


/*void prefsWindow::_ReadDaemonSettings()
{
	appLaunchView->ReadSettings();
}*/
/*void prefsWindow::_WriteDaemonSettings()
{
	appLaunchView->WriteSettings();
	/*
	BPath settingsPath;
	BString xml_text;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append("Einsteinium/daemon_settings");
	BFile archive;
	archive.SetTo(settingsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	archive.Write("<?xml version=\"1.0\"?>\n", 22);
	appLaunchView->WriteSettings(xml_text);
	archive.Write(xml_text.String(), xml_text.Length());
	archive.Unset();*/
//}

void
prefsWindow::_ReadLauncherSettings()
{
	int *scales = fLauncherSettings->GetScales();
	fScales.launches_scale = scales[LAUNCH_INDEX];
	fScales.first_launch_scale = scales[FIRST_INDEX];
	fScales.last_launch_scale = scales[LAST_INDEX];
	fScales.interval_scale = scales[INTERVAL_INDEX];
	fScales.total_run_time_scale = scales[RUNTIME_INDEX];
	fLRankingsView->SetSliderValues(fScales);

	// Read setting for action when new application is detected
//	fLExclusionsView->SetLinkInclusionDefault(fLauncherSettings->GetLinkInclusionDefaultValue());

	// Read excluded apps
	BMessage exclusionsList = fLauncherSettings->GetExclusionsList();
	if(exclusionsList.what == EL_MESSAGE_WHAT_EXCLUDED_APPS)
		fLExclusionsView->PopulateExclusionsList(exclusionsList);

	// Deskbar settings
	fLDeskbarView->SetDeskbarCount(fLauncherSettings->GetDeskbarCount());
}



void
prefsWindow::_WriteLauncherDeskbarSettings()
{
	fLauncherSettings->SaveDeskbarCount(fLDeskbarView->GetDeskbarCount());
}

void
prefsWindow::_WriteLauncherScaleSettings()
{
	// Rank scale settings
	fLRankingsView->GetSliderValues(fScales);
	int scales[5];
	scales[LAUNCH_INDEX] = fScales.launches_scale;
	scales[FIRST_INDEX] = fScales.first_launch_scale;
	scales[LAST_INDEX] = fScales.last_launch_scale;
	scales[INTERVAL_INDEX] = fScales.interval_scale;
	scales[RUNTIME_INDEX] = fScales.total_run_time_scale;
	fLauncherSettings->SaveScales(scales);
}

/*
void
prefsWindow::_WriteLauncherListInclusionSetting()
{
	BString inclusionValue;
	fLExclusionsView->GetLinkInclusionDefault(inclusionValue);
	fLauncherSettings->SaveLinkInclusionDefaultValue(inclusionValue.String());
}*/

void
prefsWindow::_WriteLauncherExclusions()
{
	BMessage exclusionsList(EL_MESSAGE_WHAT_EXCLUDED_APPS);
	fLExclusionsView->GetExclusionsList(exclusionsList);
	fLauncherSettings->SaveExclusionsList(exclusionsList);
}

