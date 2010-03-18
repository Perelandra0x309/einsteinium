/*EESettingsFile.h

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
	void					ReadSettingsFromFile(BPath file);
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
	BPath					settingsPath;
	node_ref				settingsNodeRef;//node_ref to watch for changes to settings file
	BLooper					*watchingLooper;
	bool					watchingSettingsNode;//true when settings file is being watched
	status_t				_status;
	// Engine settings that can change
	int						launch_scale, first_scale, last_scale, interval_scale, runtime_scale;
	BString					inclusionDefault;
	bool					showDeskbarMenu;
	int						deskbarMenuCount;

	void					StartWatching();
	void					StopWatching();
	int						xmlGetIntProp(xmlNodePtr cur, char *name);
	status_t				WriteSettingsToFile();
};


#endif
