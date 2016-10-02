/* ELShelfView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EL_SHELF_VIEW_H
#define EL_SHELF_VIEW_H


#include <Application.h>
#include <Deskbar.h>
#include <InterfaceKit.h>
#include <MessageRunner.h>
#include <Resources.h>
#include <Roster.h>
#include <StorageKit.h>
#include <stdio.h>

#include "EngineSubscriber.h"
//#include "IconMenuItem.h"
#include "ModifierMenuItem.h"
#include "ModifierMenu.h"
#include "LauncherSettingsFile.h"
#include "launcher_constants.h"


int	MenuItemSortLabel(const void* item1, const void* item2);

class _EXPORT ELShelfView : public BView, public EngineSubscriber {
public:
							ELShelfView();
							ELShelfView(BMessage* data);
	virtual					~ELShelfView();

	virtual void			Draw(BRect rect);
	virtual void			AttachedToWindow();
	virtual void			DetachedFromWindow();
	static ELShelfView*		Instantiate(BMessage* data);
	virtual	status_t		Archive(BMessage* data, bool deep = true) const;
	virtual void	 		MouseDown(BPoint);
	virtual void			MessageReceived(BMessage* message);

private:
	BBitmap					*fIcon;
	BPopUpMenu				*fMenu;
	//ModifierMenu			*fMenu;
	int						fItemCount;
	LauncherSettingsFile	*fSettingsFile;
	BAlert					*fEngineAlert;
	bool					fEngineAlertIsShowing, fWatchingRoster;
	void					_BuildMenu(BMessage *message);
	bool					_CheckEngineStatus(bool showWarning = false);
	void					_CloseEngineAlert();
	void					_Quit();
	void					_Subscribe();
	// virtual functions inherited from the EngineSubscriber class
	virtual void			_SubscribeFailed();
	virtual void			_SubscribeConfirmed();
	virtual void			_UpdateReceived(BMessage *message);
};

#endif	/* EL_SHELF_VIEW_H */
