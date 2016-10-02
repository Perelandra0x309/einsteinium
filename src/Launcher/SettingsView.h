/* SettingsView.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_SETTINGSVIEW_H
#define EINSTEINIUM_LAUNCHER_SETTINGSVIEW_H

#include <InterfaceKit.h>
#include <Application.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <stdio.h>
#include <stdlib.h>
#include "AppSettings.h"
#include "launcher_constants.h"

class AppSettings;

class SettingsView : public BView {
public:
							SettingsView(BRect size, AppSettings* settings);
	virtual void			AttachedToWindow();
	virtual void			MessageReceived(BMessage*);
	virtual void			KeyDown(const char* bytes, int32 numbytes);
//			bool			GetDrawTwoLines();
			uint			GetAppCount();
			void			SetAppCount(int value);
			int				GetMinIconSize();
			void			SetMinIconSize(int value);
			int				GetMaxIconSize();
			void			SetMaxIconSize(int value);
			int				GetDocIconSize();
			void			SetDocIconSize(int value);
			uint			GetRecentDocCount();
			void			SetRecentDocCount(int value);
			uint			GetRecentFolderCount();
			void			SetRecentFolderCount(int value);
			uint			GetRecentQueryCount();
			void			SetRecentQueryCount(int value);
			float			GetFontSize();
			window_look		GetWindowLook();
			bool			GetShowDeskbarMenu();
			void			SetShowDeskbarMenu(bool value);
//			window_feel		GetFloat();
			void			PopulateAppSettings(AppSettings *settings);
			void			SetAppSettings(AppSettings* settings);
//			void			GetSettingsToSave(BMessage*);
//			void			SetSavedSettings(BMessage*);
private:
//	BRadioButton			*fOneLineRB, *fTwoLinesRB;
	BSlider					*fMaxIconsizeS, *fMinIconsizeS, *fDocIconsizeS;
	BPopUpMenu				*fFontSizeMenu, *fWindowLookMenu;
	BMenuField				*fFontSizeMF, *fWindowLookMF;
	BTextControl			*fAppsCountTC, *fRecentFilesCountTC, *fRecentFoldersCountTC, *fRecentQueriesCountTC;
	BCheckBox				*fDeskbarShowCB;
//	BCheckBox				*fFloatCB;
//	BButton					*fAboutB;
	void					_SetFontSize(float size);
	void					_SetWindowLook(window_look look);
	int						_IconSizeForSliderValue(int value);
	int						_SliderValueForIconSize(int value);
	void					_SetMaxIconLabel(int value);
	void					_SetMinIconLabel(int value);
	void					_SetDocIconLabel(int value);
};

#endif
