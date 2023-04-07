#include "Header.h"

INT encode(CHAR* main_file, CHAR* encode_file)
{
	BOOL       bReadFile, bWriteFile;
	CHAR       *BufferReadOne, *BufferWrite, *BufferReadTwo,
			   *lpOneBuffer, *lpTwoBuffer, *lpTempBuffer;
	DWORD      nNumberReadBytes, nNumberWriteFile = 1;	
	HANDLE     hMainFile, hEncodeFile, hEvent;
	DWORD64    SumOfBytesOffsetRead = 0, SumOfBytesOffsetWrite = 0;
	OVERLAPPED lpoverRead, lpoverWrite;

	SetMemory(BufferReadOne, BufferWrite, BufferReadTwo);

	CreateHandleFile(&hMainFile, main_file, GENERIC_READ, OPEN_EXISTING);
	CreateHandleFile(&hEncodeFile, encode_file, GENERIC_READ | GENERIC_WRITE, CREATE_ALWAYS);

	if (hMainFile == INVALID_HANDLE_VALUE || hEncodeFile == INVALID_HANDLE_VALUE)
	{
		printf("Error puth file! GetLastError - %d", GetLastError());
		return 0;
	}

	lpOneBuffer = BufferReadOne;
	lpTwoBuffer = BufferReadTwo;

	InitOverlapped(&lpoverRead);
	InitOverlapped(&lpoverWrite);


	bReadFile = ReadFile(hMainFile, lpOneBuffer, GRANULARITY, NULL, &lpoverRead);
	
	while (TRUE)
	{
		GetOverlappedResult(hMainFile, &lpoverRead, &nNumberReadBytes, TRUE);
		SumOfBytesOffsetRead += nNumberReadBytes;

		lpoverRead.OffsetHigh = SumOfBytesOffsetRead >> 32;
		lpoverRead.Offset     = SumOfBytesOffsetRead & 0xFFFFFFFF;
		
		bReadFile = ReadFile(hMainFile, lpTwoBuffer, GRANULARITY, NULL, &lpoverRead);
		EncodeFunction(lpOneBuffer, BufferWrite, &nNumberReadBytes);
		bWriteFile = WriteFile(hEncodeFile, BufferWrite, nNumberReadBytes, NULL, &lpoverWrite);

		GetOverlappedResult(hEncodeFile, &lpoverWrite, &nNumberWriteFile, TRUE);
		SumOfBytesOffsetWrite += nNumberWriteFile;

		if (nNumberWriteFile == 0)
			break;

		lpTempBuffer = lpOneBuffer;
		lpOneBuffer  = lpTwoBuffer;
		lpTwoBuffer  = lpTempBuffer;
		
		lpoverWrite.Offset     = SumOfBytesOffsetWrite & 0xFFFFFFFF;
		lpoverWrite.OffsetHigh = SumOfBytesOffsetWrite >> 32;
	};

	ClearMemory(BufferReadOne, BufferWrite, BufferReadTwo);
	CloseHandle(hMainFile);
	CloseHandle(hEncodeFile);

	return 0;
}

VOID EncodeFunction(CHAR* pBufferRead, CHAR* pBufferWrite, DWORD* nNumberBytes)
{
	INT  RemainderOfDivision = 4 - (*nNumberBytes % 4);

	if (*nNumberBytes % 4 != 0)
	{
		INT	 TempNumberBytes = *nNumberBytes;
		CHAR TempSymbol[2] = { 0 };

		sprintf(TempSymbol, "%d", RemainderOfDivision);
		*nNumberBytes += RemainderOfDivision;

		for (int i = TempNumberBytes; i < *nNumberBytes; i++)
			pBufferRead[i] = ' ';

		pBufferWrite[*nNumberBytes] = TempSymbol[0];

	}
	int offsetE = 0;
	for (int NumberPosMask = 0, j = 0; j < *nNumberBytes; j += 4)
	{
		char tempSym;
		char tempArr[4] =  { 0 };
		tempArr[0] = pBufferRead[j];
		tempArr[1] = pBufferRead[j + 1];
		tempArr[2] = pBufferRead[j + 2];
		tempArr[3] = pBufferRead[j + 3];
		char ArrBits[4] = { 0 };
		for(int i = 0; i < 4; ++i, offsetE++){
			for(int k = 0; k < 8; ++k){
				if (KeyArrayEncode[NumberPosMask] + NumberPosMask <= 7) {
					tempSym = tempArr[0] >> (7 - (KeyArrayEncode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				else if (KeyArrayEncode[NumberPosMask] + NumberPosMask >= 8 && KeyArrayEncode[NumberPosMask] + NumberPosMask <= 15) {
					tempSym = tempArr[1] >> (7 - (KeyArrayEncode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				else if (KeyArrayEncode[NumberPosMask] + NumberPosMask >= 16 && KeyArrayEncode[NumberPosMask] + NumberPosMask <= 23) {
					tempSym = tempArr[2] >> (7 - (KeyArrayEncode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				else {
					tempSym = tempArr[3] >> (7 - (KeyArrayEncode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				NumberPosMask++;
				if (NumberPosMask == 32)
					NumberPosMask = 0;
			}
			pBufferWrite[offsetE] = ArrBits[i];
		}
	}
	if (RemainderOfDivision % 4 != 0)
	{
		*nNumberBytes += 1;
	}
}

VOID SetMemory(CHAR*& BufferOne, CHAR*& BufferTwo, CHAR*& BufferThree)
{
	BufferOne   = (CHAR*)calloc((GRANULARITY + 1), sizeof(CHAR));
	BufferTwo   = (CHAR*)calloc((GRANULARITY + 1), sizeof(CHAR));
	BufferThree = (CHAR*)calloc((GRANULARITY + 1), sizeof(CHAR));
}

VOID ClearMemory(CHAR*& BufferOne, CHAR*& BufferTwo, CHAR*& BufferThree)
{
	free(BufferOne);
	free(BufferTwo);
	free(BufferThree);
}

VOID CreateHandleFile(HANDLE* hFile, CHAR* FileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition)
{
	*hFile = CreateFileA(
		FileName,
		dwDesiredAccess,
		FILE_SHARE_READ,
		NULL,
		dwCreationDisposition,
		FILE_FLAG_OVERLAPPED,
		NULL
	);
}

VOID InitOverlapped(LPOVERLAPPED lpover)
{
	HANDLE hEvent = CreateEventA(NULL, FALSE, TRUE, NULL);

	ZeroMemory(lpover, sizeof(OVERLAPPED));

	lpover->hEvent     = hEvent;
	lpover->OffsetHigh = 0;
	lpover->Offset     = 0;
}

INT decode(CHAR* encode_file, CHAR* decode_file)
{
	BOOL       bReadFile, bWriteFile;
	CHAR       * BufferReadOne, * BufferWrite, * BufferReadTwo,
		       * lpOneBuffer, * lpTwoBuffer, * lpTempBuffer;
	DWORD      nNumberReadBytes, nNumberWriteFile = 1;
	HANDLE     hEncodeFile, hDecodeFile, hEvent;
	DWORD64    SumOfBytesOffsetRead = 0, SumOfBytesOffsetWrite = 0, SizeFile;
	OVERLAPPED lpoverRead, lpoverWrite;

	SetMemory(BufferReadOne, BufferWrite, BufferReadTwo);

	CreateHandleFile(&hEncodeFile, encode_file, GENERIC_READ, OPEN_EXISTING);
	CreateHandleFile(&hDecodeFile, decode_file, GENERIC_READ | GENERIC_WRITE, CREATE_ALWAYS);

	SizeFile = GetFileSize(hEncodeFile, NULL);

	if (hEncodeFile == INVALID_HANDLE_VALUE || hDecodeFile == INVALID_HANDLE_VALUE)
	{
		printf("Error puth file! GetLastError - %d", GetLastError());
		return 0;
	}

	lpOneBuffer = BufferReadOne;
	lpTwoBuffer = BufferReadTwo;

	InitOverlapped(&lpoverRead);
	InitOverlapped(&lpoverWrite);


	bReadFile = ReadFile(hEncodeFile, lpOneBuffer, GRANULARITY, NULL, &lpoverRead);

	while (TRUE)
	{
		GetOverlappedResult(hEncodeFile, &lpoverRead, &nNumberReadBytes, TRUE);
		SumOfBytesOffsetRead += nNumberReadBytes;

		lpoverRead.OffsetHigh = SumOfBytesOffsetRead >> 32;
		lpoverRead.Offset = SumOfBytesOffsetRead & 0xFFFFFFFF;

		bReadFile = ReadFile(hEncodeFile, lpTwoBuffer, GRANULARITY, NULL, &lpoverRead);
		DecodeFunction(lpOneBuffer, BufferWrite, &nNumberReadBytes);
		bWriteFile = WriteFile(hDecodeFile, BufferWrite, nNumberReadBytes, NULL, &lpoverWrite);

		GetOverlappedResult(hDecodeFile, &lpoverWrite, &nNumberWriteFile, TRUE);
		SumOfBytesOffsetWrite += nNumberWriteFile;

		if (nNumberWriteFile == 0)
			break;

		lpTempBuffer = lpOneBuffer;
		lpOneBuffer = lpTwoBuffer;
		lpTwoBuffer = lpTempBuffer;

		lpoverWrite.Offset = SumOfBytesOffsetWrite & 0xFFFFFFFF;
		lpoverWrite.OffsetHigh = SumOfBytesOffsetWrite >> 32;
	};

	ClearMemory(BufferReadOne, BufferWrite, BufferReadTwo);
	CloseHandle(hEncodeFile);
	CloseHandle(hDecodeFile);

	return 0;
}

VOID DecodeFunction(CHAR* pBufferRead, CHAR* pBufferWrite, DWORD* nNumberBytes)
{
	if (*nNumberBytes % 4 != 0)
	{
		INT  TempnNumberBytes = *nNumberBytes;
		CHAR CheckSpace = pBufferRead[TempnNumberBytes - 1];
		      
		INT DeleteBytes = atoi(&CheckSpace) + 1;
		
		*nNumberBytes -= DeleteBytes;
	}
	int offsetD = 0;
	for (int NumberPosMask = 0, j = 0; j < *nNumberBytes; j += 4)
	{
		char tempSym;
		char tempArr[4] = { 0 };
		tempArr[0] = pBufferRead[j];
		tempArr[1] = pBufferRead[j + 1];
		tempArr[2] = pBufferRead[j + 2];
		tempArr[3] = pBufferRead[j + 3];
		char ArrBits[4] = { 0 };
		for(int i = 0; i < 4; ++i, offsetD++){
			for(int k = 0; k < 8; ++k){
				if (KeyArrayDecode[NumberPosMask] + NumberPosMask <= 7) {
					tempSym = tempArr[0] >> (7 - (KeyArrayDecode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				else if (KeyArrayDecode[NumberPosMask] + NumberPosMask >= 8 && KeyArrayDecode[NumberPosMask] + NumberPosMask <= 15) {
					tempSym = tempArr[1] >> (7 - (KeyArrayDecode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				else if (KeyArrayDecode[NumberPosMask] + NumberPosMask >= 16 && KeyArrayDecode[NumberPosMask] + NumberPosMask < 24) {
					tempSym = tempArr[2] >> (7 - (KeyArrayDecode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				else {
					tempSym = tempArr[3] >> (7 - (KeyArrayDecode[NumberPosMask] + NumberPosMask) % 8) & 1;
					tempSym = tempSym << (7 - k);
					ArrBits[i] = ArrBits[i] | tempSym;
				}
				NumberPosMask++;
				if (NumberPosMask == 32)
					NumberPosMask = 0;
			}
			pBufferWrite[offsetD] = ArrBits[i];
		}
	}
}

INT main()
{

	CHAR puth1[100] = "D:\\PSU\\3 course\\2\\MZKS\\Etest.txt";
	CHAR puth2[100] = "D:\\PSU\\3 course\\2\\MZKS\\Dtest.txt";
	CHAR puth3[100] = "D:\\6 семестр\\МетодызащитыКС\\Тест\\OriginalFile.txt";
	CHAR puth4[100] = "D:\\PSU\\3 course\\2\\MZKS\\test.txt";
	CHAR puth5[100] = "D:\\6 семестр\\МетодызащитыКС\\Тест\\e02-1.txt";
	CHAR puth6[100] = "D:\\6 семестр\\МетодызащитыКС\\Тест\\lowbyte.txt";

	encode(puth4, puth1);
	decode(puth1, puth2);

	return 0;
}

//INT main(INT argc, CHAR* argv[])
//{
//	if (argv[1][0] == 'e')
//		encode(argv[2], argv[3]);
//	else if (argv[1][0] == 'd')
//		decode(argv[2]);
//	else
//		printf("Invalid_File_Name!");
//	return 0;
//}
