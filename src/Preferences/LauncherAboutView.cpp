/* LauncherAboutView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherAboutView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Launcher about view"

LauncherAboutView::LauncherAboutView(BRect size)
	:
	BView(size, "Launcher Status", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	// About box
	fAboutBox = new BBox("About");
	fAboutBox->SetLabel(B_TRANSLATE_COMMENT("About Einsteinum Launcher", "Box label"));
	fAboutTextView = new BTextView("About text");
	fAboutTextView->SetText(B_TRANSLATE_COMMENT("The Einsteinium Launcher is a smart application and file launcher"
					" which displays lists of applications, files, folders and queries."
					" The Launcher queries the Engine to create the list of applications that are"
					" ranked by weighted criteria that you define. This allows you to have a"
					" launcher with applications ordered the way you want them.", "About text"));
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL);
	fAboutBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
		.Add(fAboutTextView)
		.SetInsets(10, 20, 10, 10)
	;

	// Launch settings box
	fSettingsBox = new BBox("Settings");
	fSettingsBox->SetLabel(B_TRANSLATE_COMMENT("Launcher Settings", "Box label"));
	fSettingsTV = new BTextView("Settings Instructions");
	fSettingsTV->SetText(B_TRANSLATE_COMMENT("The Launcher settings are set within the Launcher application."
						"  Click the button below to make changes to the Launcher settings.", "Description text"));
	fSettingsTV->MakeSelectable(false);
	fSettingsTV->MakeEditable(false);
	fSettingsButton = new BButton(B_TRANSLATE_COMMENT("Open Launcher Settings", "Button label"), new BMessage(OPEN_LAUNCHER_SETTINGS));
	BGroupLayout *settingsBoxLayout = new BGroupLayout(B_VERTICAL);
	fSettingsBox->SetLayout(settingsBoxLayout);
	BLayoutBuilder::Group<>(settingsBoxLayout)
		.Add(fSettingsTV)
		.Add(fSettingsButton)
		.SetInsets(10, 20, 10, 10)
	;

	// Layout
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fAboutBox, 0)
		.Add(fSettingsBox, 0)
		.AddGlue()
	;

}


void
LauncherAboutView::AttachedToWindow()
{
	SetViewColor(Parent()->ViewColor());
	fAboutTextView->SetViewColor(Parent()->ViewColor());
	fSettingsTV->SetViewColor(Parent()->ViewColor());
	BView::AttachedToWindow();
}


void
LauncherAboutView::FrameResized(float width, float height)
{
	fAboutBox->Invalidate();
	fAboutTextView->Invalidate();
	InvalidateLayout();
	BView::FrameResized(width, height);
}
