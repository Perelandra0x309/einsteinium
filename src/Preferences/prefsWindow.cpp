/*prefsWindow.cpp
	Create the main window, restore and save settings
*/
#include "prefsWindow.h"

prefsWindow::prefsWindow(BRect size)
	:
	BWindow(size, "Einsteinium Preferences", B_TITLED_WINDOW, B_NOT_ZOOMABLE)
{
	//Defaults
	fEnginePrefs.launches_scale = 5;
	fEnginePrefs.first_launch_scale = 5;
	fEnginePrefs.last_launch_scale = 5;
	fEnginePrefs.interval_scale = 5;
	fEnginePrefs.total_run_time_scale = 5;

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
	fAppLaunchSI = new BStringItem("    App Relaunch");
	fPrefsListView->AddItem(fAppLaunchSI);
	//Engine settings
	fEngineBLI = new BitmapListItem(e_engine_sig, "Engine Settings");
	fPrefsListView->AddItem(fEngineBLI);
	fAttrSI = new BStringItem("    List Inclusion");
	fPrefsListView->AddItem(fAttrSI);
	fRankSI = new BStringItem("    App Rankings");
	fPrefsListView->AddItem(fRankSI);
	fDeskbarSI = new BStringItem("    Deskbar Menu");
	fPrefsListView->AddItem(fDeskbarSI);
	fMaintSI = new BStringItem("    Maintenance");
	fPrefsListView->AddItem(fMaintSI);

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
	//Recent Apps ranking prefs view
	fRankingView = new RankingSettingsView(viewRect);
	fMainView->AddChild(fRankingView);
	fRankingView->Hide();
	//Attribute settings
	fAttrView = new AttrSettingsView(viewRect);
	fMainView->AddChild(fAttrView);
	fAttrView->Hide();
	//Deskbar Settings View
	fDeskbarView = new DeskbarSettingsView(viewRect);
	fMainView->AddChild(fDeskbarView);
	fDeskbarView->Hide();
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
	minSize = fRankingView->GetMinSize();
	minHeight = max_c(minHeight, minSize.height);
	minWidth = max_c(minWidth, minSize.width);
	minSize = fAttrView->GetMinSize();
	minHeight = max_c(minHeight, minSize.height);
	minWidth = max_c(minWidth, minSize.width);
	float finalMinWidth = fPrefsListView->Frame().right + minWidth + 10;
	float finalMinHeight = minHeight + 10;
	SetSizeLimits(finalMinWidth, B_SIZE_UNLIMITED, finalMinHeight, B_SIZE_UNLIMITED);
	ResizeTo(finalMinWidth, finalMinHeight);

	//Settings
	_ReadSettings();
	Unlock();
	Show();
}


/*prefsWindow::~prefsWindow()
{
}*/


bool
prefsWindow::QuitRequested()
{
	_StoreSettings();
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
				if(item == fAppLaunchSI)
					fCurrentView = fAppLaunchView;
				else if(item == fRankSI)
					fCurrentView = fRankingView;
				else if(item == fAttrSI)
					fCurrentView = fAttrView;
				else if(item == fDeskbarSI)
					fCurrentView = fDeskbarView;
				else if(item == fMaintSI)
					fCurrentView = fMaintenanceView;
				else
					// If one of the BitmapListItems was selected, selected next ListItem
					fPrefsListView->Select(index+1);
			}

			fCurrentView->Show();
			Unlock();
			break; }
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
		// Save rank slider settings and recalculate scores
		case SAVE_RANKING: {
			_WriteEngineSettings();
			// TODO detect if engine is running first
			BMessenger messenger(e_engine_sig);
			BMessage msg(E_UPDATE_SCORES);
			messenger.SendMessage(&msg);
			break; }
		default: {
			fAppLaunchView->MessageReceived(msg);
			fRankingView->MessageReceived(msg);
			fAttrView->MessageReceived(msg);
			fDeskbarView->MessageReceived(msg);
			break; }
	}
}


void
prefsWindow::_StoreSettings()
{
	_WriteEngineSettings();
//	_WriteDaemonSettings();
}


void
prefsWindow::_ReadSettings()
{
	Lock();
	_ReadEngineSettings();
//	_ReadDaemonSettings();
	// TODO watch settings files for updates
	// TODO create one instance of settings files to share among views?
	Unlock();
}


void
prefsWindow::_ReadEngineSettings()
{
	EESettingsFile *eeSettings = new EESettingsFile();
	int *scales = eeSettings->GetScales();
	fEnginePrefs.launches_scale = scales[LAUNCH_INDEX];
	fEnginePrefs.first_launch_scale = scales[FIRST_INDEX];
	fEnginePrefs.last_launch_scale = scales[LAST_INDEX];
	fEnginePrefs.interval_scale = scales[INTERVAL_INDEX];
	fEnginePrefs.total_run_time_scale = scales[RUNTIME_INDEX];
	fRankingView->SetSliderValues(fEnginePrefs);

	// Read setting for action when new application is detected
	fAttrView->SetLinkInclusionDefault(eeSettings->GetLinkInclusionDefaultValue());

	// Deskbar settings
	bool show;
	int count;
	eeSettings->GetDeskbarSettings(show, count);
	fDeskbarView->SetDeskbarValues(show, count);

	delete eeSettings;
}


void
prefsWindow::_WriteEngineSettings()
{
	// Rank scale settings
	fRankingView->GetSliderValues(fEnginePrefs);
	EESettingsFile *eeSettings = new EESettingsFile();
	int scales[5];
	scales[LAUNCH_INDEX] = fEnginePrefs.launches_scale;
	scales[FIRST_INDEX] = fEnginePrefs.first_launch_scale;
	scales[LAST_INDEX] = fEnginePrefs.last_launch_scale;
	scales[INTERVAL_INDEX] = fEnginePrefs.interval_scale;
	scales[RUNTIME_INDEX] = fEnginePrefs.total_run_time_scale;
	eeSettings->SaveScales(scales);

	delete eeSettings;
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

