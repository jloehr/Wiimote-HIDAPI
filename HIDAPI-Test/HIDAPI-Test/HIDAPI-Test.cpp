// HIDAPI-Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WiimoteFactory.h"

int main()
{
	WiimoteDeviceVector Wiimotes;

	{
		WiimoteFactory Factory;
		Wiimotes = Factory.GetWiimoteDevices();
	}
	
	for (WiimoteDevice & Wiimote : Wiimotes)
	{
		if (Wiimote.Setup())
		{
			Wiimote.SetLEDs();
			Wiimote.SetReportMode();
			Wiimote.StartReader();
		}
	}

	system("pause");
	std::cout << "Exiting!" << std::endl;


	for (WiimoteDevice & Wiimote : Wiimotes)
	{
		Wiimote.Disconnect();
	}

	system("pause");

    return 0;
}

