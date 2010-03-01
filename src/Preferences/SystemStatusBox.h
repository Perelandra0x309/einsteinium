/*SystemStatusBox.h

*/
#ifndef EP_SYSTEMSTATUS_BOX
#define EP_SYSTEMSTATUS_BOX

#include <InterfaceKit.h>
#include <Roster.h>
#include <Path.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include "prefs_constants.h"

enum {
	STATE_NONE = 0,
	STATE_OK_RUNNING,
	STATE_OK_STOPPING,
	STATE_OK_STOPPED,
	STATE_OK_STARTING,
	STATE_OK_RESTARTING,
	STATE_ERROR_STOPPING__IS_RUNNING,
	STATE_ERROR_STOPPING__IS_NOT_RUNNING,
	STATE_ERROR_STARTING__IS_RUNNING,
	STATE_ERROR_STARTING__IS_NOT_RUNNING
};

class SystemStatusBox : public BBox
{
public:
							SystemStatusBox(BEntry entry, const char * sig);
							~SystemStatusBox();
			void			AttachedToWindow();
	virtual void			MessageReceived(BMessage*);
	virtual	void			Draw(BRect updateRect){ BBox::Draw(updateRect); }
private:
	BStringView				*statusSV;
	BString					signature, name;
	BButton					*restartB, *startstopB;
	int						currentState;
	status_t				StopService();
	status_t				StartService();
	void					SetStatusText(const char*);
	void					GetRunningState();
	void					SetState(int);

};

#endif
