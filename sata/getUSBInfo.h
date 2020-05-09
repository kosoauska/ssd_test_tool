#ifndef ___GETUSBINFO_HEADER__
#define ___GETUSBINFO_HEADER__


#define UI_MAXLEN 64

typedef struct __usb_info {
	USHORT mode;
	USHORT PID;
	USHORT VID;
	TCHAR SN[UI_MAXLEN];
	TCHAR PSTR[UI_MAXLEN];
	TCHAR VSTR[UI_MAXLEN];
} usb_info;


int getUSBInfo(TCHAR diskletter, usb_info* info);



#endif

