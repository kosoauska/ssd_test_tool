// sata.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <setupapi.h> 
#include <Ntddscsi.h>
#include <winioctl.h> //STORAGE_DEVICE_NUMBER
#include <strsafe.h>
#include <Cfgmgr32.h>
#include <locale.h>
#include <initguid.h> //error LNK2001: �޷��������ⲿ���� _GUID_DEVINTERFACE_USB_HUB

#include "device.h"
#include "debug.h"


#define ATA_IDENTIFY_CMD   0xEC
#define CMD_SCSPRIV_OUT             0xfd
#define CMD_SCSPRIV_IN              0xfe
#define ATA_SECT_SIZE               0x200
#define STAT_ATA_OK                 0x50 
#define STAT_ATA_FAIL               0x51
#define STAT_ATA_TIMEOUT            0x58
#define TYPE_ATA						0
#define TYPE_JMICRON					1
#define TYPE_INITIO						2
BYTE Buf[1024];
PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;


typedef struct _IDINFO
{
	USHORT  wGenConfig;                 // WORD 0: ������Ϣ��
	USHORT  wNumCyls;                   // WORD 1: ������
	USHORT  wReserved2;                 // WORD 2: ����
	USHORT  wNumHeads;                  // WORD 3: ��ͷ��
	USHORT  wReserved4;                 // WORD 4: ����
	USHORT  wReserved5;                 // WORD 5: ����
	USHORT  wNumSectorsPerTrack;        // WORD 6: ÿ�ŵ�������
	USHORT  wVendorUnique[3];           // WORD 7-9: �����趨ֵ
	CHAR    sSerialNumber[20];          // WORD 10-19:���к�
	USHORT  wBufferType;                // WORD 20: ��������
	USHORT  wBufferSize;                // WORD 21: �����С
	USHORT  wECCSize;                   // WORD 22: ECCУ���С
	CHAR    sFirmwareRev[8];            // WORD 23-26: �̼��汾
	CHAR    sModelNumber[40];           // WORD 27-46: �ڲ��ͺ�
	USHORT  wMoreVendorUnique;          // WORD 47: �����趨ֵ
	USHORT  wReserved48;                // WORD 48: ����
	struct {
		USHORT  reserved1:8;
		USHORT  DMA:1;                  // 1=֧��DMA
		USHORT  LBA:1;                  // 1=֧��LBA
		USHORT  DisIORDY:1;             // 1=�ɲ�ʹ��IORDY
		USHORT  IORDY:1;                // 1=֧��IORDY
		USHORT  SoftReset:1;            // 1=��ҪATA������
		USHORT  Overlap:1;              // 1=֧���ص�����
		USHORT  Queue:1;                // 1=֧���������
		USHORT  InlDMA:1;               // 1=֧�ֽ����ȡDMA
	} wCapabilities;                    // WORD 49: һ������
	USHORT  wReserved1;                 // WORD 50: ����
	USHORT  wPIOTiming;                 // WORD 51: PIOʱ��
	USHORT  wDMATiming;                 // WORD 52: DMAʱ��
	struct {
		USHORT  CHSNumber:1;            // 1=WORD 54-58��Ч
		USHORT  CycleNumber:1;          // 1=WORD 64-70��Ч
		USHORT  UnltraDMA:1;            // 1=WORD 88��Ч
		USHORT  reserved:13;
	} wFieldValidity;                   // WORD 53: �����ֶ���Ч�Ա�־
	USHORT  wNumCurCyls;                // WORD 54: CHS��Ѱַ��������
	USHORT  wNumCurHeads;               // WORD 55: CHS��Ѱַ�Ĵ�ͷ��
	USHORT  wNumCurSectorsPerTrack;     // WORD 56: CHS��Ѱַÿ�ŵ�������
	USHORT  wCurSectorsLow;             // WORD 57: CHS��Ѱַ����������λ��
	USHORT  wCurSectorsHigh;            // WORD 58: CHS��Ѱַ����������λ��
	struct {
		USHORT  CurNumber:8;            // ��ǰһ���Կɶ�д������
		USHORT  Multi:1;                // 1=��ѡ���������д
		USHORT  reserved1:7;
	} wMultSectorStuff;                 // WORD 59: ��������д�趨
	ULONG  dwTotalSectors;              // WORD 60-61: LBA��Ѱַ��������
	USHORT  wSingleWordDMA;             // WORD 62: ���ֽ�DMA֧������
	struct {
		USHORT  Mode0:1;                // 1=֧��ģʽ0 (4.17Mb/s)
		USHORT  Mode1:1;                // 1=֧��ģʽ1 (13.3Mb/s)
		USHORT  Mode2:1;                // 1=֧��ģʽ2 (16.7Mb/s)
		USHORT  Reserved1:5;
		USHORT  Mode0Sel:1;             // 1=��ѡ��ģʽ0
		USHORT  Mode1Sel:1;             // 1=��ѡ��ģʽ1
		USHORT  Mode2Sel:1;             // 1=��ѡ��ģʽ2
		USHORT  Reserved2:5;
	} wMultiWordDMA;                    // WORD 63: ���ֽ�DMA֧������
	struct {
		USHORT  AdvPOIModes:8;          // ֧�ָ߼�POIģʽ��
		USHORT  reserved:8;
	} wPIOCapacity;                     // WORD 64: �߼�PIO֧������
	USHORT  wMinMultiWordDMACycle;      // WORD 65: ���ֽ�DMA�������ڵ���Сֵ
	USHORT  wRecMultiWordDMACycle;      // WORD 66: ���ֽ�DMA�������ڵĽ���ֵ
	USHORT  wMinPIONoFlowCycle;         // WORD 67: ��������ʱPIO�������ڵ���Сֵ
	USHORT  wMinPOIFlowCycle;           // WORD 68: ��������ʱPIO�������ڵ���Сֵ
	WORD  wReserved69[22];              // WORD 69-79: ����
	struct {
		USHORT  Reserved1:1;
		USHORT  ATA1:1;                 // 1=֧��ATA-1
		USHORT  ATA2:1;                 // 1=֧��ATA-2
		USHORT  ATA3:1;                 // 1=֧��ATA-3
		USHORT  ATA4:1;                 // 1=֧��ATA/ATAPI-4
		USHORT  ATA5:1;                 // 1=֧��ATA/ATAPI-5
		USHORT  ATA6:1;                 // 1=֧��ATA/ATAPI-6
		USHORT  ATA7:1;                 // 1=֧��ATA/ATAPI-7
		USHORT  ATA8:1;                 // 1=֧��ATA/ATAPI-8
		USHORT  ATA9:1;                 // 1=֧��ATA/ATAPI-9
		USHORT  ATA10:1;                // 1=֧��ATA/ATAPI-10
		USHORT  ATA11:1;                // 1=֧��ATA/ATAPI-11
		USHORT  ATA12:1;                // 1=֧��ATA/ATAPI-12
		USHORT  ATA13:1;                // 1=֧��ATA/ATAPI-13
		USHORT  ATA14:1;                // 1=֧��ATA/ATAPI-14
		USHORT  Reserved2:1;
	} wMajorVersion;                    // WORD 80: ���汾
	USHORT  wMinorVersion;              // WORD 81: ���汾
	USHORT  wReserved82[6];             // WORD 82-87: ����
	struct {
		USHORT  Mode0:1;                // 1=֧��ģʽ0 (16.7Mb/s)
		USHORT  Mode1:1;                // 1=֧��ģʽ1 (25Mb/s)
		USHORT  Mode2:1;                // 1=֧��ģʽ2 (33Mb/s)
		USHORT  Mode3:1;                // 1=֧��ģʽ3 (44Mb/s)
		USHORT  Mode4:1;                // 1=֧��ģʽ4 (66Mb/s)
		USHORT  Mode5:1;                // 1=֧��ģʽ5 (100Mb/s)
		USHORT  Mode6:1;                // 1=֧��ģʽ6 (133Mb/s)
		USHORT  Mode7:1;                // 1=֧��ģʽ7 (166Mb/s) ???
		USHORT  Mode0Sel:1;             // 1=��ѡ��ģʽ0
		USHORT  Mode1Sel:1;             // 1=��ѡ��ģʽ1
		USHORT  Mode2Sel:1;             // 1=��ѡ��ģʽ2
		USHORT  Mode3Sel:1;             // 1=��ѡ��ģʽ3
		USHORT  Mode4Sel:1;             // 1=��ѡ��ģʽ4
		USHORT  Mode5Sel:1;             // 1=��ѡ��ģʽ5
		USHORT  Mode6Sel:1;             // 1=��ѡ��ģʽ6
		USHORT  Mode7Sel:1;             // 1=��ѡ��ģʽ7
	} wUltraDMA;                        // WORD 88:  Ultra DMA֧������
	USHORT    wReserved89[167];         // WORD 89-255
} IDINFO, *PIDINFO;


BOOL GetDiskInfo(DevHandle dev);

BOOL isUSB(LPCTSTR DevicePath)
{

	if(_wcsnicmp(DevicePath,_T("\\\\?\\usb"),7)==0)
	{
		return TRUE;
	}

	return FALSE;
}

HANDLE OpenDevice(LPCTSTR devicepath)
{
	return CreateFile(devicepath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
}

BOOL CloseDevice(HANDLE dev)
{
	return CloseHandle(dev);
}
void AdjustString(char* str, int len)
{
	char ch;
	int i;

	// �����ߵ�
	for (i = 0; i < len; i += 2)
	{
		ch = str[i];
		str[i] = str[i + 1];
		str[i + 1] = ch;
	}

	// �����Ҷ���ģ�����Ϊ����� (ȥ����ߵĿո�)
	i = 0;
	while ((i < len) && (str[i] == ' ')) i++;

	::memmove(str, &str[i], len - i);

	// ȥ���ұߵĿո�
	i = len - 1;
	while ((i >= 0) && (str[i] == ' '))
	{
		str[i] = (char)'/0';
		i--;
	}
}



// IOCTL������
// #define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_SEND_DRIVE_COMMAND   CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
// #define  DFP_RECEIVE_DRIVE_DATA   0x0007c088
#define  DFP_RECEIVE_DRIVE_DATA   CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//#define SMART_SEND_DRIVE_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
//#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define  IOCTL_SCSI_MINIPORT_IDENTIFY      ((FILE_DEVICE_SCSI << 16) + 0x0501)

// ATA/ATAPIָ��
#define  IDE_ATA_IDENTIFY           0xEC     // ATA��IDָ��(IDENTIFY DEVICE)









// ����������IDENTIFY DEVICE���������豸��Ϣ
// hDevice: �豸���
// pIdInfo:  �豸��Ϣ�ṹָ��
BOOL mptATAIdentify(DevHandle dev, PIDINFO pIdInfo)
{
	PSENDCMDINPARAMS pSCIP;      // �������ݽṹָ��
	PSENDCMDOUTPARAMS pSCOP;     // ������ݽṹָ��
	DWORD dwOutBytes;            // IOCTL������ݳ���
	BOOL bResult;                // IOCTL����ֵ

	// ��������/������ݽṹ�ռ�
	pSCIP = (PSENDCMDINPARAMS)::GlobalAlloc(LMEM_ZEROINIT, sizeof(SENDCMDINPARAMS) - 1);
	pSCOP = (PSENDCMDOUTPARAMS)::GlobalAlloc(LMEM_ZEROINIT, sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1);

	// ָ��ATA/ATAPI����ļĴ���ֵ
	//    pSCIP->irDriveRegs.bFeaturesReg = 0;
	//    pSCIP->irDriveRegs.bSectorCountReg = 0;
	//    pSCIP->irDriveRegs.bSectorNumberReg = 0;
	//    pSCIP->irDriveRegs.bCylLowReg = 0;
	//    pSCIP->irDriveRegs.bCylHighReg = 0;
	//    pSCIP->irDriveRegs.bDriveHeadReg = 0;
	pSCIP->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;

	// ָ������/������ݻ�������С
	pSCIP->cBufferSize = 0;
	pSCOP->cBufferSize = sizeof(IDINFO);

    GetDiskInfo(dev);


	// IDENTIFY DEVICE
	bResult = ::DeviceIoControl(dev.dev,        // �豸���
		DFP_RECEIVE_DRIVE_DATA,                 // ָ��IOCTL
		pSCIP, sizeof(SENDCMDINPARAMS) - 1,     // �������ݻ�����
		pSCOP, sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1,    // ������ݻ�����
		&dwOutBytes,                // ������ݳ���
		(LPOVERLAPPED)NULL);        // ��ͬ��I/O

	if (bResult == FALSE)
	{
		DLogLastErr(1,TEXT("DeviceIoControl"));
	}
	// �����豸�����ṹ
	::memcpy(pIdInfo, pSCOP->bBuffer, sizeof(IDINFO));

	// �ͷ�����/������ݿռ�
	::GlobalFree(pSCOP);
	::GlobalFree(pSCIP);

	return bResult;
}

CString IdentifyAta(LPCTSTR devicepath)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(devicepath == _T(""))

		return  _T("Nul");

	DevHandle hDev;
	setInValid(&hDev);
	BOOL st = OpenDevice(devicepath, &hDev);
	if (st == FALSE) // cannot open the drive
	{
		_tprintf(_T("createfile fail\r\n"));
		return _T("Nul");
	}

	// IDENTIFY DEVICE
	IDINFO dinfo;
	ZeroMemory(&dinfo,sizeof dinfo);
	BOOL ret = mptATAIdentify(hDev,&dinfo);
	if (ret == FALSE)
	{
		DLog(1,_T("identify error\r\n"));
		CloseDevice(hDev);
		return _T("Nul");
	}

	AdjustString(dinfo.sSerialNumber, 20);
	AdjustString(dinfo.sModelNumber, 40);
	AdjustString(dinfo.sFirmwareRev, 8);

	//AdjustString(dinfo.wReserved69, 11);

	WORD w76,w77;
	//str76=dinfo.wReserved69;
	w76=dinfo.wReserved69[7];
	w77=dinfo.wReserved69[8];
	//CString &current=&_T("");
	CString current,Testmode;
	CString max;
	if(w76 != 0x0000 && w76 != 0xFFFF)
	{
		current = max = _T("SATA/150");
	}

	if(w76 & 0x0010){current = max = _T("----");}
	else if(w76 & 0x0008){current = _T("----"); max = _T("SATA/600");}
	else if(w76 & 0x0004){current = _T("----"); max = _T("SATA/300");}
	else if(w76 & 0x0002){current = _T("----"); max = _T("SATA/150");}

	// 2013/5/1 ACS-3
	if(((w77 & 0x000E) >> 1) == 3){current = _T("SATA/600");}
	else if(((w77 & 0x000E) >> 1) == 2){current = _T("SATA/300");}
	else if(((w77 & 0x000E) >> 1) == 1){current = _T("SATA/150");}

	Testmode=_T("sataType:")+current+_T("|")+max;


	CloseDevice(hDev);

	return Testmode;
}


static BOOL queryDeviceNumber(LPCTSTR devPath,DWORD& DiskNumber ,DWORD& DiskType)
{
	BOOL result = FALSE;
	HANDLE hUSB = CreateFile(devPath, 0,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if (hUSB == INVALID_HANDLE_VALUE) // cannot open the drive
	{
		//DLogLastErr(0,TEXT("queryDeviceNumber createfile failed\r\n"));
		return result;
	}

	STORAGE_DEVICE_NUMBER sdn;
	DWORD dwBytesReturned = 0;

	BOOL status = DeviceIoControl(hUSB, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);	
	CloseHandle(hUSB);
	if(!status)
	{
	//	DLogLastErr(1,TEXT("DeviceIoControl IOCTL_STORAGE_GET_DEVICE_NUMBER failed\r\n"));
		return result;
	}
	else
	{
		DiskNumber = sdn.DeviceNumber;
		DiskType = sdn.DeviceType;
		result = TRUE;
	}

	return TRUE;
}


BOOL __queryDeviceNumber(LPCTSTR devPath,DWORD& DiskNumber ,DWORD& DiskType)
{
	BOOL result = FALSE;
	HANDLE hUSB = CreateFile(devPath, 0,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if (hUSB == INVALID_HANDLE_VALUE) // cannot open the drive
	{
		//DLogLastErr(0,TEXT("queryDeviceNumber createfile failed\r\n"));
		return result;
	}

	STORAGE_DEVICE_NUMBER sdn;
	DWORD dwBytesReturned = 0;
	BOOL status = DeviceIoControl(hUSB, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);	
	CloseHandle(hUSB);
	if(!status)
	{
		DLogLastErr(1,TEXT("DeviceIoControl IOCTL_STORAGE_GET_DEVICE_NUMBER failed\r\n"));
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

DEVINST __GetDrivesDevInstByDiskNumber( DWORD DiskNumber ,DWORD DiskType) 
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

// ��ȡӲ����Ϣ
BOOL   GetDiskInfo(DevHandle dev)
{
    HANDLE hDisk = dev.dev;
    DWORD dwBytesReturned = 0;
    GETVERSIONINPARAMS gvopVersionParam = { 0 };
    BOOL bRet = FALSE;
    unsigned int uiIDCmd = 0;
    SENDCMDINPARAMS InParams = { 0 };
    unsigned int uiDrive = 0;
    BYTE outBuffer[1024] = { 0 };
    int i = 0;
    // ���Ϳ��ƴ��뵽ָ���豸��������, ���ذ汾��Ϣ,����������豸��λ����
    bRet = ::DeviceIoControl(hDisk ,
        SMART_GET_VERSION,
        NULL,
        0,
        &gvopVersionParam,
        sizeof(gvopVersionParam),
        &dwBytesReturned,
        NULL);
    if (FALSE == bRet)
    {
 //       ShowError("DeviceIoControl");
        return FALSE;
    }
    // �豸��λ����bIDEDeviceMapλ4��Ϊ1�Ļ�, ���豸��ATAPI��������������ͨ���ϵ����豸
    if (0x10 & gvopVersionParam.bIDEDeviceMap)
    {
        // ��ȡATAPI�豸������
   //     uiIDCmd = IDE_ATAPI_IDENTIFY;
    }
    else
    {
        // ��ȡATA�豸������
        uiIDCmd = IDE_ATA_IDENTIFY;
    }
    // �������� SENDCMDINPARAMS �������
    InParams.cBufferSize = IDENTIFY_BUFFER_SIZE;
    InParams.irDriveRegs.bFeaturesReg = 0;
    InParams.irDriveRegs.bSectorCountReg = 1;
    InParams.irDriveRegs.bSectorNumberReg = 1;
    InParams.irDriveRegs.bCylLowReg = 0;
    InParams.irDriveRegs.bCylHighReg = 0;
    InParams.irDriveRegs.bDriveHeadReg = (uiDrive & 1) ? 0xB0 : 0xA0;
    InParams.irDriveRegs.bCommandReg = uiIDCmd;
    InParams.bDriveNumber = uiDrive;
    // ���Ϳ��ƴ��뵽ָ���豸��������, ��ȡӲ��������Ϣ
    bRet = ::DeviceIoControl(hDisk,
        SMART_RCV_DRIVE_DATA,
        (LPVOID)(&InParams),
        sizeof(SENDCMDINPARAMS),
        (LPVOID)outBuffer,
        1024,
        &dwBytesReturned,
        NULL);
    if (FALSE == bRet)
    {
//        ShowError("DeviceIoControl");
        return FALSE;
    }
    // Ӳ���е����к�, �ͺ�, �̼��汾�� ��������ǰ�������ֽ������ǵߵ���, ����Ҫת��Ϊ������˳��
    // ��ȡ���к�, �±�20-39
    printf("serial:");
    for (i = 20; i < 40; i = i + 2)
    {
        printf("%c%c", ((SENDCMDOUTPARAMS *)outBuffer)->bBuffer[i + 1], ((SENDCMDOUTPARAMS *)outBuffer)->bBuffer[i]);
    }
    printf("\n");
    // ��ȡ�̼��汾��, �±�46-53
    printf("firmware:");
    for (i = 46; i < 54; i = i + 2)
    {
        printf("%c%c", ((SENDCMDOUTPARAMS *)outBuffer)->bBuffer[i + 1], ((SENDCMDOUTPARAMS *)outBuffer)->bBuffer[i]);
    }  
	//��ȡ�ͺ�, �±�54-93
	printf("\n");
    printf("module:");
    for (i = 54; i < 94; i = i + 2)
    {
        printf("%c%c", ((SENDCMDOUTPARAMS *)outBuffer)->bBuffer[i + 1], ((SENDCMDOUTPARAMS *)outBuffer)->bBuffer[i]);
    }
    printf("\n");
    // �ر��豸
 //   ::CloseHandle(hDisk);
    return TRUE;
}

int proc(TCHAR dl)
{
	static TCHAR dev_buf[] = _T("\\\\.\\X:");
	dev_buf[4] = dl;


	DWORD DiskNumber,DiskType;

	if(__queryDeviceNumber(dev_buf,DiskNumber,DiskType) == FALSE)
	{
		_tprintf(_T("queryDeviceNumber failed\r\n"));
		return -1;
	}

	DEVINST devinst;
	devinst = __GetDrivesDevInstByDiskNumber(DiskNumber,DiskType);
	if(!devinst)
	{
		_tprintf(_T("GetDrivesDevInstByDiskNumber failed\r\n"));
		return -1;
	}

	//_tprintf(_T("DevicePath = %s\r\n"), pspdidd->DevicePath);
	if(_wcsnicmp(pspdidd->DevicePath,_T("\\\\?\\ide"),7)==0)
	{
		CString satamode;
		satamode = IdentifyAta(pspdidd->DevicePath);
//		_tprintf(_T("%c: %s\r\n"),dl, satamode);
		_tprintf(_T("%c:     \r\n"),dl);
		_tprintf(_T("%s      \r\n"),satamode);
	}
	else  if(_wcsnicmp(pspdidd->DevicePath,_T("\\\\?\\scsi"),8)==0)
	{
		CString satamode;
		satamode = IdentifyAta(pspdidd->DevicePath);
	//	_tprintf(_T("%c: %s\r\n"),dl, satamode);
		_tprintf(_T("%c:     \r\n"),dl);
		_tprintf(_T("%s      \r\n"),satamode);
	}
	else
	{


	}

	return 0;
}

int checkdl(TCHAR dl)
{
	static TCHAR dev_buf[] = _T("\\\\.\\X:");
	dev_buf[4] = dl;

	DWORD DiskNumber,DiskType;

	if(queryDeviceNumber(dev_buf,DiskNumber,DiskType) == FALSE)
	{
		return -1;
	}


	return 0;
}

void AddToLog(LPCTSTR InStr)
{
	_tprintf(InStr);
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = 0;
	setlocale(LC_ALL, "chs");

	pAddToLog = &AddToLog;

	TCHAR dl;

	if(argc > 1)
	{
		dl=argv[1][0];
		if(dl >= _T('a') && dl <= _T('z'))
		{
			dl -= _T('a') - _T('A');
		}

		if(argv[1][1] != _T(':') || dl < _T('A') || dl > _T('Z'))
		{
			_tprintf(_T("invalid disk letter\r\n"));
			return -1;
		}

		ret = proc(dl);
	}
	else
	{
		//for(int i=0;i<26;i++)
		{
	//		dl = _T('A')+i;
			dl = _T('L');
			if (checkdl(dl))
			{
	//			continue;
			}
			ret = proc(dl);
		}

		ret = 0;
	}


	return ret;
}

