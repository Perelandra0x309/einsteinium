/* LauncherSettingsFile.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_SETTINGSFILE_H
#define EINSTEINIUM_LAUNCHER_SETTINGSFILE_H

#include <StorageKit.h>
#include <Font.h>
#include <Handler.h>
#include <Looper.h>
#include <String.h>
#include <Window.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "launcher_constants.h"
#include "EngineSubscriber.h"
#include "signatures.h"

class LauncherSettingsFile : public BHandler
{
public:
							LauncherSettingsFile(BHandler *messageHandler=NULL);
							~LauncherSettingsFile();
	virtual void			MessageReceived(BMessage*);//received BMessages
	status_t				CheckStatus() { return fStatus; }
	int*					GetScales();
	void					GetScales(ScaleSettings* settings);
	void					SaveScales(int*);
	void					SaveScales(ScaleSettings scales);
//	const char*				GetLinkInclusionDefaultValue();
//	void					SaveLinkInclusionDefaultValue(const char*);
	int						GetDeskbarCount() { return fAppCount; }
	void					SaveDeskbarCount(int count);
	BMessage				GetExclusionsList() { return fExclusionsList; }
	void					SaveExclusionsList(BMessage &exclusionsList);
	void					AddToExclusionsList(const char *signature, const char *name);
	void					AddToExclusionsList(BMessage *msg);
	bool					GetEngineAutoLaunch() { return fLaunchEngineOnStart; }
	void					SaveEngineAutoLaunch(bool autoLaunch);
	int						GetMinAppIconSize() { return fMinAppIconSize; }
	int						GetMaxAppIconSize(){ return fMaxAppIconSize; }
	void					SaveAppIconSize(int minSize, int maxSize);
	int						GetDocIconSize() { return fDocIconSize; }
	void					SaveDocIconSize(int size);
	uint					GetDocCount() { return fRecentDocCount; }
	void					SaveDocCount(uint count);
	uint					GetFolderCount() { return fRecentFolderCount; }
	void					SaveFolderCount(uint count);
	uint					GetQueryCount() { return fRecentQueryCount; }
	void					SaveQueryCount(uint count);
	float					GetFontSize() { return fFontSize; }
	void					SaveFontSize(float fontSize);
	window_look				GetWindowLook() { return fWindowLook; }
	void					SaveWindowLook(window_look look);
	BRect					GetWindowFrame() { return fWindowFrame; }
	void					SaveWindowFrame(BRect frame);
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
	int						fMinAppIconSize, fMaxAppIconSize, fDocIconSize;
	uint					fRecentDocCount, fRecentFolderCount, fRecentQueryCount;
	float					fFontSize;
	window_look				fWindowLook;
	BRect					fWindowFrame;
//	BString					fInclusionDefault;
	int						fAppCount;
	bool					fLaunchEngineOnStart;

	void					_StartWatching();
	void					_StopWatching();
	int						_XmlGetIntProp(xmlNodePtr cur, char *name, int defaultValue=0);
	void					_ReadSettingsFromFile(BPath file);
	void					_ParseExclusionSettings(xmlDocPtr doc, xmlNodePtr cur);
	status_t				_WriteSettingsToFile();
};


#endif
