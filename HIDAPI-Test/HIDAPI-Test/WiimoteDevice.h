#pragma once

#include <vector>

class WiimoteDevice;

typedef std::vector<WiimoteDevice> WiimoteDeviceVector;
typedef std::vector<UCHAR> DataBuffer;

class WiimoteDevice
{
public:
	WiimoteDevice(HANDLE DeviceHandle, bool UseOutputReportSize);
	~WiimoteDevice();

	bool Setup();
	void Disconnect();

	void SetLEDs();
	void SetReportMode();

	void StartReader();
	void ContinuousReader();

private:
	const bool UseOutputReportSize;

	HANDLE DeviceHandle;
	HANDLE ReadThread;
	OVERLAPPED ReadIo;

	bool Run;

	SHORT OutputReportMinSize;

	void Write(DataBuffer & Buffer);
	void WriteFallback(DataBuffer & Buffer);
};

