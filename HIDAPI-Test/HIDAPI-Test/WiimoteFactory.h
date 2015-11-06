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
	BOOL CheckDevice(LPCTSTR DevicePath);

};

