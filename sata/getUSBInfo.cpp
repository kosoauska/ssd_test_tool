/* Copyright  Sinochip Semiconductors Co.,Ltd. 2009-2013.  All rights reserved.
* 
*/
/**
* @file "getUSBInfo.cpp"
* @brief main cpp
* @author WangPu
* @version 1.04a
* @date 2016-10-26
*/
// getUSBInfo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <initguid.h> //error LNK2001: 无法解析的外部符号 _GUID_DEVINTERFACE_USB_HUB
#include <Usbiodef.h> //GUID_DEVINTERFACE_USB_HUB
#include <Usbioctl.h>  //IOCTL_USB_GET_NODE_CONNECTION_INFORMATION
#include <setupapi.h>  
#include <winioctl.h> //STORAGE_DEVICE_NUMBER
#include <Cfgmgr32.h>
#include <shlwapi.h> //StrStrI
#include "getUSBInfo.h"
#include "debug.h"

//从第二个字符开始
#define DESCRIPTOR_STR(x)  (x + 1)
PTSTR GetStringDescriptor (HANDLE  hHubDevice, ULONG   ConnectionIndex, UCHAR   DescriptorIndex, USHORT);

PTSTR GetDriverKeyName (HANDLE  Hub,ULONG   ConnectionIndex)
{
	BOOL                                success;
	ULONG                               nBytes;
	USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
	PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;
	PTSTR result = NULL;
	driverKeyNameW = NULL;


	// Get the length of the name of the driver key of the device attached to
	// the specified port.
	//
	driverKeyName.ConnectionIndex = ConnectionIndex;

	success = DeviceIoControl(Hub,
		IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
		&driverKeyName,
		sizeof(driverKeyName),
		&driverKeyName,
		sizeof(driverKeyName),
		&nBytes,
		NULL);

	if (!success)
	{
		DLog(0,_T("IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME failed\r\n"));
		goto GetDriverKeyNameError;
	}

	// Allocate space to hold the driver key name
	//
	nBytes = driverKeyName.ActualLength;

	if (nBytes <= sizeof(driverKeyName))
	{
		DLog(0,_T("driverKeyName.ActualLength <= driverKeyName size\r\n"));
		goto GetDriverKeyNameError;
	}

	driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)GlobalAlloc(GPTR,nBytes);

	if (driverKeyNameW == NULL)
	{
		DLog(0,_T("ALLOC driverKeyNameW failed\r\n"));
		goto GetDriverKeyNameError;
	}

	// Get the name of the driver key of the device attached to
	// the specified port.
	//
	driverKeyNameW->ConnectionIndex = ConnectionIndex;

	success = DeviceIoControl(Hub,
		IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
		driverKeyNameW,
		nBytes,
		driverKeyNameW,
		nBytes,
		&nBytes,
		NULL);

	if (!success)
	{
		DLog(0,_T("IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME failed\r\n"));
		goto GetDriverKeyNameError;
	}

	ULONG nChars = wcslen(driverKeyNameW->DriverKeyName) + 1;
	result = (PTSTR)GlobalAlloc(GPTR,nChars * sizeof(TCHAR));
	if (result == NULL)
	{
		return NULL;
	}
	_tcscpy_s(result, nChars, driverKeyNameW->DriverKeyName);

	if (driverKeyNameW != NULL)
	{
		GlobalFree(driverKeyNameW);
		driverKeyNameW = NULL;
	}

	return result;


GetDriverKeyNameError:
	// There was an error, free anything that was allocated
	//
	if (driverKeyNameW != NULL)
	{
		GlobalFree(driverKeyNameW);
		driverKeyNameW = NULL;
	}

	return NULL;
}


PTSTR GetDeviceDriver(DEVINST devInst)
{
	TCHAR buf[MAX_DEVICE_ID_LEN];  // (Dynamically size this instead?)
	CONFIGRET   cr;
	PTSTR drivename;

	ULONG       len;


	len = sizeof(buf) / sizeof(buf[0]);
	cr = CM_Get_DevNode_Registry_Property(devInst,
		CM_DRP_DRIVER,
		NULL,
		buf,
		&len,
		0);


	if (cr == CR_SUCCESS)
	{
		DLog(1,_T("DeviceDriver: %s\n"),buf);   
	}
	else
	{
		DLog(0,_T("CM_DRP_DRIVER failed\r\n"));
		return NULL;
	}

	ULONG nChars = wcslen(buf) + 1;
	drivename = (PTSTR)GlobalAlloc(GPTR,nChars * sizeof(TCHAR));
	if (drivename == NULL)
	{
		return NULL;
	}
	_tcscpy_s(drivename, nChars, buf);

	return drivename;
}

DWORD GetDevicePortNum(HDEVINFO hDevInfo,PSP_DEVINFO_DATA pspdd)
{

	DWORD DataT;
	LPTSTR buffer = NULL;
	DWORD buffersize = 0;
	DWORD PortNum=0;

	while (!SetupDiGetDeviceRegistryProperty(
		hDevInfo,
		pspdd,
		SPDRP_ADDRESS,
		&DataT,
		(PBYTE)buffer,
		buffersize,
		&buffersize))
	{
		if (GetLastError() == 
			ERROR_INSUFFICIENT_BUFFER)
		{
			// Change the buffer size.
			if (buffer) LocalFree(buffer);
			buffer = (TCHAR*)LocalAlloc(LPTR,buffersize);
		}
		else
		{
			// Insert error handling here.
			break;
		}
	}        

	PortNum = *(PDWORD)buffer;
	DLog(1,_T("Length:%d,SPDRP_ADDRESS: %d\n"),buffersize,PortNum);   

	if (buffer) LocalFree(buffer);

	return PortNum;
}

DWORD GetDevicePortNum(DEVINST devInst)
{
	DWORD PortNum=0;
	TCHAR buf[MAX_DEVICE_ID_LEN];  // (Dynamically size this instead?)
	CONFIGRET   cr;

	ULONG       len;

	len = sizeof(buf) / sizeof(buf[0]);
	cr = CM_Get_DevNode_Registry_Property(devInst,
		CM_DRP_ADDRESS,
		NULL,
		buf,
		&len,
		0);


	if (cr == CR_SUCCESS)
	{
		PortNum = *(PDWORD)buf;
	}
	else
	{
		DLog(0,_T("CM_DRP_ADDRESS failed\r\n"));
	}

	return PortNum;
}
DWORD GetKeyPortInfo(PTSTR HubDevicePath, PTSTR driverKey, usb_info* info)
{
	PUSB_NODE_INFORMATION   hubInfo;
	BOOL                    success;
	ULONG                   nBytes;
	DWORD portnum = -1;
	// Try to hub the open device
	//
	HANDLE hHubDevice = CreateFile(HubDevicePath,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);


	if (hHubDevice == INVALID_HANDLE_VALUE)
	{
		DLog(0,_T("GetDevicePortNum open hub failed\r\n"));
		goto GetDevicePortNumError;
	}

	// Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
	//
	hubInfo = (PUSB_NODE_INFORMATION)GlobalAlloc(GPTR,sizeof(USB_NODE_INFORMATION));

	if (hubInfo == NULL)
	{
		DLog(0,_T("Alloc hubInfo failed\r\n"));
		goto GetDevicePortNumError;
	}

	//
	// Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
	// This will tell us the number of downstream ports to enumerate, among
	// other things.
	//
	success = DeviceIoControl(hHubDevice,
		IOCTL_USB_GET_NODE_INFORMATION,
		hubInfo,
		sizeof(USB_NODE_INFORMATION),
		hubInfo,
		sizeof(USB_NODE_INFORMATION),
		&nBytes,
		NULL);

	if (!success)
	{
		DLog(0,_T("IOCTL_USB_GET_NODE_INFORMATION failed\r\n"));
		goto GetDevicePortNumError;
	}

	DWORD numOfPorts = hubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts;
	DLog(1,_T("hubs have %d ports\r\n"),numOfPorts);


	//循环
	for(DWORD index = 1; index <= numOfPorts; index++)
	{

		PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;

		ULONG nBytesEx;
		BOOL        success;
		// Allocate space to hold the connection info for this port.
		// For now, allocate it big enough to hold info for 30 pipes.
		//
		// Endpoint numbers are 0-15.  Endpoint number 0 is the standard
		// control endpoint which is not explicitly listed in the Configuration
		// Descriptor.  There can be an IN endpoint and an OUT endpoint at
		// endpoint numbers 1-15 so there can be a maximum of 30 endpoints
		// per device configuration.
		//
		// Should probably size this dynamically at some point.
		//
		nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) +
			sizeof(USB_PIPE_INFO) * 30;

		connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)GlobalAlloc(GPTR,nBytesEx);

		if (connectionInfoEx == NULL)
		{
			DLog(0,_T("connectionInfoEx GlobalAlloc failed\r\n"));
			goto GetDevicePortNumError;
		}

		//
		// Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION_EX structure
		// for this port.  This will tell us if a device is attached to this
		// port, among other things.
		//
		connectionInfoEx->ConnectionIndex = index;

		success = DeviceIoControl(hHubDevice,
			IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
			connectionInfoEx,
			nBytesEx,
			connectionInfoEx,
			nBytesEx,
			&nBytesEx,
			NULL);

		if (!success)
		{
			DLog(1,_T("IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX failed\r\n"));
			PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
			ULONG                               nBytes;
			// Try using IOCTL_USB_GET_NODE_CONNECTION_INFORMATION
			// instead of IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
			//
			nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
				sizeof(USB_PIPE_INFO) * 30;

			connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)GlobalAlloc(GPTR,nBytes);
			if (connectionInfo == NULL)
			{
				GlobalFree(connectionInfoEx);
				goto GetDevicePortNumError;
			}

			connectionInfo->ConnectionIndex = index;

			success = DeviceIoControl(hHubDevice,
				IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
				connectionInfo,
				nBytes,
				connectionInfo,
				nBytes,
				&nBytes,
				NULL);

			if (!success)
			{
				DLog(0,_T("IOCTL_USB_GET_NODE_CONNECTION_INFORMATION failed\r\n"));
				GlobalFree(connectionInfo);
				GlobalFree(connectionInfoEx);
				goto GetDevicePortNumError;
			}
			// Copy IOCTL_USB_GET_NODE_CONNECTION_INFORMATION into
			// IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX structure.
			//
			connectionInfoEx->ConnectionIndex =
				connectionInfo->ConnectionIndex;

			connectionInfoEx->DeviceDescriptor =
				connectionInfo->DeviceDescriptor;

			connectionInfoEx->CurrentConfigurationValue =
				connectionInfo->CurrentConfigurationValue;

			connectionInfoEx->Speed =
				connectionInfo->LowSpeed ? UsbLowSpeed : UsbFullSpeed;

			connectionInfoEx->DeviceIsHub =
				connectionInfo->DeviceIsHub;

			connectionInfoEx->DeviceAddress =
				connectionInfo->DeviceAddress;

			connectionInfoEx->NumberOfOpenPipes =
				connectionInfo->NumberOfOpenPipes;

			connectionInfoEx->ConnectionStatus =
				connectionInfo->ConnectionStatus;

			memcpy(&connectionInfoEx->PipeList[0],
				&connectionInfo->PipeList[0],
				sizeof(USB_PIPE_INFO) * 30);

			GlobalFree(connectionInfo);
		}

		if (connectionInfoEx->ConnectionStatus == NoDeviceConnected)
		{
			GlobalFree(connectionInfoEx);
			continue;
		}
		else //	if (connectionInfoEx->ConnectionStatus != NoDeviceConnected)
		{
			PTSTR driverKeyName = GetDriverKeyName(hHubDevice,index);

			DLog(1,_T("index= %d driverKeyName=%s\r\n"),index,driverKeyName);

			if ((driverKeyName == NULL) || _tcsicmp(driverKeyName, driverKey))
			{
				GlobalFree(connectionInfoEx);
				continue;
			}
		}
		
		portnum = index;

		info->mode = connectionInfoEx->DeviceDescriptor.bcdUSB;
		info->PID = connectionInfoEx->DeviceDescriptor.idProduct;
		info->VID = connectionInfoEx->DeviceDescriptor.idVendor;


		PTSTR SN_str = GetStringDescriptor(hHubDevice,index,connectionInfoEx->DeviceDescriptor.iSerialNumber,0);
		if ( SN_str!= NULL)
		{
			//_tprintf(_T("iSerialNumber=%s\r\n"),DESCRIPTOR_STR(SN_str));

			_stprintf_s(info->SN, UI_MAXLEN,_T("%s"),DESCRIPTOR_STR(SN_str));

		}
		else
		{

			//_tprintf(_T("iSerialNumber=Nul\r\n"));
			_stprintf_s(info->SN, UI_MAXLEN,_T("Nul"));
		}

		PTSTR Manufacturer_str = GetStringDescriptor(hHubDevice,index,connectionInfoEx->DeviceDescriptor.iManufacturer,0);
		if ( Manufacturer_str!= NULL)
		{
			//_tprintf(_T("iManufacturer=%s\r\n"),DESCRIPTOR_STR(Manufacturer_str));
			_stprintf_s(info->VSTR, UI_MAXLEN,_T("%s"),DESCRIPTOR_STR(Manufacturer_str));
		}
		else
		{
			//_tprintf(_T("iManufacturer=Nul\r\n"));
			_stprintf_s(info->VSTR, UI_MAXLEN,_T("Nul"));
		}

		PTSTR Product_str = GetStringDescriptor(hHubDevice,index,connectionInfoEx->DeviceDescriptor.iProduct,0);
		if ( Product_str!= NULL)
		{
			//_tprintf(_T("iProduct=%s\r\n"),DESCRIPTOR_STR(Product_str));
			_stprintf_s(info->PSTR, UI_MAXLEN,_T("%s"),DESCRIPTOR_STR(Product_str));
		}
		else
		{
			//_tprintf(_T("iProduct=Nul\r\n"));
			_stprintf_s(info->PSTR, UI_MAXLEN,_T("Nul"));
		}


		if(SN_str != NULL)
		{
			GlobalFree(SN_str);
			SN_str = NULL;
		}

		if(Manufacturer_str != NULL)
		{
			GlobalFree(Manufacturer_str);
			Manufacturer_str = NULL;
		}

		if(Product_str != NULL)
		{
			GlobalFree(Product_str);
			Product_str = NULL;
		}

		if(connectionInfoEx != NULL)
		{
			GlobalFree(connectionInfoEx);
			connectionInfoEx = NULL;
		}


	}

GetDevicePortNumError:

	if (hHubDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hHubDevice);
		hHubDevice = INVALID_HANDLE_VALUE;
	}

	if (hubInfo)
	{
		GlobalFree(hubInfo);
	}

	return portnum;
}

DEVINST GetDrivesDevInstByDiskNumber( DWORD DiskNumber ,DWORD DiskType) 
{
	DEVINST devInstResult = 0;
	GUID* guid = (GUID*)&GUID_DEVINTERFACE_DISK;
	//GUID* guid = (GUID*)&UsbClassGuid;
	if (DiskType == FILE_DEVICE_CD_ROM)
	{
		guid = (GUID*)&GUID_DEVINTERFACE_CDROM;
	}
	HDEVINFO hDevInfo = SetupDiGetClassDevs( guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE){
		return 0;
	}

	DWORD dwIndex = 0;
	SP_DEVICE_INTERFACE_DATA devInterfaceData = {0};
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	BOOL bRet = FALSE;

	BYTE Buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;
	SP_DEVICE_INTERFACE_DATA spdid;
	SP_DEVINFO_DATA spdd;
	DWORD dwSize;

	spdid.cbSize = sizeof(spdid);

	while ( true )
	{
		bRet = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &devInterfaceData);
		if (!bRet) 
		{
			break;
		}

		SetupDiEnumInterfaceDevice(hDevInfo, NULL, guid, dwIndex, &spdid);

		dwSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL);

		if ( dwSize!=0 && dwSize<=sizeof(Buf) ) 
		{
			pspdidd->cbSize = sizeof(*pspdidd); 

			ZeroMemory((PVOID)&spdd, sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
			if ( res ) 
			{

				HANDLE hDrive = CreateFile(pspdidd->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				if ( hDrive != INVALID_HANDLE_VALUE ) 
				{
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
					if ( res ) 
					{
						if ( DiskNumber == sdn.DeviceNumber  && DiskType == sdn.DeviceType) 
						{
							CloseHandle(hDrive);
							devInstResult = spdd.DevInst;
							DLog(1,_T("DevicePath = %s\r\n"), pspdidd->DevicePath);
							//GetDevicePortNum(hDevInfo, &spdd);
							break;
						}
					}
					CloseHandle(hDrive);
				}
			}
		}
		dwIndex++;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);
	return devInstResult;
}

/**  取机器号 */
BOOL queryDeviceNumber(LPCTSTR devPath,DWORD& DiskNumber ,DWORD& DiskType)
{
	BOOL result = FALSE;
	HANDLE hUSB = CreateFile(devPath, 0,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if (hUSB == INVALID_HANDLE_VALUE) // cannot open the drive
	{
		//ErrorExit(TEXT("createfile"));
		return result;
	}

	STORAGE_DEVICE_NUMBER sdn;
	DWORD dwBytesReturned = 0;
	BOOL status = DeviceIoControl(hUSB, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);	
	CloseHandle(hUSB);
	if(!status)
	{
		//ErrorExit(TEXT("DeviceIoControl"));
		return result;
	}
	else
	{
		DiskNumber = sdn.DeviceNumber;
		DiskType = sdn.DeviceType;
		result = TRUE;
	}

	return result;
}



PTSTR GetHubDevicePathByDevInst(DEVINST devInst) 
{
	PTSTR result = NULL;
	DEVINST devInstResult = 0;
	GUID* guid = (GUID*)&GUID_DEVINTERFACE_USB_HUB;
	//GUID* guid = (GUID*)&UsbClassGuid;
	HDEVINFO hDevInfo = SetupDiGetClassDevs( guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE){
		return 0;
	}

	DWORD dwIndex = 0;
	SP_DEVICE_INTERFACE_DATA devInterfaceData = {0};
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	BOOL bRet = FALSE;

	BYTE buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buf;
	SP_DEVICE_INTERFACE_DATA spdid;
	SP_DEVINFO_DATA spdd;
	DWORD dwSize;

	spdid.cbSize = sizeof(spdid);

	while ( true )
	{
		bRet = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &devInterfaceData);
		if (!bRet) 
		{
			break;
		}

		SetupDiEnumInterfaceDevice(hDevInfo, NULL, guid, dwIndex, &spdid);

		dwSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL);

		if ( dwSize!=0 && dwSize<=sizeof(buf) ) 
		{
			pspdidd->cbSize = sizeof(*pspdidd); 

			ZeroMemory((PVOID)&spdd, sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
			if ( res ) 
			{

				//	_tprintf(_T("DevicePath = %s\r\n"), pspdidd->DevicePath);
				//	_tprintf(_T("DevInst = %d\r\n"), spdd.DevInst);

				if(spdd.DevInst == devInst)
				{
					result = (PTSTR) GlobalAlloc(GPTR,(200 * sizeof( TCHAR)));

					_tcscpy_s(result, 200, pspdidd->DevicePath);

					//	_tprintf(_T("DevicePath = %s\r\n"), pspdidd->DevicePath);
					break;
				}


			}


		}
		dwIndex++;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return result;
}

BOOL isHUB(DEVINST devInst)
{
	TCHAR buf[MAX_DEVICE_ID_LEN];  // (Dynamically size this instead?)
	CONFIGRET   cr;
	ULONG       len;


	len = sizeof(buf) / sizeof(buf[0]);
	cr = CM_Get_DevNode_Registry_Property(devInst,
		CM_DRP_SERVICE,
		NULL,
		buf,
		&len,
		0);


	if (cr == CR_SUCCESS)
	{
		DLog(1,_T("test: %s\n"),buf);   

		if (StrStrI(buf, _T("HUB")) != NULL)
		{
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		DLog(0,_T("CM_DRP_DRIVER failed\r\n"));
		return FALSE;
	}
}

//取得hub和hub直系设备，避免复合设备干扰
DEVINST GetHubInstAndCorrectDevInst(DEVINST &devinst)
//DEVINST GetHubInstFromDevInst(DEVINST &devinst)
{
	DEVINST     devInstP;


	while(TRUE)
	{
		CONFIGRET cr = CM_Get_Parent(&devInstP,devinst,0);

		if (cr != CR_SUCCESS)
		{
			return 0;
		}

		if (isHUB(devInstP) == TRUE)
		{
			return devInstP;
		}

		devinst = devInstP;
		DLog(1,_T("dev inst %d\r\n"), devinst);
	}

	return 0;
}


PTSTR GetStringDescriptor (
						   HANDLE  hHubDevice,
						   ULONG   ConnectionIndex,
						   UCHAR   DescriptorIndex,
						   USHORT  LanguageID
						   )
{
	BOOL    success;
	ULONG   nBytes;
	ULONG   nBytesReturned;

	UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
		MAXIMUM_USB_STRING_LENGTH];

	PUSB_DESCRIPTOR_REQUEST stringDescReq;
	PUSB_STRING_DESCRIPTOR  stringDesc;

	nBytes = sizeof(stringDescReqBuf);

	stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
	stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

	// Zero fill the entire request structure
	//
	memset(stringDescReq, 0, nBytes);

	// Indicate the port from which the descriptor will be requested
	//
	stringDescReq->ConnectionIndex = ConnectionIndex;

	//
	// USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
	// IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
	//
	// USBD will automatically initialize these fields:
	//     bmRequest = 0x80
	//     bRequest  = 0x06
	//
	// We must inititialize these fields:
	//     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
	//     wIndex    = Zero (or Language ID for String Descriptors)
	//     wLength   = Length of descriptor buffer
	//
	stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
		| DescriptorIndex;

	stringDescReq->SetupPacket.wIndex = LanguageID;

	stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

	// Now issue the get descriptor request.
	//
	success = DeviceIoControl(hHubDevice,
		IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
		stringDescReq,
		nBytes,
		stringDescReq,
		nBytes,
		&nBytesReturned,
		NULL);

	//
	// Do some sanity checks on the return from the get descriptor request.
	//

	if (!success)
	{
		return NULL;
	}

	if (nBytesReturned < 2)
	{
		return NULL;
	}

	if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
	{
		return NULL;
	}

	if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
	{
		return NULL;
	}

	if (stringDesc->bLength % 2 != 0)
	{
		return NULL;
	}


	PTSTR	result = (PTSTR) GlobalAlloc(GPTR,((stringDesc->bLength+1) * sizeof( TCHAR)));

	memcpy(result,stringDesc,stringDesc->bLength);

	//
	// Looks good, allocate some (zero filled) space for the string descriptor
	// node and copy the string descriptor to it.
	//

	//stringDescNode = (PSTRING_DESCRIPTOR_NODE)ALLOC(sizeof(STRING_DESCRIPTOR_NODE) + stringDesc->bLength);

	//if (stringDescNode == NULL)
	//{
	//    return NULL;
	//}

	//stringDescNode->DescriptorIndex = DescriptorIndex;
	//stringDescNode->LanguageID = LanguageID;

	//memcpy(stringDescNode->StringDescriptor,
	//       stringDesc,
	//       stringDesc->bLength);

	return result;
}



int getUSBInfo(TCHAR diskletter, usb_info* info)
{
	static TCHAR dev_buf[] = _T("\\\\.\\X:");

	dev_buf[4] =diskletter;

	DLog(1,_T("disk letter  %s\r\n"),dev_buf);


	DWORD DiskNumber,DiskType;
	DEVINST devinst;
	if(queryDeviceNumber(dev_buf,DiskNumber,DiskType) == FALSE)
	{
		DLog(0,_T("queryDeviceNumber failed\r\n"));
		return -1;
	}
	DLog(1,_T("DiskNumber=%d\r\n"),DiskNumber);
	DLog(1,_T("DiskType=%d\r\n"),DiskType);

	devinst = GetDrivesDevInstByDiskNumber(DiskNumber,DiskType);
	if(!devinst)
	{
		DLog(0,_T("GetDrivesDevInstByDiskNumber failed\r\n"));
		return -1;
	}


	DEVINST HubInst = GetHubInstAndCorrectDevInst(devinst);
	if (HubInst == 0)
	{
		DLog(0,_T("get hub inst failed\r\n"));
		return -1;
	}

	DWORD portnum = GetDevicePortNum(devinst);
	if(!portnum)
	{
		DLog(0,_T("error devinst=%d,portnum=%d\r\n"),devinst,portnum);
	}

	DLog(1,_T("devinst=%d,portnum=%d\r\n"),devinst,portnum);



	PTSTR HubDevicePath = GetHubDevicePathByDevInst(HubInst);

	if (HubDevicePath == NULL)
	{
		DLog(0,_T("GetHubDevicePath failed\r\n"));
		return -1;
	}

	DLog(1,_T("hub=%s\r\n"),HubDevicePath);

	PTSTR driverKey = GetDeviceDriver(devinst);
	if (driverKey == NULL)
	{
		DLog(0,_T("driverKey failed\r\n"));
		return -1;
	}

	portnum = GetKeyPortInfo(HubDevicePath,driverKey, info);

	if (HubDevicePath != NULL)
	{
		GlobalFree(HubDevicePath);
		HubDevicePath = NULL;
	}

	if (driverKey != NULL)
	{
		GlobalFree(driverKey);
		driverKey = NULL;
	}

	if (portnum< 0)
	{
		DLog(0,_T("GetKeyPortInfo failed\r\n"));
		return -1;
	}
	return 0;
}