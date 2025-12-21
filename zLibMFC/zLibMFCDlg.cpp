
// zLibMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "zLibMFC.h"
#include "zLibMFCDlg.h"
#include "afxdialogex.h"
#include "Compressor.h"
#include "Decompressor.h"
#include "CustomMessages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CzLibMFCDlg dialog



CzLibMFCDlg::CzLibMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CzLibMFCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pCompressor = nullptr;
	m_pDecompressor = nullptr;
}

CzLibMFCDlg::~CzLibMFCDlg()
{
	if(m_pCompressor != nullptr)
		delete m_pCompressor;
	if(m_pDecompressor != nullptr)
		delete m_pDecompressor;
}


void CzLibMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRB_BAR, m_prgBarMain);
	DDX_Control(pDX, IDC_SLD_LEVEL, m_sldCompressLevel);
	DDX_Control(pDX, IDC_CB_BUFFER_SIZE, m_cboBufferSize);
}

BEGIN_MESSAGE_MAP(CzLibMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDT_FILEPATH, &CzLibMFCDlg::OnEnChangeEdtFilepath)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CzLibMFCDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_COMPRESS, &CzLibMFCDlg::OnBnClickedBtnCompress)
	ON_BN_CLICKED(IDC_BTN_DECOMPRESS, &CzLibMFCDlg::OnBnClickedBtnDecompress)
	ON_BN_CLICKED(IDC_BTN_OUTPUT_BROWSE, &CzLibMFCDlg::OnBnClickedBtnOutputBrowse)
END_MESSAGE_MAP()


// CzLibMFCDlg message handlers

BOOL CzLibMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_sldCompressLevel.SetRange(1, 7);
	m_sldCompressLevel.SetPos(5);

	m_cboBufferSize.AddString(_T("1 MB"));
	m_cboBufferSize.AddString(_T("2 MB"));
	m_cboBufferSize.AddString(_T("3 MB"));
	m_cboBufferSize.AddString(_T("10 MB"));
	m_cboBufferSize.AddString(_T("20 MB"));
	m_cboBufferSize.AddString(_T("30 MB"));
	m_cboBufferSize.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CzLibMFCDlg::OnDestroy()
{
	if(m_pCompressor != nullptr && m_pCompressor->IsRunning())
		m_pCompressor->Stop();
	if(m_pDecompressor != nullptr && m_pDecompressor->IsRunning())
		m_pDecompressor->Stop();

	CDialogEx::OnDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CzLibMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CzLibMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CzLibMFCDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE)
		{
			OnBnClickedBtnCancel();
			return 1;
		}
	}

	if(pMsg->message == WM_COMPRESSOR_START || pMsg->message == WM_DECOMPRESSOR_START)
	{
		GetDlgItem(IDC_EDT_FILEPATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(FALSE);
		if(pMsg->message == WM_COMPRESSOR_START)
		{
			GetDlgItem(IDC_BTN_COMPRESS)->SetWindowText(_T("&Cancel"));
			GetDlgItem(IDC_BTN_DECOMPRESS)->EnableWindow(FALSE);
		}
		else if(pMsg->message == WM_DECOMPRESSOR_START)
		{
			GetDlgItem(IDC_BTN_DECOMPRESS)->SetWindowText(_T("&Cancel"));
			GetDlgItem(IDC_BTN_COMPRESS)->EnableWindow(FALSE);
		}

		m_prgBarMain.SetRange32(0, 100);
		m_prgBarMain.SetPos(0);
		m_prgBarMain.ShowWindow(SW_SHOW);
	}

	else if(pMsg->message == WM_COMPRESSOR_PROGRESS || pMsg->message == WM_DECOMPRESSOR_PROGRESS)
	{
		m_prgBarMain.SetPos((int)pMsg->wParam);
		CString strProgress;
		strProgress.Format(_T("zLibMFC [%ld %%]"), pMsg->wParam);
		SetWindowText(strProgress);
	}
	else if(pMsg->message == WM_COMPRESSOR_END || pMsg->message == WM_DECOMPRESSOR_END)
	{
		GetDlgItem(IDC_EDT_FILEPATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(TRUE);
		if(pMsg->message == WM_COMPRESSOR_END)
		{
			GetDlgItem(IDC_BTN_COMPRESS)->SetWindowText(_T("&Compress"));
			GetDlgItem(IDC_BTN_DECOMPRESS)->EnableWindow(TRUE);

		}
		else if(pMsg->message == WM_DECOMPRESSOR_END)
		{
			GetDlgItem(IDC_BTN_DECOMPRESS)->SetWindowText(_T("&Decompress"));
			GetDlgItem(IDC_BTN_COMPRESS)->EnableWindow(TRUE);
		}

		GetDlgItem(IDC_BTN_COMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
		GetDlgItem(IDC_BTN_DECOMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);

		m_prgBarMain.ShowWindow(SW_HIDE);

		if(pMsg->wParam == 0)
		{
			AfxMessageBox(_T("Done"), MB_ICONINFORMATION);
		}
		else if(pMsg->wParam == COMPRESS_ERROR_ABORTED || pMsg->wParam == DECOMPRESS_ERROR_ABORTED)
		{
			AfxMessageBox(_T("Aborted"), MB_ICONWARNING);
		}
		else
		{
			AfxMessageBox(_T("Error"), MB_ICONERROR);
		}

		SetWindowText(_T("zLibMFC"));
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CzLibMFCDlg::OnEnChangeEdtFilepath()
{
	GetDlgItem(IDC_BTN_COMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
	GetDlgItem(IDC_BTN_DECOMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
}


void CzLibMFCDlg::OnBnClickedBtnBrowse()
{
	CFileDialog dlgOpen(TRUE);
	dlgOpen.GetOFN().Flags |= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	dlgOpen.GetOFN().lpstrTitle = _T("Choose File");
	dlgOpen.GetOFN().lpstrFilter = _T("All Files\0*.*\0\0");
	if(dlgOpen.DoModal()!=IDOK)
		return;
	SetDlgItemText(IDC_EDT_FILEPATH, dlgOpen.GetPathName());
}


void CzLibMFCDlg::OnBnClickedBtnCompress()
{
	if(m_pCompressor && m_pCompressor->IsRunning())
	{
		OnBnClickedBtnCancel();
	}
	else
	{
		CString strFilePath;
		GetDlgItemText(IDC_EDT_FILEPATH, strFilePath);
		if(strFilePath.IsEmpty())
		{
			GetDlgItem(IDC_BTN_COMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
			GetDlgItem(IDC_BTN_DECOMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
			AfxMessageBox(_T("File path is empty"), MB_ICONERROR);
			return;
		}

		CString strOutputFilePath;
		GetDlgItemText(IDC_EDT_OUTPUT_FILEPATH, strOutputFilePath);

		if(m_pCompressor == nullptr)
		{
			m_pCompressor = new	CCompressor(strFilePath, strOutputFilePath, (m_cboBufferSize.GetCurSel()+1)*1024*1024, m_sldCompressLevel.GetPos(), IsDlgButtonChecked(IDC_CHB_OVERWRITE) & BST_CHECKED, this);
		}
		else
		{
			m_pCompressor->SetBufferSize((m_cboBufferSize.GetCurSel()+1)*1024*1024);
			m_pCompressor->SetOverwriteOutputFile(IsDlgButtonChecked(IDC_CHB_OVERWRITE) & BST_CHECKED);
			m_pCompressor->SetCompressLevel(m_sldCompressLevel.GetPos());
		}

		if(m_pCompressor->m_hThread == NULL)
		{
			if(m_pCompressor->CreateThread(CREATE_SUSPENDED))
			{
				m_pCompressor->ResumeThread();
			}
			else
			{
				TCHAR szError[255] = {0};
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, szError, 255, nullptr);
				AfxMessageBox(szError, MB_ICONERROR);
			}
		}
	}
}


void CzLibMFCDlg::OnBnClickedBtnDecompress()
{
	if(m_pDecompressor && m_pDecompressor->IsRunning())
	{
		OnBnClickedBtnCancel();
	}
	else
	{
		CString strFilePath;
		GetDlgItemText(IDC_EDT_FILEPATH, strFilePath);
		if(strFilePath.IsEmpty())
		{
			GetDlgItem(IDC_BTN_COMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
			GetDlgItem(IDC_BTN_DECOMPRESS)->EnableWindow(GetDlgItem(IDC_EDT_FILEPATH)->GetWindowTextLength() > 0);
			AfxMessageBox(_T("File path is empty"), MB_ICONERROR);
			return;
		}

		CString strOutputFile;
		GetDlgItemText(IDC_EDT_OUTPUT_FILEPATH, strOutputFile);

		if(m_pDecompressor == nullptr)
		{
			m_pDecompressor = new CDecompressor(strFilePath,  strOutputFile, (m_cboBufferSize.GetCurSel()+1)*1024*1024, IsDlgButtonChecked(IDC_CHB_OVERWRITE) & BST_CHECKED, this);
		}
		else
		{
			m_pDecompressor->SetBufferSize((m_cboBufferSize.GetCurSel()+1)*1024*1024);
			m_pDecompressor->SetOverwriteOutputFile(IsDlgButtonChecked(IDC_CHB_OVERWRITE) & BST_CHECKED);
		}

		if(m_pDecompressor->m_hThread == NULL)
		{
			if(m_pDecompressor->CreateThread(CREATE_SUSPENDED))
			{
				m_pDecompressor->ResumeThread();
			}
			else
			{
				TCHAR szError[255] = {0};
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, szError, 255, nullptr);
				AfxMessageBox(szError, MB_ICONERROR);
			}
		}
	}
}


void CzLibMFCDlg::OnBnClickedBtnCancel()
{
	if(m_pCompressor)
		m_pCompressor->SuspendThread();
	if(m_pDecompressor)
		m_pDecompressor->SuspendThread();

	if(AfxMessageBox(_T("Are you sure to abort?"), MB_ICONQUESTION | MB_YESNO) != IDYES)
	{
		if(m_pCompressor)
			m_pCompressor->ResumeThread();
		if(m_pDecompressor)
			m_pDecompressor->ResumeThread();
		return;
	}

	if(m_pCompressor)
	{
		m_pCompressor->Stop();
		m_pCompressor->ResumeThread();
	}
	if(m_pDecompressor)
	{
		m_pDecompressor->Stop();
		m_pDecompressor->ResumeThread();
	}
}


void CzLibMFCDlg::OnBnClickedBtnOutputBrowse()
{
	CFileDialog dlgSave(FALSE);
	dlgSave.GetOFN().Flags |= OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	dlgSave.GetOFN().lpstrTitle = _T("Choose output file");
	dlgSave.GetOFN().lpstrFilter = _T("All Files\0*.*\0\0");
	if(dlgSave.DoModal() != IDOK)
		return;

	SetDlgItemText(IDC_EDT_OUTPUT_FILEPATH, dlgSave.GetPathName());
}
