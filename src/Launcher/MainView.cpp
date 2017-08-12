/* MainView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Launcher tabs"

MainView::MainView(BRect size)
	:
	BView("MainView", B_WILL_DRAW),
//	fSelectedListView(NULL),
	fLastRecentDocRef()
{
	AppSettings *settings = GetAppSettings();
	BRect viewRect(Bounds());
	fAppsListView = new AppsListView(viewRect);
	fAppsListView->SetFontSizeForValue(settings->fontSize);
	fAppsScrollView = new BScrollView(B_TRANSLATE_COMMENT("Apps", "Tab label"),
					fAppsListView, /*B_FOLLOW_ALL_SIDES, */0, false, true);

	fDocsListView = new RecentDocsBListView(viewRect);
	fDocsListView->SetFontSizeForValue(settings->fontSize);
	fDocsScrollView = new BScrollView(B_TRANSLATE_COMMENT("Files", "Tab label"),
					fDocsListView, /*B_FOLLOW_ALL_SIDES, */0, false, true);

	fFoldersListView = new RecentFoldersBListView(viewRect);
	fFoldersListView->SetFontSizeForValue(settings->fontSize);
	fFoldersScrollView = new BScrollView(B_TRANSLATE_COMMENT("Folders & Queries", "Tab label"),
					fFoldersListView, /*B_FOLLOW_ALL_SIDES, */0, false, true);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
//		.SetInsets(-3, 0, -3, 0)
		.Add(fAppsScrollView)
		.Add(fDocsScrollView)
		.Add(fFoldersScrollView)
	.End();
/*	fAppsTab = new BTab();
	AddTab(fAppsScrollView, fAppsTab);
	fRecentDocsTab = new BTab();
	AddTab(fDocsScrollView, fRecentDocsTab);
	fRecentFoldersTab = new BTab();
	AddTab(fFoldersScrollView, fRecentFoldersTab);

	fTabCount = CountTabs();*/
	
}

/*
void
MainView::AllAttached()
{
	BTabView::AllAttached();
//	SelectDefaultTab();
}*/

/*
void
MainView::AttachedToWindow()
{
	BTabView::AttachedToWindow();
	//be_app->PostMessage(EL_UPDATE_RECENT_LISTS);
}*/

void
MainView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		// Catch mouse wheel events that are redirected from a listview that
		// is not showing and send it to the correct list view
/*		case EL_REDIRECTED_MOUSE_WHEEL_CHANGED:
		case B_MOUSE_WHEEL_CHANGED:
		{
		//	printf("Redirected mouse wheel changed message\n");
			if(fSelectedListView==fAppsListView)
				fAppsListView->HandleMouseWheelChanged(msg);
			else if(fSelectedListView==fDocsListView)
				fDocsListView->HandleMouseWheelChanged(msg);
			else if(fSelectedListView==fFoldersListView)
				fFoldersListView->HandleMouseWheelChanged(msg);
			break;
		}*/
		case EL_UPDATE_RECENT_LISTS:
		{
			_BuildAppsListViewFromRecent();
			_BuildDocsListView();
			_BuildFoldersListView();
			UpdateInfoView();
			break;
		}
		case EL_UPDATE_RECENT_LISTS_FORCE:
		{
			_BuildAppsListViewFromRecent(true);
			_BuildDocsListView(true);
			_BuildFoldersListView(true);
			UpdateInfoView();
			break;
		}
		case EL_EXCLUSIONS_CHANGED:
		{
			_BuildAppsListViewFromRecent(true);
			break;
		}
		case B_MODIFIERS_CHANGED:
		{
			// Invalidate the layout to force all ListItem objects to update
			// when a modifier key is pressed or released
//			if(fAppsListView->IsFocus())
//			{
				fAppsListView->Invalidate();
				Window()->UpdateIfNeeded();
//			}
			break;
		}
		case EL_INITIATE_INFO_VIEW_UPDATE:
			UpdateInfoView();
			InvalidateFocusView();
			break;
		default:
			BView::MessageReceived(msg);
	}
}

/*
void
MainView::KeyDown(const char* bytes, int32 numbytes)
{
	BView::KeyDown(bytes, numbytes);
	if (bytes[0] == B_TAB)
		UpdateInfoView();
	if(numbytes == 1) {
		switch(bytes[0]) {
			case B_TAB:
			{
			//	int32 selection = Selection();
			//	Select((selection+1) % fTabCount);
				if(fAppsListView->IsFocus())
					Select(1);
				else if(fDocsListView->IsFocus())
					Select(2);
				else if(fFoldersListView->IsFocus())
					Select(0);
				BView::KeyDown(bytes, numbytes);
				_UpdateSelectedListView();
				break;
			}
			case B_ESCAPE:
			{
				be_app->PostMessage(EL_HIDE_APP);
				break;
			}
			case B_RETURN:
			case B_SPACE:
			case B_DOWN_ARROW:
			case B_UP_ARROW:
			case B_LEFT_ARROW:
			case B_RIGHT_ARROW:
			{
				//fSelectedListView->KeyDown(bytes, numbytes);
				if(fAppsListView->IsFocus())
					fAppsListView->KeyDown(bytes, numbytes);
				else if(fDocsListView->IsFocus())
					fDocsListView->KeyDown(bytes, numbytes);
				else if(fFoldersListView->IsFocus())
					fFoldersListView->KeyDown(bytes, numbytes);
				break;
			}
			default:
			{
				if( bytes[0] >= 'A' && bytes[0] <= 'z')
				{
				//	printf("Key: %c\n", bytes[0]);
					if(fAppsListView->IsFocus())
						fAppsListView->ScrollToNextAppBeginningWith(bytes[0]);
					else if(fDocsListView->IsFocus())
						fDocsListView->ScrollToNextDocBeginningWith(bytes[0]);
					else if(fFoldersListView->IsFocus())
						fFoldersListView->ScrollToNextDocBeginningWith(bytes[0]);
				}
				else
					BView::KeyDown(bytes, numbytes);
			}
		}
	}
	else
	{
		BView::KeyDown(bytes, numbytes);
	}
}*/


void
MainView::Select(int32 which)
{
//	BTabView::Select(tab);
	switch (which)
	{
		case 0:
			fAppsListView->MakeFocus();
			break;
		case 1:
			fDocsListView->MakeFocus();
			break;
		case 2:
			fFoldersListView->MakeFocus();
			break;
	}
	_UpdateSelectedListView();
}


void
MainView::SelectDefaultView()
{
	Select(0);
}


void
MainView::SettingsChanged(uint32 what)
{
	switch(what)
	{
		case EL_APP_COUNT_OPTION_CHANGED:
		{
			_BuildAppsListViewFromRecent(true);
			break;
		}
		case EL_DOC_COUNT_OPTION_CHANGED:
		{
			_BuildDocsListView(true);
			break;
		}
		case EL_FOLDER_COUNT_OPTION_CHANGED:
		case EL_QUERY_COUNT_OPTION_CHANGED:
		{
			_BuildFoldersListView(true);
			break;
		}
		default:
			fAppsListView->SettingsChanged(what);
			fDocsListView->SettingsChanged(what);
			fFoldersListView->SettingsChanged(what);
	}

}


void
MainView::UpdateInfoView()
{
/*	int32 selection = Selection();
	switch(selection) {
		case 0: {
			fAppsListView->SendInfoViewUpdate();
			break;
		}
		case 1: {
			fDocsListView->SendInfoViewUpdate();
			break;
		}
		case 2: {
			fFoldersListView->SendInfoViewUpdate();
			break;
		}
	}*/
	if(fAppsListView->IsFocus())
		fAppsListView->SendInfoViewUpdate();
	else if(fDocsListView->IsFocus())
		fDocsListView->SendInfoViewUpdate();
	else if(fFoldersListView->IsFocus())
		fFoldersListView->SendInfoViewUpdate();
}


void
MainView::InvalidateFocusView()
{
	if(fAppsListView->IsFocus())
		fAppsListView->Invalidate();
	else if(fDocsListView->IsFocus())
		fDocsListView->Invalidate();
	else if(fFoldersListView->IsFocus())
		fFoldersListView->Invalidate();
}


void
MainView::_UpdateSelectedListView()
{
	fAppsListView->SetShowing(false);
	fDocsListView->SetShowing(false);
	fFoldersListView->SetShowing(false);
/*	int32 selection = Selection();
	switch(selection) {
		case 0: {*/
	if(fAppsListView->IsFocus()) {
		fAppsListView->SetShowing(true);
			Window()->UpdateIfNeeded();
//			fSelectedListView = fAppsListView;
			_BuildAppsListViewFromRecent();
			fAppsListView->SendInfoViewUpdate();
//			break;
		}
//		case 1: {
	else if(fDocsListView->IsFocus()) {
		fDocsListView->SetShowing(true);
//			fSelectedListView = fDocsListView;
			_BuildDocsListView();
			fDocsListView->SendInfoViewUpdate();
//			break;
		}
//		case 2: {
	else if(fFoldersListView->IsFocus()) {
		fFoldersListView->SetShowing(true);
//			fSelectedListView = fFoldersListView;
			_BuildFoldersListView();
			fFoldersListView->SendInfoViewUpdate();
//			break;
		}
//	}
//	fAppsListView->SetShowing(fSelectedListView==fAppsListView);
//	fDocsListView->SetShowing(fSelectedListView==fDocsListView);
//	fFoldersListView->SetShowing(fSelectedListView==fFoldersListView);*/
}


void
MainView::BuildAppsListView(BMessage *message)
{
	fAppsListView->BuildAppsListView(message);
//	Window()->Lock();
//	MakeFocus();
//	Window()->Unlock();
}


void
MainView::_BuildAppsListViewFromRecent(bool force)
{
	fAppsListView->BuildAppsListFromRecent(force);
//	Window()->Lock();
//	MakeFocus();
//	Window()->Unlock();
}


void
MainView::_BuildDocsListView(bool force)
{
	fDocsListView->BuildList(force);
//	Window()->Lock();
//	MakeFocus();
//	Window()->Unlock();
}

void
MainView::_BuildFoldersListView(bool force)
{
	fFoldersListView->BuildList(force);
//	Window()->Lock();
//	MakeFocus();
//	Window()->Unlock();
}

