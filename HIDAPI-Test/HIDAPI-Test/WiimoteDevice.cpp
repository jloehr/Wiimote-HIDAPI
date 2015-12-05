#include "stdafx.h"
#include "WiimoteDevice.h"

DWORD WINAPI WiimoteStart(_In_ LPVOID lpParameter)
{
	WiimoteDevice * Device = static_cast<WiimoteDevice *>(lpParameter);
	Device->ContinuousReader();

	return 0;
}

WiimoteDevice::WiimoteDevice(HANDLE DeviceHandle, bool UseOutputReportSize)
	:UseOutputReportSize(UseOutputReportSize), DeviceHandle(DeviceHandle), ReadThread(NULL), Run(FALSE), OutputReportMinSize(0)
{
	ZeroMemory(&ReadIo, sizeof(ReadIo));
}

WiimoteDevice::~WiimoteDevice()
{
}

bool WiimoteDevice::Setup()
{
	PHIDP_PREPARSED_DATA PreparsedData = NULL;
	HIDP_CAPS Caps;
	BOOLEAN Result;
	NTSTATUS Status;

	std::cout << "Setting up: \t" << DeviceHandle << std::endl;
	std::cout << "Resize Output: \t" << (UseOutputReportSize ? "Yes" : "No") << std::endl;

	Result = HidD_GetPreparsedData(DeviceHandle, &PreparsedData);
	if (!Result)
	{
		std::cout << "GetPreparsedData Failed!" << std::endl;
		return false;
	}

	Status = HidP_GetCaps(PreparsedData, &Caps);
	if (Status < 0)
	{
		std::cout << "GetPreparsedData Failed!" << std::endl;
		HidD_FreePreparsedData(PreparsedData);
		return false;
	}

	std::cout << std::dec;

	std::cout << "\tUsage: " << Caps.Usage << std::endl;
	std::cout << "\tUsagePage: " << Caps.UsagePage << std::endl;
	std::cout << "\tInputReportByteLength: " << Caps.InputReportByteLength << std::endl;
	std::cout << "\tOutputReportByteLength: " << Caps.OutputReportByteLength << std::endl;
	std::cout << "\tFeatureReportByteLength: " << Caps.FeatureReportByteLength << std::endl;

	OutputReportMinSize = Caps.OutputReportByteLength;

	HidD_FreePreparsedData(PreparsedData);

	return true;
}

void WiimoteDevice::Disconnect()
{
	if (Run)
	{
		Run = false;
		do {
			SetEvent(ReadIo.hEvent);
		} while (WaitForSingleObject(ReadThread, 100) == WAIT_TIMEOUT);
	}
	
	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DeviceHandle);
		DeviceHandle = INVALID_HANDLE_VALUE;
	}
	
	if (ReadIo.hEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ReadIo.hEvent);
		ReadIo.hEvent= INVALID_HANDLE_VALUE;
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

void WiimoteDevice::StartReader()
{
	Run = true;
	ReadIo.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ReadThread = CreateThread(NULL, 0, WiimoteStart, this, 0, NULL);
}

void WiimoteDevice::ContinuousReader()
{
	UCHAR Buffer[255];
	DWORD BytesRead;

	while(Run)
	{
		ZeroMemory(Buffer, sizeof(Buffer));
		ResetEvent(ReadIo.hEvent);
		BytesRead = 0;
		BOOL Result = ReadFile(DeviceHandle, &Buffer, sizeof(Buffer), &BytesRead, &ReadIo);
		if (!Result)
		{
			DWORD Error = GetLastError();
			if (Error != ERROR_IO_PENDING)
			{
				std::cout << "Read Failed: " << std::hex << Error << std::endl;
				continue;
			}
			else
			{
				if (!GetOverlappedResult(DeviceHandle, &ReadIo, &BytesRead, TRUE))
				{
					Error = GetLastError();
					std::cout << "Read Failed: " << std::hex << Error << std::endl;
					continue;
				}

				if(ReadIo.Internal == STATUS_PENDING)
				{
					std::cout << "Read Interrupted" << std::endl;
					if(!CancelIo(DeviceHandle))
					{
						Error = GetLastError();
						std::cout << "Cancel IO Faile: " << std::hex << Error << std::endl;
					}
					continue;
				}
			}
		}

		std::cout << "Read  " << std::dec << BytesRead << " Bytes from " << "0x" << std::hex << DeviceHandle << " : ";
		for (size_t i = 0; i < BytesRead; i++)
		{
			std::cout.width(2);
			std::cout.fill(' ');
			std::cout << std::hex << static_cast<UINT>(Buffer[i]) << " ";
		}

		std::cout << std::endl;
	}
}

void WiimoteDevice::Write(DataBuffer & Buffer)
{
	DWORD BytesWritten;
	OVERLAPPED Overlapped;
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	std::cout << "0x" << std::hex << DeviceHandle << std::endl;

	if ((UseOutputReportSize) && (Buffer.size() < OutputReportMinSize))
	{
		std::cout << "Resizing Buffer" << std::endl;
		Buffer.resize(OutputReportMinSize, 0);
	}

	BOOL Result = WriteFile(DeviceHandle, Buffer.data(), (DWORD)Buffer.size(), &BytesWritten, &Overlapped);
	if (!Result)
	{
		DWORD Error = GetLastError();

		if (Error == ERROR_INVALID_USER_BUFFER)
		{
			std::cout << "Falling back to SetOutputReport" << std::endl;
			WriteFallback(Buffer);
		}
		
		if (Error != ERROR_IO_PENDING)
		{
			std::cout << "Write Failed: " << std::hex << Error << std::endl;
			return;
		}
	}

	if (!GetOverlappedResult(DeviceHandle, &Overlapped, &BytesWritten, TRUE))
	{
		DWORD Error = GetLastError();
		std::cout << "Write Failed: " << std::hex << Error << std::endl;
	}
}

void WiimoteDevice::WriteFallback(DataBuffer & Buffer)
{
	BOOL Result = HidD_SetOutputReport(DeviceHandle, Buffer.data(), Buffer.size());
	if (!Result)
	{
		DWORD Error = GetLastError();
		std::cout << "SetOutputReport Failed: " << std::hex << Error << std::endl;
	}
}