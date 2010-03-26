/* RelaunchSettingsView
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_RELAUNCH_VIEW
#define EP_RELAUNCH_VIEW

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <fstream>
#include "AppRefFilter.h"
#include "EDSettingsFile.h"
#include "prefs_constants.h"


class RelaunchAppItem;

class RelaunchSettingsView : public BView {
public:
					RelaunchSettingsView(BRect);
					~RelaunchSettingsView();
	virtual void	MessageReceived(BMessage*);
//	void			WriteSettings();
	void			ReadSettings();
	BSize			GetMinSize();
private:
	AppRefFilter	*fAppFilter;
	BFilePanel		*fAppsPanel;
	EDSettingsFile	*fDaemonSettings;
	AppRelaunchSettings *fDefaultSettings;
	RelaunchAppItem	*fSelectedItem;
	BListView		*fAppsLView;
	BScrollView		*fAppsSView;
	BButton			*fAddAppB, *fRemoveAppB;
	BBox			*fRelaunchBox;
	BRadioButton	*fAutoRelaunchRB, *fPromptRelaunchRB, *fDontRelaunchRB;
	void			_SaveSelectedItemSettings();
	void			_ClearItemSettings();
	void			_UpdateSelectedItem();
	void			_RecallItemSettings();
};

class RelaunchAppItem : public BListItem {
public:
					RelaunchAppItem(const char*, BPath);
					RelaunchAppItem(AppRelaunchSettings*);
					~RelaunchAppItem();
	virtual void	DrawItem(BView*, BRect, bool);
	int				ICompare(RelaunchAppItem*);
private:
	AppRelaunchSettings *fSettings;
//	void			Update(BView *owner, const BFont *font);
//	virtual bool	InitiateDrag(BPoint point, int32 index, bool wasSelected);
	friend class RelaunchSettingsView;
};

int SortRelaunchAppItems(const void*, const void*);

#endif
