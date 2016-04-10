#pragma once
#include "Toolbar.h"

class CSharedPreferencesSelection
{
public:
	CSharedPreferencesSelection(sGlobalInstances poGlobalInstances);
	~CSharedPreferencesSelection(void);

	void UpdateControls();

protected: 

	bool OnCommand(int piCommand, WPARAM wParam, LPARAM lParam);

	void DoEnableControls();

	void DoFillControls();
	void DoFillResolution();
	void DoFillColorDepth();
	void DoFillShowInterfaceCheckbox();
	void DoFillUseDocumentFeederCheckbox();

	void DoApplySettings();
	void DoApplyPrintSettings();
	void DoApplyScanSettings();

	int  GetIntegerFromEditControl(HWND phEditControl);

	sGlobalInstances oGlobalInstances;
	CCopyingMachineCore* oCore;
	CTracer* oTrace;

	bool bControlsEnabled;

	HWND hColorBox;
	HWND hResolutionBox;
	HWND hShowInterfaceCheckBox;
	HWND hUseDocumentFeeder;
	HWND hCopiesEdit;
	HWND hCopiesSpin;
	HWND hSliderContrast;
	HWND hSliderBrightness;

private:

	};
