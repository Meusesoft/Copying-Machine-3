#pragma once
#include "listview.h"
#include "CopyingMachineCore.h"

class CPagePropertiesListview :
	public CListview
{
public:
	CPagePropertiesListview(sGlobalInstances pInstances, std::wstring psWindowName);
	~CPagePropertiesListview(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra);

	virtual void SaveSettings();
	virtual void LoadSettings();

	void ClearProperties();
	void SetProperties(CCopyDocumentPage* poPage);
	void Localize(HINSTANCE);

protected:

	void DoAddProperty(PROPID piPropertyId);
	void DoProcessProperty(PropertyItem cProperty);
	bool DoProcessPropertyLabel(PropertyItem cProperty, LVITEM &pcItem);
	void DoProcessPropertyValue(PropertyItem cProperty, LVITEM &pcItem);
	void DoProcessPropertyValueShort(PropertyItem cProperty, LVITEM &pcItem);
	void DoProcessPropertyValueRational(PropertyItem cProperty, LVITEM &pcItem);
	void DoProcessPropertyValueSRational(PropertyItem cProperty, LVITEM &pcItem);

private:

	CCopyDocumentPage* oCurrentPage;
	sCopyDocumentPageProperties oPageProperties;
};
