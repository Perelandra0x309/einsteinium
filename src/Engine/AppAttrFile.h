/*AppAttrFile.h
	This is a subclass of a BFile which contains all functions for
	initializing, reading, changing, and writing the attributes
	for each application which is monitored
*/
#ifndef EINSTEINIUM_ENGINE_APPATTRFILE_H
#define EINSTEINIUM_ENGINE_APPATTRFILE_H


#include <StorageKit.h>
#include <SupportKit.h>
#include <Alert.h>
#include <stdlib.h>
#include <iostream.h>
#include <fs_attr.h>
#include <fstream>
#include "db_interface.h"
#include "einsteinium_engine.h"

class AppAttrFile : public BFile
{public:
					AppAttrFile(const char*, const BEntry*);
					AppAttrFile(const BEntry*);
					~AppAttrFile();
	void			UpdateAppLaunched();
	void			UpdateAppQuit();
	uint32			getScore() { return appStats.getScore(); }
	const char*		getSig() { return appStats.getSig(); }
	const char*		getPath() { return appStats.getPath(); }
	const char*		getFilename() { return appStats.getFilename(); }
	bool			getIgnore() { return E_ignore; }
	void			setIgnore(bool ignore);
	void			rescanData();
	void			calculateScore();
	void			CopyAppStatsInto(AppStats*);
private:
	bool			/*dirty_data, */new_session;
	AppStats		appStats;
	BEntry			app_entry, appAttrEntry, appDataEntry;
	bool			E_ignore;
	time_t			EE_session, session;
	//Functions
	void			initData(bool);
	float			getQuartileVal(const double*, double);
	void			readAttrValues();
	void			writeAttrValues();
};

#endif
