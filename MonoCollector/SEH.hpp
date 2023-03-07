#pragma once
#include <windows.h>
#include <exception>

class seh_excpetion : std::exception
{
	typedef ULONG(WINAPI* fpRtlNtStatusToDosError)(NTSTATUS Status);

public:
	seh_excpetion(unsigned int nExceptionCode, _EXCEPTION_POINTERS* pstExcptionInfor) :
		m_nExceptionCode(0),
		m_pExcptionInfor(NULL),
		m_szMsgBuff(NULL),
		m_hNtModule(NULL),
		RtlNtStatusToDosError(NULL)
	{
		m_nExceptionCode = nExceptionCode;
		m_pExcptionInfor = pstExcptionInfor;
		m_hNtModule = GetModuleHandle("NTDLL.DLL");
		if (NULL != m_hNtModule)
		{
			RtlNtStatusToDosError = (fpRtlNtStatusToDosError)GetProcAddress(m_hNtModule, "RtlNtStatusToDosError");
		}
	}

	virtual ~seh_excpetion()
	{
		m_nExceptionCode = 0;
		m_pExcptionInfor = NULL;
		RtlNtStatusToDosError = NULL;

		if (NULL != m_szMsgBuff)
		{
			LocalFree(m_szMsgBuff);
			m_szMsgBuff = NULL;
		}
	};

	const char* what() const noexcept
	{
		if (RtlNtStatusToDosError != NULL)
		{
			DWORD nConvertLen = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
				m_hNtModule,
				RtlNtStatusToDosError(m_nExceptionCode),
				0,
				(char*)&m_szMsgBuff,
				0,
				NULL);

			if (0 != nConvertLen)
			{
				return m_szMsgBuff;
			}
		}

		return "SEH_UNKNOW_ERROR";
	}

	const PEXCEPTION_POINTERS info() const
	{
		return m_pExcptionInfor;
	}

	const unsigned int code() const
	{
		return m_nExceptionCode;
	}

private:
	HMODULE m_hNtModule;

	unsigned int m_nExceptionCode;
	char* m_szMsgBuff;
	PEXCEPTION_POINTERS m_pExcptionInfor;
	fpRtlNtStatusToDosError RtlNtStatusToDosError;

public:
	static void(__cdecl TranslateSEHtoCE)(unsigned int nExceptionCode, struct _EXCEPTION_POINTERS* pstExcptionInfor)
	{
		throw seh_excpetion(nExceptionCode, pstExcptionInfor);
	}

};


