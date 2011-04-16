/* LauncherSettingsFile.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_SETTINGSFILE_H
#define EINSTEINIUM_LAUNCHER_SETTINGSFILE_H

#include <StorageKit.h>
#include <Handler.h>
#include <Looper.h>
#include <String.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "launcher_constants.h"
#include "EngineSubscriber.h"

class LauncherSettingsFile : public BHandler
{
public:
							LauncherSettingsFile(BHandler *messageHandler=NULL);
							~LauncherSettingsFile();
	virtual void			MessageReceived(BMessage*);//received BMessages
	status_t				CheckStatus() { return fStatus; }
	int*					GetScales();
	void					SaveScales(int*);
//	const char*				GetLinkInclusionDefaultValue();
//	void					SaveLinkInclusionDefaultValue(const char*);
	int						GetDeskbarCount() { return fDeskbarMenuCount; }
	void					SaveDeskbarCount(int count);
	BMessage				GetExclusionsList() { return fExclusionsList; }
	void					SaveExclusionsList(BMessage &exclusionsList);
	bool					GetEngineAutoLaunch() { return fLaunchEngineOnStart; }
	void					SaveEngineAutoLaunch(bool autoLaunch);
	int						scales[5];
private:
	BPath					fSettingsPath;
	node_ref				fSettingsNodeRef;//node_ref to watch for changes to settings file
	BLooper					*fWatchingLooper;
	bool					fWatchingSettingsNode;//true when settings file is being watched
	status_t				fStatus;
	BMessage				fExclusionsList;
	BHandler				*fExternalMessageHandler;
	// Engine settings that can change
	int						fLaunchScale, fFirstScale, fLastScale, fIntervalScale, fRuntimeScale;
//	BString					fInclusionDefault;
	int						fDeskbarMenuCount;
	bool					fLaunchEngineOnStart;

	void					_StartWatching();
	void					_StopWatching();
	int						_XmlGetIntProp(xmlNodePtr cur, char *name);
	void					_ReadSettingsFromFile(BPath file);
	void					_ParseExclusionSettings(xmlDocPtr doc, xmlNodePtr cur);
	status_t				_WriteSettingsToFile();
};


#endif
