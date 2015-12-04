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

	bool Setup();
	void Disconnect();

	void SetLEDs();
	void SetReportMode();

	void StartReader();
	void ContinuousReader();

private:
	HANDLE DeviceHandle;
	HANDLE ReadThread;
	OVERLAPPED ReadIo;

	bool Run;

	SHORT OutputReportMinSize;

	void Write(DataBuffer & Buffer);
};

