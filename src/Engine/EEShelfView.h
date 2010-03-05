/*
 */
#ifndef EE_SHELF_VIEW_H
#define EE_SHELF_VIEW_H


#include <InterfaceKit.h>
#include <Application.h>
#include <StorageKit.h>
#include <Roster.h>
#include <Deskbar.h>
#include <Resources.h>
#include "IconMenuItem.h"
#include "EsIcons.h"
#include "IconUtils.h"
#include "engine_constants.h"

#include <stdio.h>

class _EXPORT EEShelfView : public BView {
public:
						EEShelfView(BRect frame);
						EEShelfView(BMessage* data);
	virtual				~EEShelfView();

	virtual void		Draw(BRect rect);
	virtual void		AttachedToWindow();
	static EEShelfView*	Instantiate(BMessage* data);
	virtual	status_t	Archive(BMessage* data, bool deep = true) const;
	virtual void	 	MouseDown(BPoint);
	virtual void		MessageReceived(BMessage* message);
//	virtual void		Pulse();

private:
	BPopUpMenu*			_BuildMenu(BMessage *message);
	BBitmap*			fIcon;
	BPopUpMenu*			fMenu;
};

#endif	/* EE_SHELF_VIEW_H */
