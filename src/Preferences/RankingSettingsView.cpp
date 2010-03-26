/* RankingSettingsView.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "RankingSettingsView.h"

/*	Definitions and objects for the view to change settings for calculation
	of app rank in the engine
*/

rgb_color kFillColor = {200,255,200,0};
rgb_color kBarColor = {205,205,255,0};

RankingSettingsView::RankingSettingsView(BRect size)
	:
	BView(size, "Recent Apps Rank", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor(bg_color);

	fLaunchesSl = new BSlider("Launches", "Number of Launches",
							new BMessage(EE_LAUNCHES_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fLaunchesSl);
	BBox *launchesBox = new BBox("Launches Box");
	launchesBox->SetBorder(B_PLAIN_BORDER);
	launchesBox->SetLabel("Total number of launches:");
	launchesBox->AddChild(fLaunchesSl);
	fFirstSl = new BSlider("First Launch", "First Launch",
							new BMessage(EE_FIRST_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fFirstSl);
	BBox *firstBox = new BBox("First Box");
	firstBox->SetBorder(B_PLAIN_BORDER);
	firstBox->SetLabel("Date of very first launch:");
	firstBox->AddChild(fFirstSl);
	fLastSl = new BSlider("Last Launch", "Last Launch",
							new BMessage(EE_LAST_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fLastSl);
	BBox *lastBox = new BBox("Last Box");
	lastBox->SetBorder(B_PLAIN_BORDER);
	lastBox->SetLabel("Date of most recent launch:");
	lastBox->AddChild(fLastSl);
	fIntervalSl = new BSlider("Interval", "Interval between last two launches",
							new BMessage(EE_INTERVAL_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fIntervalSl);
	BBox *intervalBox = new BBox("Interval Box");
	intervalBox->SetBorder(B_PLAIN_BORDER);
	intervalBox->SetLabel("Time lapsed between the last two launches:");
	intervalBox->AddChild(fIntervalSl);
	fRuntimeSl = new BSlider("RunTime", "Total Running Time",
							new BMessage(EE_RUNTIME_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fRuntimeSl);
	BBox *runtimeBox = new BBox("Runtime Box");
	runtimeBox->SetBorder(B_PLAIN_BORDER);
	runtimeBox->SetLabel("Total lifetime running time:");
	runtimeBox->AddChild(fRuntimeSl);
	fSetB = new BButton("Set Button", "Save and Recalculate Scores", new BMessage(SAVE_RANKING));
	fSlidersBox = new BBox("Rank Sliders");
	fSlidersBox->SetLabel("Rank Weights");
	fSlidersBox->AddChild(BGroupLayoutBuilder(B_VERTICAL, 5)
		.AddGlue()
		.Add(launchesBox)
		.AddGlue()
		.Add(firstBox)
		.AddGlue()
		.Add(lastBox)
		.AddGlue()
		.Add(intervalBox)
		.AddGlue()
		.Add(runtimeBox)
		.AddGlue()
		.Add(BGroupLayoutBuilder(B_HORIZONTAL, 5)
			.AddGlue()
			.Add(fSetB)
		)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(fSlidersBox)
	);
}


RankingSettingsView::~RankingSettingsView()
{	}


void
RankingSettingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case EE_LAUNCHES_SL_CHANGED:
		{	_SetSliderScaleLabel(fLaunchesSl, fLaunchesSl->Value());
			break; }
		case EE_FIRST_SL_CHANGED:
		{	_SetSliderScaleLabel(fFirstSl, fFirstSl->Value());
			break; }
		case EE_LAST_SL_CHANGED:
		{	_SetSliderScaleLabel(fLastSl, fLastSl->Value());
			break; }
		case EE_INTERVAL_SL_CHANGED:
		{	_SetSliderScaleLabel(fIntervalSl, fIntervalSl->Value());
			break; }
		case EE_RUNTIME_SL_CHANGED:
		{	_SetSliderScaleLabel(fRuntimeSl, fRuntimeSl->Value());
			break; }
	}
}


// Set the standard slider properties
void
RankingSettingsView::_InitSlider(BSlider *slider)
{
	slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	slider->SetHashMarkCount(11);
	slider->SetLimitLabels("Decrease Rank","Increase Rank");
	slider->SetBarColor(kBarColor);
	slider->UseFillColor(true, &kFillColor);
}


void
RankingSettingsView::_SetSliderScaleLabel(BSlider* sldr, int n)
{
	BString label;
	if(sldr==fLaunchesSl)
		label.SetTo("More launches will ");
	else if(sldr==fFirstSl)
		label.SetTo("More recent dates will ");
	else if(sldr==fLastSl)
		label.SetTo("More recent dates will ");
	else if(sldr==fIntervalSl)
		label.SetTo("Shorter interval will ");
	else if(sldr==fRuntimeSl)
		label.SetTo("Higher runtime total will ");
	else
		return;
	switch(n)
	{
		case -5:
		{
			label.Append("very heavily decrease the rank");
			break;
		}
		case -4:
		{
			label.Append("heavily decrease the rank");
			break;
		}
		case -3:
		{
			label.Append("moderately decrease the rank");
			break;
		}
		case -2:
		{
			label.Append("slightly decrease the rank");
			break;
		}
		case -1:
		{
			label.Append("minimally decrease the rank");
			break;
		}
		case 0:
		{
			label.Append("have no effect");
			break;
		}
		case 1:
		{
			label.Append("minimally increase the rank");
			break;
		}
		case 2:
		{
			label.Append("slightly increase the rank");
			break;
		}
		case 3:
		{
			label.Append("moderately increase the rank");
			break;
		}
		case 4:
		{
			label.Append("heavily increase the rank");
			break;
		}
		case 5:
		{
			label.Append("very heavily increase the rank");
			break;
		}
	}
	sldr->SetLabel(label.String());
}


void
RankingSettingsView::_SetSliderScale(BSlider* sldr, int n)
{
	sldr->SetValue(n);
	_SetSliderScaleLabel(sldr, n);
}


void
RankingSettingsView::SetSliderValues(engine_prefs& prefs)
{
	_SetSliderScale(fLaunchesSl, prefs.launches_scale);
	_SetSliderScale(fFirstSl, prefs.first_launch_scale);
	_SetSliderScale(fLastSl, prefs.last_launch_scale);
	_SetSliderScale(fIntervalSl, prefs.interval_scale);
	_SetSliderScale(fRuntimeSl, prefs.total_run_time_scale);
}


void
RankingSettingsView::GetSliderValues(engine_prefs& prefs)
{
	prefs.launches_scale = fLaunchesSl->Value();
	prefs.first_launch_scale = fFirstSl->Value();
	prefs.last_launch_scale = fLastSl->Value();
	prefs.interval_scale = fIntervalSl->Value();
	prefs.total_run_time_scale = fRuntimeSl->Value();
}


BSize
RankingSettingsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = MinSize().width + 10;
	size.height = MinSize().height;
	return size;
}
