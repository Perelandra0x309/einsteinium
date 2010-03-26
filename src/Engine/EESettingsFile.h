/* EESettingsFile.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EE_SETTINGSFILE_H
#define EE_SETTINGSFILE_H

#include <StorageKit.h>
#include <Handler.h>
#include <Looper.h>
#include <String.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "engine_constants.h"

class EESettingsFile : public BHandler
{
public:
							EESettingsFile();
							~EESettingsFile();
	virtual void			MessageReceived(BMessage*);//received BMessages
	status_t				CheckStatus();
	int*					GetScales();
	void					SaveScales(int*);
	const char*				GetLinkInclusionDefaultValue();
	void					SaveLinkInclusionDefaultValue(const char*);
	void					GetDeskbarSettings(bool &show, int &count);
	void					SaveDeskbarSettings(bool show, int count);

	int						scales[5];
private:
	BPath					fSettingsPath;
	node_ref				fSettingsNodeRef;//node_ref to watch for changes to settings file
	BLooper					*fWatchingLooper;
	bool					fWatchingSettingsNode;//true when settings file is being watched
	status_t				fStatus;
	// Engine settings that can change
	int						fLaunchScale, fFirstScale, fLastScale, fIntervalScale, fRuntimeScale;
	BString					fInclusionDefault;
	bool					fShowDeskbarMenu;
	int						fDeskbarMenuCount;

	void					_StartWatching();
	void					_StopWatching();
	int						_XmlGetIntProp(xmlNodePtr cur, char *name);
	void					_ReadSettingsFromFile(BPath file);
	status_t				_WriteSettingsToFile();
};


#endif
