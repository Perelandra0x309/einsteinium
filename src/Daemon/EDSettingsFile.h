/*EDSettingsFile.h

*/
#ifndef ED_SETTINGSFILE_H
#define ED_SETTINGSFILE_H

#include <StorageKit.h>
#include <Handler.h>
#include <Looper.h>
#include <List.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "daemon_constants.h"
#include "AppRelaunchSettings.h"

class EDSettingsFile : public BLooper
{
public:
							EDSettingsFile();
							~EDSettingsFile();
	void					SaveSettings(BList*, int);
	void					UpdateActionForApp(const char *_signature, const char *_relaunch);
	AppRelaunchSettings*	FindSettingsForApp(const char *sig);
	status_t				CheckInitStatus() { return _initStatus; }
	int						GetDefaultRelaunchAction() { return defaultRelaunchAction; }
	BList					GetSettingsList() { return settingsList; }
	virtual void			MessageReceived(BMessage*);//received BMessages
private:
	BList					settingsList;
	BPath					settingsPath;
	node_ref				settingsNodeRef;//node_ref to watch for changes to settings file
	bool					watchingSettingsNode;//true when settings file is being watched
	int						defaultRelaunchAction;
	status_t				_initStatus;
	void					StartWatching();
	void					StopWatching();
	status_t				ReadSettingsFromFile(BPath file);
	void					parseSettings(xmlDocPtr doc, xmlNodePtr cur);
	status_t				WriteSettingsToFile(BPath file);
};


template < class itemType >
void deleteList(BList& list, itemType* item)//delete all items in list
{	do
	{	item = static_cast<itemType *>(list.RemoveItem(int32(0)));
		delete item;
	}while(item);
	return;
}

#endif
