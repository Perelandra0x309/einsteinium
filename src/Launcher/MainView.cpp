/* MainView.cpp
 * Copyright 2013-2017 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main View"

MainView::MainView(BRect size)
	:
	BView("MainView", B_WILL_DRAW),
	fLastRecentDocRef()
{
	AppSettings *settings = GetAppSettings();
	BRect viewRect(Bounds());
	fAppsListView = new AppsListView(viewRect);
	fAppsListView->SetFontSizeForValue(settings->fontSize);
	fAppsScrollView = new BScrollView("Apps", fAppsListView, 0, false, true);

	fDocsListView = new RecentDocsBListView(viewRect);
	fDocsListView->SetFontSizeForValue(settings->fontSize);
	fDocsScrollView = new BScrollView("Files", fDocsListView, 0, false, true);

	fFoldersListView = new RecentFoldersBListView(viewRect);
	fFoldersListView->SetFontSizeForValue(settings->fontSize);
	fFoldersScrollView = new BScrollView("Folders_Queries", fFoldersListView, 0, false, true);
	
	
	BStringView *appSV = new BStringView("Apps",
		B_TRANSLATE_COMMENT("Applications", "Apps list view header"));
	appSV->SetAlignment(B_ALIGN_CENTER);
//	appSV->SetFont(be_bold_font);
	BStringView *filesSV = new BStringView("Files",
		B_TRANSLATE_COMMENT("Files", "Files list view header"));
	filesSV->SetAlignment(B_ALIGN_CENTER);
//	filesSV->SetFont(be_bold_font);
	BStringView *foldersSV = new BStringView("F&Q",
		B_TRANSLATE_COMMENT("Folders & Queries", "Folders & Queries list view header"));
	foldersSV->SetAlignment(B_ALIGN_CENTER);
//	foldersSV->SetFont(be_bold_font);
	float maxWidth = appSV->StringWidth(appSV->Text());
	maxWidth = std::max(maxWidth, filesSV->StringWidth(filesSV->Text()));
	maxWidth = std::max(maxWidth, foldersSV->StringWidth(foldersSV->Text()));
	appSV->SetExplicitMinSize(BSize(maxWidth, B_SIZE_UNSET));
	filesSV->SetExplicitMinSize(BSize(maxWidth, B_SIZE_UNSET));
	foldersSV->SetExplicitMinSize(BSize(maxWidth, B_SIZE_UNSET));
	
	BLayoutBuilder::Grid<>(this, 0, 0)
		.Add(appSV, 0, 0)
		.Add(fAppsScrollView, 0, 1)
		.Add(filesSV, 1, 0)
		.Add(fDocsScrollView, 1, 1)
		.Add(foldersSV, 2, 0)
		.Add(fFoldersScrollView, 2, 1)
	.End();
}


void
MainView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
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
			fAppsListView->Invalidate();
			Window()->UpdateIfNeeded();
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void
MainView::Select(int32 which)
{
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
MainView::SelectedListViewChanged()
{
	_UpdateSelectedListView();
	InvalidateFocusView();
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
	fAppsListView->SetIsShowing(false);
	fDocsListView->SetIsShowing(false);
	fFoldersListView->SetIsShowing(false);
	if(fAppsListView->IsFocus()) {
		fAppsListView->SetIsShowing(true);
		Window()->UpdateIfNeeded();
		_BuildAppsListViewFromRecent();
		fAppsListView->SendInfoViewUpdate();
	}
	else if(fDocsListView->IsFocus()) {
		fDocsListView->SetIsShowing(true);
		_BuildDocsListView();
		fDocsListView->SendInfoViewUpdate();
	}
	else if(fFoldersListView->IsFocus()) {
		fFoldersListView->SetIsShowing(true);
		_BuildFoldersListView();
		fFoldersListView->SendInfoViewUpdate();
	}
}


void
MainView::BuildAppsListView(BMessage *message)
{
	fAppsListView->BuildAppsListView(message);
}


void
MainView::_BuildAppsListViewFromRecent(bool force)
{
	fAppsListView->BuildAppsListFromRecent(force);
}


void
MainView::_BuildDocsListView(bool force)
{
	fDocsListView->BuildList(force);
}

void
MainView::_BuildFoldersListView(bool force)
{
	fFoldersListView->BuildList(force);
}
