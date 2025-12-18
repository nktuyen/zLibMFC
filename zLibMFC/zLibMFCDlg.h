
// zLibMFCDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

class CCompressor;
class CDecompressor;
// CzLibMFCDlg dialog
class CzLibMFCDlg : public CDialogEx
{
// Construction
public:
	CzLibMFCDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CzLibMFCDlg();

	enum { IDD = IDD_ZLIBTEST_DIALOG };
protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
protected:
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEnChangeEdtFilepath();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnCompress();
	afx_msg void OnBnClickedBtnDecompress();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnBnClickedBtnOutputBrowse();

	DECLARE_MESSAGE_MAP()
private:
	HICON m_hIcon;
	CCompressor* m_pCompressor;
	CDecompressor* m_pDecompressor;
	CProgressCtrl m_prgBarMain;
	CSliderCtrl m_sldCompressLevel;
	CComboBox m_cboBufferSize;
};
