# Wiimote-HIDAPI

This started as a simple program to test the current state of the Windows HIDAPI in regards of Nintendo Wii Remotes.
But it grew into an example program to show off an implementation for the communication with Wiimotes, 
that works on Windows 7 and higher, support "-TR" Wiimotes, the Toshiba Bluetooth Stack, as well as the DolphinBar.

## Current State

The program shows, that on Windows 8 and higher "-TR" Wiimotes are working completly fine with the Microsoft Bluetooth Stack 
and the Toshiba Bluetooth Stack is not required anymore, especially as the common "hack".
However a bug in the default Microsoft HID Class Driver on Windows 7 renders the "WriteFile"-Method unusable on Windows 7, 
therefore it is not possible to use the HIDAPI to send data to "-TR" Wiimotes. 

## HIDAPI

### Sending & Recieving

[Sending HID Reports:](https://msdn.microsoft.com/en-us/library/windows/hardware/ff543402(v=vs.85).aspx)
- [`WriteFile`](https://msdn.microsoft.com/en-us/library/windows/desktop/aa365747(v=vs.85).aspx) _(Preferred)_
- [`HidD_SetOutputReport`](https://msdn.microsoft.com/en-us/library/windows/hardware/ff539690(v=vs.85).aspx)

[Recieving HID Reports:](https://msdn.microsoft.com/en-us/library/windows/hardware/ff542426(v=vs.85).aspx)
- [`ReadFile`](https://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx) _(Preferred)_
- [`HidD_GetInputReport`](https://msdn.microsoft.com/en-us/library/windows/hardware/ff538945(v=vs.85).aspx)

The MSDN Design Guides [Sending HID Reports](https://msdn.microsoft.com/en-us/library/windows/hardware/ff543402(v=vs.85).aspx) 
and [Obtaining HID Reports](https://msdn.microsoft.com/en-us/library/windows/hardware/ff542426(v=vs.85).aspx) are stating, 
that `WriteFile` and `ReadFile` are the preferred methods to send and recieve data from the device.
Additionally sending data to a "-TR" Wiimote via `WriteFile` is working fine, whereas using `HidD_SetOutputReport` will result in the Wiimote turning off.

### Issues with `WriteFile`

As the MSDN Desgin Guide [Sending HID Reports by Kernel-Mode Drivers](https://msdn.microsoft.com/en-us/library/windows/hardware/ff543397(v=vs.85).aspx)
(`WriteFile` will send out an IRP_MJ_WRITE request to the driver interface) suggests, 
the output report buffers shall have the size of the largest output report supported by the device. 
In case of the Wiimote this is 22 Byte.

This seems to be currently enforced by the Microsoft HID Class Driver on Windows 7 and the Toshiba Bluetooth Stack, as they will fail `WriteFile`attempts with the error `ERROR_INVALID_USER_BUFFER`, when the buffer size is less.

On Windows 7 however more bytes than the acutal report are sent to the device, which produces an error on the Wiimote. 
It is unknown whether this is a bug or intented behaviour.
The Toshiba Bluetooth Stack in contrast only sends the appropiate amount of bytes according to the used report to the device.

On Windows 8 and higher, the output report buffer can be arbitrary in size, as the given amount of byte are submitted to the device.

This results in the following table of compability.

### Table of compability

x | Toshiba Stack | Win 7 | Win 8.1 | Win 10
------------ | ------------- | ------------- | ------------- | -------------
**WriteFile _Largest Report Size_** | + | - | - | -
**WriteFile _Acutal Report Size_** | - | - | + | +
**SetOutputReport** | - | +* | +* | +*

_* does not support "-TR" when connected via Bluetooth_

## Method Priority Order

This leads us to the following order of prioritized methods:

1. Detect whether the Microsoft Stack or the Toshiba Stack is used for the Wii Remote.
2. In case of Toshiba Stack, use `WriteFile` with the largest report size for the buffer
3. In case of Microsoft Stack, try `WriteFile` with the actual report size
4. If `WriteFile` fails, fall back to `HidD_SetOutputReport`

## Detecting Stack

To detect the used stack for the Wiimote, the provider property of the used HID Class Driver is evaluated.
As the enumerated Wiimote Devices are just raw PDO's, that are acting as interfaces for the HID Class Driver and 
don't have a driver directly associated with, it is neccessary to move one node up in the device tree 
to get to the device node that is associated with the HID Class Driver. To do so the [PnP Configuration Manager API](https://msdn.microsoft.com/en-us/library/windows/hardware/ff549713(v=vs.85).aspx)
is used.

## Why `WriteFile` supports "-TR"

It is believed, that the usage of `HidD_SetOutputReport` will result in sending the output report via the Control Channel.
This is not supported by "-TR" Wiimotes, as they will immediately shut down.
In contrast `WriteFile` seems to send the data to device via the Interrput/Data Channel. 

## DolphinBar

The Mayflash DolphinBar enumerates Wiimotes as USB Devices, resulting in using the Microsoft HID Class Driver.
Therefore `WriteFile` won't work on Windows 7 for Wiimotes connected through the DolphinBar either.
However as the DolphinBar takes care of the Bluetooth communication and the outgoing data is send via USB to the DolphinBar,
`HidD_SetOutputReport` does support "-TR" Wiimotes as long as they are connected through a DolphinBar.

# License

This project is licensed under the terms of the MIT license.
