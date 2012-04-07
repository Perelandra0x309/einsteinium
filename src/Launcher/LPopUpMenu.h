/* LPopUpMenu.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_LPOPUPMENU_H
#define EINSTEINIUM_LAUNCHER_LPOPUPMENU_H

#include <InterfaceKit.h>

class LPopUpMenu : public BPopUpMenu {
public:
					LPopUpMenu(const char* name);
	virtual void	AttachedToWindow();
	virtual void	Hide();
	bool			IsShowing() { return fIsShowing; }

private:
	bool			fIsShowing;
};

#endif
