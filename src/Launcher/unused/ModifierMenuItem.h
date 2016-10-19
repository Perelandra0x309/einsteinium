/* ModifierMenuItem.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_MODIFIERMENUITEM_H
#define EINSTEINIUM_LAUNCHER_MODIFIERMENUITEM_H


#include "IconMenuItem.h"
#include "launcher_constants.h"

class ModifierMenuItem : public IconMenuItem
{
public:
						ModifierMenuItem(const char *label, BMessage *message,
										const BNodeInfo *nodeInfo, icon_size which);
	virtual void		DrawContent();
	virtual status_t	Invoke(BMessage* message = NULL);
	void				ResetModifiers();
private:
	BString				fLabel, fRemoveLabel;
	uint32				fLastModifier;
	bool				fRemoveEnabled;
};


#endif
