#include "stdafx.h"

#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <comdef.h>
#include <assert.h>
#include <strsafe.h>


pAddToLog_t pAddToLog = NULL;

//记录日志
void DLog(int loglevel, LPCTSTR fmt, ...)
{
	if(loglevel)
	{
#ifdef NDEBUG
		return;
#endif
	}
	TCHAR buf[1024];
	va_list argList;


	va_start(argList,fmt);
	int len = _vsctprintf(fmt, argList);

	//buf = (TCHAR*)malloc( len * sizeof(TCHAR) );

	assert(buf != NULL);
	//_vstprintf(buf, fmt, argList);
	StringCchVPrintf(buf, sizeof(buf),	fmt, argList);
	va_end( argList );

	if (pAddToLog != NULL)
	{
		pAddToLog(buf);
	}

	//free( buf );
}

//记录日志
void DLogLastErr(int loglevel, LPCTSTR fmt, ...)
{
	if(loglevel)
	{
#ifdef NDEBUG
		return;
#endif
	}
	TCHAR* buf;
	va_list argList;
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	va_start(argList,fmt);
	int len = _vsctprintf(fmt, argList);

	buf = (TCHAR*)LocalAlloc(LMEM_ZEROINIT, (len+2) * sizeof(TCHAR)); 

	assert(buf != NULL);
	//_vstprintf(buf, fmt, argList);
	StringCchVPrintf(buf,(len+2),fmt,argList); 
	va_end( argList );

	if (pAddToLog != NULL)
	{
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)buf) + 40) * sizeof(TCHAR)); 
		StringCchPrintf((LPTSTR)lpDisplayBuf, 
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("%s failed with error %d: %s"), 
			buf, dw, lpMsgBuf); 

		pAddToLog((LPTSTR)lpDisplayBuf);
		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);

	}
	LocalFree( buf );
}

void DLogBufFormat(int loglevel, PUCHAR buf, int buflen,LPTSTR bufname)
{
	DLog(loglevel,_T("%s   %d\r\n"),bufname,buflen);
	DLog(loglevel,_T("0x0000| "));
	for(int i=0;i<buflen;i++)
	{
		DLog(loglevel,_T("%02x "),buf[i]);
		if(i%16==15)
		{
			DLog(loglevel,_T("\r\n"));
			if(i<buflen-1)
			{
				DLog(loglevel,_T("0x%04x| "),i+1);
			}

		}
	}
	DLog(loglevel,_T("\r\n"));

}

void DLogBuf(int loglevel, PUCHAR buf, int buflen,LPTSTR bufname)
{
	DLog(loglevel,_T("%s   %d\r\n"),bufname,buflen);

	for(int i=0;i<buflen;i++)
	{
		DLog(loglevel,_T("%02x"),buf[i]);
	}
	DLog(loglevel,_T("\r\n"));

}


void DLogBufBigEndian(int loglevel, PUCHAR buf, int buflen,LPTSTR bufname)
{
	DLog(loglevel,_T("%s   %d\r\n"),bufname,buflen);
	for(int i=0;i<buflen;)
	{
		DLog(loglevel,_T("%02x"),buf[i+3]);
		DLog(loglevel,_T("%02x"),buf[i+2]);
		DLog(loglevel,_T("%02x"),buf[i+1]);
		DLog(loglevel,_T("%02x"),buf[i]);
		i+=4;
	}
	DLog(loglevel,_T("\r\n"));

}

void BufReverse(PUCHAR buf, int buflen)
{
	UCHAR c;
	for(int i=0;i<buflen/2;i++)
	{
		c = buf[i];
		buf[i] = buf[buflen-1-i];
		buf[buflen-1-i] = c;
	}
}

//小于256字节
TCHAR * ANSItoUnicode(UCHAR * str, UINT strlen)
{
	static TCHAR outbuf[256];
	memset(outbuf,0,sizeof(outbuf));
	int wlen = MultiByteToWideChar(CP_ACP,0,(LPCSTR)str,strlen,NULL,0);
	if(wlen>sizeof(outbuf)/sizeof(TCHAR))
		wlen = sizeof(outbuf)/sizeof(TCHAR);
	MultiByteToWideChar(CP_ACP,0,(LPCSTR)str,strlen,outbuf,wlen);
	return outbuf;
}

//buflen为宽字符长度，-1表示到NULL
char * UnicodetoANSI(WCHAR * str, UINT strlen)
{
	static char outbuf[256];
	memset(outbuf,0,sizeof(outbuf));
	int wlen = WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)str,strlen,NULL,0,NULL,0);
	if(wlen>sizeof(outbuf))
		wlen = sizeof(outbuf);
	WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)str,strlen,outbuf,wlen,NULL,0);
	return outbuf;
}