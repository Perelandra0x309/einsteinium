/* ModifierMenuItem.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "ModifierMenuItem.h"

ModifierMenuItem::ModifierMenuItem(const char *label, BMessage *message,
											const BNodeInfo *nodeInfo, icon_size which)
	:IconMenuItem(label, message, nodeInfo, which),
	fLabel(label),
	fRemoveLabel(label),
	fLastModifier(0),
	fRemoveEnabled(false)
{
	fRemoveLabel.Prepend("Remove ");
}


void
ModifierMenuItem::DrawContent()
{
	uint32 modifier = modifiers();
	if(modifier != fLastModifier )
	{
		bool command_key = modifier & B_COMMAND_KEY;
		if( command_key )
		{
			//SetLabel(fRemoveLabel);
			BString truncLabel(fRemoveLabel);
			float width = Frame().Width()-50; //(ContentLocation().x + 39);
			BFont font;
			font.TruncateString(&truncLabel, B_TRUNCATE_END, width);
			SetLabel(truncLabel);
			fRemoveEnabled = true;
		}
		else
		{
			SetLabel(fLabel);
			fRemoveEnabled = false;
		}
		fLastModifier = modifier;
	}
	IconMenuItem::DrawContent();
}


status_t
ModifierMenuItem::Invoke(BMessage* message = NULL)
{
	// Add the remove option parameter to the message
	if(!message)
		message = Message();
	BMessage clone(*message);
	clone.AddBool(EL_REMOVE_APPLICATION, fRemoveEnabled);
	return IconMenuItem::Invoke(&clone);
}


void
ModifierMenuItem::ResetModifiers()
{
	fLastModifier=0;
	fRemoveEnabled=false;
	SetLabel(fLabel);
}
