/* SettingsWindow.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(AppSettings* settings, ScaleSettings* scales,
								BMessage *appExclusions)
	:
	BWindow(BRect(), "Einsteinium Launcher Settings", B_FLOATING_WINDOW_LOOK,
	B_MODAL_ALL_WINDOW_FEEL, B_NOT_ZOOMABLE | /*B_NOT_RESIZABLE | */B_ASYNCHRONOUS_CONTROLS)
{
	Lock();
	BRect bounds = Bounds();
	fLayoutView = new SettingsView(bounds, settings);
	fRankingView = new LauncherRankingsView(bounds, scales);
	fExclusionsView = new LauncherExclusionsView(bounds, appExclusions);
//	AddChild(fView);
	BTabView *tabView = new BTabView("Settings", B_WIDTH_FROM_LABEL);
	BTab *guiTab = new BTab();
	tabView->AddTab(fLayoutView, guiTab);
	BTab *rankTab = new BTab();
	tabView->AddTab(fRankingView, rankTab);
	BTab *exclusionsTab = new BTab();
	tabView->AddTab(fExclusionsView, exclusionsTab);

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(tabView)
		.SetInsets(2, 2, 2, 2)
	);

	BSize size = ChildAt(0)->PreferredSize();
	ResizeTo(size.width, size.height);

	Unlock();
}


bool
SettingsWindow::QuitRequested()
{
	Hide();
	return false;
}


void
SettingsWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{

		case EL_APP_ICON_OPTION_DRAG:
		case EL_DOC_ICON_OPTION_DRAG:
		{
			fLayoutView->MessageReceived(msg);
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
SettingsWindow::Show(BRect mainWindowFrame)
{
	// detect if there is enough room in the main screen
	BPoint windowLeftTop(mainWindowFrame.right + 10, mainWindowFrame.top);
	BScreen screen;
	if(screen.IsValid())
	{
		BRect screenFrame = screen.Frame();
		float availableWidth = screenFrame.right - mainWindowFrame.right;
		if(availableWidth < Frame().Width())
			windowLeftTop.x = screenFrame.right - Frame().Width() - 5;
		float availableHeight = screenFrame.bottom - mainWindowFrame.top;
		if(availableHeight < Frame().Height())
			windowLeftTop.y = screenFrame.bottom - Frame().Height() - 5;
	}
	MoveTo(windowLeftTop.x, windowLeftTop.y);
	BWindow::Show();
}


AppSettings
SettingsWindow::GetAppSettings()
{
	return fLayoutView->GetAppSettings();
}


ScaleSettings
SettingsWindow::GetScaleSettings()
{
	ScaleSettings settings;
	fRankingView->GetSliderValues(settings);
	return settings;
}


BMessage
SettingsWindow::GetAppExclusions()
{
	BMessage list(EL_MESSAGE_WHAT_EXCLUDED_APPS);
	fExclusionsView->GetExclusionsList(list);
	return list;
}

void
SettingsWindow::SetAppExclusions(BMessage *exclusionsList)
{
	fExclusionsView->PopulateExclusionsList(*exclusionsList);
}
