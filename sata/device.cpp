#include "StdAfx.h"
#include "device.h"
#include <strsafe.h>
#include <assert.h>
#include <stddef.h>
#include "debug.h"
//#include "../../header/mptool_common.h"

//using namespace mptool_cmd;


/* priv cmd ID */
#define CMD_SCSPRIV_OUT             0xfd
#define CMD_SCSPRIV_IN              0xfe
#define CMD_SCSPRIV_OUT_DATAONLY             0xfc

/* status */
#define STAT_ATA_OK                 0x50 
#define STAT_ATA_FAIL               0x51
#define STAT_ATA_TIMEOUT            0x58

#define ATA_SECT_SIZE               0x200

int ATARegsToCdb(UCHAR* regs,DWORD Type, SCSI_PASS_THROUGH_DIRECT * sptd)
{
	int m_port = 0;
	switch(Type)
	{
	case TYPE_JMICRON:
		sptd->CdbLength = 14;
		sptd->Cdb[0] = 0xdf;
		sptd->Cdb[1] = (sptd->DataIn == SCSI_IOCTL_DATA_IN)?0x10:0x00;
		sptd->Cdb[2] = 0x00;
		sptd->Cdb[3] = (unsigned char)(sptd->DataTransferLength >> 8);
		sptd->Cdb[4] = (unsigned char)(sptd->DataTransferLength);
		sptd->Cdb[5] = regs[0];
		sptd->Cdb[6] = regs[1];
		sptd->Cdb[7] = regs[2];
		sptd->Cdb[8] = regs[3];
		sptd->Cdb[9] = regs[4];
		sptd->Cdb[10] = regs[5] | (m_port == 0? 0xa0:0xb0);
		sptd->Cdb[11] = regs[6];
		sptd->Cdb[12] = 0x06;
		sptd->Cdb[13] = 0x7b;
		break;
	case TYPE_INITIO:
		sptd->CdbLength = 16;
		sptd->Cdb[0] = 0xa1;
		sptd->Cdb[1] = 0x06;
		sptd->Cdb[2] = 0x00;
		sptd->Cdb[3] = regs[0];
		sptd->Cdb[4] = regs[1];
		sptd->Cdb[5] = regs[2];
		sptd->Cdb[6] = regs[3];
		sptd->Cdb[7] = regs[4];
		sptd->Cdb[8] = regs[5];
		sptd->Cdb[9] = regs[6];
		break;
	default:
		return -1;
	}
	
	return 0;
}

int SendAtaCommandOverScsiD(HANDLE hDevice, DWORD Type, ATA_PASS_THROUGH_DIRECT* pData)

{
	ULONG length = 0,
		returned = 0;
	BOOL status = 0;

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ZeroMemory(&sptdwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

	sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptdwb.sptd.PathId = 0;
	sptdwb.sptd.TargetId = 1;
	sptdwb.sptd.Lun = 0;
	sptdwb.sptd.CdbLength = 16;
	sptdwb.sptd.SenseInfoLength = SPT_SENSE_LENGTH;
	if (pData->AtaFlags & ATA_FLAGS_DATA_IN)
	{
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
	}
	else
	{
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
	}
	sptdwb.sptd.DataBuffer = pData->DataBuffer;
	sptdwb.sptd.DataTransferLength = pData->DataTransferLength;
	sptdwb.sptd.TimeOutValue = pData->TimeOutValue;
	sptdwb.sptd.SenseInfoOffset =
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);


	if (ATARegsToCdb(&pData->CurrentTaskFile[0],Type,&sptdwb.sptd))
	{
		return -1;
	}
	

	status = DeviceIoControl(hDevice,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptdwb,
		length,
		&sptdwb,
		length,
		&returned,
		FALSE);

	if(!status)
	{
		DLogLastErr(0,TEXT("SendAtaCommandOverScsiD DeviceIoControl"));
		return -2;
	}

	if(sptdwb.sptd.ScsiStatus)
	{
		//PrintBuf(sptwb.ucSenseBuf,sizeof(sptwb.ucSenseBuf),_T("ucSenseBuf"));
		DLog(0,TEXT("SendAtaCommandOverScsiD ScsiStatus = %d\r\n"),sptdwb.sptd.ScsiStatus);
		return  -3;
	}

	return 0;
}


int SendAtaCommandOverScsi(HANDLE hDevice, DWORD Type, ATA_PASS_THROUGH_EX* pData)
{
	ULONG length = 0,
		returned = 0;
	BOOL status = 0;

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ZeroMemory(&sptdwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

	sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptdwb.sptd.PathId = 0;
	sptdwb.sptd.TargetId = 1;
	sptdwb.sptd.Lun = 0;
	sptdwb.sptd.CdbLength = 16;
	sptdwb.sptd.SenseInfoLength = SPT_SENSE_LENGTH;
	if (pData->AtaFlags & ATA_FLAGS_DATA_IN)
	{
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
	}
	else
	{
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
	}
	sptdwb.sptd.DataBuffer = (BYTE*)pData + pData->DataBufferOffset;
	sptdwb.sptd.DataTransferLength = pData->DataTransferLength;
	sptdwb.sptd.TimeOutValue = pData->TimeOutValue;
	sptdwb.sptd.SenseInfoOffset =
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

	if (ATARegsToCdb(&pData->CurrentTaskFile[0],Type,&sptdwb.sptd))
	{
		return -1;
	}

	status = DeviceIoControl(hDevice,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptdwb,
		length,
		&sptdwb,
		length,
		&returned,
		FALSE);

	if(!status)
	{
		DLogLastErr(0,TEXT("SendAtaCommandOverScsi DeviceIoControl"));
		return -1;
	}

	if(sptdwb.sptd.ScsiStatus)
	{
		//PrintBuf(sptwb.ucSenseBuf,sizeof(sptwb.ucSenseBuf),_T("ucSenseBuf"));
		DLog(0,TEXT("SendAtaCommandOverScsi ScsiStatus = %d\r\n"),sptdwb.sptd.ScsiStatus);
		return  -2;
	}

	return TRUE;

}


int IO_D(DevHandle hDevice, PUCHAR buf, int bufsize,  UCHAR ATACMD, UCHAR flag)
{
	ATA_PASS_THROUGH_DIRECT *papt; 
	BYTE Buffer[sizeof(ATA_PASS_THROUGH_DIRECT)]; 
	papt = (ATA_PASS_THROUGH_DIRECT *)Buffer; 

	ZeroMemory(papt, sizeof(Buffer)); 
	papt->Length = sizeof(ATA_PASS_THROUGH_DIRECT); 
	papt->AtaFlags = flag | ATA_FLAGS_DRDY_REQUIRED;
#if DMAENABLE
	if((ATACMD == CMD_SCSPRIV_OUT) || (ATACMD == CMD_SCSPRIV_IN) || (ATACMD == CMD_SCSPRIV_OUT_DATAONLY))
		papt->AtaFlags |= ATA_FLAGS_USE_DMA; 
#endif

	papt->DataTransferLength = bufsize; 
	papt->TimeOutValue = 2; 
	papt->DataBuffer = buf; 

	DLog(1,TEXT("bufsize = %d\r\n"),bufsize);


	if((ATACMD == CMD_SCSPRIV_OUT) || (ATACMD == CMD_SCSPRIV_IN) || (ATACMD == CMD_SCSPRIV_OUT_DATAONLY))
		papt->CurrentTaskFile[1] = bufsize/(ATA_SECT_SIZE);

	papt->CurrentTaskFile[6] = ATACMD;


	int sizea = papt->Length;

	BOOL bResult = TRUE; 
	DWORD dwWriteSize; 
	if (hDevice.isUsb == FALSE)
	{

		bResult = DeviceIoControl( 
			hDevice.dev, 
			IOCTL_ATA_PASS_THROUGH_DIRECT, 
			papt, sizea, 
			papt, sizea, 
			&dwWriteSize, NULL 
			); 

		DLog(1,TEXT("dwWriteSize = %d\r\n"),dwWriteSize);

		if (bResult == FALSE)
		{
			DLogLastErr(0,TEXT("DeviceIoControl"));
			return -1;
		}


		if (papt->CurrentTaskFile[6] != STAT_ATA_OK)
		{
			DLog(1,TEXT("status = %d\r\n"),papt->CurrentTaskFile[6]);
			return -2;
		}
	}
	else
	{
#define USBRETRYTIMES 3
		int retry = USBRETRYTIMES;
		while(retry--)
		{
			if (SendAtaCommandOverScsiD(hDevice.dev,TYPE_INITIO, papt))
			{
				if (retry)
				{
					continue;
				}
				else
				{
					return -3;
				}
			}
			else
			{
				break;
			}
		}
	

	}

	return 0;
}

inline int Out_D(DevHandle hDevice, PUCHAR buf, int bufsize)
{
	return IO_D(hDevice,buf,bufsize,CMD_SCSPRIV_OUT,ATA_FLAGS_DATA_OUT);
}	

inline int Out_D_DataOnly(DevHandle hDevice, PUCHAR buf, int bufsize)
{
	return IO_D(hDevice,buf,bufsize,CMD_SCSPRIV_OUT_DATAONLY,ATA_FLAGS_DATA_OUT);
}	

inline int In_D(DevHandle hDevice, PUCHAR buf, int bufsize)
{
	return IO_D(hDevice,buf,bufsize,CMD_SCSPRIV_IN,ATA_FLAGS_DATA_IN);
}	





#define USBDEVPATHPREFIX "\\\\?\\usb"

BOOL OpenDevice(LPCTSTR devicepath, DevHandle* dev)
{
	dev->dev = CreateFile(devicepath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if(_tcsnicmp(devicepath,_T(USBDEVPATHPREFIX),_tcslen(_T(USBDEVPATHPREFIX)))==0)
	{
		dev->isUsb = TRUE;
	}
	else
	{
		dev->isUsb = FALSE;
	}

	if (INVALID_HANDLE_VALUE == dev->dev)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CloseDevice(DevHandle dev)
{
	return CloseHandle(dev.dev);
}

BOOL isInValid(DevHandle dev)
{
	return (dev.dev == INVALID_HANDLE_VALUE);
}
void setInValid(DevHandle* dev)
{
	dev->dev = INVALID_HANDLE_VALUE;
}