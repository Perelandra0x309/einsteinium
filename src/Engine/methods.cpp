/*methods.cpp

*/
#include "methods.h"


//Get entry for application with signature sig
BEntry getEntryFromSig(const char* sig)
{
	entry_ref ref;
	be_roster->FindApp(sig, &ref);
	BEntry entry(&ref);
	return entry;
/*	BVolumeRoster v_roster;//use the volume roster to search all volumes
	BVolume vol;
	BQuery query;
	BEntry entry;
	BString pred("BEOS:APP_SIG==");//looking for app sig, set query predicate string
	pred.Append(sig);
	v_roster.Rewind();//rewind roster to the beginning of the volume list
	while(v_roster.GetNextVolume(&vol)==B_NO_ERROR)//vol points to next volume in roster
	{	if(vol.KnowsAttr() && vol.KnowsMime() && vol.KnowsQuery())//make sure searching is possible
		{	query.Clear();//initialize query with predicate (must be done after each search)
			query.SetPredicate(pred.String());
			query.SetVolume(&vol);
			if(query.Fetch()!=B_OK) continue;//Query returned no results, try next volume
			if(query.GetNextEntry(&entry)==B_OK)//entry found
			{	break;//This works, but if there are multiple entrys found only the first
						//will be returned
			}
		}
	}
	return entry;*/
}
/*
template < class itemType >
void deleteList(BList& list, itemType* item)
{	do
	{	item = static_cast<itemType *>(list.RemoveItem(int32(0)));
		if(item) delete item;
	}while(item);
}
*/
