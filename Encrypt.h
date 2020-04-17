
// your header code here
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>
#include <atlstr.h>
#include <locale.h>
#include <string>
#include <iostream>




#include <Setupapi.h>
#include <winioctl.h>
#include <cfgmgr32.h>

using namespace std;
#ifdef USE_BLOCK_CIPHER
// defines for RC2 block cipher
#define ENCRYPT_ALGORITHM   CALG_RC2
#define ENCRYPT_BLOCK_SIZE  8
#else
// defines for RC4 stream cipher
#define ENCRYPT_ALGORITHM   CALG_RC4
#define ENCRYPT_BLOCK_SIZE  1
#endif

// This sample uses an explicit key length instead of a default key length
// so that it can be used between different relases of the OS that have different
// default key lengths. Key length here is set to 128 bits.

#define KEYLENGTH 0x00800000
static BOOL CAPIEncryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword);
void GetUsbDrivers();
DWORD GetPhysicalDriveSerialNumber(char usbDrive, UINT nDriveNumber, CString& strSerialNumber);
//void _cdecl main(int argc, char *argv[]);
bool FlashHazirla(int argc,char *argv[]);
static BOOL CAPIEncryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword);
char *encryptDecrypt(char *toEncrypt);
char *SezarSifrele(char *mesaj, int anahtar);
char *SezarSifreAc(char *mesaj, int anahtar);
DEVINST GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType, char* szDosDeviceName);
