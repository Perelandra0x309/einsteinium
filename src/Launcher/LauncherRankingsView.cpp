/* LauncherRankingsView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherRankingsView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Settings 'Apps Ranking' tab"

/*	Definitions and objects for the view to change settings for calculation
	of app rank in the engine
*/

rgb_color kFillColor = {200,255,200,0};
rgb_color kBarColor = {205,205,255,0};

LauncherRankingsView::LauncherRankingsView(BRect size, ScaleSettings* scales)
	:
	BView(size, "Apps Ranking", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
//	SetViewColor(bg_color);

	fLaunchesSl = new BSlider("Launches", "Number of Launches",
							NULL, -5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fLaunchesSl, EL_LAUNCHES_SL_CHANGED);
	BBox *launchesBox = new BBox("Launches Box");
	launchesBox->SetBorder(B_PLAIN_BORDER);
	launchesBox->SetLabel(B_TRANSLATE_COMMENT("Total number of launches:", "Box label"));
	launchesBox->AddChild(fLaunchesSl);
	fFirstSl = new BSlider("First Launch", "First Launch",
							NULL, -5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fFirstSl, EL_FIRST_SL_CHANGED);
	BBox *firstBox = new BBox("First Box");
	firstBox->SetBorder(B_PLAIN_BORDER);
	firstBox->SetLabel(B_TRANSLATE_COMMENT("Date of very first launch:", "Box label"));
	firstBox->AddChild(fFirstSl);
	fLastSl = new BSlider("Last Launch", "Last Launch",
							NULL, -5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fLastSl, EL_LAST_SL_CHANGED);
	BBox *lastBox = new BBox("Last Box");
	lastBox->SetBorder(B_PLAIN_BORDER);
	lastBox->SetLabel(B_TRANSLATE_COMMENT("Date of most recent launch:", "Box label"));
	lastBox->AddChild(fLastSl);
	fIntervalSl = new BSlider("Interval", "Interval between last two launches",
							NULL, -5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fIntervalSl, EL_INTERVAL_SL_CHANGED);
	BBox *intervalBox = new BBox("Interval Box");
	intervalBox->SetBorder(B_PLAIN_BORDER);
	intervalBox->SetLabel(B_TRANSLATE_COMMENT("Time lapsed between the last two launches:", "Box label"));
	intervalBox->AddChild(fIntervalSl);
	fRuntimeSl = new BSlider("RunTime", "Total Running Time",
							NULL, -5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	_InitSlider(fRuntimeSl, EL_RUNTIME_SL_CHANGED);
	BBox *runtimeBox = new BBox("Runtime Box");
	runtimeBox->SetBorder(B_PLAIN_BORDER);
	runtimeBox->SetLabel(B_TRANSLATE_COMMENT("Total lifetime running time:", "Box label"));
	runtimeBox->AddChild(fRuntimeSl);
	fSetB = new BButton("Set Button", B_TRANSLATE_COMMENT("Save and Recalculate Ranks", "Button label"), new BMessage(EL_SAVE_RANKING));
	fSetB->SetToolTip(B_TRANSLATE_COMMENT("Click this button once the sliders are at their proper values.\n"
							"The Launcher menu will be updated with these new settings.", "Button tooltip"));
	fSlidersBox = new BBox("Rank Sliders");
	fSlidersBox->SetLabel(B_TRANSLATE_COMMENT("Application Ranking Weights", "Box label"));

	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL, 5);
	fSlidersBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
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
		.Add(fSetB)
		.SetInsets(10, 20, 10, 10)
	;
	fSetB->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_VERTICAL_CENTER));

/*	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fSlidersBox)
		.SetInsets(4, 4, 4, 4)
	;*/

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 3)
		.Add(fSlidersBox)
		.SetInsets(4, 4, 4, 4)
	);

	SetSliderValues(*scales);
}

/*
LauncherRankingsView::~LauncherRankingsView()
{	}*/

void
LauncherRankingsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fLaunchesSl->SetTarget(this);
	fFirstSl->SetTarget(this);
	fLastSl->SetTarget(this);
	fIntervalSl->SetTarget(this);
	fRuntimeSl->SetTarget(this);
	fSetB->SetTarget(this);
}

void
LauncherRankingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case EL_LAUNCHES_SL_CHANGED:
		{	_SetSliderScaleLabel(fLaunchesSl, fLaunchesSl->Value());
			_SetSlidersChanged(true);
			break; }
		case EL_FIRST_SL_CHANGED:
		{	_SetSliderScaleLabel(fFirstSl, fFirstSl->Value());
			_SetSlidersChanged(true);
			break; }
		case EL_LAST_SL_CHANGED:
		{	_SetSliderScaleLabel(fLastSl, fLastSl->Value());
			_SetSlidersChanged(true);
			break; }
		case EL_INTERVAL_SL_CHANGED:
		{	_SetSliderScaleLabel(fIntervalSl, fIntervalSl->Value());
			_SetSlidersChanged(true);
			break; }
		case EL_RUNTIME_SL_CHANGED:
		{	_SetSliderScaleLabel(fRuntimeSl, fRuntimeSl->Value());
			_SetSlidersChanged(true);
			break; }
		case EL_SAVE_RANKING: {
			_SetSlidersChanged(false);
			be_app->PostMessage(EL_SAVE_RANKING);
			break; }
		default: {
			BView::MessageReceived(msg);
			break; }
	}
}


void
LauncherRankingsView::FrameResized(float width, float height)
{
	fSlidersBox->Invalidate();
	fLaunchesSl->Invalidate();
	fFirstSl->Invalidate();
	fLastSl->Invalidate();
	fIntervalSl->Invalidate();
	fRuntimeSl->Invalidate();
	fSetB->Invalidate();
	BView::FrameResized(width, height);
}


// Set the standard slider properties
void
LauncherRankingsView::_InitSlider(BSlider *slider, uint32 message)
{
	slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	slider->SetHashMarkCount(11);
	slider->SetLimitLabels(B_TRANSLATE_COMMENT("Decrease Rank", "Slider low text"),
						B_TRANSLATE_COMMENT("Increase Rank", "Slider high text"));
	slider->SetBarColor(kBarColor);
	slider->UseFillColor(true, &kFillColor);
	slider->SetModificationMessage(new BMessage(message));
	_SetSliderScale(slider, 0);
}


void
LauncherRankingsView::_SetSliderScaleLabel(BSlider* sldr, int n)
{
	BString label, tooltip(B_TRANSLATE_COMMENT("This slider will set how much the application's final rank is determined by\n", "Slider tooltip part 1"));
	if(sldr==fLaunchesSl)
	{
		label.SetTo(B_TRANSLATE_COMMENT("More launches will ", "Slider label part 1"));
		tooltip.Append(B_TRANSLATE_COMMENT("the total number of times the application has been launched.", "Slider tooltip part 2"));
	}
	else if(sldr==fFirstSl)
	{
		label.SetTo(B_TRANSLATE_COMMENT("More recent dates will ", "Slider label part 1"));
		tooltip.Append(B_TRANSLATE_COMMENT("the very first date and time the application was launched.", "Slider tooltip part 2"));
	}
	else if(sldr==fLastSl)
	{
		label.SetTo(B_TRANSLATE_COMMENT("More recent dates will ", "Slider label part 1"));
		tooltip.Append(B_TRANSLATE_COMMENT("the most recent date and time the application was launched.", "Slider tooltip part 2"));
	}
	else if(sldr==fIntervalSl)
	{
		label.SetTo(B_TRANSLATE_COMMENT("Shorter interval will ", "Slider label part 1"));
		tooltip.Append(B_TRANSLATE_COMMENT("the length of time between the application's two most recent launches.", "Slider tooltip part 2"));
	}
	else if(sldr==fRuntimeSl)
	{
		label.SetTo(B_TRANSLATE_COMMENT("Higher runtime total will ", "Slider label part 1"));
		tooltip.Append(B_TRANSLATE_COMMENT("the total cumulative running time of the application since its first launch.", "Slider tooltip part 2"));
	}
	else
		return;
	switch(n)
	{
		case -5:
		{
			label.Append(B_TRANSLATE_COMMENT("very heavily decrease the rank", "Slider label part 2"));
			break;
		}
		case -4:
		{
			label.Append(B_TRANSLATE_COMMENT("heavily decrease the rank", "Slider label part 2"));
			break;
		}
		case -3:
		{
			label.Append(B_TRANSLATE_COMMENT("moderately decrease the rank", "Slider label part 2"));
			break;
		}
		case -2:
		{
			label.Append(B_TRANSLATE_COMMENT("slightly decrease the rank", "Slider label part 2"));
			break;
		}
		case -1:
		{
			label.Append(B_TRANSLATE_COMMENT("minimally decrease the rank", "Slider label part 2"));
			break;
		}
		case 0:
		{
			label.Append(B_TRANSLATE_COMMENT("have no effect", "Slider label part 2"));
			break;
		}
		case 1:
		{
			label.Append(B_TRANSLATE_COMMENT("minimally increase the rank", "Slider label part 2"));
			break;
		}
		case 2:
		{
			label.Append(B_TRANSLATE_COMMENT("slightly increase the rank", "Slider label part 2"));
			break;
		}
		case 3:
		{
			label.Append(B_TRANSLATE_COMMENT("moderately increase the rank", "Slider label part 2"));
			break;
		}
		case 4:
		{
			label.Append(B_TRANSLATE_COMMENT("heavily increase the rank", "Slider label part 2"));
			break;
		}
		case 5:
		{
			label.Append(B_TRANSLATE_COMMENT("very heavily increase the rank", "Slider label part 2"));
			break;
		}
	}
	sldr->SetLabel(label.String());
	sldr->SetToolTip(tooltip);
}


void
LauncherRankingsView::_SetSliderScale(BSlider* sldr, int n)
{
	sldr->SetValue(n);
	_SetSliderScaleLabel(sldr, n);
}


void
LauncherRankingsView::_SetSlidersChanged(bool changed)
{
	fSlidersChanged = changed;
	fSetB->SetEnabled(changed);
	if(changed)
	{
		// Disable the button if all slider values are 0
		if( fLaunchesSl->Value()==0 && fFirstSl->Value()==0 && fLastSl->Value()==0
			&& fIntervalSl->Value()==0 && fRuntimeSl->Value()==0 )
			fSetB->SetEnabled(false);
	}
}


void
LauncherRankingsView::SetSliderValues(ScaleSettings& prefs)
{
	_SetSliderScale(fLaunchesSl, prefs.launches_scale);
	_SetSliderScale(fFirstSl, prefs.first_launch_scale);
	_SetSliderScale(fLastSl, prefs.last_launch_scale);
	_SetSliderScale(fIntervalSl, prefs.interval_scale);
	_SetSliderScale(fRuntimeSl, prefs.total_run_time_scale);
	_SetSlidersChanged(false);
}


void
LauncherRankingsView::GetSliderValues(ScaleSettings& prefs)
{
	prefs.launches_scale = fLaunchesSl->Value();
	prefs.first_launch_scale = fFirstSl->Value();
	prefs.last_launch_scale = fLastSl->Value();
	prefs.interval_scale = fIntervalSl->Value();
	prefs.total_run_time_scale = fRuntimeSl->Value();
}

/*
BSize
LauncherRankingsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = MinSize().width + 10;
	size.height = MinSize().height;
	return size;
}*/
