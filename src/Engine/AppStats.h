/* AppStats.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_ENGINE_APPSTATS_H
#define EINSTEINIUM_ENGINE_APPSTATS_H


#include <KernelKit.h>
#include <StorageKit.h>
#include <String.h>

class AppStats {
public:
					AppStats();
	const char*		GetSig() { return app_sig.String(); }
	const char*		GetPath() { return app_path.String(); }
	const char*		GetFilename() { return app_filename.String(); }

	time_t			GetLastLaunch() { return last_launch; }
	time_t			GetFirstLaunch() { return first_launch; }
	uint32			GetLaunchCount() { return launch_count; }
	uint32			GetLastInterval() { return last_interval; }
	uint32			GetRunningTime() { return total_run_time; }
	int				GetScore() { return score; }
//	status_t		InitCheck() { return dataOK; }
/*	void			SetSig(const char *sig) { app_sig.SetTo(sig); }
	void			SetFilename(const char *name) { app_filename.SetTo(name); }
	void			SetPath(const char *path) { app_path.SetTo(path); }
	void			SetLastLaunch(time_t tm) { last_launch = tm; }
	void			SetFirstLaunch(time_t tm) { first_launch = tm; }
	void			SetLastInterval(uint32 tm) { last_interval = tm; }
	void			IncrementLaunchCount() { launch_count++; }
	void			IncreaseTotalRunTime(uint32 tm) { total_run_time += tm; }
	*/
	BString			app_sig, app_path, app_filename;
	uint32			launch_count, last_interval, total_run_time;
	time_t			last_launch, first_launch;
	int				score;

private:
//	status_t		dataOK;

};

//Sort functions
template < class itemType >
int AppStatsSort(itemType (*getFunc)(AppStats*), const void*, const void*);
//Sort AppStats objects by their score
int AppStatsSortScore(const void*, const void*);
int GetStatsScore(AppStats*);
//Sort AppStats objects by their last time run
int AppStatsSortLastLaunch(const void*, const void*);
time_t GetStatsLastLaunch(AppStats*);
//Sort AppStats objects by their first launch time
int AppStatsSortFirstLaunch(const void*, const void*);
time_t GetStatsFirstLaunch(AppStats*);
//Sort AppStats objects by their launch count
int AppStatsSortLaunchCount(const void*, const void*);
uint32 GetStatsLaunchCount(AppStats*);
//Sort AppStats objects by their last interval
int AppStatsSortLastInterval(const void*, const void*);
uint32 GetStatsLastInterval(AppStats*);
//Sort AppStats objects by their total running time
int AppStatsSortRunningTime(const void*, const void*);
uint32 GetStatsRunningTime(AppStats*);

#endif
