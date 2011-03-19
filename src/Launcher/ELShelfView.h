/* ELShelfView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EL_SHELF_VIEW_H
#define EL_SHELF_VIEW_H


#include <Application.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Deskbar.h>
#include <Resources.h>
#include <Roster.h>
#include <stdio.h>

#include "EESettingsFile.h"
#include "EngineSubscriber.h"
#include "IconMenuItem.h"
#include "launcher_constants.h"


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
//	virtual void			Pulse();

private:
	BBitmap*				fIcon;
	BPopUpMenu*				fMenu;
	int						fItemCount;
	EESettingsFile			*fSettingsFile;
	void					_BuildMenu(BMessage *message);
	void					_Quit();
	virtual void			_SubscribeFailed();
	virtual void			_SubscribeConfirmed();
	virtual void			_UpdateReceived(BMessage *message);
};

#endif	/* EL_SHELF_VIEW_H */
