/* AppAttrFile.cpp
 * Copyright 2011 Brian Hill
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
	fEngineCurrentSession(0)
{
	BPath appPath(appEntry), appAttrPath;
		//create paths for both the application and its attribute file
	find_directory(B_USER_SETTINGS_DIRECTORY, &appAttrPath);
	appAttrPath.Append("Einsteinium/Applications");
	BString fileName(appPath.Leaf());
	fileName.Append(".db");
	appAttrPath.Append(fileName.String());
	fAppAttrEntry.SetTo(appAttrPath.Path());
	uint32 openMode = B_READ_WRITE;
	bool newFile = false;
		//flag to indicate if a new file must be created
//	printf("Finding appInfo file %s\n", appAttrPath.Path());
	if(!fAppAttrEntry.Exists())
	{	// TODO If the filename changed, need to search all attr files for app_sig
		// But for now...
//		printf("Creating %s\n", appAttrPath.Path());
		openMode |= B_CREATE_FILE;
		newFile = true;
	}
	SetTo(&fAppAttrEntry, openMode);

	if(newFile)
	{
		// Create database tables
		Edb_Init(appAttrPath.Path());

		// Initialize starting attributes and stats
		BNode attrNode(&fAppAttrEntry);
		BNodeInfo attrNodeInfo(&attrNode);
		attrNodeInfo.SetType(e_app_attr_filetype);
			//Set the file MIME type

		// Set standard attributes
		fAppStats.app_sig.SetTo(appSig);
		fAppStats.app_filename.SetTo(appPath.Leaf());
		fAppStats.app_path.SetTo(appPath.Path());

		fNewSession = true;
		_WriteAttrValues();
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
	fEngineCurrentSession(0)
{
	_InitData(false);
}


void
AppAttrFile::_InitData(bool newFile)
{
	fEngineCurrentSession = ((einsteinium_engine*)be_app)->GetSession();

	// Read attributes from existing file
	if(!newFile)
		_ReadAttrValues();
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

	// Update database
	BPath dataPath(&fAppAttrEntry);
	Edb_Add_Launch_Time(dataPath.Path(), fEngineCurrentSession, now);

	// Update the attributes
	if(fAppStats.launch_count == 0)
		fAppStats.first_launch = now;
	fAppStats.launch_count++;
	if(fAppStats.last_launch > 0)
		fAppStats.last_interval = now - fAppStats.last_launch;
			//update the time interval between the last two launches
	fAppStats.last_launch = now;

	_WriteAttrValues();
}


void
AppAttrFile::UpdateAppQuit()
{
	time_t now = time(NULL);

	// Update database
	BPath dataPath(&fAppAttrEntry);
	Edb_Add_Quit_Time(dataPath.Path(), fEngineCurrentSession, now);

	// Update the attributes
	if( (!fNewSession) && (fAppStats.last_launch!=0) )
		fAppStats.total_run_time += (now - fAppStats.last_launch);

	_WriteAttrValues();
}


void
AppAttrFile::RescanData()
{
	// Recreate attributes by scanning entire data
	BPath dataPath(&fAppAttrEntry);
	Edb_Rescan_Data(dataPath.Path(), &fAppStats);
	_WriteAttrValues();
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
	{	ReadAttrString(ATTR_APP_SIG_NAME, &fAppStats.app_sig);
		//Check to be sure the application has not moved
		if(!fAppEntry.Exists())
		{
			fAppEntry = GetEntryFromSig(fAppStats.app_sig);
			appPath.SetTo(&fAppEntry);
			fAppStats.app_path.SetTo(appPath.Path());
		}
	}
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
	//Checksum
	//Class (app_sig, mimeype)
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
	//Checksum
	//Class (app_sig, mimeype)
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
