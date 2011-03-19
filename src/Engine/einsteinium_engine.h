/* einsteinium_engine.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_ENGINE_H
#define EINSTEINIUM_ENGINE_H

#include <AppKit.h>
#include <StorageKit.h>
#include <Deskbar.h>
#include "AppStats.h"
#include "AppAttrFile.h"
#include "EESettingsFile.h"
#include "engine_constants.h"
#include "EngineSubscriber.h"
#include "methods.h"


struct Subscriber {
	int16 count;
	int32 uniqueID;
	int8 launch_scale,
		first_scale,
		last_scale,
		interval_scale,
		runtime_scale;
	BMessenger messenger;
	BList appStatsList;
	int appStatsSortOrder;
	double fQuartiles[Q_ARRAY_SIZE];
	bool currentAppIsExcluded;
	uint currentAppPreviousRank;
};

class einsteinium_engine : public BApplication {
public:
					einsteinium_engine();
//					~einsteinium_engine();
	virtual bool	QuitRequested();
	virtual void	ReadyToRun();
	virtual void	ArgvReceived(int32, char**);
	virtual void	MessageReceived(BMessage*);
	const time_t	GetSession() { return fEngineSession; }
private:
	BPath			fSettingsDirPath;
	EESettingsFile	*fSettingsFile;
	bool			fWatchingRoster;//will be true when roster is being watched
//	BMessageRunner	*fQuartileRunner;
	const time_t	fEngineSession;//current session number
	BList			fSubscribersList;
	// functions
	void			_Unsubscribe(int32 uniqueID);
	void			_SendListToSubscriber(Subscriber *subscriber);
	void			_PopulateAppRankMessage(BList *appStatsList, BMessage *message, int count);
//	void			_ForEachAttrFile(int action, BList *appStatsList = NULL);
//	void			_RescanAllAttrFiles();
//	void			_RescanAttrFile(BEntry*);
	bool			_DetermineExclusion(Subscriber *subscriber, const char* signature);
	void			_CreateAppStatsList(Subscriber *subscriber, int sortAction=SORT_BY_NONE);
	void			_SortAppStatsList(Subscriber *subscriber, int sortAction);
	void			_EmptyAppStatsList(BList &list);
	uint			_FindAppStatsRank(Subscriber *subscriber, const char* signature);
	void			_UpdateQuartiles(Subscriber *subscriber);
	template < class itemType >
	void			_GetQuartiles(itemType (*)(AppStats*), BList&, double*);
	void			_CalculateScores(Subscriber *subscriber);
	void			_CalculateScore(Subscriber *subscriber, AppStats *appStats);
	float			_GetQuartileValue(const double *Q, double d);
};


template < class itemType >
void DeleteList(BList&, itemType*);

#endif
