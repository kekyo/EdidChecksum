////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Edid checksum - Calculate and fix (in-place) EDID database checksum.
// Copyright (c) Kouji Matsui, All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

class File
{
private:
	HANDLE hFile_;

	File(const File &);
	void operator =(const File &);

public:
	File(HANDLE hFile) throw()
		: hFile_(hFile)
	{
	}

	~File() throw()
	{
		if (hFile_ != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile_);
		}
	}

	bool IsOpened() const throw()
	{
		return hFile_ != INVALID_HANDLE_VALUE;
	}

	operator HANDLE() const throw()
	{
		return const_cast<HANDLE>(hFile_);
	}
};

class View
{
private:
	unsigned char *pMap_;

	View(const View &);
	void operator =(const View &);

public:
	View(LPVOID pMap) throw()
		: pMap_(static_cast<unsigned char *>(pMap))
	{
	}

	~View() throw()
	{
		if (pMap_ != 0)
		{
			UnmapViewOfFile(pMap_);
		}
	}

	bool IsViewed() const throw()
	{
		return pMap_ != 0;
	}

	const unsigned char &operator [](const DWORD index) const throw()
	{
		return pMap_[index];
	}

	unsigned char &operator [](const DWORD index) throw()
	{
		return pMap_[index];
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc <= 1)
	{
		_tprintf(_T("Edid checksum - Calculate and fix (in-place) EDID database checksum.\n"));
		_tprintf(_T("Copyright (c) 2014 Kouji Matsui, All rights reserved.\n"));
		_tprintf(_T("usage: EdidChecksum <edid bin file>\n"));
		return 0;
	}

	const TCHAR *pFileName = argv[1];
	File file = CreateFile(pFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (file.IsOpened() == false)
	{
		_tprintf(_T("EdidChecksum: Cannot open file %s\n"), pFileName);
		return 1;
	}

	DWORD high = 0;
	const DWORD length = GetFileSize(file, &high);
	if (((length != 128) && (length != 126)) || (high > 0))
	{
		_tprintf(_T("EdidChecksum: Invalid file size\n"));
		return 2;
	}

	File fileMapping = CreateFileMapping(file, 0, PAGE_READWRITE, 0, 128, 0);
	if (fileMapping.IsOpened() == false)
	{
		_tprintf(_T("EdidChecksum: Cannot map file\n"));
		return 3;
	}

	View view = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 128);
	if (view.IsViewed() == false)
	{
		_tprintf(_T("EdidChecksum: Cannot view file\n"));
		return 4;
	}

	DWORD sum = 0;
	for (DWORD index = 0; index <= 126; index++)
	{
		sum += view[index];
	}

	const unsigned char checkSum = static_cast<unsigned char>(256 - (sum & 0xff));

	_tprintf(_T("EdidChecksum: Computed, 0x%02x --> 0x%02x\n"), view[127], checkSum);

	view[127] = checkSum;

	return 0;
}

