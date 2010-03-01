/*RankingSettingsView.cpp
	Create view for altering rank calculation method
*/
#include "RankingSettingsView.h"

rgb_color fillColor = {200,255,200,0};
rgb_color barColor = {205,205,255,0};

RankingSettingsView::RankingSettingsView(BRect size)
	:BView(size, "Recent Apps Rank", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{	SetViewColor(bg_color);

	launchesSl = new BSlider("Launches", "Number of Launches",
							new BMessage(EE_LAUNCHES_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	InitSlider(launchesSl);
	BBox *launchesBox = new BBox("Launches Box");
	launchesBox->SetBorder(B_PLAIN_BORDER);
	launchesBox->SetLabel("Total number of launches:");
	launchesBox->AddChild(launchesSl);
	firstSl = new BSlider("First Launch", "First Launch",
							new BMessage(EE_FIRST_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	InitSlider(firstSl);
	BBox *firstBox = new BBox("First Box");
	firstBox->SetBorder(B_PLAIN_BORDER);
	firstBox->SetLabel("Date of very first launch:");
	firstBox->AddChild(firstSl);
	lastSl = new BSlider("Last Launch", "Last Launch",
							new BMessage(EE_LAST_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	InitSlider(lastSl);
	BBox *lastBox = new BBox("Last Box");
	lastBox->SetBorder(B_PLAIN_BORDER);
	lastBox->SetLabel("Date of most recent launch:");
	lastBox->AddChild(lastSl);
	intervalSl = new BSlider("Interval", "Interval between last two launches",
							new BMessage(EE_INTERVAL_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	InitSlider(intervalSl);
	BBox *intervalBox = new BBox("Interval Box");
	intervalBox->SetBorder(B_PLAIN_BORDER);
	intervalBox->SetLabel("Time lapsed between the last two launches:");
	intervalBox->AddChild(intervalSl);
	runtimeSl = new BSlider("RunTime", "Total Running Time",
							new BMessage(EE_RUNTIME_SL_CHANGED),
							-5, 5, B_HORIZONTAL, B_TRIANGLE_THUMB);
	InitSlider(runtimeSl);
	BBox *runtimeBox = new BBox("Runtime Box");
	runtimeBox->SetBorder(B_PLAIN_BORDER);
	runtimeBox->SetLabel("Total lifetime running time:");
	runtimeBox->AddChild(runtimeSl);
	setB = new BButton("Set Button", "Save and Recalculate Scores", new BMessage(SAVE_RANKING));
	slidersBox = new BBox("Rank Sliders");
	slidersBox->SetLabel("Rank Weights");
	slidersBox->AddChild(BGroupLayoutBuilder(B_VERTICAL, 5)
		/*.Add(launchesSl)
		.Add(firstSl)
		.Add(lastSl)
		.Add(intervalSl)
		.Add(runtimeSl)*/
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
			.Add(setB)
		)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(slidersBox)
	);

}
RankingSettingsView::~RankingSettingsView()
{	}
void RankingSettingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case EE_LAUNCHES_SL_CHANGED:
		{	setSliderScaleLabel(launchesSl, launchesSl->Value());
			break; }
		case EE_FIRST_SL_CHANGED:
		{	setSliderScaleLabel(firstSl, firstSl->Value());
			break; }
		case EE_LAST_SL_CHANGED:
		{	setSliderScaleLabel(lastSl, lastSl->Value());
			break; }
		case EE_INTERVAL_SL_CHANGED:
		{	setSliderScaleLabel(intervalSl, intervalSl->Value());
			break; }
		case EE_RUNTIME_SL_CHANGED:
		{	setSliderScaleLabel(runtimeSl, runtimeSl->Value());
			break; }
	}
}

// Set the standard slider properties
void RankingSettingsView::InitSlider(BSlider *slider)
{
	slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	slider->SetHashMarkCount(11);
	slider->SetLimitLabels("Decrease Rank","Increase Rank");
	slider->SetBarColor(barColor);
	slider->UseFillColor(true, &fillColor);
}

void RankingSettingsView::setSliderScaleLabel(BSlider* sldr, int n)
{	BString label;
	if(sldr==launchesSl)
	{	label.SetTo("More launches will "); }
	else if(sldr==firstSl)
	{	label.SetTo("More recent dates will "); }
	else if(sldr==lastSl)
	{	label.SetTo("More recent dates will "); }
	else if(sldr==intervalSl)
	{	label.SetTo("Shorter interval will "); }
	else if(sldr==runtimeSl)
	{	label.SetTo("Higher runtime total will "); }
	else { return; }
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
void RankingSettingsView::setSliderScale(BSlider* sldr, int n)
{	sldr->SetValue(n);
	setSliderScaleLabel(sldr, n);
}
void RankingSettingsView::setSliderValues(engine_prefs& prefs)
{	setSliderScale(launchesSl, prefs.launches_scale);
	setSliderScale(firstSl, prefs.first_launch_scale);
	setSliderScale(lastSl, prefs.last_launch_scale);
	setSliderScale(intervalSl, prefs.interval_scale);
	setSliderScale(runtimeSl, prefs.total_run_time_scale);
}
void RankingSettingsView::getSliderValues(engine_prefs& prefs)
{	prefs.launches_scale = launchesSl->Value();
	prefs.first_launch_scale = firstSl->Value();
	prefs.last_launch_scale = lastSl->Value();
	prefs.interval_scale = intervalSl->Value();
	prefs.total_run_time_scale = runtimeSl->Value();
}

BSize RankingSettingsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = MinSize().width + 10;
	size.height = MinSize().height;
	return size;
}
