/* LauncherExclusionsView.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_EXCLUSIONS_VIEW
#define EP_L_EXCLUSIONS_VIEW

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <Roster.h>
#include <LayoutBuilder.h>
#include <GroupLayoutBuilder.h>
#include <Catalog.h>
#include "AppRefFilter.h"
#include "launcher_constants.h"
#include "EngineSubscriber.h"


class ExcludeItem;

class LauncherExclusionsView : public BView {
public:
					LauncherExclusionsView(BRect, BMessage *appExclusions);
					~LauncherExclusionsView();
	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage*);
	virtual void	FrameResized(float width, float height);
	bool			AddExclusion(BMessage* refMsg);
	bool			RemoveSelectedExclusion();
	void			UpdateSelectedItem();
//	void			SetLinkInclusionDefault(const char*);
//	void			GetLinkInclusionDefault(BString &value);
	void			PopulateExclusionsList(BMessage &exclusionsList);
	void			GetExclusionsList(BMessage &list);
//	BSize			GetMinSize();
private:
	BBox			/**fDefaultSettingsBox, */*fSettingsBox;
	BButton			*fAddB, *fRemoveB;
//	BRadioButton	*fPromptRB, *fIncludeRB, *fIgnoreRB;
	BListView		*fExclusionLView;
	BScrollView		*fExclusionSView;
	ExcludeItem		*fSelectedItem;
	AppRefFilter	*fAppFilter;
	BFilePanel		*fAppsPanel;
	void			_RebuildExclusionsList(BMessage &exclusionsList);
	void			_EmptyExclusionsList();
};

class ExcludeItem : public BListItem {
public:
					ExcludeItem(entry_ref srcRef, const char* sig);
					~ExcludeItem();
	int				ICompare(ExcludeItem*);
private:
	BString			fAppSig, fAppName;
	entry_ref		fAppRef;
	int32			fIconSize;
	BBitmap*		fIcon;
	void			DrawItem(BView*, BRect, bool);
	friend class LauncherExclusionsView;
};

int SortExcludeItems(const void*, const void*);


#endif
