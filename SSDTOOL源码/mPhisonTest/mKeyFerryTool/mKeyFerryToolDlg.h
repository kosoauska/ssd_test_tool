
// mKeyFerryToolDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <Strsafe.h>

#include "HListBox.h"

// CmKeyFerryToolDlg �Ի���
class CmKeyFerryToolDlg : public CDialogEx
{
// ����
public:
	CmKeyFerryToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MKEYFERRYTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	HANDLE m_hDev;

	HANDLE m_hThread;

	CButton m_btn_backup;

	/**  ��־�ؼ� */
	CEdit m_CEdit_Log;

	/** �豸�б�ؼ� */
	CHListBox m_ClB_HW;

	/** ��Կ�ļ�·�� */
	CString m_keypath;
	/** ��Կ�ļ�·���༭��*/
	CEdit m_ed_keypath;

	CString m_path_ExeDir;
	CString m_path_Conf;
	CString m_path_py;
	CString m_path_Setup;
	CString m_exe_path;
	CString m_rv_path;
	CString m_sn_path;
	CString m_file_name;
	CString m_auto;
	BOOL  m_last_flag;
	CString m_SN;
	CString m_SN_Prefix;
	int m_SN_Begin;
	CEdit m_ed_SNBegin;
	CEdit m_ed_SNPRE;
	int m_count;
// ʵ��
protected:
	HICON m_hIcon;

	INT OnSaveFile(CString &filePath);
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//���
	/**
	* @brief ���豸����γ��¼�
	*
	* @param [in] nEventType �¼�����
	*   - DBT_DEVICEARRIVAL
	*   - DBT_DEVICEREMOVECOMPLETE
	* @param [in] dwData PDEV_BROADCAST_DEVICEINTERFACE
	*
	* @return �ɹ�����true
	*/
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	/**
	* @brief �г������豸
	*
	* @param [in] pguid u��GUID
	*
	* @return �ɹ�����0
	*/
	int listdevice(CONST GUID * pguid);

	/**
	* @brief ���豸id����豸·��
	*
	* @param [in] DevId �豸id
	*
	* @return �豸·��
	*/
	PCTSTR GetDevPathFromDeviceId(PCTSTR DevId);
	BOOL CheckResult(BOOL bCheck);
	DECLARE_MESSAGE_MAP()


public:
	void StartFerryKey();

	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedGenKey();
	afx_msg void OnBnClickedBackup();
	afx_msg void OnBnClickedTest();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnTest();
};

//RegisterDeviceNotification��Ҫ�����
// Copy from HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
static const GUID GUID_DEVINTERFACE_LIST[] =
{

	{ 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },

	{ 0x53f56307, 0xb6bf, 0x11d0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },

	{ 0x53F56308, 0xB6BF, 0x11D0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } }
};
