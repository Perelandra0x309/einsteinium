/* ModifierMenu.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "ModifierMenu.h"

ModifierMenu::ModifierMenu(const char* title,
									bool radioMode = true,
									bool autoRename = true,
									menu_layout layout = B_ITEMS_IN_COLUMN)
	:BPopUpMenu(title, radioMode, autoRename, layout),
	fRefMenuItemCount(0)
{
}


void
ModifierMenu::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_MODIFIERS_CHANGED:
		{
			// Invalidate the layout to force all MenuItem objects to update
			// when a modifier key is pressed or released
			InvalidateLayout();
			break;
		}
	}
	BPopUpMenu::MessageReceived(message);
}


void
ModifierMenu::Show()
{
	// Reset the menu items to a starting state
	ModifierMenuItem *item;
	for(int i=0; i<fRefMenuItemCount; i++)
	{
		item = (ModifierMenuItem*)ItemAt(i);
		item->ResetModifiers();
	}
	BPopUpMenu::Show();
}
