/* LauncherRankingsView.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_RANKINGS_VIEW
#define EP_L_RANKINGS_VIEW

#include <InterfaceKit.h>
//#include <SupportKit.h>
#include <Application.h>
#include <LayoutBuilder.h>
#include <GroupLayoutBuilder.h>
#include "launcher_constants.h"

class LauncherRankingsView : public BView {
public:
					LauncherRankingsView(BRect, ScaleSettings* scales);
//					~LauncherRankingsView();
	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage*);
	virtual void	FrameResized(float width, float height);
	void			SetSliderValues(ScaleSettings& prefs);
	void			GetSliderValues(ScaleSettings& prefs);
//	BSize			GetMinSize();
private:
	BSlider			*fLaunchesSl, *fFirstSl, *fLastSl, *fIntervalSl, *fRuntimeSl;
	BButton			*fSetB;
	BBox			*fSlidersBox;
	bool			fSlidersChanged;
	void			_InitSlider(BSlider *slider, uint32 message);
	void			_SetSliderScaleLabel(BSlider*, int);
	void			_SetSliderScale(BSlider*, int);
	void			_SetSlidersChanged(bool changed);
};

#endif
