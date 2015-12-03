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
		std::cout << "--- New Device ---" << std::endl;
		CheckEnumeratedDeviceInterface();
		DeviceIndex++;
		std::cout << "------------------" << std::endl;
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

	Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, NULL, &DeviceInfoData);
 
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

	std::cout << "DevicePath: \t0x" << std::hex << OpenDevice << std::endl;

	HIDD_ATTRIBUTES HidAttributes;
	HidAttributes.Size = sizeof(HidAttributes);

	BOOL Result = HidD_GetAttributes(OpenDevice, &HidAttributes);
	if (!Result)
	{
		std::cout << "Failed to get hid attributes" << std::endl;
		return FALSE;
	}

	std::cout << "VID&PID: \t" << HidAttributes.VendorID << " - " << HidAttributes.ProductID << std::endl;

	TCHAR ProductName[255];

	if (HidD_GetProductString(OpenDevice, ProductName, 255))
	{
		std::wcout << "HID Name: \t" << ProductName << std::endl;
	}

	PrintDeviceName(DeviceInfoSet, &DeviceInfoData);
	PrintDriverInfo(DeviceInfoSet, &DeviceInfoData);

	return (HidAttributes.VendorID == 0x057e) && ((HidAttributes.ProductID == 0x0306) || (HidAttributes.ProductID == 0x0330));
}

void WiimoteFactory::PrintDeviceName(HDEVINFO & DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
	DWORD RequiredSize = 0;
	DEVPROPTYPE DevicePropertyType;

	SetupDiGetDeviceProperty(DeviceInfoSet, DeviceInfoData, &DEVPKEY_NAME, &DevicePropertyType, NULL, 0, &RequiredSize, 0);

	PBYTE Buffer = (PBYTE)malloc(RequiredSize);
	ZeroMemory(Buffer, RequiredSize);

	BOOL Result = SetupDiGetDeviceProperty(DeviceInfoSet, DeviceInfoData, &DEVPKEY_NAME, &DevicePropertyType, Buffer, RequiredSize, NULL, 0);
	if (!Result)
	{
		std::cout << "Error getting Device Name property: 0x" << std::hex << GetLastError() << std::endl;
	}
	else
	{
		std::wcout << "Device Name: \t" << (PWCHAR) Buffer << std::endl;
	}

	free(Buffer);
}

void WiimoteFactory::PrintDriverInfo(HDEVINFO & DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
	SP_DRVINFO_DATA DriverInfo;
	ZeroMemory(&DriverInfo, sizeof(SP_DRVINFO_DATA));
	DriverInfo.cbSize = sizeof(SP_DRVINFO_DATA);

	BOOL Result = SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfo);

	if (Result)
	{
		std::cout << "Driver Type: \t" << ((DriverInfo.DriverType == SPDIT_CLASSDRIVER) ? "Class Driver" : "Compatible Driver") << std::endl;
		std::cout << "Description: \t" << DriverInfo.Description << std::endl;
		std::cout << "MfgName: \t" << DriverInfo.MfgName << std::endl;
		std::cout << "ProviderName: \t" << DriverInfo.ProviderName << std::endl;
	}
	else
	{
		DWORD Error = GetLastError();
		if (Error == ERROR_NO_DRIVER_SELECTED)
		{
			std::cout << "No driver for the device" << std::endl;
		}
		else
		{
			std::cout << "Error getting Driver Info 0x" << std::hex << Error << std::endl;
		}
	}
}