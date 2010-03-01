/*AppRefFilter.h
	Filters only applications in an open file panel
*/
#ifndef EP_APPREFFILTER_H
#define EP_APPREFFILTER_H

#include <FilePanel.h>
#include <NodeInfo.h>

class AppRefFilter : public BRefFilter
{public:
					AppRefFilter();
					//~AppRefFilter();
	virtual bool	Filter(const entry_ref *ref, BNode *node, struct stat_beos *st,
							const char *filetype);
};

#endif
