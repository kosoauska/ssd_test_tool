
// mKeyFerryToolDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <Strsafe.h>

#include "HListBox.h"

// CmKeyFerryToolDlg 对话框
class CmKeyFerryToolDlg : public CDialogEx
{
// 构造
public:
	CmKeyFerryToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MKEYFERRYTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	HANDLE m_hDev;

	HANDLE m_hThread;

	CButton m_btn_backup;

	/**  日志控件 */
	CEdit m_CEdit_Log;

	/** 设备列表控件 */
	CHListBox m_ClB_HW;

	/** 密钥文件路径 */
	CString m_keypath;
	/** 密钥文件路径编辑框*/
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
// 实现
protected:
	HICON m_hIcon;

	INT OnSaveFile(CString &filePath);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//添加
	/**
	* @brief 新设备插入拔出事件
	*
	* @param [in] nEventType 事件类型
	*   - DBT_DEVICEARRIVAL
	*   - DBT_DEVICEREMOVECOMPLETE
	* @param [in] dwData PDEV_BROADCAST_DEVICEINTERFACE
	*
	* @return 成功返回true
	*/
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	/**
	* @brief 列出所有设备
	*
	* @param [in] pguid u盘GUID
	*
	* @return 成功返回0
	*/
	int listdevice(CONST GUID * pguid);

	/**
	* @brief 从设备id获得设备路径
	*
	* @param [in] DevId 设备id
	*
	* @return 设备路径
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

//RegisterDeviceNotification需要下面的
// Copy from HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
static const GUID GUID_DEVINTERFACE_LIST[] =
{

	{ 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },

	{ 0x53f56307, 0xb6bf, 0x11d0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },

	{ 0x53F56308, 0xB6BF, 0x11D0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } }
};
