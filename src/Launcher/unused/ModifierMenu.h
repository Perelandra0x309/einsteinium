/* ModifierMenu.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_MODIFIERMENU_H
#define EINSTEINIUM_LAUNCHER_MODIFIERMENU_H

#include <PopUpMenu.h>
#include "ModifierMenuItem.h"


class ModifierMenu : public BPopUpMenu
{
public:
						ModifierMenu(const char* title,
									bool radioMode = true,
									bool autoRename = true,
									menu_layout layout = B_ITEMS_IN_COLUMN);
	virtual void		MessageReceived(BMessage* message);
	virtual void		Show();
	void				SetRefCount(int32 count) { fRefMenuItemCount = count; }
private:
	int					fRefMenuItemCount;
};


#endif
