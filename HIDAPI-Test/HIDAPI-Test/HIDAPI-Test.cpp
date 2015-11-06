// HIDAPI-Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	GUID HidGuid;
	HidD_GetHidGuid(&HidGuid);

	HDEVINFO DeviceInfoSet = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if (!DeviceInfoSet)
	{
		std::cout << "No HID Devices Found!" << std::endl;
		system("pause");
		return 0;
	}

	DWORD DeviceIndex = 0;

	SP_DEVINFO_DATA DeviceInfoData;
	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);


	while (
		SetupDiEnumDeviceInterfaces(DeviceInfoSet, NULL, &HidGuid, DeviceIndex, &DeviceInterfaceData))
	{
		DeviceIndex++;

		BOOL Result;
		DWORD RequiredSize;

		Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, NULL, 0, &RequiredSize, NULL);

		PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, NULL, NULL);

		std::cout << "Path" << std::endl;
		std::wcout << DeviceInterfaceDetailData->DevicePath << std::endl;

		free(DeviceInterfaceDetailData);
	}

	if (DeviceIndex == 0)
	{
		std::cout << "No Device Enumerated!" << std::endl;
	}

	if (DeviceInfoSet) 
	{
		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
	}

	system("pause");
    return 0;
}

