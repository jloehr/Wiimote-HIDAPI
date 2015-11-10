#include "stdafx.h"
#include "WiimoteFactory.h"


WiimoteFactory::WiimoteFactory()
	:DeviceIndex(0), DeviceInfoSet(NULL), OpenDevice(INVALID_HANDLE_VALUE)
{
	HidD_GetHidGuid(&HidGuid);

	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
}


WiimoteFactory::~WiimoteFactory()
{
	if (DeviceInfoSet)
	{
		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
		DeviceInfoSet = NULL;
	}
}

WiimoteDeviceVector WiimoteFactory::GetWiimoteDevices()
{
	DeviceInfoSet = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (!DeviceInfoSet)
	{
		std::cout << "No HID Devices Found!" << std::endl;
		return WiimoteDevices;
	}

	while (SetupDiEnumDeviceInterfaces(DeviceInfoSet, NULL, &HidGuid, DeviceIndex, &DeviceInterfaceData))
	{
		std::cout << "New Device" << std::endl;
		CheckEnumeratedDeviceInterface();
		DeviceIndex++;
	}

	if (DeviceIndex == 0)
	{
		std::cout << "No Device Enumerated!" << std::endl;
	}

	return WiimoteDevices;
}

void WiimoteFactory::CheckEnumeratedDeviceInterface()
{
	BOOL Result;
	DWORD RequiredSize;

	Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, NULL, 0, &RequiredSize, NULL);

	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
	DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, NULL, NULL);

	BOOL IsWiimote = CheckDevice(DeviceInterfaceDetailData->DevicePath);

	if (IsWiimote)
	{
		WiimoteDevices.push_back(WiimoteDevice(OpenDevice));
	}
	else
	{
		CloseHandle(OpenDevice);
	}

	OpenDevice = INVALID_HANDLE_VALUE;
	free(DeviceInterfaceDetailData);
}

BOOL WiimoteFactory::CheckDevice(LPCTSTR DevicePath)
{
	OpenDevice = CreateFile(DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (OpenDevice == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to open Device." << std::endl;
		return FALSE;
	}

	std::cout << "0x" << std::hex << OpenDevice << std::endl;

	HIDD_ATTRIBUTES HidAttributes;
	HidAttributes.Size = sizeof(HidAttributes);

	BOOL Result = HidD_GetAttributes(OpenDevice, &HidAttributes);
	if (!Result)
	{
		std::cout << "Failed to get hid attributes" << std::endl;
		return FALSE;
	}

	std::cout << HidAttributes.VendorID << " - " << HidAttributes.ProductID << std::endl;

	TCHAR ProductName[255];

	if (HidD_GetProductString(OpenDevice, ProductName, 255))
	{
		std::wcout << ProductName << std::endl;
	}

	return (HidAttributes.VendorID == 0x057e) && ((HidAttributes.ProductID == 0x0306) || (HidAttributes.ProductID == 0x0330));
}