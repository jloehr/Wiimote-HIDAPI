#include "stdafx.h"
#include "WiimoteDevice.h"


WiimoteDevice::WiimoteDevice(HANDLE DeviceHandle)
	:DeviceHandle(DeviceHandle)
{
}

WiimoteDevice::~WiimoteDevice()
{
}

void WiimoteDevice::Disconnect()
{
	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DeviceHandle);
		DeviceHandle = INVALID_HANDLE_VALUE;
	}
}

void WiimoteDevice::SetLEDs()
{
	DataBuffer Buffer({ 0x11, 0x60 });

	Write(Buffer);
}

void WiimoteDevice::SetReportMode()
{
	DataBuffer Buffer({0x12, 0x00, 0x30 });

	Write(Buffer);
}

void WiimoteDevice::Write(const DataBuffer & Buffer)
{
	DWORD BytesWritten;
	OVERLAPPED Overlapped;
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	std::cout << "0x" << std::hex << DeviceHandle << std::endl;

	BOOL Result = WriteFile(DeviceHandle, Buffer.data(), Buffer.size(), &BytesWritten, &Overlapped);
	if (!Result)
	{
		DWORD Error = GetLastError();
		
		if (Error != ERROR_IO_PENDING)
		{
			std::cout << "Write Failed: " << std::hex << Error << std::endl;
		}
	}
}