/* SettingsWindow.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "SettingsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Settings window"

SettingsWindow::SettingsWindow(AppSettings* settings, ScaleSettings* scales,
								BMessage *appExclusions)
	:
	BWindow(BRect(), B_TRANSLATE_COMMENT("Einsteinium Launcher Settings", "Settings window title"),
		B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS)
{
	Lock();
	BRect bounds = Bounds();
	fLayoutView = new SettingsView(bounds, settings);
	fRankingView = new LauncherRankingsView(bounds, scales);
	fExclusionsView = new LauncherExclusionsView(bounds, appExclusions);
	fTabView = new BTabView("Settings", B_WIDTH_FROM_LABEL);
	BTab *guiTab = new BTab();
	fTabView->AddTab(fLayoutView, guiTab);
	BTab *rankTab = new BTab();
	fTabView->AddTab(fRankingView, rankTab);
	BTab *exclusionsTab = new BTab();
	fTabView->AddTab(fExclusionsView, exclusionsTab);

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fTabView)
		.SetInsets(2, 2, 2, 2)
	);
	
	BSize size = ChildAt(0)->PreferredSize();
	SetSizeLimits(size.Width(), 9999, size.Height(), 9999);

	PopulateAppSettings(settings);
	Unlock();
}


bool
SettingsWindow::QuitRequested()
{
	fTabView->MakeFocus(true);
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
		case EL_DESKBAR_OPTION_CHANGED:
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


void
SettingsWindow::SelectTab(uint32 which)
{
	switch(which) {
		case EL_SHOW_SETTINGS_LAYOUT:
			Lock();
			fTabView->Select(0);
			Unlock();
			break;
		case EL_SHOW_SETTINGS_RANKINGS:
			Lock();
			fTabView->Select(1);
			Unlock();
			break;
		case EL_SHOW_SETTINGS_EXCLUSIONS:
			Lock();
			fTabView->Select(2);
			Unlock();
			break;
	}
}


/*
AppSettings
SettingsWindow::GetAppSettings()
{
	return fLayoutView->GetAppSettings();
}*/

void
SettingsWindow::PopulateAppSettings(AppSettings *settings)
{
	return fLayoutView->PopulateAppSettings(settings);
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
	fExclusionsView->PopulateExclusionsListView(*exclusionsList);
}
