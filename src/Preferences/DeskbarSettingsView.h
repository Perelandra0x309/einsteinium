/*DeskbarSettingsView.h

*/
#ifndef EP_DESKBARSETTINGS_VIEW
#define EP_DESKBARSETTINGS_VIEW
#include <InterfaceKit.h>
#include <AppKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include "prefs_constants.h"
#include "engine_constants.h"
#include "EESettingsFile.h"
#include <stdlib.h>

class DeskbarSettingsView : public BView
{public:
					DeskbarSettingsView(BRect);
//					~DeskbarSettingsView();
	virtual void	MessageReceived(BMessage*);
	void			SetDeskbarValues(bool show, int count);
private:
	BCheckBox		*showDeskbarCB;
	BTextControl	*itemCountTC;
	BBox			*deskbarBox;
};

#endif
