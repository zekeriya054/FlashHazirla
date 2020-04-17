/******************************************************************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.
\******************************************************************************/
#include "Encrypt.h"
#include<stdio.h>
void GetUsbDrivers() {
	TCHAR szDrive[] = _T("A:");
	DWORD uDriveMask = GetLogicalDrives();
	if (uDriveMask == 0)

		printf("GetLogicalDrives() failed with failure code: %x\n", GetLastError());


	else

	{

		printf("This machine has the following logical drives:\n");

		while (uDriveMask)

		{
			// Use the bitwise AND, 1â€"available, 0-not available
			//printf("%d\n",uDriveMask);
			if (uDriveMask & 1)
				if (DRIVE_REMOVABLE == GetDriveType(szDrive)) {
					printf("%s ", (const char *)szDrive);
				}

			// increment, check next drive

			++szDrive[0];

			// shift the bitmask binary right

			uDriveMask >>= 1;

		}

	}
}


DEVINST GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType, char* szDosDeviceName,char* szDeviceId)
{
	bool IsFloppy = (strstr(szDosDeviceName, "\\Floppy") != NULL); // who knows a better way?

	GUID* guid;

	switch (DriveType) {
	case DRIVE_REMOVABLE:
		if (IsFloppy) {
			guid = (GUID*)&GUID_DEVINTERFACE_FLOPPY;
		}
		else {
			guid = (GUID*)&GUID_DEVINTERFACE_DISK;
		}
		break;
	case DRIVE_FIXED:
		guid = (GUID*)&GUID_DEVINTERFACE_DISK;
		break;
	case DRIVE_CDROM:
		guid = (GUID*)&GUID_DEVINTERFACE_CDROM;
		break;
	default:
		return 0;
	}

	// Get device interface info set handle for all devices attached to system
	HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE) {
		return 0;
	}

	// Retrieve a context structure for a device interface of a device information set
	DWORD dwIndex = 0;
	long res;

	BYTE Buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;
	SP_DEVICE_INTERFACE_DATA         spdid;
	SP_DEVINFO_DATA                  spdd;
	DWORD                            dwSize;

	spdid.cbSize = sizeof(spdid);

	while (true) {
		res = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &spdid);
		if (!res) {
			break;
		}

		dwSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL); // check the buffer size

		if (dwSize != 0 && dwSize <= sizeof(Buf)) {

			pspdidd->cbSize = sizeof(*pspdidd); // 5 Bytes!

			ZeroMemory(&spdd, sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
			if (res) {

				// in case you are interested in the USB serial number:
				// the device id string contains the serial number if the device has one,
				// otherwise a generated id that contains the '&' char...

				DEVINST DevInstParent = 0;
				CM_Get_Parent(&DevInstParent, spdd.DevInst, 0);
				char szDeviceIdString[MAX_PATH];
				CM_Get_Device_ID(DevInstParent, szDeviceIdString, MAX_PATH, 0);



				// open the disk or cdrom or floppy
				HANDLE hDrive = CreateFile(pspdidd->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				if (hDrive != INVALID_HANDLE_VALUE) {
					// get its device number
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
					if (res) {
						if (DeviceNumber == (long)sdn.DeviceNumber) {  // match the given device number with the one of the current device
							//MessageBoxA(NULL, szDeviceIdString, "khkj", MB_OK);
							strcpy(szDeviceId,szDeviceIdString);
							//getch();
							CloseHandle(hDrive);
							SetupDiDestroyDeviceInfoList(hDevInfo);
							return spdd.DevInst;
						}
					}
					CloseHandle(hDrive);
				}
			}
		}
		dwIndex++;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}



DWORD GetPhysicalDriveSerialNumber(char usbDrive,UINT nDriveNumber, CString& strSerialNumber)
{
	DWORD dwResult = NO_ERROR;
	strSerialNumber.Empty();

	// Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
	CString strDrivePath;
	strDrivePath.Format(_T("\\\\.\\%c:"), usbDrive);
	// call CreateFile to get a handle to physical drive
	HANDLE hDevice = ::CreateFile(strDrivePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == hDevice)
		return ::GetLastError();

	// set the input STORAGE_PROPERTY_QUERY data structure
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	// get the necessary output buffer size
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
	DWORD dwBytesReturned = 0;
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		::CloseHandle(hDevice);
		return dwResult;
	}

	// allocate the necessary memory for the output buffer
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
	ZeroMemory(pOutBuffer, dwOutBufferSize);

	// get the storage device descriptor
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer, dwOutBufferSize,
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		delete[]pOutBuffer;
		::CloseHandle(hDevice);
		return dwResult;
	}

	// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
	// followed by additional info like vendor ID, product ID, serial number, and so on.
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset != 0)
	{
		// finally, get the serial number
		strSerialNumber = CString(pOutBuffer + dwSerialNumberOffset);
	}

	// perform cleanup and return
	delete[]pOutBuffer;
	::CloseHandle(hDevice);
	return dwResult;
}
/* Xor*/
char * encryptDecrypt(char *toEncrypt) {
	char *output = toEncrypt;
	char xPas[] = "BADE";
	for (int i = 0; i < strlen(toEncrypt); i++)
		output[i] = toEncrypt[i] ^ xPas[i%strlen(xPas)];

	return output;
}
/* sezar þifre*/
char * SezarSifrele(char *mesaj,int anahtar)
{
	//char mesaj[100], ch;
	unsigned char ch;
	int i;
	for (i = 0; mesaj[i] != '\0'; ++i) {
		ch = mesaj[i];

		if (ch >= 'a' && ch <= 'z') {
			ch = ch + anahtar;

			if (ch > 'z') {
				ch = ch - 'z' + 'a' - 1;

			}

			mesaj[i] = ch;
		}

		else if (ch >= 'A' && ch <= 'Z') {
			ch = ch + anahtar;

			if (ch > 'Z') {
				ch = ch - 'Z' + 'A' - 1;
			}
			mesaj[i] = ch;
		}
		else if (ch >= '0' && ch <= '9') {
			ch = ch + anahtar;
			if (ch > '9') {
				ch = ch - '9' + '0' - 1;
			}
			mesaj[i] = ch;
		}
	}
	char *encrypted = encryptDecrypt(mesaj);
	return encrypted;
}

/* sezar þifre aç*/
char *SezarSifreAc(char *mesaj,int anahtar)
{
	unsigned char ch;
	int i;
	char *decrypted = encryptDecrypt(mesaj);
	//char *decrypted = mesaj;
	for (i = 0; decrypted[i] != '\0'; ++i)
	{
		ch = decrypted[i];

		if (ch >= 'a' && ch <= 'z')
		{
			ch = ch - anahtar;

			if (ch < 'a')
			{
				ch = ch + 'z' - 'a' + 1;
			}
			decrypted[i] = ch;
		}

		else if (ch >= 'A' && ch <= 'Z')
		{
			ch = ch - anahtar;

			if (ch > 'a')
			{
				ch = ch + 'Z' - 'A' + 1;
			}
			decrypted[i] = ch;
		}
		else if (ch >= '0' && ch <= '9')
		{
			ch = ch - anahtar;
			if (ch < '0')
			{
				ch = ch + '9' - '0' + 1;
			}
			decrypted[i] = ch;
		}
	}

	return decrypted;
}
/*****************************************************************************/
bool FlashHazirla(int argc, char *argv[])
{
	setlocale(LC_ALL, "Turkish");
	char szSource[]		  = "serial.txt";
	PCHAR szUsbPath       = NULL;
    char szDestination[]  = "  \\anahtar.txt";
	PCHAR szPassword = NULL;
	errno_t err;
	UINT nDriveNumber = 1;
	CString strSerialNumber;
	FILE *hSifre = NULL;
	bool cevap = true;

    if(argc != 3) {
        printf("KULLANIMI: encrypt <Anahtar hazýrlanacak sürücü> <þifre>\n");
        exit(1);
    }

    // Parse arguments.
    szUsbPath       = argv[1];
   // szDestination  = argv[2];
	
    szPassword = argv[2];
	szDestination[0]= argv[1][0];
	szDestination[1] = argv[1][1];
	CString strReport;
	char usbSerial[500];



	char DriveLetter =argv[1][0];
	DriveLetter &= ~0x20; // uppercase

	if (DriveLetter < 'A' || DriveLetter > 'Z') {
		return 1;
	}

	char szRootPath[] = "X:\\";   // "X:\"  -> for GetDriveType
	szRootPath[0] = DriveLetter;

	char szDevicePath[] = "X:";   // "X:"   -> for QueryDosDevice
	szDevicePath[0] = DriveLetter;

	char szVolumeAccessPath[] = "\\\\.\\X:";   // "\\.\X:"  -> to open the volume
	szVolumeAccessPath[4] = DriveLetter;

	long DeviceNumber = -1;

	// open the storage volume
	HANDLE hVolume = CreateFile(szVolumeAccessPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (hVolume == INVALID_HANDLE_VALUE) {
		return 1;
	}

	// get the volume's device number
	STORAGE_DEVICE_NUMBER sdn;
	DWORD dwBytesReturned = 0;
	long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
	if (res) {
		DeviceNumber = sdn.DeviceNumber;
	}
	CloseHandle(hVolume);

	if (DeviceNumber == -1) {
		return 1;
	}

	// get the drive type which is required to match the device numbers correctely
	UINT DriveType = GetDriveType(szRootPath);

	// get the dos device name (like \device\floppy0) to decide if it's a floppy or not - who knows a better way?
	char szDosDeviceName[MAX_PATH];
	res = QueryDosDevice(szDevicePath, szDosDeviceName, MAX_PATH);
	if (!res) {
		return 1;
	}
	char deviceID[MAX_PATH];
	//TCHAR buffer[MAX_PATH];
	// get the device instance handle of the storage volume by means of a SetupDi enum and matching the device number
	DEVINST DevInst = GetDrivesDevInstByDeviceNumber(DeviceNumber, DriveType, szDosDeviceName,deviceID);

	//MessageBoxA(NULL, deviceID, "kjk", MB_OK);
	if (DevInst == 0) {
		return 1;
	}
	else {
		if (!CAPIEncryptFile(deviceID, szDestination, szPassword)) {
			printf("Error encrypting file!\n");
			cevap = false;
			//exit(1);
		}
	}


	/*
	DWORD dwResult = GetPhysicalDriveSerialNumber(szDestination[0],nDriveNumber, strSerialNumber);
	
	if (NO_ERROR == dwResult)
	{
		
		
		strcpy(usbSerial, (LPCTSTR)strSerialNumber);
		if(!CAPIEncryptFile(usbSerial, szDestination, szPassword)) {
		printf("Error encrypting file!\n");
		cevap = false;
		//exit(1);
		}

		//strReport.Format(_T("Drive #%u serial number: '%s'"), nDriveNumber, strSerialNumber);
	}
	else
	{
		strReport.Format(_T("GetPhysicalDriveSerialNumber failed. Error: %u"), dwResult);

		return false;
	}
	*/
   //exit(0);
	
done :
	if (hSifre) fclose(hSifre);

	return cevap;
}

/*****************************************************************************/
static BOOL CAPIEncryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword)
{
    FILE *hSource      = NULL;
    FILE *hDestination = NULL;
	errno_t err;
    INT eof = 0;

    HCRYPTPROV hProv   = 0;
    HCRYPTKEY hKey     = 0;
    HCRYPTKEY hXchgKey = 0;
    HCRYPTHASH hHash   = 0;

    PBYTE pbKeyBlob = NULL;
    DWORD dwKeyBlobLen;

    PBYTE pbBuffer = NULL;
    DWORD dwBlockLen;
    DWORD dwBufferLen;
    DWORD dwCount;

    BOOL status = FALSE;

    // Open source file.
	/*
	err=fopen_s(&hSource,szSource,"rb");
    if(err !=0) {
        printf("Error opening Plaintext file!\n");
        goto done;
    }*/
	DWORD attr = GetFileAttributes(szDestination);
	SetFileAttributes(szDestination, attr - FILE_ATTRIBUTE_HIDDEN);
    // Open destination file.
	err=fopen_s(&hDestination,szDestination,"wb");
	if(err != 0){
        printf("Error opening Ciphertext file!\n");
		MessageBoxA(NULL, "Dosya okuma hatasý!!!", "Hata", MB_OK|MB_ICONERROR);
        goto done;
    }

    // Get handle to the CSP. In order to be used with different OSs 
	// with different default provides, the CSP is explicitly set. 
	// If the Microsoft Enhanced Provider is not installed, set parameter
	// three to MS_DEF_PROV 
    
	if(!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        printf("Error %x during CryptAcquireContext!\n", GetLastError());
        goto done;
    }

    if(szPassword == NULL) {
        // Encrypt the file with a random session key.

        // Create a random session key.
        if(!CryptGenKey(hProv, ENCRYPT_ALGORITHM, KEYLENGTH | CRYPT_EXPORTABLE, &hKey)) {
            printf("Error %x during CryptGenKey!\n", GetLastError());
            goto done;
        }

        // Get handle to key exchange public key.
        if(!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hXchgKey)) {
            printf("Error %x during CryptGetUserKey!\n", GetLastError());
            goto done;
        }

        // Determine size of the key blob and allocate memory.
        if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, NULL, &dwKeyBlobLen)) {
            printf("Error %x computing blob length!\n", GetLastError());
            goto done;
        }
        if((pbKeyBlob = (unsigned char *) malloc(dwKeyBlobLen)) == NULL) {
            printf("Out of memory!\n");
            goto done;
        }

        // Export session key into a simple key blob.
        if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, pbKeyBlob, &dwKeyBlobLen)) {
            printf("Error %x during CryptExportKey!\n", GetLastError());
            goto done;
        }

        // Release key exchange key handle.
        CryptDestroyKey(hXchgKey);
        hXchgKey = 0;

        // Write size of key blob to destination file.
        fwrite(&dwKeyBlobLen, sizeof(DWORD), 1, hDestination);
        if(ferror(hDestination)) {
            printf("Error writing header!\n");
            goto done;
        }

        // Write key blob to destination file.
        fwrite(pbKeyBlob, 1, dwKeyBlobLen, hDestination);
        if(ferror(hDestination)) {
            printf("Error writing header!\n");
            goto done;
        }

    } else {
        // Encrypt the file with a session key derived from a password.

        // Create a hash object.
        if(!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
            printf("Error %x during CryptCreateHash!\n", GetLastError());
            goto done;
        }

        // Hash in the password data.
        if(!CryptHashData(hHash, (const unsigned char *)szPassword, (DWORD)strlen(szPassword), 0)) {
            printf("Error %x during CryptHashData!\n", GetLastError());
            goto done;
        }

        // Derive a session key from the hash object.
        if(!CryptDeriveKey(hProv, ENCRYPT_ALGORITHM, hHash, KEYLENGTH, &hKey)) {
            printf("Error %x during CryptDeriveKey!\n", GetLastError());
            goto done;
        }

        // Destroy the hash object.
        CryptDestroyHash(hHash);
        hHash = 0;
    }

    // Determine number of bytes to encrypt at a time. This must be a multiple
    // of ENCRYPT_BLOCK_SIZE.
    dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;

    // Determine the block size. If a block cipher is used this must have
    // room for an extra block.
#ifdef USE_BLOCK_CIPHER
        dwBufferLen = dwBlockLen + ENCRYPT_BLOCK_SIZE;
#else
        dwBufferLen = dwBlockLen;
#endif

    // Allocate memory.
    if((pbBuffer = (unsigned char *) malloc(dwBufferLen)) == NULL) {
        printf("Out of memory!\n");
        goto done;
    }
	//MessageBox(NULL, szSource, "lklkl", MB_OK);
	memcpy((byte *) pbBuffer, (char *)szSource,dwBufferLen);
	dwCount = strlen((char *)szSource);
	if (!CryptEncrypt(hKey, 0, eof, 0, pbBuffer, &dwCount, dwBufferLen)) {
		//MessageBox(NULL, (char *)pbBuffer, "lklkl", MB_OK);
		printf("bytes required:%d\n", dwCount);
		printf("Error %x during CryptEncrypt!\n", GetLastError());
		goto done;
		
	}
	fwrite(pbBuffer, 1, dwCount, hDestination);
	if (ferror(hDestination)) {
		printf("Error writing Ciphertext!\n");
		goto done;
	}

    // Encrypt source file and write to Source file.
/*
    do {
        // Read up to 'dwBlockLen' bytes from source file.
        dwCount = (DWORD)fread(pbBuffer, 1, dwBlockLen, hSource);
        if(ferror(hSource)) {
            printf("Error reading Plaintext!\n");
            goto done;
        }
        eof = feof(hSource);

        // Encrypt data
        if(!CryptEncrypt(hKey, 0, eof, 0, pbBuffer, &dwCount, dwBufferLen)) {
            printf("bytes required:%d\n",dwCount);
            printf("Error %x during CryptEncrypt!\n", GetLastError());
            goto done;
        }

        // Write data to destination file.
        fwrite(pbBuffer, 1, dwCount, hDestination);
        if(ferror(hDestination)) {
            printf("Error writing Ciphertext!\n");
            goto done;
        }
    } while(!feof(hSource));
	*/
    status = TRUE;

    //printf("OK\n");

    done:

    // Close files.
   // if(hSource) fclose(hSource);
    if(hDestination) fclose(hDestination);
	attr = GetFileAttributes(szDestination);
	SetFileAttributes(szDestination, attr + FILE_ATTRIBUTE_HIDDEN);
    // Free memory.
    if(pbKeyBlob) free(pbKeyBlob);
    if(pbBuffer) free(pbBuffer);

    // Destroy session key.
    if(hKey) CryptDestroyKey(hKey);

    // Release key exchange key handle.
    if(hXchgKey) CryptDestroyKey(hXchgKey);

    // Destroy hash object.
    if(hHash) CryptDestroyHash(hHash);

    // Release provider handle.
    if(hProv) CryptReleaseContext(hProv, 0);

    return(status);
}
