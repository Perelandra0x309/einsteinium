/*EMaintenanceView.h
	Definitions and objects for the view to change settings for calculation
	of app rank in the engine
*/
#ifndef EP_EMAINTENANCE_VIEW
#define EP_EMAINTENANCE_VIEW
#include <InterfaceKit.h>
#include <fs_attr.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include "prefs_constants.h"
#include "SystemStatusBox.h"
#include "engine_constants.h"
#include "methods.h"

class EMaintenanceView : public BView
{public:
					EMaintenanceView(BRect);
					~EMaintenanceView();
	virtual void	AllAttached();
	virtual void	MessageReceived(BMessage*);
	void			ResizeStatusBox();
private:
	bool			watchingRoster;
	BButton			*ranksB, *quartilesB, *dataB;
	BTextView		*ranksTV, *quartilesTV, *dataTV;
	BBox			*rankBox;
	SystemStatusBox *statusBox;
	void			SetButtonEnabledState();
};

#endif
