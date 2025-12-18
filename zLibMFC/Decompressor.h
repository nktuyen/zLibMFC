#pragma once

#include "afxwin.h"

#define DECOMPRESS_ERROR_SUCCESS					0
#define DECOMPRESS_ERROR_ABORTED						1
#define DECOMPRESS_ERROR_INPUT_FILE_NOT_FOUND		2
#define DECOMPRESS_ERROR_INPUT_FILE_CANNOT_OPEN		3
#define DECOMPRESS_ERROR_OUTPUT_FILENOT_SPECIFIED	4
#define DECOMPRESS_ERROR_OUTPUT_FILE_EXIST			5
#define DECOMPRESS_ERROR_OUTPUT_FILE_CANNOT_CREATE	6
#define DECOMPRESS_ERROR_OUTPUT_FILE_CANNOT_WRITE	7
#define DECOMPRESS_ERROR_ZLIB_BASE					99

class CDecompressor : public CWinThread
{
public:
	CDecompressor(const CString& strInputFile, const CString& strOutputFile, UINT nMaxBufferSize = 1024*1024, BOOL bOverwriteOutputFile = FALSE, CWnd* pOwnerWnd = nullptr);
	virtual ~CDecompressor(void);
	void Stop();
	BOOL IsRunning();
public:
	inline void SetBufferSize(int nSize) { m_nBufferSize = nSize; }
	inline void SetOverwriteOutputFile(BOOL bOverwrite) { m_bOverwriteOutputFileIfExist = bOverwrite; }
private:
	BOOL InitInstance() override;
	int ExitInstance() override;
	int Run() override;
private:
	BOOL m_bRunning;
	CMutex m_runMutex;
	CString m_strInputFile;
	CString m_strOutputFile;
	UINT m_nBufferSize;
	CWnd* m_pOwnerWnd;
	int m_nExitCode;
	unsigned char* m_pBufferIn;
	unsigned char* m_pBufferOut;
	BOOL m_bOverwriteOutputFileIfExist;
};

