/*AppRefFilter.cpp
	Filters only applications in an open file panel
*/

#include "AppRefFilter.h"

AppRefFilter::AppRefFilter()
	:BRefFilter()
{

}

bool AppRefFilter::Filter(const entry_ref *ref, BNode *node, struct stat_beos *st,
							const char *filetype)
{	if(strcmp("application/x-vnd.Be-directory", filetype)==0) { return true; }//folders
	else if(strcmp("application/x-vnd.Be-volume", filetype)==0) { return true; }//volumes
	else if(strcmp("application/x-vnd.Be-elfexecutable", filetype)==0) { return true; }//apps
	else if(strcmp("application/x-vnd.be-elfexecutable", filetype)==0) { return true; }//hack for Haiku?
	else if(strcmp("application/x-vnd.Be-symlink", filetype)==0)//symlinks
	{	BEntry linkedEntry(ref, true);
		if(linkedEntry.InitCheck()!=B_OK) { return false; }
		BNode linkedNode(&linkedEntry);
		if(linkedNode.InitCheck()!=B_OK) { return false; }
		BNodeInfo linkedNodeInfo(&linkedNode);
		if(linkedNodeInfo.InitCheck()!=B_OK) { return false; }
		char *type = new char[B_ATTR_NAME_LENGTH];
		bool pass(false);
		if(linkedNodeInfo.GetType(type)!=B_OK) { pass = false; }
		else if(strcmp("application/x-vnd.Be-directory", type)==0) { pass = true; }//folders
		else if(strcmp("application/x-vnd.Be-volume", type)==0) { pass = true; }//volumes
		else if(strcmp("application/x-vnd.Be-elfexecutable", type)==0) { pass = true; }//apps
		else if(strcmp("application/x-vnd.be-elfexecutable", type)==0) { pass = true; }//hack for Haiku??
		delete[] type;
		return pass;
	}
	return false;
}
