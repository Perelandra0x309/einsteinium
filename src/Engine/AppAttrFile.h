/* AppAttrFile.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_ENGINE_APPATTRFILE_H
#define EINSTEINIUM_ENGINE_APPATTRFILE_H

#include <StorageKit.h>
#include <SupportKit.h>
#include "db_interface.h"
#include "einsteinium_engine.h"


class AppAttrFile : public BFile {
public:
					AppAttrFile(const char*, const BEntry*);
					AppAttrFile(const BEntry*);
					~AppAttrFile();
	void			UpdateAppLaunched();
	void			UpdateAppQuit();
	const char*		GetSig() { return fAppStats.GetSig(); }
	const char*		GetPath() { return fAppStats.GetPath(); }
	const char*		GetFilename() { return fAppStats.GetFilename(); }
	void			RescanData();
	AppStats*		CloneAppStats();
private:
	bool			fNewSession;
	AppStats		fAppStats;
	BEntry			fAppEntry, fAppAttrEntry;
	time_t			fEngineCurrentSession;
	//Functions
	void			_InitData(bool);
	void			_ReadAttrValues();
	void			_WriteAttrValues();
};

#endif
