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
	// TODO use defaults
	fScales.launches_scale = 1;
	fScales.first_launch_scale = 1;
	fScales.last_launch_scale = 1;
	fScales.interval_scale = 1;
	fScales.total_run_time_scale = 1;

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
	fDaemonBLI = new BitmapListItem(e_daemon_sig, "Daemon Settings");
	fPrefsListView->AddItem(fDaemonBLI);
	fDAppLaunchSI = new BStringItem("    App Relaunch");
	fPrefsListView->AddItem(fDAppLaunchSI);
	//Engine settings
	fEngineBLI = new BitmapListItem(e_engine_sig, "Engine Settings");
	fPrefsListView->AddItem(fEngineBLI);
	fEMaintSI = new BStringItem("    Maintenance");
	fPrefsListView->AddItem(fEMaintSI);
	//Launcher settings
	fLauncherBLI = new BitmapListItem(e_launcher_sig, "Launcher Settings");
	fPrefsListView->AddItem(fLauncherBLI);
	fLDeskbarSI = new BStringItem("    Deskbar Menu");
	fPrefsListView->AddItem(fLDeskbarSI);
	fLRankSI = new BStringItem("    App Rankings");
	fPrefsListView->AddItem(fLRankSI);
	fLExclusionsSI = new BStringItem("    App Exclusions");
// TODO impliment exclusions
//	fPrefsListView->AddItem(fLExclusionsSI);


	BFont font;
	fPrefsListView->GetFont(&font);
	fPrefsListView->ResizeTo(fDaemonBLI->GetWidth(&font) + 10, fPrefsListView->Frame().Height());
	BScrollView *prefsScrollView = new BScrollView("List Scrollview", fPrefsListView,
		B_FOLLOW_TOP_BOTTOM);
	fMainView->AddChild(prefsScrollView);


	//Empty settings view
	viewRect.Set(prefsScrollView->Frame().right + BORDER_SIZE, BORDER_SIZE,
					Bounds().right - BORDER_SIZE, Bounds().bottom - BORDER_SIZE);
	fEmptySettingsView = new BView(viewRect, "Empty SettingsView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fEmptySettingsView->SetViewColor(bg_color);
	BBox *box = new BBox(BRect(0,0,fEmptySettingsView->Bounds().right,
				fEmptySettingsView->Bounds().bottom), "Empty Box", B_FOLLOW_ALL_SIDES);
	box->SetLabel("About Einsteinium Prefs");
	// TODO add about text
	fEmptySettingsView->AddChild(box);
	fMainView->AddChild(fEmptySettingsView);
	fEmptySettingsView->Hide();

	//Relaunch Apps View
	fAppLaunchView = new RelaunchSettingsView(viewRect);
	fMainView->AddChild(fAppLaunchView);
	fAppLaunchView->Hide();
	//Launcher ranking settings
	fLRankingsView = new LauncherRankingsView(viewRect);
	fMainView->AddChild(fLRankingsView);
	fLRankingsView->Hide();
	//Launcher exclusions settings
	fLExclusionsView = new LauncherExclusionsView(viewRect);
	fMainView->AddChild(fLExclusionsView);
	fLExclusionsView->Hide();
	//Deskbar Settings View
	fLDeskbarView = new LauncherDeskbarView(viewRect);
	fMainView->AddChild(fLDeskbarView);
	fLDeskbarView->Hide();
	//Engine Maintenance View
	fMaintenanceView = new EMaintenanceView(viewRect);
	fMainView->AddChild(fMaintenanceView);
	fMaintenanceView->Hide();

	fCurrentView = fEmptySettingsView;
	fCurrentView->Show();

	// resize main window and set min size based on the min sizes of each view
	float minHeight=0, minWidth=0;
	BSize minSize = fAppLaunchView->GetMinSize();
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
	fMaintenanceView->ResizeStatusBox();
}


void
prefsWindow::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case PREFS_ITEM_CHANGED: {
			Lock();
			fCurrentView->Hide();
			int32 index = fPrefsListView->CurrentSelection();

			if(index < 0)
				//No selection
				fCurrentView = fEmptySettingsView;
			else {
				//Item selected
				BStringItem *item = (BStringItem*)fPrefsListView->ItemAt(index);
				if(item == fDAppLaunchSI)
					fCurrentView = fAppLaunchView;
				else if(item == fEMaintSI)
					fCurrentView = fMaintenanceView;
				else if(item == fLRankSI)
					fCurrentView = fLRankingsView;
				else if(item == fLExclusionsSI)
					fCurrentView = fLExclusionsView;
				else if(item == fLDeskbarSI)
					fCurrentView = fLDeskbarView;
				else
					// If one of the BitmapListItems was selected, selected next ListItem
					fPrefsListView->Select(index+1);
			}

			fCurrentView->Show();
			Unlock();
			break; }
		// TODO revisit these
		case E_RECALC_SCORES: {
			BMessenger messenger(e_engine_sig);
			BMessage msg(E_UPDATE_SCORES);
			messenger.SendMessage(&msg);
			break; }
		case E_RECALC_QUARTS: {
			BMessenger messenger(e_engine_sig);
			BMessage msg(E_UPDATE_QUARTILES);
			messenger.SendMessage(&msg);
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
			// TODO detect if engine is running first
			BMessenger messenger(e_engine_sig);
			// TODO change to be specific for the launcher
			BMessage msg(E_UPDATE_SCORES);
//			messenger.SendMessage(&msg);
			break; }
		// Save exclusion settings
		case EL_LIST_INCLUSION_CHANGED: {
			_WriteLauncherListInclusionSetting();
			break; }
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
		default: {
			fAppLaunchView->MessageReceived(msg);
			fLRankingsView->MessageReceived(msg);
//			fLDeskbarView->MessageReceived(msg);
			BWindow::MessageReceived(msg);
			break; }
	}
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
	fLExclusionsView->SetLinkInclusionDefault(fLauncherSettings->GetLinkInclusionDefaultValue());

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


void
prefsWindow::_WriteLauncherListInclusionSetting()
{
	BString inclusionValue;
	fLExclusionsView->GetLinkInclusionDefault(inclusionValue);
	fLauncherSettings->SaveLinkInclusionDefaultValue(inclusionValue.String());
}

void
prefsWindow::_WriteLauncherExclusions()
{
	BMessage exclusionsList(EL_MESSAGE_WHAT_EXCLUDED_APPS);
	fLExclusionsView->GetExclusionsList(exclusionsList);
	fLauncherSettings->SaveExclusionsList(exclusionsList);
}

