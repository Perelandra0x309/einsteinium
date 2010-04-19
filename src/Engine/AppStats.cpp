/* AppStats.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AppStats.h"

/*	This class contains functions for calculating various statistics
	for each AppAttrFile.
*/

AppStats::AppStats()
	:
	//dataOK(B_ERROR),
	app_sig(""),
	app_path(""),
	score(0),
	last_launch(0),
	first_launch(0),
	launch_count(0),
	last_interval(0),
	total_run_time(0)
{
//	dataOK = B_OK;
}


AppStats::AppStats(AppStats *sourceStats)
{
	app_sig.SetTo(sourceStats->app_sig);
	app_path.SetTo(sourceStats->app_path);
	app_filename.SetTo(sourceStats->app_filename);
	score = sourceStats->score;
	launch_count = sourceStats->launch_count;
	last_launch = sourceStats->last_launch;
	first_launch = sourceStats->first_launch;
	last_interval = sourceStats->last_interval;
	total_run_time = sourceStats->total_run_time;
}


//Main sorting method
template < class itemType >
int
AppStatsSort(itemType (*getFunc)(AppStats*),
					const void* appStats1, const void* appStats2)
{
	itemType stat1 = getFunc(*(AppStats**)appStats1);//get first stat
	itemType stat2 = getFunc(*(AppStats**)appStats2);//get second stat
	if(stat1 < stat2) return 1;//object1 should be after object2
	else if(stat1 > stat2) return -1;//object1 should be before object2
	else return 0;//both objects have the same value
}

//Sort AppStats objects by their score
int AppStatsSortScore(const void* appStats1, const void* appStats2)
{	return AppStatsSort( GetStatsScore, appStats1, appStats2); }
int GetStatsScore(AppStats *stats)
{	return stats->GetScore(); }

//Sort AppStats objects by their last launch time
int AppStatsSortLastLaunch(const void* appStats1, const void* appStats2)
{	return AppStatsSort(GetStatsLastLaunch, appStats1, appStats2); }
time_t GetStatsLastLaunch(AppStats* stats)
{	return stats->GetLastLaunch(); }

//Sort AppStats objects by their first launch time
int AppStatsSortFirstLaunch(const void* appStats1, const void* appStats2)
{	return AppStatsSort(GetStatsFirstLaunch, appStats1, appStats2); }
time_t GetStatsFirstLaunch(AppStats* stats)
{	return stats->GetFirstLaunch(); }

//Sort AppStats objects by their launch count
int AppStatsSortLaunchCount(const void* appStats1, const void* appStats2)
{	return AppStatsSort(GetStatsLaunchCount, appStats1, appStats2); }
uint32 GetStatsLaunchCount(AppStats* stats)
{	return stats->GetLaunchCount(); }

//Sort AppStats objects by their last interval
int AppStatsSortLastInterval(const void* appStats1, const void* appStats2)
{	return AppStatsSort(GetStatsLastInterval, appStats1, appStats2); }
uint32 GetStatsLastInterval(AppStats* stats)
{	return stats->GetLastInterval(); }

//Sort AppStats objects by their total running time
int AppStatsSortRunningTime(const void* appStats1, const void* appStats2)
{	return AppStatsSort(GetStatsRunningTime, appStats1, appStats2); }
uint32 GetStatsRunningTime(AppStats* stats)
{	return stats->GetRunningTime(); }
