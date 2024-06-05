
/** $VER: Exceptions.h (2024.06.05) P. Stuer **/

#pragma once

#include "framework.h"

std::string GetErrorMessage(DWORD errorCode, const std::string & errorMessage) noexcept;

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
    Win32Exception(HRESULT errorCode, const std::string & errorMessage) noexcept : std::runtime_error(GetErrorMessage((DWORD) errorCode, errorMessage)), _ErrorCode((DWORD) errorCode) { }

    DWORD GetErrorCode() const { return _ErrorCode; }

private:
    DWORD _ErrorCode;
};
