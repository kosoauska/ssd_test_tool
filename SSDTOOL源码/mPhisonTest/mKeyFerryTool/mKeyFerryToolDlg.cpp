
// mKeyFerryToolDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "mKeyFerryTool.h"
#include "mKeyFerryToolDlg.h"
#include "afxdialogex.h"


#define FILENAME_CONFIG "conf.ini"
#define FILENAME_PY "phsion_disk_check.py"
#define FILENAME_SETPY "phsion_setup.py"
#define EXE_NAME "disk_check.exe"
#define RESULT_NAME "test_disk_cnt.txt"
#define PHISON_SERIAL "SPTL7BQIN00" //11
#define SAVESN_FILENAME "save_sn.txt"

//#ifdef CHAOYUE
//	#define PHISONSSD "\\\\?\\SCSI#Disk&Ven_SATA&Prod_"
//#else
// #define PHISONSSD "\\\\?\\SCSI#Disk&Ven_SSD&Prod_"
//#endif

#define PHISONSATA "\\\\?\\SCSI#Disk&Ven_SATA&Prod_"
#define PHISONSSD "\\\\?\\SCSI#Disk&Ven_SSD&Prod_"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
#include <Dbt.h>   //DEV_BROADCAST_DEVICEINTERFACE
#include <comdef.h> //_com_error
#include <setupapi.h>  
#include <Cfgmgr32.h>  //CM_Get_Child ��Ҫװwin ddk winsdDk
#include <winioctl.h>
#include <afxmt.h>   //CCriticalSection

CmKeyFerryToolDlg * c_keyferrytoolDlg;
void AddToLog(LPCTSTR InStr);

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmKeyFerryToolDlg �Ի���



CmKeyFerryToolDlg::CmKeyFerryToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MKEYFERRYTOOL_DIALOG, pParent)
	, m_keypath(_T(""))
	, m_SN_Prefix(_T(""))
	, m_SN_Begin(0)
	, m_count(0)
	, m_auto(_T("0"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmKeyFerryToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ClB_HW);
	DDX_Control(pDX, IDC_EDIT_KEY, m_ed_keypath);
	DDX_Control(pDX, IDC_EDIT_LOG, m_CEdit_Log);
	DDX_Control(pDX, IDC_BTN_START, m_btn_backup);
	DDX_Control(pDX, IDC_EDIT_NUM, m_ed_SNBegin);
	DDX_Control(pDX, IDC_EDIT_PREFIX, m_ed_SNPRE);
	DDX_Text(pDX, IDC_EDIT_PREFIX, m_SN_Prefix);
	DDX_Text(pDX, IDC_EDIT_NUM, m_SN_Begin);
	DDV_MaxChars(pDX, m_SN_Prefix,18);
	DDV_MinMaxInt(pDX, m_SN_Begin, 0,999999);
}

BEGIN_MESSAGE_MAP(CmKeyFerryToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BROWSE, &CmKeyFerryToolDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_BUTTON1, &CmKeyFerryToolDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CmKeyFerryToolDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_TEST, &CmKeyFerryToolDlg::OnBnClickedBtnTest)
END_MESSAGE_MAP()


// CmKeyFerryToolDlg ��Ϣ�������

BOOL CmKeyFerryToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//get file path
	GetModuleFileName(NULL, m_path_ExeDir.GetBuffer(2048),2048);
	m_path_ExeDir.ReleaseBuffer();
	m_path_ExeDir = m_path_ExeDir.Left(m_path_ExeDir.ReverseFind('\\') + 1);
	m_path_Conf = m_path_ExeDir + _T(FILENAME_CONFIG);
	m_path_py = m_path_ExeDir + _T(FILENAME_PY);
	m_path_Setup = m_path_ExeDir + _T(FILENAME_SETPY);
	m_exe_path = m_path_ExeDir + _T(EXE_NAME);
	m_rv_path = m_path_ExeDir + _T(RESULT_NAME);
	m_sn_path = m_path_ExeDir + _T(SAVESN_FILENAME);
	SetDlgItemText(IDC_EDIT_FILENAME,_T("save_sn.txt"));
	SetDlgItemText(IDC_EDIT_PREFIX, _T("aaabbb"));
	((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(TRUE);
	
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	c_keyferrytoolDlg = this;
	m_CEdit_Log.LimitText();
	//ע���豸����
	HDEVNOTIFY hDevNotify;
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	for (int i = 1; i<sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++) {
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
		hDevNotify = RegisterDeviceNotification(this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if (!hDevNotify) {
			AfxMessageBox(CString("Can't register device notification: ") + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	/////////////
	listdevice(&GUID_DEVINTERFACE_LIST[2]);
	listdevice(&GUID_DEVINTERFACE_LIST[1]);
	//CheckResult();
	m_last_flag = 0;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

BOOL CmKeyFerryToolDlg::CheckResult(BOOL bCheck)
{
	BOOL bRet = FALSE;
	CStdioFile myFile;
	CStdioFile SNFile;
	CFileException fileException;

	if (myFile.Open(m_rv_path, CFile::typeText | CFile::modeReadWrite), &fileException)
	{
		myFile.SeekToBegin();
		CString rv;
		myFile.ReadString(rv);
		if (rv == _T("1"))
		{
			bRet = TRUE;
			myFile.SeekToBegin();
			myFile.WriteString(_T("0"));
		}
		else
		{
			bRet = FALSE;
		}

	}
	else
	{
		TRACE("Can't open file %s,error=%u\n", m_rv_path, fileException.m_cause);
		bRet =  FALSE;
	}

	if (SNFile.Open(m_sn_path, CFile::typeText | CFile::modeReadWrite), &fileException)
	{
		SNFile.SeekToBegin();
		CString sn;
		SNFile.ReadString(sn);
		//show string
		if (bCheck)
		{
			if (sn.Left(11) == _T(PHISON_SERIAL))
			{
				SetDlgItemText(IDC_EDIT_PHISON_SN, sn);
				UpdateData(FALSE);
			}
			else
			{
				SetDlgItemText(IDC_EDIT_PHISON_SN, _T("invalid sn!!!"));
				UpdateData(FALSE);
			}
		}
		else
		{
			SetDlgItemText(IDC_EDIT_PHISON_SN, sn);
			UpdateData(FALSE);
		}
		
		SNFile.SeekToBegin();
		SNFile.WriteString(_T("                "));
	}
	else
	{
		bRet = FALSE;
	}

	myFile.Close();
	SNFile.Close();
	return bRet;
}

PCTSTR CmKeyFerryToolDlg::GetDevPathFromDeviceId(PCTSTR DevId)
{
	CString result;
	result.Empty();
	HDEVINFO hInfo;
	SP_DEVINFO_DATA DeviceInfoData;

	hInfo = SetupDiGetClassDevs(NULL,
		DevId, // Enumerator
		0,
		//DIGCF_ALLCLASSES | DIGCF_PRESENT);
		DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);

	if (hInfo == INVALID_HANDLE_VALUE)
	{
		// Insert error handling here.
		return result;
	}

	SP_INTERFACE_DEVICE_DATA Interface_Info;
	Interface_Info.cbSize = sizeof(Interface_Info);

	for (int i = 0;; i++)
	{
		if (!SetupDiEnumInterfaceDevice(hInfo, NULL, &GUID_DEVINTERFACE_LIST[1], i, &Interface_Info))
		{
			SetupDiDestroyDeviceInfoList(hInfo);
			return result;
		}
		DWORD needed; // get the required lenght
		SetupDiGetDeviceInterfaceDetail(hInfo, &Interface_Info, NULL, 0, &needed, NULL);
		PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(needed);
		if (!detail)

		{
			SetupDiDestroyDeviceInfoList(hInfo);
			return result;
		}
		// fill the device details
		detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hInfo, &Interface_Info, detail, needed, NULL, &DeviceInfoData))
		{
			free((PVOID)detail);
			SetupDiDestroyDeviceInfoList(hInfo);
			return result;
		}
		else
		{
			//return detail->DevicePath;
			continue;

		}
		free((PVOID)detail);
	}

	SetupDiDestroyDeviceInfoList(hInfo);

	return result;
}



//�г����е�usb�豸
int CmKeyFerryToolDlg::listdevice(CONST GUID * pguid)
{
	HDEVINFO hInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;
	//CString classsz = _T("USB");
	// Create a HDEVINFO with all present devices.
	hInfo = SetupDiGetClassDevs(pguid,
		NULL, // Enumerator
		0,
		//DIGCF_ALLCLASSES | DIGCF_PRESENT);
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hInfo == INVALID_HANDLE_VALUE)
	{
		// Insert error handling here.
		return 1;
	}

	SP_INTERFACE_DEVICE_DATA Interface_Info;
	Interface_Info.cbSize = sizeof(Interface_Info);

	for (i = 0;; i++)
	{
		if (!SetupDiEnumInterfaceDevice(hInfo, NULL, pguid, i, &Interface_Info))
		{
			SetupDiDestroyDeviceInfoList(hInfo);
			return(i);

		}
		DWORD needed; // get the required lenght
		SetupDiGetDeviceInterfaceDetail(hInfo, &Interface_Info, NULL, 0, &needed, NULL);
		PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(needed);
		if (!detail)

		{
			SetupDiDestroyDeviceInfoList(hInfo);
			return(i);
		}
		// fill the device details
		detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hInfo, &Interface_Info, detail, needed, NULL, &DeviceInfoData))
		{
			free((PVOID)detail);
			SetupDiDestroyDeviceInfoList(hInfo);
			return(i);
		}
		else
		{
			if ((_tcsnicmp(detail->DevicePath, _T(PHISONSSD), strlen(PHISONSSD)) == 0) 
				|| (_tcsnicmp(detail->DevicePath, _T(PHISONSATA), strlen(PHISONSATA)) == 0))
			//if (_tcsnicmp(detail->DevicePath, _T(PHISONSATA), strlen(PHISONSATA)) == 0)
			{
				m_ClB_HW.AddString(detail->DevicePath);
			}

		}
		free((PVOID)detail);
	}

	if (GetLastError() != NO_ERROR &&
		GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		// Insert error handling here.
		return 1;
	}

	//  Cleanup
	SetupDiDestroyDeviceInfoList(hInfo);

	return 0;
}


//���豸����γ��¼�
BOOL CmKeyFerryToolDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)(LPARAM)dwData;
	PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;

	switch (nEventType)
	{
	case DBT_DEVICEARRIVAL:
		// A device has been inserted adn is now available.
		if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			//if (_tcsnicmp(pDevInf->dbcc_name, _T(PHISONSSD), strlen(PHISONSSD)) == 0)
		if ((_tcsnicmp(pDevInf->dbcc_name, _T(PHISONSSD), strlen(PHISONSSD)) == 0)
		|| (_tcsnicmp(pDevInf->dbcc_name, _T(PHISONSATA), strlen(PHISONSATA)) == 0))
				//if (0 == 0)
			{
				m_ClB_HW.AddString(pDevInf->dbcc_name);
				
				//
				CString sformat;
				GetDlgItem(IDC_EDIT_PREFIX)->GetWindowText(m_SN_Prefix);
				m_SN_Begin = GetDlgItemInt(IDC_EDIT_NUM);
				sformat.Format(_T("%%s%%0%dd"), (int)log10((double)999999) + 1);
				m_SN.Format(sformat, m_SN_Prefix, m_SN_Begin);

				GetDlgItem(IDC_EDIT_FILENAME)->GetWindowText(m_file_name);
				if (((CButton *)GetDlgItem(IDC_CHECK2))->GetCheck())
					m_auto = _T("1");
				//2�����нű�
				CString cmd;
				cmd = "/c " + m_exe_path + " " + m_file_name + " " + m_SN + " " + m_auto;
				/*if (((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck())
				{
					//win PE
					ShellExecute(NULL, "open", "X:\\Windows\\system32\\cmd.exe", cmd, NULL, SW_SHOWNORMAL);
				}*/
				ShellExecute(NULL, "open", "cmd.exe", cmd, NULL, SW_SHOWNORMAL);
			}
		}
		break;

	case DBT_DEVICEREMOVECOMPLETE:
		// Device has been removed.
		if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			//if (_tcsnicmp(pDevInf->dbcc_name, _T(PHISONSSD), strlen(PHISONSSD)) == 0)
			if ((_tcsnicmp(pDevInf->dbcc_name, _T(PHISONSSD), strlen(PHISONSSD)) == 0)
				|| (_tcsnicmp(pDevInf->dbcc_name, _T(PHISONSATA), strlen(PHISONSATA)) == 0))
			{
				int idx = m_ClB_HW.FindString(0, pDevInf->dbcc_name);
				if (idx >= 0 && idx < m_ClB_HW.GetCount())
				{
					m_ClB_HW.DeleteString(idx);
				}
				//check if success
				if (CheckResult(((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck()))
				{
					m_last_flag = 1;
					//int event = 0;
					//ͳ���Զ�+1
					m_count++;
					TCHAR ValBuf[16];
					StringCchPrintf(ValBuf, sizeof(ValBuf), _T("%i"), m_count);
					WritePrivateProfileString(_T("TOTAL_TEST"), _T("COUNT"), ValBuf, m_path_Conf);
					//���к��Զ�+1
					m_SN_Begin++;
					SetDlgItemInt(IDC_EDIT_NUM, m_SN_Begin);
					SetDlgItemInt(IDC_STATIC_COUNT, m_count);
					//add to log success
					//get SN
					CString ps_SN;
					GetDlgItemText(IDC_EDIT_PHISON_SN,ps_SN);
					CString str;
					str = ps_SN + _T("++++++���Գɹ�.\n");
					AddToLog(str);
				}
				else
				{
					//add to log failed
					CString str;
					//get SN
					CString ps_SN;
					GetDlgItemText(IDC_EDIT_PHISON_SN, ps_SN);
					str = ps_SN + _T("--------����ʧ��.\n");
					AddToLog(str);
				}
			}

			//kill process
			//TCHAR command[] = _T("/c taskkill /im HDTunePro.exe /f");
			//ShellExecute(NULL, "open", "cmd.exe", command, NULL, SW_SHOWNORMAL);
			//event = 0;
			//TCHAR ValBuf[16];
			//StringCchPrintf(ValBuf, sizeof(ValBuf), _T("%i"), event);
			//WritePrivateProfileString(_T("HOTPLUG"), _T("EVENT"), ValBuf, m_path_Conf);
		}
		break;

	default:
		break;
	}

	return true;
}


void CmKeyFerryToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CmKeyFerryToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CmKeyFerryToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//initdialog���趨��m_CEdit_Log����ַ�����limittext����UINT_MAX,��������ReplaceSel������д�룬setwindowtext����
void AddToLog(LPCTSTR InStr)
{
	CEdit * edlog;
	if (c_keyferrytoolDlg)
		edlog = &c_keyferrytoolDlg->m_CEdit_Log;
	else
		return;

	int len = edlog->GetWindowTextLength();

	edlog->SetSel(len, -1);
	//m_CEdit_Log.SetSel(-1);
	//�����setsel ReplaceSelĬ���滻��괦
	edlog->ReplaceSel(InStr);
	//�������һ�� Ĭ����Ϊ������ע�͵�
	//m_CEdit_Log.SendMessage(WM_VSCROLL,SB_BOTTOM,0);
	//logit_d(InStr);
}

void AddToLogf(LPCTSTR pstrFormat, ...)
{
	CString str;
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);

	AddToLog(str);
}



void CmKeyFerryToolDlg::OnBnClickedBrowse()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString szFilters = _T("Text Files (*.exe)|*.exe|All Files (*.*)|*.*||");
	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fileDlg(TRUE, _T("exe"), _T("*.exe"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, this);

	// Display the file dialog. When user clicks OK, fileDlg.DoModal()
	// returns IDOK.
	if (fileDlg.DoModal() == IDOK)
	{
		CString m_strPathname = fileDlg.GetPathName();
		m_keypath = m_strPathname;
		//UpdateData(FALSE);
		m_ed_keypath.SetWindowText(m_keypath);
	}

}
#define  SECTOR_LEN (512)
#define  KEY_LEN  (16)

static INT64 WIN32llseek(HANDLE fd, INT64 offset, int whence)
{
	long lo, hi;
	DWORD err;

	lo = offset & 0xffffffff;
	hi = offset >> 32;
	lo = SetFilePointer(fd, lo, &hi, whence);
	if (lo == 0xFFFFFFFF && (err = GetLastError()) != NO_ERROR) {
		return -1;
	}
	return ((INT64)hi << 32) | (INT64)lo;
}


//�����ļ�  
INT CmKeyFerryToolDlg::OnSaveFile(CString &filePath)
{
	BOOL isOpen = FALSE;        //�Ƿ��(����Ϊ����)  
								//	CString defaultDir = L"E:\\FileTest";   //Ĭ�ϴ򿪵��ļ�·��  
								//	CString fileName = L"test.doc";         //Ĭ�ϴ򿪵��ļ���  
	CString defaultDir = _T("");   //Ĭ�ϴ򿪵��ļ�·��  
	CString fileName = _T("");         //Ĭ�ϴ򿪵��ļ��� 
	CString filter = _T("�ļ� (*.*)|*.*||");   //�ļ����ǵ�����  
											 //CString filter = _T("|||");   //�ļ����ǵ�����  
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
	//openFileDlg.GetOFN().lpstrInitialDir = _T("E:\\FileTest\\test.doc");  
	INT_PTR result = openFileDlg.DoModal();
	//INT result = openFileDlg.DoModal();
	filePath = defaultDir + "\\" + fileName;
	if (result == IDOK) {
		filePath = openFileDlg.GetPathName();
	}
	else
		return -1;
	//CWnd::SetDlgItemTextW(IDC_EDIT_DEST, filePath);  

	return 0;
}


void CmKeyFerryToolDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//2�����нű�
	CString cmd;
	if (((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck())
	{
		//win PE
		cmd = "/c " + m_keypath + " " + m_path_Setup;
		ShellExecute(NULL, "open", "X:\\Windows\\system32\\cmd.exe", cmd, NULL, SW_SHOWNORMAL);
	}
	else
	{
		TCHAR command[] = _T("/c python.exe phsion_setup.py");
		ShellExecute(NULL, "open", "cmd.exe", command, NULL, SW_SHOWNORMAL);
	}
}


void CmKeyFerryToolDlg::OnBnClickedBtnClear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT_LOG)->SetWindowText(_T(""));
}


void CmKeyFerryToolDlg::OnBnClickedBtnTest()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString sformat;
	GetDlgItem(IDC_EDIT_PREFIX)->GetWindowText(m_SN_Prefix);
	m_SN_Begin = GetDlgItemInt(IDC_EDIT_NUM);
	sformat.Format(_T("%%s%%0%dd"), (int)log10((double)999999)+1);
	m_SN.Format(sformat,m_SN_Prefix,m_SN_Begin);
	m_SN_Begin++;
	SetDlgItemInt(IDC_EDIT_NUM, m_SN_Begin);

}
