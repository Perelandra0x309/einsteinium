/* AppMenuItem.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AppMenuItem.h"

AppMenuItem::AppMenuItem(entry_ref entry, BMessage *message)
	:
	BMenuItem(entry.name, message),
	fName(entry.name),
	fEntryRef(entry),
	fSuper(NULL),
	fIcon(NULL),
	fOffsetWidth(0),
	fInitStatus(B_ERROR)
{
	SetIconSize(B_MINI_ICON);
	fInitStatus = B_OK;
}


AppMenuItem::~AppMenuItem()
{
	delete fIcon;
}


void
AppMenuItem::SetIconSize(int value)
{
	fIconSize = value;
	fOffsetWidth = fIconSize + 2*kIconMargin;
	_GetIcon();
}


void
AppMenuItem::_GetIcon()
{
	delete fIcon;
	fIcon = NULL;
	if(fIconSize == 0)
		return;

	BNode node;
	status_t result = B_ERROR;
	if (node.SetTo(&fEntryRef) == B_OK) {
		BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
		fIcon = new BBitmap(iconRect, 0, B_RGBA32);
		BNodeInfo nodeInfo(&node);
		result = nodeInfo.GetTrackerIcon(fIcon, icon_size(fIconSize));
		if(result!=B_OK)
		{
			// Get icon from mime type
			char mimeString[B_MIME_TYPE_LENGTH];
			BMimeType nodeType;
			if(nodeInfo.GetType(mimeString) == B_OK)
			{
				nodeType.SetTo(mimeString);
				result = nodeType.GetIcon(fIcon, icon_size(fIconSize));
			}
			// Get super type icon
			if(result!=B_OK)
			{
				BMimeType superType;
				if(nodeType.GetSupertype(&superType)==B_OK)
				{
					result = superType.GetIcon(fIcon, icon_size(fIconSize));
				}

			}
		}
	}
	if(result!=B_OK)
	{
		delete fIcon;
		fIcon = NULL;
	}
}


void
AppMenuItem::DrawContent()
{
	//init values the first time drawing is requested
	if(fSuper == NULL)
	{
		fSuper = Menu();
		BFont font;
		fSuper->GetFont(&font);
		font.GetHeight(&fFontHeight);
	}

	fSuper->SetDrawingMode(B_OP_OVER);
	//draw icon
	if (fIcon) {
		fSuper->DrawBitmap(fIcon);
	}

	//offset by icon spacing and font height
	fSuper->MovePenBy(fOffsetWidth, fFontHeight.ascent + 1);

	float labelWidth, labelHeight;
	GetContentSize(&labelWidth, &labelHeight);
	float maxContentWidth = fSuper->MaxContentWidth();
	float frameWidth = maxContentWidth > 0 ? maxContentWidth
		: fSuper->Frame().Width();

	if (roundf(frameWidth) >= roundf(labelWidth))
		fSuper->DrawString(fName);
	else {
		// truncate label to fit
		char* truncatedLabel = new char[strlen(fName) + 4];
		TruncateLabel(frameWidth, truncatedLabel);
		fSuper->DrawString(truncatedLabel);
		delete[] truncatedLabel;
	}
}


void
AppMenuItem::GetContentSize(float *_width, float *_height)
{
	BMenuItem::GetContentSize(_width, _height);
	//Add 2 pixels to accommodate icon
	(*_height)+=2;
	(*_width)+=fOffsetWidth;
}
