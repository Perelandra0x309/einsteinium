/* AppAttrFile.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AppAttrFile.h"

/*	This is a subclass of a BFile which contains all functions for
	initializing, reading, changing, and writing the attributes
	for each application which is monitored
*/


AppAttrFile::AppAttrFile(const char *appSig, const BEntry *appEntry)
	:
	BFile(),
	fNewSession(false),
	fAppStats(),
	fAppEntry(*appEntry),
	fIgnoreInLists(false),
	fEngineCurrentSession(0)
{
	BPath appPath(appEntry), appAttrPath;
		//create paths for both the application and its attribute file
	find_directory(B_USER_SETTINGS_DIRECTORY, &appAttrPath);
	appAttrPath.Append("Einsteinium/Applications");
	appAttrPath.Append(appPath.Leaf());
	fAppAttrEntry.SetTo(appAttrPath.Path());
	uint32 openMode = B_READ_WRITE;
	bool newFile = false;
		//flag to indicate if a new file must be created
	printf("Finding appInfo file %s\n", appAttrPath.Path());
	if(!fAppAttrEntry.Exists())
	{	// TODO If the filename changed, need to search all attr files for app_sig
		// But for now...
		printf("Creating %s\n", appAttrPath.Path());
		openMode |= B_CREATE_FILE;
		newFile = true;
	}
	SetTo(&fAppAttrEntry, openMode);

	if(newFile)
	{
		// Initialize starting attributes and stats
		BNode attrNode(&fAppAttrEntry);
		BNodeInfo attrNodeInfo(&attrNode);
		attrNodeInfo.SetType(e_app_attr_filetype);
			//Set the file MIME type
		fAppStats.app_sig.SetTo(appSig);
		fAppStats.app_filename.SetTo(appPath.Leaf());
		fAppStats.app_path.SetTo(appPath.Path());
		_WriteAttrValues();

		fNewSession = true;
		BString text("Einsteinium detected a new application, \"");
		text.Append(appPath.Leaf());
		text.Append("\".  Would you like this app to be included in ranking lists?");
		BMessage *msg = new BMessage(E_SET_IGNORE_ATTR);
		msg->AddString("app_signature", appSig);
		(new BAlert("",text.String(), "Ignore", "Include"))
				->Go(new BInvoker(msg, be_app_messenger));
			// BInvoker takes ownership of the message so don't delete it
	}
	// TODO if file exists, verify signature matches to catch applications with identical names


	_InitData(newFile);
}


//opening an existing AppAttrFile (read only)
AppAttrFile::AppAttrFile(const BEntry *attrEntry)
	:
	BFile(attrEntry, B_READ_ONLY),
	fNewSession(false),
	fAppStats(),
	fAppEntry(),
	fAppAttrEntry(*attrEntry),
	fIgnoreInLists(false),
	fEngineCurrentSession(0)
{
	_InitData(false);
}


void
AppAttrFile::_InitData(bool newFile)
{
	fEngineCurrentSession = ((einsteinium_engine*)be_app)->GetSession();

	// Database path
	BPath appAttrPath(&fAppAttrEntry);
	BString dataPathStr(appAttrPath.Path());
	dataPathStr.Append(".db");
	fAppDataEntry.SetTo(dataPathStr.String());

	if(!newFile)
		_ReadAttrValues();
			// Read attributes from existing file

	else if(fAppDataEntry.Exists())
		RescanData();
			// New attribute file, look for existing data file, if it exists rescan data
}


AppAttrFile::~AppAttrFile()
{
	Unset();
}


void
AppAttrFile::UpdateAppLaunched()
{
	time_t now = time(NULL);
		//current time

	//update database
	BPath appDataPath(&fAppDataEntry);
	Edb_Add_Launch_Time(appDataPath.Path(), fEngineCurrentSession, now);

	// Update the attributes
	if(fAppStats.launch_count == 0)
		fAppStats.first_launch = now;
	fAppStats.launch_count++;
	if(fAppStats.last_launch > 0)
		fAppStats.last_interval = now - fAppStats.last_launch;
			//update the time interval between the last two launches
	fAppStats.last_launch = now;

	CalculateScore();
}


void
AppAttrFile::UpdateAppQuit()
{
	time_t now = time(NULL);

	//update database
	BPath appDataPath(&fAppDataEntry);
	Edb_Add_Quit_Time(appDataPath.Path(), fEngineCurrentSession, now);

	// Update the attributes
	if( (!fNewSession) && (fAppStats.last_launch!=0) )
		fAppStats.total_run_time += (now - fAppStats.last_launch);

	CalculateScore();
}


void
AppAttrFile::CalculateScore()
{
	const int *scales = ((einsteinium_engine*)be_app)->GetScalesPtr();
	int launch_scale=scales[0],
		first_scale=scales[1],
		last_scale=scales[2],
		interval_scale=scales[3],
		runtime_scale=scales[4];

	const double *Quart = ((einsteinium_engine*)be_app)->GetQuartilesPtr();
	float launch_val(0), first_launch_val(0), last_launch_val(0), last_interval_val(0),
			total_run_time_val(0);
	if( Quart!=NULL )
	{	launch_val = _GetQuartileVal(Quart+Q_LAUNCHES_INDEX, fAppStats.launch_count);
		first_launch_val = _GetQuartileVal(Quart+Q_FIRST_LAUNCH_INDEX, fAppStats.first_launch);
		last_launch_val = _GetQuartileVal(Quart+Q_LAST_LAUNCH_INDEX, fAppStats.last_launch);
		last_interval_val = _GetQuartileVal(Quart+Q_LAST_INTERVAL_INDEX, fAppStats.last_interval);
		total_run_time_val = _GetQuartileVal(Quart+Q_TOTAL_RUN_TIME_INDEX, fAppStats.total_run_time);

		int max_scale = 100000;
		// If last interval is zero (only one launch) put quartile value at .5 so that this
		// statistic does not adversly effect the score
		if(fAppStats.last_interval == 0)
			last_interval_val = .5;

		fAppStats.score = int(  max_scale*(launch_val * launch_scale)
					+ max_scale*(first_launch_val * first_scale)
					+ max_scale*(last_launch_val * last_scale)
					+ max_scale*((1 - last_interval_val) * interval_scale)
						// Need to reverse interval scale, because longer intervals decrease the score
						// TODO or do I change the sorting method to sort descending??
					+ max_scale*(total_run_time_val * runtime_scale));

	}
	_WriteAttrValues();
}


// Calculate the quartile value of where d lies in the quartile range Q
float
AppAttrFile::_GetQuartileVal(const double *Q, double d)
{//	printf("EIN: quartiles\n");
//	printf("Q4: %f, Q3: %f, Q2: %f, Q1: %f, Q0: %f\n", Q[4], Q[3], Q[2], Q[1], Q[0]);
	int index, index_offset = 0;
	if(d >= Q[0] && d<Q[1]) { index = 0; }
	else if(d >= Q[1] && d<Q[2]) { index = 1; }
	else if(d >= Q[2] && d<Q[3]) { index = 2; }
	else if(d >= Q[3] && d<Q[4]) { index = 3; }
	else if(d == Q[4]) { return 1; }
	// For values that lie outside the current quartile range
	else if(d < Q[0]) { index = 0; }
	else if(d > Q[4])
	{	index = 4;
		index_offset = 1;
		// since there is no Q[5], index offset will modify the calculation below to
		// use Q[3] and Q[4] to find the quartile value range
	}
	else { return .5; }

	return (.25*index + .25*((d - Q[index])/(Q[index+1-index_offset] - Q[index-index_offset])));
}


void
AppAttrFile::RescanData()
{
	BPath appDataPath(&fAppDataEntry);
	Edb_Rescan_Data(appDataPath.Path(), &fAppStats);
	CalculateScore();
}


void
AppAttrFile::SetIgnore(bool ignore)
{
	fIgnoreInLists = ignore;
	WriteAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &fIgnoreInLists, sizeof(fIgnoreInLists));
}


void
AppAttrFile::_ReadAttrValues()
{	//Read each attribute.  If attribute isn't found, initialize to proper default value
	attr_info info;
	//E Session
	time_t lastDataSession;
	if(GetAttrInfo(ATTR_SESSION_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_SESSION_NAME, B_INT32_TYPE, 0, &lastDataSession, sizeof(time_t)); }
	fNewSession = ( (fEngineCurrentSession != lastDataSession) || (lastDataSession==0) );
	//App path
	if(GetAttrInfo(ATTR_APP_PATH_NAME, &info) == B_NO_ERROR)
	{	ReadAttrString(ATTR_APP_PATH_NAME, &fAppStats.app_path); }
	else
	{	 }// TODO what??
	//get path from application path
	BPath appPath(fAppStats.app_path.String());//Path to actual application
	fAppEntry.SetTo(appPath.Path());
	//Read each attribute.  If attribute isn't found, initialize to proper default value
	//Application signature
	if(GetAttrInfo(ATTR_APP_SIG_NAME, &info) == B_OK)
	{	ReadAttrString(ATTR_APP_SIG_NAME, &fAppStats.app_sig); }
	else
	{	BNode appNode(appPath.Path());//get signature from application node
		if(appNode.GetAttrInfo("BEOS:APP_SIG", &info) == B_OK)
		{	appNode.ReadAttrString("BEOS:APP_SIG", &fAppStats.app_sig); }
		else printf("Couldn't find app sig for file %s\n", appPath.Path());
	}
	//Displayed name
	//App filename
	if(GetAttrInfo(ATTR_APP_FILENAME_NAME, &info) == B_NO_ERROR)
	{	ReadAttrString(ATTR_APP_FILENAME_NAME, &fAppStats.app_filename); }
	else
	{	fAppStats.app_filename.SetTo(appPath.Leaf()); }//get filename from path
	//Number of launches
	if(GetAttrInfo(ATTR_LAUNCHES_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_LAUNCHES_NAME, B_INT32_TYPE, 0, &fAppStats.launch_count, sizeof(uint32)); }
	//Score
	if(GetAttrInfo(ATTR_SCORE_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_SCORE_NAME, B_INT32_TYPE, 0, &fAppStats.score, sizeof(int)); }
	//Checksum
	//Class (app_sig, mimeype)
	//Ignore (default, not app specific)
	if(GetAttrInfo(ATTR_IGNORE_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &fIgnoreInLists, sizeof(fIgnoreInLists)); }
	//Owner?? this might need to be app specific
	//Date last activated
	if(GetAttrInfo(ATTR_LAST_LAUNCH_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_LAST_LAUNCH_NAME, B_INT32_TYPE, 0, &fAppStats.last_launch, sizeof(time_t)); }
	else// TODO if this isn't found use first launch date???  Undecided yet.
	{	}
	//Date first activated
	if(GetAttrInfo(ATTR_FIRST_LAUNCH_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_FIRST_LAUNCH_NAME, B_INT32_TYPE, 0, &fAppStats.first_launch, sizeof(time_t)); }
	else//first launch should be the same as file creation date
	{	time_t creation_t;
		if(GetCreationTime(&creation_t)==B_OK)
		{	fAppStats.first_launch = creation_t; }
	//	else printf("Created stat not found\n");
	}
	//Time between last two activations
	if(GetAttrInfo(ATTR_LAST_INTERVAL_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_LAST_INTERVAL_NAME, B_INT32_TYPE, 0, &fAppStats.last_interval, sizeof(uint32)); }
	else
	{	}
	//Total running time
	if(GetAttrInfo(ATTR_TOTAL_RUNTIME_NAME, &info) == B_NO_ERROR)
	{	ReadAttr(ATTR_TOTAL_RUNTIME_NAME, B_INT32_TYPE, 0, &fAppStats.total_run_time, sizeof(uint32)); }
	else
	{	}
	//Average daily interval
	//Int_reg
	//daily_int_reg
}


void
AppAttrFile::_WriteAttrValues()
{	//E session
	if(fNewSession)
	{	WriteAttr(ATTR_SESSION_NAME, B_INT32_TYPE, 0, &fEngineCurrentSession,
			sizeof(fEngineCurrentSession)); }
	//Application signature
	if(strcmp(fAppStats.app_sig.String(),"")!=0)
	{	WriteAttrString(ATTR_APP_SIG_NAME, &fAppStats.app_sig); }
	//Displayed name
	//App filename
	WriteAttrString(ATTR_APP_FILENAME_NAME, &fAppStats.app_filename);
	//App path
	WriteAttrString(ATTR_APP_PATH_NAME, &fAppStats.app_path);
	//Number of launches
	WriteAttr(ATTR_LAUNCHES_NAME, B_INT32_TYPE, 0, &fAppStats.launch_count,
		sizeof(fAppStats.launch_count));
	//Score
	WriteAttr(ATTR_SCORE_NAME, B_INT32_TYPE, 0, &fAppStats.score, sizeof(fAppStats.score));
	//Checksum
	//Class (app_sig, mimeype)
	//Ignore (default, not app specific)
	WriteAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &fIgnoreInLists, sizeof(fIgnoreInLists));
	//Owner?? this might need to be app specific
	//Date last activated
	WriteAttr(ATTR_LAST_LAUNCH_NAME, B_INT32_TYPE, 0, &fAppStats.last_launch,
		sizeof(fAppStats.last_launch));
	//Date first activated
	WriteAttr(ATTR_FIRST_LAUNCH_NAME, B_INT32_TYPE, 0, &fAppStats.first_launch,
		sizeof(fAppStats.first_launch));
	//Time between last two activations
	WriteAttr(ATTR_LAST_INTERVAL_NAME, B_INT32_TYPE, 0, &fAppStats.last_interval,
		sizeof(fAppStats.last_interval));
	//Total running time
	WriteAttr(ATTR_TOTAL_RUNTIME_NAME, B_INT32_TYPE, 0, &fAppStats.total_run_time,
		sizeof(fAppStats.total_run_time));

	//Average daily interval
	//Int_reg
	//daily_int_reg
}


AppStats*
AppAttrFile::CloneAppStats()
{
	AppStats* newStats = new AppStats(&fAppStats);
	return newStats;
}
