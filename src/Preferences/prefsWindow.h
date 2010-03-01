/*prefsWindow.h
	definitions and objects for the main window.
*/
#ifndef EP_WINDOW_H
#define EP_WINDOW_H
#include <InterfaceKit.h>
#include "engine_constants.h"
#include "RelaunchSettingsView.h"
#include "RankingSettingsView.h"
#include "AttrSettingsView.h"
#include "EMaintenanceView.h"
#include "BitmapListItem.h"
#include "EESettingsFile.h"


class prefsWindow : public BWindow
{
public:
							prefsWindow(BRect);
							~prefsWindow();
	virtual	bool			QuitRequested();
//	virtual void			FrameResized(float width, float height);
	virtual void			MessageReceived(BMessage*);
private:
	engine_prefs			e_prefs;
	BView					*mainView, *emptySettingsView, *curView;
	BListView				*prefsListView;
	BitmapListItem			*daemonBLI, *engineBLI;
	BStringItem				*appLaunchSI, *rankSI, *attrSI, *maintSI;
	RelaunchSettingsView	*appLaunchView;
	RankingSettingsView		*rankingView;
	AttrSettingsView		*attrView;
	EMaintenanceView		*e_maintenanceView;
	BitmapListItem*			CreateDaemonBLI();
	BitmapListItem*			CreateEngineBLI();
	void					storeSettings();
	void					readSettings();
	void					readEngineSettings();
	void					writeEngineSettings();
	void					readDaemonSettings();
	void					writeDaemonSettings();
};

#endif