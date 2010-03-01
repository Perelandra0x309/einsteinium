#include "db_interface.h"


int Edb_open(sqlite3** dbpp, const char* file_path)
{	int ret=0;
//	printf("Opening database %s\n", file_path);
	// Check to see if database file already exists before attempting to open it.
	BEntry fileEntry(file_path);
	bool newDatabase = !fileEntry.Exists();

	// Open datebase
	ret = sqlite3_open(file_path, dbpp);
	if( ret ){
    	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*dbpp));
    	Edb_close(*dbpp);
		return ret;
	}
	// Need to create table for new database
	if(newDatabase)
	{
		//create table time_data(app_time_data blob);
		BString sqlcmd("create table time_data(app_time_data blob);");
		char *errorMsg = NULL;
		ret = sqlite3_exec(*dbpp, sqlcmd.String(), NULL, NULL, &errorMsg);
		if( ret!=SQLITE_OK ){
			fprintf(stderr, "SQL error:%s\n", errorMsg);
			sqlite3_free(errorMsg);
			Edb_close(*dbpp);
		}
	}

	return ret;
}


int Edb_close(sqlite3* dbp)
{	int ret=0;
	ret = sqlite3_close(dbp);
	if( ret ){
    	fprintf(stderr, "Can't close database: %s\n", sqlite3_errmsg(dbp));
	}
	return ret;
}


int Edb_print(sqlite3* dbp)
{
	int ret;
	BString sqlcmd("select * from time_data;");
	sqlite3_stmt *stmt;
	printf("Printing database contents:\n");

	// prepare the SQL statement
	ret = sqlite3_prepare_v2(dbp, sqlcmd.String(), -1, &stmt, 0);
	if( ret ){
		fprintf(stderr, "SQL Select error: %s: %d : %s\n", sqlcmd.String(), ret, sqlite3_errmsg(dbp));
	}else{
		// execute the statement
		do{
			ret = sqlite3_step(stmt);
			switch( ret ){
				case SQLITE_DONE:
					break;
				case SQLITE_ROW: {
					app_time_data *ptr = (app_time_data *)sqlite3_column_blob(stmt, 0);
				//	printf("Ptr=%d\n", ptr);
					printf("ee_session=%ld, launch_time=%ld, quit_time=%ld\n", ptr->ee_session,
								ptr->launch_time, ptr->quit_time);
					break; }
				default:
					fprintf(stderr, "Error: %d : %s\n",  ret, sqlite3_errmsg(dbp));
					break;
			}
		}while( ret==SQLITE_ROW );
		// finalize the statement to release resources
		sqlite3_finalize(stmt);
	}
	return ret;
}


/*int Edb_appendData(sqlite3* dbp, app_time_data* tm_data)
{
	int ret=1;
	sqlite3_stmt *stmt;

	// insert timestamp data
	const char *sqlcmd = "INSERT INTO time_data VALUES(?);";

	do {
		ret = sqlite3_prepare_v2(dbp, sqlcmd, -1, &stmt, 0);
		if( ret ){
			fprintf(stderr, "SQL Prepare error: %d : %s\n", ret, sqlite3_errmsg(dbp));
			return ret;
		}

		ret = sqlite3_bind_blob(stmt, 1, tm_data, sizeof(*tm_data), SQLITE_STATIC);
		if( ret ){
			fprintf(stderr, "SQL Bind Blob error: %d : %s\n", ret, sqlite3_errmsg(dbp));
			return ret;
		}

		ret = sqlite3_step(stmt);
		if( ret!= SQLITE_DONE ){
			fprintf(stderr, "SQL Step error: %d : %s\n", ret, sqlite3_errmsg(dbp));
		}

		// finalize the statement to release resources
		sqlite3_finalize(stmt);

	} while( ret==SQLITE_SCHEMA );

	return ret;
}*/


int Edb_insertData(sqlite3* dbp, app_time_data* tm_data, long long int row_key = 0)
{
	int ret=1;
	sqlite3_stmt *stmt;
	BString sqlcmd;

	//create sql to replace row defined by row_key
	if(row_key)
	{
		char keyStr[64];
		sprintf(keyStr, "%lld", row_key);
//		printf("keyStr=%s\n", keyStr);
		sqlcmd.SetTo("REPLACE INTO time_data (ROWID, app_time_data) VALUES(")
				.Append(keyStr).Append(", ?);");
//		printf("Replace sql: %s\n", sqlcmd.String());
	}
	//create sql to insert a new record
	else
	{
		sqlcmd.SetTo("INSERT INTO time_data VALUES(?);");
//		printf("Insert sql: %s\n", sqlcmd.String());
	}

	do {
		ret = sqlite3_prepare_v2(dbp, sqlcmd.String(), -1, &stmt, 0);
		if( ret ){
			fprintf(stderr, "SQL Prepare error: %d : %s\n", ret, sqlite3_errmsg(dbp));
			return ret;
		}
		//Add the tm_data struct to the sql statement as BLOB data
		ret = sqlite3_bind_blob(stmt, 1, tm_data, sizeof(*tm_data), SQLITE_STATIC);
		if( ret ){
			fprintf(stderr, "SQL Bind Blob error: %d : %s\n", ret, sqlite3_errmsg(dbp));
			return ret;
		}

		ret = sqlite3_step(stmt);
		if( ret!= SQLITE_DONE ){
			fprintf(stderr, "SQL Step error: %d : %s\n", ret, sqlite3_errmsg(dbp));
		}

		// finalize the statement to release resources
		sqlite3_finalize(stmt);

	} while( ret==SQLITE_SCHEMA );

	return ret;
}


int Edb_getLastData(sqlite3* dbp, app_time_data* tm_data, long long int* row_key)
{
	int ret = 1;
	sqlite3_stmt *stmt;

	// Get the last ROWID used in the database
	BString sqlcmd("select max(rowid) from time_data;");
	ret = sqlite3_prepare_v2(dbp, sqlcmd.String(), -1, &stmt, 0);
	if( ret ){
		fprintf(stderr, "SQL Select error: %s: %d : %s\n", sqlcmd.String(), ret, sqlite3_errmsg(dbp));
		return ret;
	}
	do{
		ret = sqlite3_step(stmt);
		switch( ret ){
			case SQLITE_DONE:
				break;
			case SQLITE_ROW: {
				*row_key = sqlite3_column_int64(stmt, 0);
				break; }
			default:
				fprintf(stderr, "Error: %d : %s\n",  ret, sqlite3_errmsg(dbp));
				break;
		}
	}while( ret==SQLITE_ROW );
	// finalize the statement to release resources
	sqlite3_finalize(stmt);

	// Get the data from the last rowid record
	char keyStr[64];
	sprintf(keyStr, "%lld", *row_key);
	sqlcmd.SetTo("select * from time_data where rowid = ").Append(keyStr).Append(";");
//	printf("Running sql: %s\n", sqlcmd.String());
	ret = sqlite3_prepare_v2(dbp, sqlcmd.String(), -1, &stmt, 0);
	if( ret ){
		fprintf(stderr, "SQL Select error: %s: %d : %s\n", sqlcmd.String(), ret, sqlite3_errmsg(dbp));
		return ret;
	}
	// execute the statement
	do{
		ret = sqlite3_step(stmt);
		switch( ret ){
			case SQLITE_DONE:
				break;
			case SQLITE_ROW: {
				app_time_data *ptr = (app_time_data *)sqlite3_column_blob(stmt, 0);
			//	printf("ee_session=%ld, launch_time=%ld, quit_time=%ld\n", ptr->ee_session,
			//				ptr->launch_time, ptr->quit_time);
				tm_data->ee_session = ptr->ee_session;
				tm_data->launch_time = ptr->launch_time;
				tm_data->quit_time = ptr->quit_time;
				break; }
			default:
				fprintf(stderr, "Error: %d : %s\n",  ret, sqlite3_errmsg(dbp));
				break;
		}
	}while( ret==SQLITE_ROW );
	// finalize the statement to release resources
	sqlite3_finalize(stmt);

	return ret;
}


void Edb_Add_Launch_Time(const char* file_path, time_t session_tm, time_t launch_tm)
{	sqlite3 *dbp = NULL;
	app_time_data tm_data;
	memset(&tm_data, 0, sizeof(tm_data));
	tm_data.ee_session = session_tm;
	tm_data.launch_time = launch_tm;
	int ret = Edb_open(&dbp, file_path);
	if( ret != 0 ) return;
	Edb_insertData(dbp, &tm_data);
//	Edb_print(dbp);
	Edb_close(dbp);
}


void Edb_Add_Quit_Time(const char* file_path, time_t session_tm, time_t quit_tm)
{
	int ret;
	sqlite3 *dbp = NULL;
	app_time_data tm_data;
	ret = Edb_open(&dbp, file_path);
	if( ret != 0 ) return;
	memset(&tm_data, 0, sizeof(tm_data));
	long long int key;
	if( Edb_getLastData(dbp, &tm_data, &key) == SQLITE_DONE)
	{
		// check last record session id against the current ee_session
		if(session_tm == tm_data.ee_session)
		{
			tm_data.quit_time = quit_tm;
			ret = Edb_insertData(dbp, &tm_data, key);
		}
		// New engine session, insert new record
		else
		{
			tm_data.ee_session = session_tm;
			tm_data.launch_time = 0;
			tm_data.quit_time = quit_tm;
			Edb_insertData(dbp, &tm_data);
		}
	}
//	Edb_print(dbp);
	Edb_close(dbp);
}

void Edb_Rescan_Data(const char* file_path, AppStats* stats)
{
	int ret;
	sqlite3 *dbp = NULL;

	ret = Edb_open(&dbp, file_path);
	if( ret != 0 ) return;

	BString sqlcmd("select * from time_data order by rowid;");
	sqlite3_stmt *stmt;
	ret = sqlite3_prepare_v2(dbp, sqlcmd.String(), -1, &stmt, 0);
	if( ret ){
		fprintf(stderr, "SQL Select error: %s: %d : %s\n", sqlcmd.String(), ret, sqlite3_errmsg(dbp));
		return;
	}
	// execute the statement
	time_t launch_t(0), quit_t(0), launch_t_old(0);
	time_t first_launch(0), last_launch(0), last_interval(0), total_run_time(0);
	int launch_count = 0;
	do{
		ret = sqlite3_step(stmt);
		switch( ret ){
			case SQLITE_DONE:
				break;
			case SQLITE_ROW: {
				app_time_data *tm_data = (app_time_data *)sqlite3_column_blob(stmt, 0);
			//	printf("Found row: ee_session=%ld, launch_time=%ld, quit_time=%ld\n", tm_data->ee_session,
			//				tm_data->launch_time, tm_data->quit_time);
				launch_t = tm_data->launch_time;
				quit_t = tm_data->quit_time;
				if(launch_t_old == 0) { launch_t_old = launch_t; }
				if(launch_t != 0)
				{	launch_count++;//launch count
					last_launch = launch_t;//last date launched
					if(first_launch == 0)//first date launched
					{	first_launch = launch_t; }
					if(quit_t != 0)
					{	total_run_time += quit_t - launch_t; }//total run time
					last_interval = launch_t - launch_t_old;//last interval time
					launch_t_old = launch_t;
				}

				break; }
			default:
				fprintf(stderr, "Error: %d : %s\n",  ret, sqlite3_errmsg(dbp));
				break;
		}
	}while( ret==SQLITE_ROW );
	// Save stats
	stats->launch_count = launch_count;
	stats->first_launch = first_launch;
	stats->last_launch = last_launch;
	stats->last_interval = last_interval;
	stats->total_run_time = total_run_time;

	// finalize the statement to release resources
	sqlite3_finalize(stmt);

	Edb_close(dbp);
}
