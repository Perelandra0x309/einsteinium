/* AttrSettingsView.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_ATTR_VIEW
#define EP_ATTR_VIEW

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <Roster.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <SpaceLayoutItem.h>
#include <fs_attr.h>
#include "AppRefFilter.h"
#include "EESettingsFile.h"
#include "engine_constants.h"
#include "prefs_constants.h"


class AppAttrItem;

class AttrSettingsView : public BView {
public:
					AttrSettingsView(BRect);
					~AttrSettingsView();
	virtual void	MessageReceived(BMessage*);
	void			SetLinkInclusionDefault(const char*);
	BSize			GetMinSize();
private:
	BPath			fSettingsPath;
	AppRefFilter	*fAppFilter;
	BFilePanel		*fAppsPanel;
	BBox			*fDefaultSettingsBox, *fSettingsBox;
	BButton			*fAddB, *fRemoveB;
	BCheckBox		*fIgnoreCB;
	BRadioButton	*fPromptRB, *fIncludeRB, *fIgnoreRB;
	BListView		*fAttrLView;
	BScrollView		*fAttrSView;
	AppAttrItem		*fSelectedItem;
	void			_RebuildAttrList();
	void			_EmptyAttrList();
	void			_SaveChangedItem(AppAttrItem *item);
	void			_SaveItemSettings();
	void			_ClearItemSettings();
	void			_UpdateSelectedItem();
	void			_RecallItemSettings();
	void			_SaveIncludeSetting(const char*);
};

class AppAttrItem : public BListItem {
public:
					AppAttrItem(const char*, BPath&);
//					~AppAttrItem();
	int				ICompare(AppAttrItem*);
private:
	bool			fIgnore;
	BString			fAttrPath, fAppSig, fAppName;
	void			DrawItem(BView*, BRect, bool);
//	void			Update(BView *owner, const BFont *font);
//	virtual bool	InitiateDrag(BPoint point, int32 index, bool wasSelected);
	friend class AttrSettingsView;
};

int SortAppAttrItems(const void*, const void*);

#endif
