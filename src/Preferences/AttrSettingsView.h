/*AttrSettingsView.h

*/
#ifndef EP_ATTR_VIEW
#define EP_ATTR_VIEW
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <StorageKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <SpaceLayoutItem.h>
#include <fs_attr.h>
#include "prefs_constants.h"
#include "AppRefFilter.h"
#include "engine_constants.h"
#include "EESettingsFile.h"

class AppAttrItem;
class AttrSettingsView : public BView
{public:
					AttrSettingsView(BRect);
					~AttrSettingsView();
	virtual void	MessageReceived(BMessage*);
	void			SetLinkInclusionDefault(const char*);
	BSize			GetMinSize();
//	void			SaveChanges();
private:
	BPath			settingsPath;
	AppRefFilter	*appFilter;
	BFilePanel		*appsPanel;
	BBox			*defaultSettingsBox, *settingsBox;
	BButton			*addB, *removeB;
	BCheckBox		*ignoreCB;
	BRadioButton	*promptRB, *includeRB, *ignoreRB;
	BListView		*attrLView;
	BScrollView		*attrSView;
	AppAttrItem		*selectedItem;
	void			RebuildAttrList();
	void			EmptyAttrList();
	void			SaveChangedItem(AppAttrItem *item);
	void			saveItemSettings();
	void			clearItemSettings();
	void			updateSelectedItem();
	void			recallItemSettings();
	void			saveIncludeSetting(const char*);
};
class AppAttrItem : public BListItem
{public:
					AppAttrItem(const char*, BPath&);
//					~AppAttrItem();
	int				ICompare(AppAttrItem*);
private:
	bool			/*changed, */s_ignore;
	BString			attrPath, appSig, appName;
	void			DrawItem(BView*, BRect, bool);
//	void			Update(BView *owner, const BFont *font);
//	virtual bool	InitiateDrag(BPoint point, int32 index, bool wasSelected);
	friend class AttrSettingsView;
};

	int				SortAppAttrItems(const void*, const void*);
#endif
