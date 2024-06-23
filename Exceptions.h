
/** $VER: Exceptions.h (2024.06.23) P. Stuer **/

#pragma once

#include "framework.h"

std::string GetErrorMessage(DWORD errorCode, const std::string & errorMessage) noexcept;

inline std::string GetErrorMessage(HRESULT hResult, const std::string & errorMessage) noexcept
{
    return GetErrorMessage((DWORD) hResult, errorMessage);
}

#pragma warning(disable: 4820) // 'x' bytes padding added after data member 'y'

class ComponentException : public std::runtime_error
{
public:
    ComponentException(const std::string & errorMessage) noexcept : std::runtime_error(errorMessage) { }
};

class Win32Exception : public std::runtime_error
{
public:
    Win32Exception(const std::string & errorMessage) noexcept : Win32Exception(::GetLastError(), errorMessage) { }
    Win32Exception(DWORD errorCode, const std::string & errorMessage) noexcept : std::runtime_error(GetErrorMessage(errorCode, errorMessage)), _ErrorCode(errorCode) { }
    Win32Exception(HRESULT hResult, const std::string & errorMessage) noexcept : std::runtime_error(GetErrorMessage((DWORD) hResult, errorMessage)), _ErrorCode((DWORD) hResult) { }

    DWORD GetErrorCode() const { return _ErrorCode; }

private:
    DWORD _ErrorCode;
};
