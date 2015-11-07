#pragma once

#include <vector>

class WiimoteDevice;

typedef std::vector<WiimoteDevice> WiimoteDeviceVector;
typedef std::vector<UCHAR> DataBuffer;

class WiimoteDevice
{
public:
	WiimoteDevice(HANDLE DeviceHandle);
	~WiimoteDevice();

	void Disconnect();

	void SetLEDs();

private:
	HANDLE DeviceHandle;

	void Write(const DataBuffer & Buffer);
};

