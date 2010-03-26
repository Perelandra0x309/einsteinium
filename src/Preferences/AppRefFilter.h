/* AppRefFilter.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_APPREFFILTER_H
#define EP_APPREFFILTER_H

#include <FilePanel.h>
#include <NodeInfo.h>

class AppRefFilter : public BRefFilter {
public:
					AppRefFilter();
					//~AppRefFilter();
	virtual bool	Filter(const entry_ref *ref, BNode *node, struct stat_beos *st,
							const char *filetype);
};

#endif
