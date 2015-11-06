#include "stdafx.h"
#include "WiimoteDevice.h"


WiimoteDevice::WiimoteDevice(HANDLE DeviceHandle)
	:DeviceHandle(DeviceHandle)
{
}

WiimoteDevice::~WiimoteDevice()
{
	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DeviceHandle);
	}
}
