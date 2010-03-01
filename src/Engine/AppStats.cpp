/*AppStats.cpp
	This will contain functions for calculating various statistics
	for each AppAttrFile.
*/
#include "AppStats.h"


AppStats::AppStats()
	:/*dataOK(B_ERROR)
	,*/app_sig("")
	,app_path("")
	,score(0)
	,last_launch(0)
	,first_launch(0)
	,launch_count(0)
	,last_interval(0)
	,total_run_time(0)
{
//	dataOK = B_OK;
}

//Main sorting method
template < class itemType >
int AppStatsSort(itemType (*getFunc)(AppStats*),
					const void* appStats1, const void* appStats2)
{	itemType stat1 = getFunc(*(AppStats**)appStats1);//get first stat
	itemType stat2 = getFunc(*(AppStats**)appStats2);//get second stat
	if(stat1 < stat2) return 1;//object1 should be after object2
	else if(stat1 > stat2) return -1;//object1 should be before object2
	else return 0;//both objects have the same value
}
//Sort AppStats objects by their score
int AppStatsSortScore(const void* appStats1, const void* appStats2)
{	return AppStatsSort( getStatsScore, appStats1, appStats2); }
int getStatsScore(AppStats *stats)
{	return stats->getScore(); }
//Sort AppStats objects by their last launch time
int AppStatsSortLastLaunch(const void* appStats1, const void* appStats2)
{	return AppStatsSort(getStatsLastLaunch, appStats1, appStats2); }
time_t getStatsLastLaunch(AppStats* stats)
{	return stats->getLastLaunch(); }
//Sort AppStats objects by their first launch time
int AppStatsSortFirstLaunch(const void* appStats1, const void* appStats2)
{	return AppStatsSort(getStatsFirstLaunch, appStats1, appStats2); }
time_t getStatsFirstLaunch(AppStats* stats)
{	return stats->getFirstLaunch(); }
//Sort AppStats objects by their launch count
int AppStatsSortLaunchCount(const void* appStats1, const void* appStats2)
{	return AppStatsSort(getStatsLaunchCount, appStats1, appStats2); }
uint32 getStatsLaunchCount(AppStats* stats)
{	return stats->getLaunchCount(); }
//Sort AppStats objects by their last interval
int AppStatsSortLastInterval(const void* appStats1, const void* appStats2)
{	return AppStatsSort(getStatsLastInterval, appStats1, appStats2); }
uint32 getStatsLastInterval(AppStats* stats)
{	return stats->getLastInterval(); }
//Sort AppStats objects by their total running time
int AppStatsSortRunningTime(const void* appStats1, const void* appStats2)
{	return AppStatsSort(getStatsRunningTime, appStats1, appStats2); }
uint32 getStatsRunningTime(AppStats* stats)
{	return stats->getRunningTime(); }
