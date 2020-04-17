
// FlashHazirlaDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CFlashHazirlaDlg dialog
class CFlashHazirlaDlg : public CDialogEx
{
// Construction
public:
	CFlashHazirlaDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FLASHHAZIRLA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit parola;
	CEdit parolaTekrar;
	CButton btnAyarHazirla;
	CButton btnFlashHazirla;
	CComboBox cmbSurucu;
	afx_msg void OnBnClickedButton3();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnBnClickedButton1();
	CString usb;
	char sifreliMesaj[500], *cozulmusMesaj;
	bool CFlashHazirlaDlg::Sifrele(void);
};
