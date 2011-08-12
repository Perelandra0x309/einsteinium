/* EDSettingsFile.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ED_SETTINGSFILE_H
#define ED_SETTINGSFILE_H

#include <Handler.h>
#include <List.h>
#include <Looper.h>
#include <StorageKit.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "AppRelaunchSettings.h"
#include "daemon_constants.h"

class EDSettingsFile : public BHandler {
public:
							EDSettingsFile(BHandler *messageHandler=NULL);
							~EDSettingsFile();
	void					UpdateDefaultAction(int _relaunch);
	void					UpdateActionForApp(const char *_signature, int _relaunch);
	void					RemoveApp(const char *_signature);
	AppRelaunchSettings*	FindSettingsForApp(const char *sig);
	status_t				CheckInitStatus() { return fInitStatus; }
	int						GetDefaultRelaunchAction() { return fDefaultRelaunchAction; }
	BList					GetSettingsList() { return fSettingsList; }
	virtual void			MessageReceived(BMessage*);//received BMessages
private:
	BList					fSettingsList;
	BPath					fSettingsPath;
	node_ref				fSettingsNodeRef;//node_ref to watch for changes to settings file
	BLooper					*fWatchingLooper;
	bool					fWatchingSettingsNode;//true when settings file is being watched
	BHandler				*fExternalMessageHandler;
	int						fDefaultRelaunchAction;
	status_t				fInitStatus;
	void					_StartWatching();
	void					_StopWatching();
	status_t				_ReadSettingsFromFile(BPath file);
	void					_ParseSettings(xmlDocPtr doc, xmlNodePtr cur);
	int						_TranslateRelaunchXML(xmlChar *value);
	status_t				_WriteSettingsToFile(BPath file);
};


template < class itemType >
void
DeleteList(BList& list, itemType* item)//delete all items in list
{	do
	{	item = static_cast<itemType *>(list.RemoveItem(int32(0)));
		delete item;
	}while(item);
}

#endif
