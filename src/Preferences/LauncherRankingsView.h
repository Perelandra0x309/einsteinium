/* LauncherRankingsView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_RANKINGS_VIEW
#define EP_L_RANKINGS_VIEW

#include <InterfaceKit.h>
#include <SupportKit.h>
#include <LayoutBuilder.h>
#include <fstream>
#include "prefs_constants.h"
#include "SystemStatusBox.h"

class LauncherRankingsView : public BView {
public:
					LauncherRankingsView(BRect);
//					~LauncherRankingsView();
	virtual void	MessageReceived(BMessage*);
	virtual void	FrameResized(float width, float height);
	void			SetSliderValues(scale_settings& prefs);
	void			GetSliderValues(scale_settings& prefs);
	BSize			GetMinSize();
private:
	BSlider			*fLaunchesSl, *fFirstSl, *fLastSl, *fIntervalSl, *fRuntimeSl;
	BButton			*fSetB;
	BBox			*fSlidersBox;
	bool			fSlidersChanged;
	void			_InitSlider(BSlider *slider);
	void			_SetSliderScaleLabel(BSlider*, int);
	void			_SetSliderScale(BSlider*, int);
	void			_SetSlidersChanged(bool changed);
};

#endif
