/******************************************************************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.
\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>
#include <atlstr.h>
#include <locale.h>
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
			// Use the bitwise AND, 1�"available, 0-not available
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

/*****************************************************************************/
void _cdecl main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Turkish");
	char szSource[]		  = "serial.txt";
	PCHAR szUsbPath       = NULL;
    char szDestination[]  = "  \\anahtar.txt";
    PCHAR szPassword      = NULL;
	errno_t err;
	UINT nDriveNumber = 1;
	CString strSerialNumber;
	FILE *hSifre = NULL;


    if(argc != 3) {
        printf("KULLANIMI: encrypt <Anahtar haz�rlanacak s�r�c�> <�ifre>\n");
        exit(1);
    }

    // Parse arguments.
    szUsbPath       = argv[1];
   // szDestination  = argv[2];
	
    szPassword = argv[2];
	szDestination[0]= argv[1][0];
	szDestination[1] = argv[1][1];
	CString strReport;
	DWORD dwResult = GetPhysicalDriveSerialNumber(szDestination[0],nDriveNumber, strSerialNumber);
	if (NO_ERROR == dwResult)
	{
		printf_s("%s\n", (LPCTSTR)strSerialNumber);

		err = fopen_s(&hSifre, szSource, "wb");
		printf("%s\n", szSource);
		if (err != 0) {
			printf("Error opening Plaintext file!\n");
			goto done;
		}
		fwrite(strSerialNumber, strlen(strSerialNumber),1, hSifre);
		fclose(hSifre);

		if(!CAPIEncryptFile(szSource, szDestination, szPassword)) {
		printf("Error encrypting file!\n");
		exit(1);
		}


		//strReport.Format(_T("Drive #%u serial number: '%s'"), nDriveNumber, strSerialNumber);
	}
	else
	{
		strReport.Format(_T("GetPhysicalDriveSerialNumber failed. Error: %u"), dwResult);
	}
   exit(0);
done :
	if (hSifre) fclose(hSifre);
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
	err=fopen_s(&hSource,szSource,"rb");
    if(err !=0) {
        printf("Error opening Plaintext file!\n");
        goto done;
    }

    // Open destination file.
	err=fopen_s(&hDestination,szDestination,"wb");
	if(err != 0){
        printf("Error opening Ciphertext file!\n");
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

    // Encrypt source file and write to Source file.
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

    status = TRUE;

    printf("OK\n");

    done:

    // Close files.
    if(hSource) fclose(hSource);
    if(hDestination) fclose(hDestination);

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
