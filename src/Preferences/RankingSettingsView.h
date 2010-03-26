/* RankingSettingsView.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
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

class RankingSettingsView : public BView {
public:
					RankingSettingsView(BRect);
					~RankingSettingsView();
	virtual void	MessageReceived(BMessage*);
	void			SetSliderValues(engine_prefs& prefs);
	void			GetSliderValues(engine_prefs& prefs);
	BSize			GetMinSize();
private:
	BSlider			*fLaunchesSl, *fFirstSl, *fLastSl, *fIntervalSl, *fRuntimeSl;
	BButton			*fSetB;
	BBox			*fSlidersBox;
	void			_InitSlider(BSlider *slider);
	void			_SetSliderScaleLabel(BSlider*, int);
	void			_SetSliderScale(BSlider*, int);
};

#endif
