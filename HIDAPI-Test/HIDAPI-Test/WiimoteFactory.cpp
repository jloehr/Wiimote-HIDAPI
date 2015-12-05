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
		WiimoteDevices.push_back(WiimoteDevice(OpenDevice, IsUsingToshibaStack()));
	}
	else
	{
		CloseHandle(OpenDevice);
	}

	OpenDevice = INVALID_HANDLE_VALUE;
	free(DeviceInterfaceDetailData);
}

bool WiimoteFactory::CheckDevice(LPCTSTR DevicePath)
{
	OpenDevice = CreateFile(DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (OpenDevice == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to open Device." << std::endl;
		return false;
	}

	std::cout << "Device Handle: \t0x" << std::hex << OpenDevice << std::endl;

	HIDD_ATTRIBUTES HidAttributes;
	HidAttributes.Size = sizeof(HidAttributes);

	BOOL Result = HidD_GetAttributes(OpenDevice, &HidAttributes);
	if (!Result)
	{
		std::cout << "Failed to get hid attributes" << std::endl;
		return false;
	}

	std::cout << "VID&PID: \t" << HidAttributes.VendorID << " - " << HidAttributes.ProductID << std::endl;

	WCHAR ProductName[255];
	ZeroMemory(ProductName, sizeof(ProductName));

	if (HidD_GetProductString(OpenDevice, ProductName, 255))
	{
		std::wcout << "HID Name: \t" << ProductName << std::endl;
	}

	PrintDriverInfo(DeviceInfoSet, &DeviceInfoData);
	PrintDeviceTreeInfo(3, DeviceInfoData.DevInst);

	return (HidAttributes.VendorID == 0x057e) && ((HidAttributes.ProductID == 0x0306) || (HidAttributes.ProductID == 0x0330));
}

bool WiimoteFactory::IsUsingToshibaStack()
{
	HDEVINFO ParentDeviceInfoSet;
	SP_DEVINFO_DATA ParentDeviceInfoData;
	std::vector<WCHAR> ParentDeviceID;

	bool Result = GetParentDevice(DeviceInfoData.DevInst, ParentDeviceInfoSet, &ParentDeviceInfoData, ParentDeviceID);
	if (!Result)
	{
		return false;
	}

	std::wstring Provider = GetDeviceProperty(ParentDeviceInfoSet, &ParentDeviceInfoData, &DEVPKEY_Device_DriverProvider);

	SetupDiDestroyDeviceInfoList(ParentDeviceInfoSet);

	return (Provider == L"TOSHIBA");
}

void WiimoteFactory::PrintDeviceTreeInfo(UINT Levels, DEVINST ChildDevice)
{
	if (Levels == 0)
	{
		return;
	}

	std::cout << "  +----+  " << std::endl;

	HDEVINFO ParentDeviceInfoSet;
	SP_DEVINFO_DATA ParentDeviceInfoData;
	std::vector<WCHAR> ParentDeviceID;

	bool Result = GetParentDevice(ChildDevice, ParentDeviceInfoSet, &ParentDeviceInfoData, ParentDeviceID);
	if (!Result)
	{
		return;
	}

	std::wcout << "Device ID: \t" << ParentDeviceID.data() << std::endl;
	PrintDriverInfo(ParentDeviceInfoSet, &ParentDeviceInfoData);

	PrintDeviceTreeInfo(Levels - 1, ParentDeviceInfoData.DevInst);

	SetupDiDestroyDeviceInfoList(ParentDeviceInfoSet);
}

void WiimoteFactory::PrintDriverInfo(HDEVINFO & DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"Device Name", &DEVPKEY_NAME);
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"Description", &DEVPKEY_Device_DriverDesc);
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"Provider", &DEVPKEY_Device_DriverProvider);
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"Manufacturer", &DEVPKEY_Device_Manufacturer);
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"PDO", &DEVPKEY_Device_PDOName);
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"Bus Reported", &DEVPKEY_Device_BusReportedDeviceDesc);
	PrintDeviceProperty(DeviceInfoSet, DeviceInfoData, L"Device Driver", &DEVPKEY_Device_Driver);
}

void WiimoteFactory::PrintDeviceProperty(HDEVINFO & DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const PWCHAR PropertyName, const DEVPROPKEY * Property)
{
	std::wstring Result = GetDeviceProperty(DeviceInfoSet, DeviceInfoData, Property);
	if (Result.empty())
	{
		std::wcout << "Error getting Device Property (" << PropertyName << "): 0x" << std::hex << GetLastError() << std::endl;
	}
	else
	{
		std::wcout << PropertyName << ": \t" << Result << std::endl;
	}
}

bool WiimoteFactory::GetParentDevice(const DEVINST & ChildDevice, HDEVINFO & ParentDeviceInfoSet, PSP_DEVINFO_DATA ParentDeviceInfoData, std::vector<WCHAR> & ParentDeviceID)
{
	ULONG Status;
	ULONG ProblemNumber;
	CONFIGRET Result;

	Result = CM_Get_DevNode_Status(&Status, &ProblemNumber, ChildDevice, 0);
	if (Result != CR_SUCCESS)
	{
		std::cout << "Something wrong woth the Device Node!" << std::endl;
		return false;
	}

	DEVINST ParentDevice;

	Result = CM_Get_Parent(&ParentDevice, ChildDevice, 0);
	if (Result != CR_SUCCESS)
	{
		std::cout << "Error getting parent: 0x" << std::hex << Result << std::endl;
		 return false;
	}

	ParentDeviceID.resize(MAX_DEVICE_ID_LEN);

	Result = CM_Get_Device_ID(ParentDevice, ParentDeviceID.data(), (ULONG)ParentDeviceID.size(), 0);
	if (Result != CR_SUCCESS)
	{
		std::cout << "Error getting parent device id: 0x" << std::hex << Result << std::endl;
		return false;
	}

	ParentDeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
	ZeroMemory(ParentDeviceInfoData, sizeof(SP_DEVINFO_DATA));
	ParentDeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);

	if (!SetupDiOpenDeviceInfo(ParentDeviceInfoSet, ParentDeviceID.data(), NULL, 0, ParentDeviceInfoData))
	{
		std::cout << "Error getting Device Info Data: 0x" << std::hex << GetLastError() << std::endl;
		SetupDiDestroyDeviceInfoList(ParentDeviceInfoSet);
		return false;
	}

	return true;
}

std::wstring WiimoteFactory::GetDeviceProperty(HDEVINFO & DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const DEVPROPKEY * Property)
{
	DWORD RequiredSize = 0;
	DEVPROPTYPE DevicePropertyType;

	SetupDiGetDeviceProperty(DeviceInfoSet, DeviceInfoData, Property, &DevicePropertyType, NULL, 0, &RequiredSize, 0);

	std::vector<BYTE> Buffer(RequiredSize, 0);

	BOOL Result = SetupDiGetDeviceProperty(DeviceInfoSet, DeviceInfoData, Property, &DevicePropertyType, Buffer.data(), RequiredSize, NULL, 0);
	if (!Result)
	{
		return std::wstring();
	}

	return std::wstring((PWCHAR)Buffer.data());
}