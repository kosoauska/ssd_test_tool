
// mKeyFerryTool.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CmKeyFerryToolApp: 
// �йش����ʵ�֣������ mKeyFerryTool.cpp
//

class CmKeyFerryToolApp : public CWinApp
{
public:
	CmKeyFerryToolApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CmKeyFerryToolApp theApp;