#ifndef ___DEVICE__H_
#define ___DEVICE__H_

#include <Ntddscsi.h>
//#include <Devioctl.h>
#include <winioctl.h> 

//RegisterDeviceNotification需要下面的
// Copy from HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
static const GUID GUID_DEVINTERFACE_LIST[] = 
{
	// GUID_DEVINTERFACE_USB_DEVICE
	{ 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
	//GUID_DEVINTERFACE_USB_DEVICE,
	// GUID_DEVINTERFACE_DISK
	{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
	//GUID_DEVINTERFACE_DISK,
	// GUID_DEVINTERFACE_HID, 
	//{ 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },

	// GUID_NDIS_LAN_CLASS
	//{ 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }

	//// GUID_DEVINTERFACE_COMPORT
	//{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },

	//// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
	//{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },

	//// GUID_DEVINTERFACE_PARALLEL
	//{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },

	//// GUID_DEVINTERFACE_PARCLASS
	//{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
	//GUID_DEVINTERFACE_CDROM 	{53F56308-B6BF-11D0-94F2-00A0C91EFB8B}

	{ 0x53F56308, 0xB6BF, 0x11D0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } }
};

/* 一些设备GUID定义 */ //与#include <winioctl.h> 冲突,已经在<winioctl.h> 定义
//static const GUID GUID_DEVINTERFACE_USB_DEVICE = { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
//static const GUID GUID_DEVINTERFACE_DISK = { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } };
//static const GUID GUID_DEVINTERFACE_CDROM = { 0x53F56308, 0xB6BF, 0x11D0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } };
//static const GUID GUID_DEVINTERFACE_VOLUME = { 0x53F5630D, 0xB6BF, 0x11D0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } };

#define SPT_SENSE_LENGTH 32
#define SPTWB_DATA_LENGTH 1024


typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
	SCSI_PASS_THROUGH spt;
	ULONG             Filler;      // realign buffers to double word boundary
	UCHAR             ucSenseBuf[SPT_SENSE_LENGTH];
	UCHAR             ucDataBuf[SPTWB_DATA_LENGTH];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT sptd;
	ULONG             Filler;      // realign buffer to double word boundary
	UCHAR             ucSenseBuf[SPT_SENSE_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;


#define TYPE_ATA						0
#define TYPE_JMICRON					1
#define TYPE_INITIO						2

#define DMAENABLE 1

typedef struct __devHandle
{
	HANDLE dev;
	BOOL isUsb; //是否usb 
} DevHandle;

BOOL OpenDevice(LPCTSTR devicepath, DevHandle* dev);
BOOL CloseDevice(DevHandle dev);
BOOL isInValid(DevHandle dev);
void setInValid(DevHandle* dev);

//当ATACMD时，ataflag即为参数flag，当ATACMD为CMD_SCSPRIV_OUT或CMD_SCSPRIV_IN时，flag会与上ATA_FLAGS_USE_DMA | ATA_FLAGS_DRDY_REQUIRED
int IO_D(DevHandle hDevice, PUCHAR buf, int bufsize, UCHAR ATACMD, UCHAR flag);
int CmdIn(DevHandle hDevice, PUCHAR cmdbuf,PUCHAR buf, int bufsize);
int CmdOut(DevHandle hDevice, PUCHAR buf, int bufsize);
int CmdOut_DataOnly(DevHandle hDevice, PUCHAR buf, int bufsize); 


#endif