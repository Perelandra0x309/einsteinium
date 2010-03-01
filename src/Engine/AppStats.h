/*AppStats.h
	This will contain functions for calculating various statistics
	for each AppAttrFile.
*/
#ifndef EINSTEINIUM_ENGINE_APPSTATS_H
#define EINSTEINIUM_ENGINE_APPSTATS_H


#include <KernelKit.h>
#include <StorageKit.h>
#include <String.h>

class AppStats
{public:
					AppStats();
	const char*		getSig() { return app_sig.String(); }
	const char*		getPath() { return app_path.String(); }
	const char*		getFilename() { return app_filename.String(); }

	time_t			getLastLaunch() { return last_launch; }
	time_t			getFirstLaunch() { return first_launch; }
	uint32			getLaunchCount() { return launch_count; }
	uint32			getLastInterval() { return last_interval; }
	uint32			getRunningTime() { return total_run_time; }
	int				getScore() { return score; }
//	status_t		InitCheck() { return dataOK; }
/*	void			setSig(const char *sig) { app_sig.SetTo(sig); }
	void			setFilename(const char *name) { app_filename.SetTo(name); }
	void			setPath(const char *path) { app_path.SetTo(path); }
	void			setLastLaunch(time_t tm) { last_launch = tm; }
	void			setFirstLaunch(time_t tm) { first_launch = tm; }
	void			setLastInterval(uint32 tm) { last_interval = tm; }
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
int getStatsScore(AppStats*);
//Sort AppStats objects by their last time run
int AppStatsSortLastLaunch(const void*, const void*);
time_t getStatsLastLaunch(AppStats*);
//Sort AppStats objects by their first launch time
int AppStatsSortFirstLaunch(const void*, const void*);
time_t getStatsFirstLaunch(AppStats*);
//Sort AppStats objects by their launch count
int AppStatsSortLaunchCount(const void*, const void*);
uint32 getStatsLaunchCount(AppStats*);
//Sort AppStats objects by their last interval
int AppStatsSortLastInterval(const void*, const void*);
uint32 getStatsLastInterval(AppStats*);
//Sort AppStats objects by their total running time
int AppStatsSortRunningTime(const void*, const void*);
uint32 getStatsRunningTime(AppStats*);

#endif
