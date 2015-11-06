#pragma once

#include <vector>

class WiimoteDevice;

typedef std::vector<WiimoteDevice> WiimoteDeviceVector;

class WiimoteDevice
{
public:
	WiimoteDevice(HANDLE DeviceHandle);
	~WiimoteDevice();

private:
	HANDLE DeviceHandle;
};

