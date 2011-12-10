/* methods.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "methods.h"


//Get entry for application with signature sig
BEntry
GetEntryFromSig(const char* sig)
{
	entry_ref ref;
	be_roster->FindApp(sig, &ref);
	BEntry entry(&ref);
	return entry;
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
