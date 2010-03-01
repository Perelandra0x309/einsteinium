#ifndef EINSTEINIUM_BERKELEYDB_INTERFACE_H
#define EINSTEINIUM_BERKELEYDB_INTERFACE_H

#include <Entry.h>
#include <String.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include "AppStats.h"

struct app_time_data{
	time_t ee_session, launch_time, quit_time;
};

// public interface functions
void Edb_Add_Launch_Time(const char*, time_t, time_t);
void Edb_Add_Quit_Time(const char*, time_t, time_t);
void Edb_Rescan_Data(const char*, AppStats*);

// the heart of the database manipulation functions
int Edb_open(sqlite3**, const char*);
int Edb_close(sqlite3*);
int Edb_print(sqlite3*);
//int Edb_appendData(sqlite3*, app_time_data*);
int Edb_insertData(sqlite3*, app_time_data*, long long int);
int Edb_getLastData(sqlite3*, app_time_data*, long long int*);


#endif
