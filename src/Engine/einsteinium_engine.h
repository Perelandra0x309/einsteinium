/* einsteinium_engine.h
 * Copyright 2010 Brian Hill
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
#include "EEShelfView.h"
#include "engine_constants.h"
#include "methods.h"


struct Subscriber {
	int16 count;
	int32 uniqueID;
	BMessenger messenger;
};

class einsteinium_engine : public BApplication {
public:
					einsteinium_engine();
//					~einsteinium_engine();
	virtual bool	QuitRequested();
	virtual void	ReadyToRun();
	virtual void	ArgvReceived(int32, char**);
	virtual void	MessageReceived(BMessage*);
	const double*	GetQuartilesPtr() const { return fQuartiles; }
	const int*		GetScalesPtr() const { return fSettingsFile->GetScales(); }
	const time_t	GetSession() { return fEngineSession; }
private:
	BPath			fSettingsDirPath;
	EESettingsFile	*fSettingsFile;
	bool			fWatchingRoster;//will be true when roster is being watched
	BMessageRunner	*fQuartileRunner;
	const time_t	fEngineSession;//current session number
	double			fQuartiles[30];
	BList			fSubscribersList;
//	BQuery			fRankQuery;
	int32			fShelfViewId;
	// functions
	void			_SendListToSubscriber(BList *appStatsList, Subscriber *subscriber);
	void			_PopulateAppRankMessage(BList *appStatsList, BMessage *message, int count);
	void			_ShowShelfView(bool showShelfView, int shelfViewCount);
//	void			_DoRankQuery();
	void			_ForEachAttrFile(int action, BList *appStatsList = NULL);
	void			_RescanAllAttrFiles();
	void			_RescanAttrFile(BEntry*);
	void			_UpdateAllAttrScores();
	void			_UpdateAttrScore(BEntry*);
	BList			_CreateAppStatsList(int sortAction=SORT_BY_NONE);
	void			_SortAppStatsList(BList &list, int sortAction);
	void			_EmptyAppStatsList(BList &list);
	uint			_FindAppStatsRank(BList &appStatsList, const char* signature);
	void			_UpdateQuartiles();
	template < class itemType >
	void			_GetQuartiles(itemType (*)(AppStats*), BList&, double*);
	void			_WriteQuartiles(BFile*, double*);
	void			_WriteQuartilesNamed(BFile*, double*, const char*);
//	bool			_ReadQuartiles(BFile*, double*);
//	bool			_ReadQuartilesNamed(BFile*, double*, const char*);
};


template < class itemType >
void DeleteList(BList&, itemType*);

#endif
