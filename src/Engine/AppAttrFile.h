/* AppAttrFile.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_ENGINE_APPATTRFILE_H
#define EINSTEINIUM_ENGINE_APPATTRFILE_H

#include <Alert.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <fs_attr.h>
#include <fstream>
#include <iostream.h>
#include <stdlib.h>
#include "db_interface.h"
#include "einsteinium_engine.h"


class AppAttrFile : public BFile {
public:
					AppAttrFile(const char*, const BEntry*);
					AppAttrFile(const BEntry*);
					~AppAttrFile();
	void			UpdateAppLaunched();
	void			UpdateAppQuit();
	uint32			GetScore() { return fAppStats.GetScore(); }
	const char*		GetSig() { return fAppStats.GetSig(); }
	const char*		GetPath() { return fAppStats.GetPath(); }
	const char*		GetFilename() { return fAppStats.GetFilename(); }
	bool			GetIgnore() { return fIgnoreInLists; }
	void			SetIgnore(bool ignore);
	void			RescanData();
	void			CalculateScore();
	void			CopyAppStatsInto(AppStats*);
private:
	bool			/*dirty_data, */fNewSession;
	AppStats		fAppStats;
	BEntry			fAppEntry, fAppAttrEntry, fAppDataEntry;
	bool			fIgnoreInLists;
	time_t			fEngineCurrentSession;
	//Functions
	void			_InitData(bool);
	float			_GetQuartileVal(const double*, double);
	void			_ReadAttrValues();
	void			_WriteAttrValues();
};

#endif
