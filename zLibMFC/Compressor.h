#pragma once

#include "afxwin.h"

#define COMPRESS_ERROR_SUCCESS						0
#define COMPRESS_ERROR_ABORTED						1
#define COMPRESS_ERROR_INPUT_FILE_NOT_FOUND			2
#define COMPRESS_ERROR_INPUT_FILE_CANNOT_OPEN		3
#define COMPRESS_ERROR_OUTPUT_FILENOT_SPECIFIED		4
#define COMPRESS_ERROR_OUTPUT_FILE_EXIST			5
#define COMPRESS_ERROR_OUTPUT_FILE_CANNOT_CREATE	6
#define COMPRESS_ERROR_INPUT_FILE_CANNOT_READ		7
#define COMPRESS_ERROR_OUTPUT_FILE_CANNOT_WRITE		8
#define COMPRESS_ERROR_ZLIB_BASE					99

class CCompressor : public CWinThread
{
public:
	CCompressor(const CString& strInputFile, const CString& strOutputFile, UINT nMaxBufferSize = 1024*1024, int nCompressLevel = 1, BOOL bOverwriteOutputFile = FALSE, CWnd* pOwnerWnd = nullptr);
	virtual ~CCompressor(void);
	void Stop();
	BOOL IsRunning();
public:
	inline void SetBufferSize(int nSize) { m_nBufferSize = nSize; }
	inline void SetOverwriteOutputFile(BOOL bOverwrite) { m_bOverwriteOutputFileIfExist = bOverwrite; }
	inline void SetCompressLevel(int nLevel) { m_nCompressLevel = nLevel; }
private:
	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;
	virtual int Run() override;
private:
	BOOL m_bRunning;
	CMutex m_runMutex;
	CString m_strInputFile;
	CString m_strOutputFile;
	BOOL m_bOverwriteOutputFileIfExist;
	UINT m_nBufferSize;
	CWnd* m_pOwnerWnd;
	int m_nExitCode;
	unsigned char* m_pBufferIn;
	unsigned char* m_pBufferOut;
	int m_nCompressLevel;
};

