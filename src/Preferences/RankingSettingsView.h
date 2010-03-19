/*RankingSettingsView.h
	Definitions and objects for the view to change settings for calculation
	of app rank in the engine
*/
#ifndef EP_RANKING_VIEW
#define EP_RANKING_VIEW
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <fstream>
#include "prefs_constants.h"
#include "SystemStatusBox.h"

class RankingSettingsView : public BView
{public:
					RankingSettingsView(BRect);
					~RankingSettingsView();
	virtual void	MessageReceived(BMessage*);
	void			setSliderValues(engine_prefs& prefs);
	void			getSliderValues(engine_prefs& prefs);
	BSize			GetMinSize();
private:
	BSlider			*launchesSl, *firstSl, *lastSl, *intervalSl, *runtimeSl;
	BButton			*setB;
	BTextView		*ranksTV, *quartilesTV, *dataTV;
	BBox			*slidersBox, *rankBox;
	void			InitSlider(BSlider *slider);
	void			setSliderScaleLabel(BSlider*, int);
	void			setSliderScale(BSlider*, int);
};

#endif
