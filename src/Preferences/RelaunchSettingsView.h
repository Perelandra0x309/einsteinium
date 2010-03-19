/*RelaunchSettingsView
	Definitions and objects for the view containing settings for relaunching
	apps with the daemon
*/
#ifndef EP_RELAUNCH_VIEW
#define EP_RELAUNCH_VIEW
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <StorageKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <fstream>
#include "prefs_constants.h"
#include "AppRefFilter.h"
#include "EDSettingsFile.h"

class RelaunchAppItem;
class RelaunchSettingsView : public BView
{public:
					RelaunchSettingsView(BRect);
					~RelaunchSettingsView();
	virtual void	MessageReceived(BMessage*);
//	void			WriteSettings();
	void			ReadSettings();
	BSize			GetMinSize();
private:
	AppRefFilter	*appFilter;
	BFilePanel		*appsPanel;
	EDSettingsFile	*edSettings;
	AppRelaunchSettings *defaultSettings;
	RelaunchAppItem	*selectedItem;
	BListView		*appsLView;
	BScrollView		*appsSView;
	BButton			*addAppB, *removeAppB;
	BBox			*relaunchBox;
	BRadioButton	*autoRelaunchRB, *promptRelaunchRB, *dontRelaunchRB;
	void			saveSelectedItemSettings();
	void			clearItemSettings();
	void			updateSelectedItem();
	void			recallItemSettings();
};

class RelaunchAppItem : public BListItem
{public:
					RelaunchAppItem(const char*, BPath);
					RelaunchAppItem(AppRelaunchSettings*);
					~RelaunchAppItem();
	int				ICompare(RelaunchAppItem*);
private:
	AppRelaunchSettings *settings;
	void			DrawItem(BView*, BRect, bool);
//	void			Update(BView *owner, const BFont *font);
//	virtual bool	InitiateDrag(BPoint point, int32 index, bool wasSelected);
	friend class RelaunchSettingsView;
};

	int				SortRelaunchAppItems(const void*, const void*);
#endif
