/* LPopUpMenu.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */

#include "LPopUpMenu.h"

LPopUpMenu::LPopUpMenu(const char* name)
	:BPopUpMenu(name, false, false),
	fIsShowing(false)
{

}

void
LPopUpMenu::AttachedToWindow()
{
	fIsShowing = true;
	BPopUpMenu::AttachedToWindow();
}

void
LPopUpMenu::Hide()
{
	fIsShowing = false;
	BPopUpMenu::Hide();
}
