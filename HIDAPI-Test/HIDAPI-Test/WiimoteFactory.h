#pragma once

#include "stdafx.h"
#include "WiimoteDevice.h"

class WiimoteFactory
{
public:
	WiimoteFactory();
	~WiimoteFactory();

	WiimoteDeviceVector GetWiimoteDevices();

private:
	WiimoteDeviceVector WiimoteDevices;

	GUID HidGuid;
	HDEVINFO DeviceInfoSet;
	DWORD DeviceIndex;

	SP_DEVINFO_DATA DeviceInfoData;
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;

	HANDLE OpenDevice;

	void CheckEnumeratedDeviceInterface();
	bool CheckDevice(LPCTSTR DevicePath);

	bool IsUsingToshibaStack();

	void PrintDeviceTreeInfo(UINT Levels, DEVINST ChildDevice);
	void PrintDriverInfo(HDEVINFO &DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData);
	void PrintDeviceProperty(HDEVINFO &DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const PWCHAR PropertyName, const DEVPROPKEY * Property);
	
	bool GetParentDevice(const DEVINST & ChildDevice, HDEVINFO & ParentDeviceInfoSet, PSP_DEVINFO_DATA ParentDeviceInfoData, std::vector<WCHAR> & ParentDeviceID);
	std::wstring GetDeviceProperty(HDEVINFO &DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const DEVPROPKEY * Property);

};

