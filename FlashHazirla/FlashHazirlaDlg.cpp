
// FlashHazirlaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlashHazirla.h"
#include "FlashHazirlaDlg.h"
#include "afxdialogex.h"
#include <dbt.h>
#include <iostream>
#include <fstream>
#include "..\Encrypt.h"
#include<string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define BUFSIZE MAX_PATH
#define FILESYSNAMEBUFSIZE MAX_PATH


// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CFlashHazirlaDlg dialog



CFlashHazirlaDlg::CFlashHazirlaDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FLASHHAZIRLA_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFlashHazirlaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, parola);
	DDX_Control(pDX, IDC_EDIT2, parolaTekrar);
	DDX_Control(pDX, IDC_BUTTON1, btnAyarHazirla);
	DDX_Control(pDX, IDC_BUTTON3, btnFlashHazirla);
	DDX_Control(pDX, IDC_COMBO1, cmbSurucu);
}

BEGIN_MESSAGE_MAP(CFlashHazirlaDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFlashHazirlaDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON3, &CFlashHazirlaDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CFlashHazirlaDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CFlashHazirlaDlg message handlers

BOOL CFlashHazirlaDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFlashHazirlaDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFlashHazirlaDlg::OnPaint()
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
HCURSOR CFlashHazirlaDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFlashHazirlaDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CFlashHazirlaDlg::OnBnClickedButton3()
{
	char surucu[500];
	char *argv[3];
	argv[0] =NULL;
	CString item;
	cmbSurucu.GetWindowText(item);
	if (item.GetLength() == 0) MessageBoxA(NULL, "Lütfen anahtar hazýrlanacak sürücüyü\nseçin yada yazýn", "Hata!!!", MB_OK | MB_ICONERROR);
	//if (!Sifrele()) MessageBoxA(NULL, "Lütfen parola alanýný boþ geçmeyin\n", "Hata!!!", MB_OK | MB_ICONERROR);
	if (Sifrele()) {
		wcstombs(surucu, item.GetBuffer(MAX_PATH), 500);
		argv[1] = surucu;
		argv[2]= sifreliMesaj;
	//	MessageBoxA(NULL, argv[2], "en", MB_OK);
		//strcpy(argv[2], *sifreliMesaj);
		//argv[2] = sifreliMesaj;
		if (FlashHazirla(3, argv) == true) MessageBoxA(NULL, "Anahtar baþarýlý bir þekilde\nhazýrlandý", "Ýþlem Baþarýlý", MB_OK | MB_ICONINFORMATION);
		else MessageBoxA(NULL, "Anahtar oluþturalamadý", "Hata!!!", MB_OK | MB_ICONERROR);
	}

	
}
char FirstDriveFromMask(ULONG unitmask)
{
	char i;

	for (i = 0; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}

	return (i + 'A');
}

BOOL CFlashHazirlaDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
	char usbDrive;
	DWORD lpMaximumComponentLength;
	DWORD dwSysFlags;
	char FileSysNameBuf[FILESYSNAMEBUFSIZE];

	TCHAR wszVolumeName[MAX_PATH + 1];
	_TCHAR Buffer[MAX_PATH + 1];
	switch (message)
	{
		case WM_DEVICECHANGE:
			switch (wParam) 
			{
			 case DBT_DEVICEARRIVAL:
				 if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
				 {
					 PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
					 usbDrive = FirstDriveFromMask(lpdbv->dbcv_unitmask);
					 usb.Empty();
					 usb.AppendChar(usbDrive);
					 usb = usb + _T(":\\");
					 cmbSurucu.AddString(usb);
					 cmbSurucu.SetCurSel(0);
				 }
		     break;
			 case DBT_DEVICEREMOVECOMPLETE:
				 if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
				 {
					 cmbSurucu.ResetContent();
					 PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
					 
					 usbDrive = FirstDriveFromMask(lpdbv->dbcv_unitmask);
					 usb.AppendChar(usbDrive);
					// cmbSurucu.AddString(usb);
					 CString item;
					 for (int i = 0; i < cmbSurucu.GetCount(); i++) {
						 cmbSurucu.GetLBText(0, item);
						 if (item.Compare(usb) == 0) {
							 cmbSurucu.DeleteString(i);
						 }
						
					 }
					 //cmbSurucu.SetCurSel(0);
				 }
			 break;

				 
			}
	default:
		break;
	}
	return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}
bool CFlashHazirlaDlg::Sifrele(void) 
{
	CString p, pt, p2;
	char ptr[MAX_PATH];
	parola.GetWindowTextW(p);
	parolaTekrar.GetWindowTextW(pt);
	if (p.GetLength() > 0) {
		if (p.Compare(pt) == 0) {
			wcstombs(ptr, p.GetBuffer(MAX_PATH), MAX_PATH);
			strcpy(sifreliMesaj,SezarSifrele(ptr, 8));
			//MessageBoxA(NULL, sifreliMesaj, "en", MB_OK);
			return true;
		}
		else  MessageBoxW(L"Parolalar uyuþmuyor", L"Hata!!!", MB_OK | MB_ICONERROR);
	} else MessageBoxW(L"Parola 6 karakterden az olamaz", L"Hata!!!", MB_OK | MB_ICONERROR);
	return false;
}

void CFlashHazirlaDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	// Open destination file.
	FILE *dosya;
	errno_t err;

	if (Sifrele()) {
		err = fopen_s(&dosya, "ayar.dat", "wb");

		if (err != 0) {
			MessageBoxW(L"Ayar dosyasý oluþturulamadý...", L"Hata!!!", MB_OK | MB_ICONERROR);

		}
		else {/*
			fprintf(dosya, "Windows Registry Editor Version 5.00\n");
			fprintf(dosya, "\n[HKEY_LOCAL_MACHINE\\SOFTWARE\\ET]\n");
			fprintf(dosya, "\"EtKey\"=\"%s\"\n", sifreliMesaj);
			fprintf(dosya, "\n[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ET]\n");
			fprintf(dosya, "\"EtKey\"=\"%s\"\n", sifreliMesaj);*/
			fprintf(dosya, "\\Registry\\Machine\\Software\\ET[1 5 7 11 17]\n");
			fprintf(dosya, "EtKey=%s\n", sifreliMesaj);
			fprintf(dosya, "\\Registry\\Machine\\Software\\WOW6432Node\\ET[1 5 7 11 17]\n");
			fprintf(dosya, "EtKey=%s\n", sifreliMesaj);
			fclose(dosya);
		   // MessageBoxA(NULL, sifreliMesaj, "en", MB_OK);
			//cozulmusMesaj = SezarSifreAc(sifreliMesaj, 8);
			//MessageBoxA(NULL, cozulmusMesaj, "en", MB_OK);
			MessageBoxW(L"Ayar dosyasý baþarýyla oluþturuldu", L"Ýþlem Baþarýlý...", MB_OK | MB_ICONINFORMATION);
		}
	}
	//else  MessageBoxW(L"Parola oluþturma hataslý...", L"Hata!!!", MB_OK | MB_ICONERROR);

}
