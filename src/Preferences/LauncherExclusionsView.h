/* LauncherExclusionsView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_EXCLUSIONS_VIEW
#define EP_L_EXCLUSIONS_VIEW

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <Roster.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <SpaceLayoutItem.h>
#include "AppRefFilter.h"
#include "launcher_constants.h"
#include "prefs_constants.h"
#include "EngineSubscriber.h"


class ExcludeItem;

class LauncherExclusionsView : public BView {
public:
					LauncherExclusionsView(BRect);
					~LauncherExclusionsView();
//	virtual void	MessageReceived(BMessage*);
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
	void			_RebuildExclusionsList(BMessage &exclusionsList);
	void			_EmptyExclusionsList();
};

class ExcludeItem : public BListItem {
public:
					ExcludeItem(const char *sig, const char *name);
	int				ICompare(ExcludeItem*);
private:
	BString			fAppSig, fAppName;
	void			DrawItem(BView*, BRect, bool);
	friend class LauncherExclusionsView;
};

int SortExcludeItems(const void*, const void*);


#endif
