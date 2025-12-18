#include "stdafx.h"
#include "Compressor.h"
#include "CustomMessages.h"
#include "zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)

#endif

CCompressor::CCompressor(const CString& strInputFile, const CString& strOutputFile, UINT nMaxBufferSize /* = 1024*1024 */, int nCompressLevel /* = 1 */, BOOL bOverwriteOutputFile /* = FALSE */, CWnd* pOwnerWnd /* = nullptr */)
	: CWinThread()
	, m_strInputFile(strInputFile)
	, m_strOutputFile(strOutputFile)
	, m_nBufferSize(nMaxBufferSize)
	, m_pOwnerWnd(pOwnerWnd)
	, m_nExitCode(0)
	, m_pBufferIn(nullptr)
	, m_pBufferOut(nullptr)
	, m_bOverwriteOutputFileIfExist(bOverwriteOutputFile)
	, m_nCompressLevel(nCompressLevel)
{	
	m_bAutoDelete = FALSE;
}


CCompressor::~CCompressor(void)
{
	if(IsRunning())
		Stop();
	if(m_hThread != NULL)
	{
		DWORD dwCode = 0;
		if(GetExitCodeThread(m_hThread, &dwCode) == STILL_ACTIVE)
			WaitForSingleObject(m_hThread, INFINITE);
	}

	if(m_pBufferIn != nullptr)
		delete[] m_pBufferIn;

	if(m_pBufferOut != nullptr)
		delete[] m_pBufferOut;
}

void CCompressor::Stop()
{
	CSingleLock locker(&m_runMutex);
	m_bRunning = FALSE;
}

BOOL CCompressor::IsRunning()
{
	CSingleLock LOCKER(&m_runMutex);
	BOOL res = m_bRunning;

	return res;
}

BOOL CCompressor::InitInstance()
{
	CWinThread::InitInstance();

	m_bRunning = TRUE;
	m_nExitCode = COMPRESS_ERROR_SUCCESS;

	if(!PathFileExists(m_strInputFile))
	{
		m_nExitCode = COMPRESS_ERROR_INPUT_FILE_NOT_FOUND;
		return FALSE;
	}

	if(m_strOutputFile.IsEmpty())
	{
		m_nExitCode = COMPRESS_ERROR_OUTPUT_FILENOT_SPECIFIED;
		return FALSE;
	}

	if(PathFileExists(m_strOutputFile))
	{
		if(!m_bOverwriteOutputFileIfExist)
		{
			m_nExitCode = COMPRESS_ERROR_OUTPUT_FILE_EXIST;
			return FALSE;
		}
		else
		{
			try
			{
				DeleteFile(m_strOutputFile);
			}
			catch (CException* e)
			{
				e->Delete();
			}
		}
	}

	if(m_pBufferIn == nullptr)
	{
		m_pBufferIn = new unsigned char[m_nBufferSize+1];
		memset(m_pBufferIn, 0, m_nBufferSize+1);
	}

	if(m_pBufferOut == nullptr)
	{
		m_pBufferOut = new unsigned char[m_nBufferSize+1];
		memset(m_pBufferOut, 0, m_nBufferSize+1);
	}

	if(m_pOwnerWnd)
		m_pOwnerWnd->PostMessage(WM_COMPRESSOR_START);

	return TRUE;
}

int CCompressor::ExitInstance()
{
	m_bRunning = FALSE;

	CWinThread::ExitInstance();

	m_hThread = NULL;

	if(m_pOwnerWnd)
		m_pOwnerWnd->PostMessage(WM_COMPRESSOR_END, m_nExitCode);

	return m_nExitCode;
}

int CCompressor::Run()
{
	int status = 0;
	int flush = 0;
	unsigned int have = 0;
	z_stream z;


	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	status = deflateInit(&z, m_nCompressLevel);
	if(status != Z_OK)
	{
		m_nExitCode = COMPRESS_ERROR_ZLIB_BASE + status;
	}
	else
	{
		if(m_pBufferIn == nullptr)
		{
			m_pBufferIn = new unsigned char[m_nBufferSize+1];
		}
		memset(m_pBufferIn, 0, m_nBufferSize+1);

		if(m_pBufferOut == nullptr)
		{
			m_pBufferOut = new unsigned char[m_nBufferSize+1];
		}
		memset(m_pBufferOut, 0, m_nBufferSize+1);

		HANDLE hInputFile = CreateFile(m_strInputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, nullptr);
		if(hInputFile == INVALID_HANDLE_VALUE)
		{
			m_nExitCode = COMPRESS_ERROR_INPUT_FILE_CANNOT_OPEN;
		}
		else
		{
			LARGE_INTEGER liFileSize = {0};
			GetFileSizeEx(hInputFile, &liFileSize);
			LARGE_INTEGER liTotalReadBytes = {0};
			HANDLE hOutputFile = CreateFile(m_strOutputFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
			if(hOutputFile == INVALID_HANDLE_VALUE)
			{
				m_nExitCode = COMPRESS_ERROR_OUTPUT_FILE_CANNOT_CREATE;
			}
			else
			{
				DWORD dwReadBytes = 0;
				DWORD dwWritenBytes = 0;
				int count = 0;
				z.avail_in = 0;
				z.next_out = m_pBufferOut;
				z.avail_out = m_nBufferSize;
				do {
					if ( z.avail_in == 0 ) {
						z.next_in = m_pBufferIn;
						if(ReadFile(hInputFile, m_pBufferIn, m_nBufferSize, &dwReadBytes, nullptr))
						{
							z.avail_in = dwReadBytes;
							liTotalReadBytes.QuadPart += dwReadBytes;
						}
						else
							z.avail_in = 0;
					}
					if ( z.avail_in == 0 )
						break;
					status = deflate(&z, Z_NO_FLUSH );
					count = m_nBufferSize - z.avail_out;
					if (count > 0)
					{
						if(!WriteFile(hOutputFile, m_pBufferOut, count, &dwWritenBytes, nullptr))
						{
							m_nExitCode = COMPRESS_ERROR_OUTPUT_FILE_CANNOT_WRITE;
							break;
						}
						else
						{
							z.next_out = m_pBufferOut;
							z.avail_out = m_nBufferSize;
						}
					}

					if(m_pOwnerWnd)
						m_pOwnerWnd->PostMessage(WM_COMPRESSOR_PROGRESS, (WPARAM) ((double)liTotalReadBytes.QuadPart / (double) liFileSize.QuadPart * (double)100.0));
				} while(IsRunning());

				if(!IsRunning())
					m_nExitCode = COMPRESS_ERROR_ABORTED;

				(void)deflateEnd(&z);

				CloseHandle(hOutputFile);
				hOutputFile = INVALID_HANDLE_VALUE;
			}
			CloseHandle(hInputFile);
			hInputFile = INVALID_HANDLE_VALUE;
		}
	}
	int res = ExitInstance();
	
	return res;
}