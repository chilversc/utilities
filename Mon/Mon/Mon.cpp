// Mon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

LPCTSTR deviceName = _T("\\\\.\\DISPLAY2");

void DetatchSecondMonitor ()
{
	DEVMODE mode;

	ZeroMemory (&mode, sizeof(DEVMODE));
	mode.dmSize = sizeof(DEVMODE);
	mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS | DM_DISPLAYORIENTATION;

	ChangeDisplaySettingsEx (deviceName, &mode, NULL, CDS_UPDATEREGISTRY | CDS_NORESET, NULL);
	ChangeDisplaySettingsEx (NULL, 0, NULL, 0, NULL);
}

void AttachSecondMonitor ()
{
	DEVMODE mode;

	ZeroMemory (&mode, sizeof(DEVMODE));
	mode.dmSize = sizeof(DEVMODE);

	mode.dmBitsPerPel = 32;
	mode.dmPelsWidth = 1920;
	mode.dmPelsHeight = 1200;
	mode.dmPosition.x = -1920;
	mode.dmPosition.y = 0;
	mode.dmDisplayFrequency = 60;
	mode.dmDisplayOrientation = 0;
	mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS | DM_DISPLAYORIENTATION;

	LONG result = ChangeDisplaySettingsEx (deviceName, &mode, NULL, CDS_UPDATEREGISTRY | CDS_NORESET, NULL);

	if (result == DISP_CHANGE_SUCCESSFUL) {
		// http://support.microsoft.com/kb/308216
		ChangeDisplaySettingsEx (NULL, NULL, NULL, 0, NULL);
	}
}

void ListMonitors ()
{
	DISPLAY_DEVICE device;
	DWORD deviceNumber;

	DEVMODE mode;
	
	ZeroMemory (&device, sizeof(DISPLAY_DEVICE));
	device.cb = sizeof(DISPLAY_DEVICE);

	deviceNumber = 0;
	while (EnumDisplayDevices (NULL, deviceNumber, &device, 0)) {
		printf ("Device: %ls (%ls)\n", device.DeviceName, device.DeviceString);
		
		ZeroMemory (&mode, sizeof(DEVMODE));
		mode.dmSize = sizeof(DEVMODE);

		if (EnumDisplaySettingsEx (device.DeviceName, ENUM_CURRENT_SETTINGS, &mode, 0)) {
			printf ("\tResolution: %d x %d\n", mode.dmPelsWidth, mode.dmPelsHeight);
			printf ("\tLocation: %d x %d\n", mode.dmPosition.x, mode.dmPosition.y);
			printf ("\tFrequency: %d\n", mode.dmDisplayFrequency);
			printf ("\tOrientation: %d\n", mode.dmDisplayOrientation);
			printf ("\tBPP: %d\n", mode.dmBitsPerPel);
		}
	
		ZeroMemory (&device, sizeof(DISPLAY_DEVICE));
		device.cb = sizeof(DISPLAY_DEVICE);

		deviceNumber++;
	}
}

void PrintUsage ()
{
	printf ("Syntax: mon.exe [attach | detach | list]");
}

bool IsMatch (LPCTSTR a, LPCTSTR b)
{
	return CSTR_EQUAL == CompareStringEx (LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE, a, -1, b, -1, NULL, NULL, 0);
}

int _tmain (int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		PrintUsage ();
	} else {
		LPCTSTR operation = argv [1];

		if (IsMatch (_T("list"), operation)) {
			ListMonitors ();
		} else if (IsMatch (_T("attach"), operation)) {
			AttachSecondMonitor ();
		} else if (IsMatch (_T("detach"), operation)) {
			DetatchSecondMonitor ();
		} else {
			PrintUsage ();
		}
	}

	return 0;
}
