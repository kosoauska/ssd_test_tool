#ifndef _DEBUG_H_
#define _DEBUG_H_


#include <Windows.h>
#include <tchar.h> 


void DLog(int loglevel, LPCTSTR fmt, ...);
void DLogLastErr(int loglevel, LPCTSTR fmt, ...);
void DLogBufFormat(int loglevel, PUCHAR buf, int buflen,LPTSTR bufname);
void DLogBuf(int loglevel, PUCHAR buf, int buflen,LPTSTR bufname);
void DLogBufBigEndian(int loglevel, PUCHAR buf, int buflen,LPTSTR bufname);
void BufReverse(PUCHAR buf, int buflen);
TCHAR * ANSItoUnicode(UCHAR * buf, UINT buflen);
char * UnicodetoANSI(WCHAR * buf, UINT buflen);

typedef void (*pAddToLog_t)(LPCTSTR InStr);

extern "C"  pAddToLog_t pAddToLog;

#endif