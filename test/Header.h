#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>

#define GRANULARITY 64000

SHORT KeyArrayEncode[32] = { 24, 9, 0, 5, 17, 21, 22, -7, 10, 3, 1, 14, -7, 9, -13, 1, -13, -8, -5, -15, -1, -4, -16, -8, -10, -5, -19, -4, 2, 2, -3, -2};
SHORT KeyArrayDecode[32] = {7, 13, 0, 13, 15, 7, 16, 19, -5, 8, -9, -1, -3, 5, 10, 8, -1, 4, -10, 1, 5, -17, -9, 4, -24, -14, -21, 3, -22, 2, -2, -2};

VOID CreateHandleFile(HANDLE* hFile, CHAR* FileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition);
VOID InitOverlapped(LPOVERLAPPED lpover);
VOID EncodeFunction(CHAR* pBufferRead, CHAR* pBufferWrite, DWORD* nNumberBytes);
VOID SetMemory(CHAR*& BufferOne, CHAR*& BufferTwo, CHAR*& BufferThree);
VOID ClearMemory(CHAR*& BufferOne, CHAR*& BufferTwo, CHAR*& BufferThree);
VOID DecodeFunction(CHAR* pBufferRead, CHAR* pBufferWrite, DWORD* nNumberBytes);
INT decode(CHAR* encode_file, CHAR* decode_file);
INT encode(CHAR* main_file, CHAR* encode_file);
