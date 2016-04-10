#include "StdAfx.h"
#include "MenuIcon.h"
#include "Bitmap.h"

CMenuIcon::CMenuIcon(HINSTANCE phInstance)
{
	hInstance = phInstance;
	hMenuIcons = NULL;

	//Get the version of windows. The method of showing an icon is different in
	//WindowsXP and Windows Vista
	OSVERSIONINFO cVersionInformation;

	cWindowsVersion = eWindowsUnknown;
	cVersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if (GetVersionEx(&cVersionInformation)) {

		cWindowsVersion = eWindowsOlderThanXP;
		if (cVersionInformation.dwMajorVersion==5 && cVersionInformation.dwMinorVersion>0) 
			cWindowsVersion = eWindowsXP;
		if (cVersionInformation.dwMajorVersion>=6) cWindowsVersion = eWindowsNewerThanVista;
		if (cVersionInformation.dwMajorVersion==6 && cVersionInformation.dwMinorVersion==0) cWindowsVersion = eWindowsVista;
		}
	
	//Create the initial image list
	if (cWindowsVersion==eWindowsXP) {
		hMenuIcons = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
		}
	}

CMenuIcon::~CMenuIcon(void)
{
	//Destory the imagelist (Windows XP)
	if (hMenuIcons!=NULL) ImageList_Destroy(hMenuIcons);

	//Delete the bitmaps in the menu icons vector (Windows Vista)
	for (long lIndex=0; lIndex<(long)oMenuIcons.size(); lIndex++) {

		DeleteObject(oMenuIcons[lIndex]);
		}

	//clear all vectors
	oMenuIcons.clear();
	oMenuCommandIcons.clear();
}

//this function is the message handler ownerdrawing the menu icons
bool 
CMenuIcon::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;

	//Ownerdraw the items is only for WindowsXP
	if (cWindowsVersion!=eWindowsXP) return false; 
	
	//Process the messages
	bReturn = false;
	
	switch (message) {

		case WM_MEASUREITEM:

			//Set the measures of the item.
			LPMEASUREITEMSTRUCT oMeasureItem;

			oMeasureItem = (LPMEASUREITEMSTRUCT)lParam;

			if ((oMeasureItem!=NULL) && (oMeasureItem->CtlType == ODT_MENU)) {
				
				oMeasureItem->itemWidth = 20;
				oMeasureItem->itemHeight = 16;

				bReturn = true;
				}

			break;

		case WM_DRAWITEM: 

			//Draw the icon in front of the menu item.
			LPDRAWITEMSTRUCT oDrawItem;

			oDrawItem = (LPDRAWITEMSTRUCT)lParam;

			if (oDrawItem!=NULL) {
				
				if (oDrawItem->CtlType == ODT_MENU) {

					long lIndex;

					lIndex = GetIcon(oDrawItem->itemID);

					if (lIndex!=-1) {
					
						ImageList_Draw(hMenuIcons, 
										oMenuCommandIcons[lIndex].iImagelistIndex,
										oDrawItem->hDC,
										oDrawItem->rcItem.left, oDrawItem->rcItem.top,
										ILD_TRANSPARENT);
						bReturn = true;
						}
					}
				}
	
			break;
		}

	return bReturn;
	}

//This function iterates through the menu items and changes the
//item its properties so it is 'ownerdrawn'.
void 
CMenuIcon::ProcessMenu(HMENU phMenu) {

    MENUITEMINFO cMenuItemInfo;
	long lNumberItems;
	long lIconIndex;
	bool bIconPresent;
	
	lNumberItems = GetMenuItemCount(phMenu);
	bIconPresent = false;

    cMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	
	for (long lIndex=0; lIndex<lNumberItems; lIndex++) {

		cMenuItemInfo.fMask = MIIM_FTYPE | MIIM_BITMAP | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
		GetMenuItemInfo(phMenu, lIndex, TRUE, &cMenuItemInfo);

		//Enabled the menu item
		if (GetEnableMenuCommand(cMenuItemInfo.wID)) {
			cMenuItemInfo.fState &= ~MFS_DISABLED;
			cMenuItemInfo.fState |= MFS_ENABLED;
			}
		else {
			cMenuItemInfo.fState |= MFS_DISABLED;
			cMenuItemInfo.fState &= ~MFS_ENABLED;
			}

		//Check the menu item
		if (GetCheckMenuCommand(cMenuItemInfo.wID)) {
			cMenuItemInfo.fState &= ~MFS_UNCHECKED;
			cMenuItemInfo.fState |= MFS_CHECKED;
			}
		else {
			cMenuItemInfo.fState |= MFS_UNCHECKED;
			cMenuItemInfo.fState &= ~MFS_CHECKED;
			}

		//cMenuItemInfo.fMask = MIIM_STATE;
		SetMenuItemInfo(phMenu, lIndex, TRUE, &cMenuItemInfo);

		lIconIndex = GetIcon(cMenuItemInfo.wID);

		if (lIconIndex!=-1) {
	
			if (cWindowsVersion==eWindowsXP) {
				//There is an icon present. Change the properties so 
				//the callback will be made.
				cMenuItemInfo.hbmpItem = HBMMENU_CALLBACK;
				cMenuItemInfo.fType = MFT_STRING;		
				SetMenuItemInfo(phMenu, lIndex, TRUE, &cMenuItemInfo);
				bIconPresent = true;
				}

			if (cWindowsVersion==eWindowsVista || cWindowsVersion==eWindowsNewerThanVista) {
				
				if ((cMenuItemInfo.fState & MFS_CHECKED)==0) {
				
					//There is an icon present. Add the icon as a bitmap to the menuitem,
					//only if the item isn't checked. Adding an icon as bitmap will overwrite
					//this checkmark.
					SetMenuItemBitmaps(phMenu, lIndex, MF_BYPOSITION, 
						oMenuIcons[lIconIndex], oMenuIcons[lIconIndex]);
					}
				else {

					SetMenuItemBitmaps(phMenu, lIndex, MF_BYPOSITION, NULL, NULL);
					}
				}
			}
		
		//Iterate also through the submenu
		if (cMenuItemInfo.hSubMenu!=NULL) {

			ProcessMenu(cMenuItemInfo.hSubMenu);
			}		
		}

	//Add the style of the menu if an icon is present
	if (bIconPresent) {

		MENUINFO cMenuInfo;
		cMenuInfo.cbSize = sizeof(MENUINFO);
		cMenuInfo.fMask = MIM_STYLE;
		GetMenuInfo(phMenu, &cMenuInfo);
		cMenuInfo.dwStyle = MNS_CHECKORBMP | MNS_AUTODISMISS;
		SetMenuInfo(phMenu, &cMenuInfo);
		}
	}


//This function adds a menu command - icon combination to the
//vector.
void 
CMenuIcon::AddMenuCommandIcon(int piMenuCommand, int piMenuIcon, COLORREF pcTransparentColor) {

	sMenuCommandIcon cMenuCommandIcon;
	HBITMAP			 hBitmap;
	Bitmap*			 oBitmap;
	bool			 bFound;

	bFound = false;

	//loop through the known combinations. Only new ones are added
	for (long lIndex=0; lIndex < (long)oMenuCommandIcons.size() && !bFound; lIndex++) {

		bFound = (oMenuCommandIcons[lIndex].iMenuCommand == piMenuCommand &&
				  oMenuCommandIcons[lIndex].iMenuIcon    == piMenuIcon);
		};
		
	if (!bFound) {

		//the give combination hasn't been found, add this new combination
		cMenuCommandIcon.iMenuCommand = piMenuCommand;
		cMenuCommandIcon.iMenuIcon    = piMenuIcon;

		hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(piMenuIcon));			
		
		if (cWindowsVersion == eWindowsXP) {
			
			//In WindowsXP add the new icon to the imagelist
			cMenuCommandIcon.iImagelistIndex = ImageList_AddMasked(hMenuIcons, (HBITMAP)hBitmap, pcTransparentColor);
			}

		if (cWindowsVersion == eWindowsVista || cWindowsVersion == eWindowsNewerThanVista) {

			//In Windows Vista convert the icon the a 32bit ARGB image and add it to
			//our own bitmap vector
			oBitmap = new Bitmap(hBitmap, NULL);
			DeleteObject(hBitmap);

			Color cColor;	//the color of the current pixel
			BYTE* cBytes;	//the memory block containing the data of the ARGB bitmap
			long lPointer;	//pointer to a position in the memory block


			//A quick-and-dirty approach to convert the image to ARGB data
			cBytes = (BYTE*)malloc(oBitmap->GetWidth() * oBitmap->GetHeight() * 4);

			for (long lX = 0; lX<(long)oBitmap->GetWidth(); lX++) {

				for (long lY = 0; lY<(long)oBitmap->GetHeight(); lY++) {

					oBitmap->GetPixel(lX, lY, &cColor);

					lPointer = lX * 4 + lY * oBitmap->GetWidth() * 4;

					cBytes[lPointer+3] = 0xFF;
					cBytes[lPointer+2] = cColor.GetRed();
					cBytes[lPointer+1] = cColor.GetGreen();
					cBytes[lPointer+0] = cColor.GetBlue();

					if (cColor.ToCOLORREF()==pcTransparentColor) {

						cBytes[lPointer+0] = 0x00;
						cBytes[lPointer+1] = 0x00;
						cBytes[lPointer+2] = 0x00;
						cBytes[lPointer+3] = 0x00;
						}
					}
				}

			Bitmap* oARGBBitmap;
			oARGBBitmap = new Bitmap(oBitmap->GetWidth(), oBitmap->GetHeight(),
									 4 * oBitmap->GetWidth(),
									 PixelFormat32bppARGB,
									 cBytes);

			oARGBBitmap->GetHBITMAP(Color(0,0,0,0), &hBitmap);

			//Save the data to the icons vector
			cMenuCommandIcon.iImagelistIndex = oMenuIcons.size();
			oMenuIcons.push_back(hBitmap);

			//Clean up
			free(cBytes);
			delete oBitmap;
			delete oARGBBitmap;
			}

		oMenuCommandIcons.push_back(cMenuCommandIcon);
		}
	}

void 
CMenuIcon::AddMenuCommandIcon(int piMenuCommand, std::wstring psPNGImage) {

	sMenuCommandIcon cMenuCommandIcon;
	HBITMAP			 hBitmap;
	Bitmap*			 oBitmap;
	bool			 bFound;

	bFound = false;

	//loop through the known combinations. Only new ones are added
	for (long lIndex=0; lIndex < (long)oMenuCommandIcons.size() && !bFound; lIndex++) {

		bFound = (oMenuCommandIcons[lIndex].iMenuCommand == piMenuCommand &&
				  oMenuCommandIcons[lIndex].iMenuIcon    == -1 &&
				  oMenuCommandIcons[lIndex].sMenuIcon	 == psPNGImage);
		};
		
	if (!bFound) {

		//the give combination hasn't been found, add this new combination
		cMenuCommandIcon.iMenuCommand = piMenuCommand;
		cMenuCommandIcon.iMenuIcon    = -1;
		cMenuCommandIcon.sMenuIcon	  = psPNGImage;


		oBitmap = CBitmap::LoadFromResource(psPNGImage.c_str(), L"PNG", hInstance);

		if (oBitmap) {
			if (cWindowsVersion == eWindowsXP) {
				
				//In WindowsXP add the new icon to the imagelist
				cMenuCommandIcon.iImagelistIndex = CBitmap::AddToImagelist(oBitmap, hMenuIcons);
				}

			if (cWindowsVersion == eWindowsVista || cWindowsVersion == eWindowsNewerThanVista) {

				//In Windows Vista convert the icon the a 32bit ARGB image and add it to
				//our own bitmap vector
				oBitmap->GetHBITMAP(Color(0,0,0,0), &hBitmap);

				//Save the data to the icons vector
				cMenuCommandIcon.iImagelistIndex = oMenuIcons.size();
				oMenuIcons.push_back(hBitmap);

				//Clean up
				delete oBitmap;
				}
			oMenuCommandIcons.push_back(cMenuCommandIcon);
			}
		}
	}

void 
CMenuIcon::ReplaceMenuCommandIcon(int piMenuCommand, HBITMAP phBitmap) {

	sMenuCommandIcon cMenuCommandIcon;
	bool			 bFound;
	long			 lItem;

	bFound = false;
	lItem = -1;

	//loop through the known combinations. Only nexisting ones are replaced
	for (long lIndex=0; lIndex < (long)oMenuCommandIcons.size() && !bFound; lIndex++) {

		bFound = (oMenuCommandIcons[lIndex].iMenuCommand == piMenuCommand);
		lItem = lIndex;
		};
		
	if (bFound && lItem!=-1) {

		if (cWindowsVersion == eWindowsXP) {
			
			//In WindowsXP add the new icon to the imagelist
			ImageList_Replace(hMenuIcons, oMenuCommandIcons[lItem].iImagelistIndex, phBitmap, NULL);
			}

		if (cWindowsVersion == eWindowsVista || cWindowsVersion == eWindowsNewerThanVista) {

			//In Windows Vista replace the icon (a 32bit ARGB image) in our own bitmap vector
			DeleteObject(oMenuIcons[oMenuCommandIcons[lItem].iImagelistIndex]);
			oMenuIcons[oMenuCommandIcons[lItem].iImagelistIndex] = phBitmap;
			}
		}
	}


//This function retrieves the icon that belongs to the given command.
//If there isn't an icon the result is -1.
long 
CMenuIcon::GetIcon(int piMenuCommand) {

	long lResult;
	long lIndex;

	lResult = -1;
	lIndex = 0;

	while (lIndex<(long)oMenuCommandIcons.size() && lResult==-1) {
		
		if (oMenuCommandIcons[lIndex].iMenuCommand == piMenuCommand) 
			lResult = oMenuCommandIcons[lIndex].iImagelistIndex;

		lIndex++;
		}

	return lResult;
	}

//This function saves the menu command state (enabled or disabled)
void 
CMenuIcon::SetEnableMenuCommand(int piMenuCommand, bool pbEnabled) {

	bool bFound;
	long lIndex;
	sMenuCommandEnabled cItemEnabled;

	bFound = false;
	lIndex = (long)oMenuCommandEnabled.size();

	//See if the item is already present in the vector, and if so update it
	while (lIndex>0 && !bFound) {

		lIndex--;

		if (oMenuCommandEnabled[lIndex].iMenuCommand == piMenuCommand) {

			bFound = true;
			oMenuCommandEnabled[lIndex].bEnabled = pbEnabled;
			}
		}

	//The command isn't present in the vector yet, add it.
	if (!bFound) {

		cItemEnabled.bEnabled = pbEnabled;
		cItemEnabled.bChecked = false;
		cItemEnabled.iMenuCommand = piMenuCommand;

		oMenuCommandEnabled.push_back(cItemEnabled);
		}
	}

//Retrieves if the give menu item is enabled or disabled
bool
CMenuIcon::GetEnableMenuCommand(int piMenuCommand) {

	bool bResult;
	bool bFound;
	long lIndex;

	bResult = true;
	bFound = false;
	lIndex = (long)oMenuCommandEnabled.size();

	//See if the item is already present in the vector, and if so update it
	while (lIndex>0 && !bFound) {

		lIndex--;

		if (oMenuCommandEnabled[lIndex].iMenuCommand == piMenuCommand) {

			bFound = true;
			bResult = oMenuCommandEnabled[lIndex].bEnabled;
			}
		}

	return bResult;
	}

//This function saves the menu command state (enabled or disabled)
void 
CMenuIcon::SetCheckMenuCommand(int piMenuCommand, bool pbChecked) {

	bool bFound;
	long lIndex;
	sMenuCommandEnabled cItemEnabled;

	bFound = false;
	lIndex = (long)oMenuCommandEnabled.size();

	//See if the item is already present in the vector, and if so update it
	while (lIndex>0 && !bFound) {

		lIndex--;

		if (oMenuCommandEnabled[lIndex].iMenuCommand == piMenuCommand) {

			bFound = true;
			oMenuCommandEnabled[lIndex].bChecked = pbChecked;
			}
		}

	//The command isn't present in the vector yet, add it.
	if (!bFound) {

		cItemEnabled.bEnabled = true;
		cItemEnabled.bChecked = pbChecked;
		cItemEnabled.iMenuCommand = piMenuCommand;

		oMenuCommandEnabled.push_back(cItemEnabled);
		}
	}

//Retrieves if the give menu item is checked
bool
CMenuIcon::GetCheckMenuCommand(int piMenuCommand) {

	bool bResult;
	bool bFound;
	long lIndex;

	bResult = false;
	bFound = false;
	lIndex = (long)oMenuCommandEnabled.size();

	//See if the item is already present in the vector, and if so update it
	while (lIndex>0 && !bFound) {

		lIndex--;

		if (oMenuCommandEnabled[lIndex].iMenuCommand == piMenuCommand) {

			bFound = true;
			bResult = oMenuCommandEnabled[lIndex].bChecked;
			}
		}

	return bResult;
	}

