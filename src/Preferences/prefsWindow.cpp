/*prefsWindow.cpp
	Create the main window, restore and save settings
*/
#include "prefsWindow.h"

prefsWindow::prefsWindow(BRect size)
	:BWindow(size, "Einsteinium Preferences", B_TITLED_WINDOW, B_NOT_ZOOMABLE)
{
	//Defaults
	e_prefs.launches_scale = 5;
	e_prefs.first_launch_scale = 5;
	e_prefs.last_launch_scale = 5;
	e_prefs.interval_scale = 5;
	e_prefs.total_run_time_scale = 5;


	Lock();
	BRect viewRect(Bounds());
	mainView = new BView(viewRect, "Background View", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	mainView->SetViewColor(bg_color);
	AddChild(mainView);
	//Prefs list view
	viewRect.Set(BORDER_SIZE, BORDER_SIZE, BORDER_SIZE + 100, Bounds().bottom - BORDER_SIZE);
	prefsListView = new BListView(viewRect, "Preferences", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	prefsListView->SetSelectionMessage(new BMessage(PREFS_ITEM_CHANGED));
	//Daemon settings
	daemonBLI = CreateDaemonBLI();
	prefsListView->AddItem(daemonBLI);
	appLaunchSI = new BStringItem("    App Relaunch");
	prefsListView->AddItem(appLaunchSI);
	//Engine settings
	engineBLI = CreateEngineBLI();
	prefsListView->AddItem(engineBLI);
	attrSI = new BStringItem("    List Inclusion");
	prefsListView->AddItem(attrSI);
	rankSI = new BStringItem("    App Rankings");
	prefsListView->AddItem(rankSI);
	deskbarSI = new BStringItem("    Deskbar Menu");
	prefsListView->AddItem(deskbarSI);
	maintSI = new BStringItem("    Maintenance");
	prefsListView->AddItem(maintSI);

	BFont font;
	prefsListView->GetFont(&font);
	prefsListView->ResizeTo(daemonBLI->GetWidth(&font) + 10, prefsListView->Frame().Height());
	BScrollView *prefsScrollView = new BScrollView("List Scrollview", prefsListView, B_FOLLOW_TOP_BOTTOM);
	mainView->AddChild(prefsScrollView);


	//Empty settings view
	viewRect.Set(prefsScrollView->Frame().right + BORDER_SIZE, BORDER_SIZE,
					Bounds().right - BORDER_SIZE, Bounds().bottom - BORDER_SIZE);

	emptySettingsView = new BView(viewRect, "Empty SettingsView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	emptySettingsView->SetViewColor(bg_color);
	BBox *box = new BBox(BRect(0,0,emptySettingsView->Bounds().right,
				emptySettingsView->Bounds().bottom), "Empty Box", B_FOLLOW_ALL_SIDES);
	box->SetLabel("About Einsteinium Prefs");
	// TODO add about text
	emptySettingsView->AddChild(box);
	mainView->AddChild(emptySettingsView);
	emptySettingsView->Hide();

	//Relaunch Apps View
	appLaunchView = new RelaunchSettingsView(viewRect);
	mainView->AddChild(appLaunchView);
	appLaunchView->Hide();
	//Recent Apps ranking prefs view
	rankingView = new RankingSettingsView(viewRect);
	mainView->AddChild(rankingView);
	rankingView->Hide();
	//Attribute settings
	attrView = new AttrSettingsView(viewRect);
	mainView->AddChild(attrView);
	attrView->Hide();
	//Deskbar Settings View
	deskbarView = new DeskbarSettingsView(viewRect);
	mainView->AddChild(deskbarView);
	deskbarView->Hide();
	//Engine Maintenance View
	e_maintenanceView = new EMaintenanceView(viewRect);
	mainView->AddChild(e_maintenanceView);
	e_maintenanceView->Hide();

	curView = emptySettingsView;
	curView->Show();

	// resize main window and set min size based on the min sizes of each view
	float minHeight=0, minWidth=0;
	BSize minSize = appLaunchView->GetMinSize();
	minHeight = minSize.height;
	minWidth = minSize.width;
	minSize = rankingView->GetMinSize();
	minHeight = max_c(minHeight, minSize.height);
	minWidth = max_c(minWidth, minSize.width);
	minSize = attrView->GetMinSize();
	minHeight = max_c(minHeight, minSize.height);
	minWidth = max_c(minWidth, minSize.width);
	float finalMinWidth = prefsListView->Frame().right + minWidth + 10;
	float finalMinHeight = minHeight + 10;
	SetSizeLimits(finalMinWidth, B_SIZE_UNLIMITED, finalMinHeight, B_SIZE_UNLIMITED);
	ResizeTo(finalMinWidth, finalMinHeight);

	//Settings
	readSettings();
	Unlock();
	Show();
}
prefsWindow::~prefsWindow()
{
}
BitmapListItem* prefsWindow::CreateDaemonBLI()
{
	const int32 kImageWidth = 16;
	const int32 kImageHeight = 16;
	const color_space kImageColorSpace = B_CMAP8;
	BRect iconRect(0, 0, kImageWidth - 1, kImageHeight - 1);

	const uint8 daemonImageBits[] = {
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
	0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f,
	0x1f, 0x1f, 0x0f, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x1f,
	0x1f, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0xd9, 0x0a,
	0x1f, 0x1f, 0x11, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00, 0xab,
	0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x14, 0x00, 0x00, 0x14, 0x1f, 0x00, 0xd9, 0x00,
	0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x14, 0x1f, 0x1f, 0x1f, 0x00, 0xd9, 0xab,
	0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x14, 0x00, 0x00, 0x14, 0x1f, 0x00, 0x5b, 0x00,
	0x1f, 0x1f, 0x0a, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x14, 0x00, 0x1f, 0x00, 0x00, 0x11,
	0x1f, 0x0a, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x14, 0x1f, 0x00, 0x11, 0x1f,
	0x0a, 0x1f, 0x5b, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x1f,
	0x00, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f,
	0x11, 0x00, 0xd9, 0x5b, 0x00, 0xd9, 0x5b, 0x00, 0xd9, 0x5b, 0x00, 0x11, 0x1f, 0x1f, 0x1f, 0x1f,
	0x1f, 0x00, 0x5b, 0x00, 0xd9, 0x5b, 0x00, 0xd9, 0x5b, 0x00, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
	0x1f, 0x1f, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
	};
	BitmapListItem *newBLI = new BitmapListItem(daemonImageBits, iconRect, kImageColorSpace, "Daemon Settings");
	return newBLI;
}
BitmapListItem* prefsWindow::CreateEngineBLI()
{
	const int32 kImageWidth = 16;
	const int32 kImageHeight = 16;
	const color_space kImageColorSpace = B_CMAP8;
	BRect iconRect(0, 0, kImageWidth - 1, kImageHeight - 1);

	const uint8 engineImageBits[] = {
	0x1f, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x1f,
	0x1f, 0x00, 0xae, 0xae, 0x00, 0x00, 0x86, 0x86, 0x00, 0x00, 0x60, 0x60, 0x00, 0x1f, 0x1f, 0x1f,
	0x1f, 0x00, 0xae, 0xae, 0xae, 0x00, 0x86, 0x86, 0x00, 0x60, 0x60, 0x00, 0x00, 0x1f, 0x1f, 0x1f,
	0x00, 0x00, 0x00, 0xae, 0x86, 0x86, 0x60, 0x86, 0x86, 0x86, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f,
	0x00, 0xae, 0xae, 0x86, 0x86, 0x86, 0x00, 0x00, 0x86, 0x60, 0x60, 0x00, 0x00, 0x00, 0x1f, 0x1f,
	0x00, 0xae, 0x86, 0x86, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00,
	0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xae, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00,
	0x0e, 0x00, 0xae, 0x00, 0xae, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x14, 0x00, 0x00, 0x14, 0x1f, 0x00,
	0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x14, 0x1f, 0x1f, 0x1f, 0x00,
	0x1f, 0x1f, 0x00, 0x00, 0x0e, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x14, 0x00, 0x00, 0x14, 0x1f, 0x00,
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x14, 0x00, 0x1f, 0x00,
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x14, 0x1f, 0x00,
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00,
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	BitmapListItem *newBLI = new BitmapListItem(engineImageBits, iconRect, kImageColorSpace, "Engine Settings");
	return newBLI;
}


bool prefsWindow::QuitRequested()
{
	storeSettings();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}


void prefsWindow::FrameResized(float width, float height)
{
	e_maintenanceView->ResizeStatusBox();
}


void prefsWindow::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case PREFS_ITEM_CHANGED: {
			Lock();
			curView->Hide();
			int32 index = prefsListView->CurrentSelection();

			if(index < 0)//No selection
			{	curView = emptySettingsView; }
			else//Item selected
			{	BStringItem *item = (BStringItem*)prefsListView->ItemAt(index);
				if(item == appLaunchSI)
				{	curView = appLaunchView; }
				else if(item == rankSI)
				{	curView = rankingView; }
				else if(item == attrSI)
				{	curView = attrView; }
				else if(item == deskbarSI)
				{	curView = deskbarView; }
				else if(item == maintSI)
				{	curView = e_maintenanceView; }
				else
				{	// If one of the BitmapListItems was selected, selected next ListItem
					prefsListView->Select(index+1);
				}
			}

			curView->Show();
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
			writeEngineSettings();
			// TODO detect if engine is running first
			BMessenger messenger(e_engine_sig);
			BMessage msg(E_UPDATE_SCORES);
			messenger.SendMessage(&msg);
			break; }
		default: {
			appLaunchView->MessageReceived(msg);
			rankingView->MessageReceived(msg);
			attrView->MessageReceived(msg);
			deskbarView->MessageReceived(msg);
			break; }
	}
}
void prefsWindow::storeSettings()
{
	writeEngineSettings();
//	writeDaemonSettings();
}
void prefsWindow::readSettings()
{
	Lock();
	readEngineSettings();
//	readDaemonSettings();
	// TODO watch settings files for updates
	// TODO create one instance of settings files to share among views?
	Unlock();
}
void prefsWindow::readEngineSettings()
{
	EESettingsFile *eeSettings = new EESettingsFile();
	int *scales = eeSettings->GetScales();
	e_prefs.launches_scale = scales[LAUNCH_INDEX];
	e_prefs.first_launch_scale = scales[FIRST_INDEX];
	e_prefs.last_launch_scale = scales[LAST_INDEX];
	e_prefs.interval_scale = scales[INTERVAL_INDEX];
	e_prefs.total_run_time_scale = scales[RUNTIME_INDEX];
	rankingView->setSliderValues(e_prefs);

	// Read setting for action when new application is detected
	attrView->SetLinkInclusionDefault(eeSettings->GetLinkInclusionDefaultValue());

	// Deskbar settings
	bool show;
	int count;
	eeSettings->GetDeskbarSettings(show, count);
	deskbarView->SetDeskbarValues(show, count);

	delete eeSettings;
}
void prefsWindow::writeEngineSettings()
{
	// Rank scale settings
	rankingView->getSliderValues(e_prefs);
	EESettingsFile *eeSettings = new EESettingsFile();
	int scales[5];
	scales[LAUNCH_INDEX] = e_prefs.launches_scale;
	scales[FIRST_INDEX] = e_prefs.first_launch_scale;
	scales[LAST_INDEX] = e_prefs.last_launch_scale;
	scales[INTERVAL_INDEX] = e_prefs.interval_scale;
	scales[RUNTIME_INDEX] = e_prefs.total_run_time_scale;
	eeSettings->SaveScales(scales);

	delete eeSettings;

}
/*void prefsWindow::readDaemonSettings()
{
	appLaunchView->ReadSettings();
}*/
/*void prefsWindow::writeDaemonSettings()
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

