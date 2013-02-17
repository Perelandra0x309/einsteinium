/* MainView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainView.h"

MainView::MainView(BRect size)
	:
	BTabView("MainView", B_WIDTH_FROM_LABEL),
	fSelectedListView(NULL),
	fLastRecentDocRef()
{
	AppSettings *settings = GetAppSettings();
	BRect viewRect(Bounds());
//	viewRect.InsetBy(-2,-2);
//	viewRect.right-=(B_V_SCROLL_BAR_WIDTH+6);
//	viewRect.bottom-=(TabHeight()+6);
	fAppsListView = new AppsListView(viewRect);
	fAppsListView->SetFontSizeForValue(settings->fontSize);
	fAppsScrollView = new BScrollView("Apps", fAppsListView, /*B_FOLLOW_ALL_SIDES, */0, false, true, B_NO_BORDER);

	fDocsListView = new RecentDocsBListView(viewRect);
	fDocsListView->SetFontSizeForValue(settings->fontSize);
	fDocsScrollView = new BScrollView("Files", fDocsListView, /*B_FOLLOW_ALL_SIDES, */0, false, true, B_NO_BORDER);

	fFoldersListView = new RecentFoldersBListView(viewRect);
	fFoldersListView->SetFontSizeForValue(settings->fontSize);
	fFoldersScrollView = new BScrollView("Folders & Queries", fFoldersListView, /*B_FOLLOW_ALL_SIDES, */0, false, true, B_NO_BORDER);

	fAppsTab = new BTab();
	AddTab(fAppsScrollView, fAppsTab);
	fRecentDocsTab = new BTab();
	AddTab(fDocsScrollView, fRecentDocsTab);
	fRecentFoldersTab = new BTab();
	AddTab(fFoldersScrollView, fRecentFoldersTab);

	fTabCount = CountTabs();
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
		case EL_REDIRECTED_MOUSE_WHEEL_CHANGED:
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
		}
		case EL_UPDATE_RECENT_LISTS:
		{
			_BuildAppsListViewFromRecent();
			_BuildDocsListView();
			_BuildFoldersListView();
			break;
		}
		case EL_UPDATE_RECENT_LISTS_FORCE:
		{
			_BuildAppsListViewFromRecent(true);
			_BuildDocsListView(true);
			_BuildFoldersListView(true);
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
			if(fSelectedListView==fAppsListView)
			{
				fAppsListView->Invalidate();
				Window()->UpdateIfNeeded();
			}
			break;
		}
		default:
			BTabView::MessageReceived(msg);
	}
}


void
MainView::KeyDown(const char* bytes, int32 numbytes)
{
	if(numbytes == 1) {
		switch(bytes[0]) {
			case B_TAB:
			{
				int32 selection = Selection();
				Select((selection+1) % fTabCount);
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
				fSelectedListView->KeyDown(bytes, numbytes);
				break;
			}
			default:
			{
				if( bytes[0] >= 'A' && bytes[0] <= 'z')
				{
				//	printf("Key: %c\n", bytes[0]);
					if(fSelectedListView==fAppsListView)
						fAppsListView->ScrollToNextAppBeginningWith(bytes[0]);
					else if(fSelectedListView==fDocsListView)
						fDocsListView->ScrollToNextDocBeginningWith(bytes[0]);
					else if(fSelectedListView==fFoldersListView)
						fFoldersListView->ScrollToNextDocBeginningWith(bytes[0]);
				}
				else
					BTabView::KeyDown(bytes, numbytes);
			}
		}
	}
	else
	{
		BTabView::KeyDown(bytes, numbytes);
	}
}


void
MainView::Select(int32 tab)
{
	BTabView::Select(tab);
	_UpdateSelectedListView();
}


void
MainView::SelectDefaultTab()
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
MainView::_UpdateSelectedListView()
{
	int32 selection = Selection();
	switch(selection) {
		case 0: {
			Window()->UpdateIfNeeded();
			fSelectedListView = fAppsListView;
			_BuildAppsListViewFromRecent();
			fAppsListView->SendInfoViewUpdate();
			break;
		}
		case 1: {
			fSelectedListView = fDocsListView;
			_BuildDocsListView();
			fDocsListView->SendInfoViewUpdate();
			break;
		}
		case 2: {
			fSelectedListView = fFoldersListView;
			_BuildFoldersListView();
			fFoldersListView->SendInfoViewUpdate();
			break;
		}
	}
	fAppsListView->SetShowing(fSelectedListView==fAppsListView);
	fDocsListView->SetShowing(fSelectedListView==fDocsListView);
	fFoldersListView->SetShowing(fSelectedListView==fFoldersListView);
}


void
MainView::BuildAppsListView(BMessage *message)
{
	fAppsListView->BuildAppsListView(message);
	MakeFocus();
}


void
MainView::_BuildAppsListViewFromRecent(bool force=false)
{
	fAppsListView->BuildAppsListFromRecent(force);
	MakeFocus();
}


void
MainView::_BuildDocsListView(bool force=false)
{
	fDocsListView->BuildList(force);
	MakeFocus();
}

void
MainView::_BuildFoldersListView(bool force=false)
{
	fFoldersListView->BuildList(force);
	MakeFocus();
}

