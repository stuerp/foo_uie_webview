
/** $VER: Support.cpp (2024.12.31) P. Stuer **/

#include "pch.h"

#include "Encoding.h"

const wchar_t * strrstr(const wchar_t * __restrict s1, const wchar_t *__restrict s2) noexcept;

/// <summary>
/// Gets the handle of the module that contains the executing code.
/// </summary>
HMODULE GetCurrentModule() noexcept
{
    HMODULE hModule = NULL;

    ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR) GetCurrentModule, &hModule);

    return hModule;
}

/// <summary>
/// Expands the environment variables in the specified string.
/// </summary>
const std::wstring ExpandEnvironmentStrings(const wchar_t * src) noexcept
{
    DWORD Size = ::ExpandEnvironmentStringsW(src, nullptr, 0) + 1;

    std::wstring Dst;

    Dst.resize(Size);

    ::ExpandEnvironmentStringsW(src, (LPWSTR) Dst.data(), (DWORD) Dst.size());

    return Dst;
}

/// <summary>
/// Gets the error message of the specified error code.
/// </summary>
std::wstring GetErrorMessage(DWORD errorCode) noexcept
{
    std::wstring Text;

    Text.resize(256);

    if (::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0, Text.data(), (DWORD) Text.size(), nullptr) != 0)
    {
        // Remove a trailing "\r\n".
        wchar_t * p = (wchar_t *) ::strrstr(Text.c_str(), L"\r\n");

        if (p != nullptr)
            *p = '\0';

        // Remove a trailing period ('.').
        p = (wchar_t *) ::wcsrchr(Text.c_str(), '.');

        if (p != nullptr)
            *p = '\0';
    }
    else
        Text = ::FormatText(L"Failed to get error message for error code (0x%08X)", ::GetLastError());

    return ::FormatText(L"%s (0x%08X)", Text.c_str(), errorCode);
}

/// <summary>
/// Returns a pointer to the last occurance of a string.
/// </summary>
const wchar_t * strrstr(const wchar_t * __restrict s1, const wchar_t *__restrict s2) noexcept
{
    const size_t l1 = ::wcslen(s1);
    const size_t l2 = ::wcslen(s2);

    if (l2 > l1)
        return nullptr;

    for (const wchar_t * s = s1 + (l1 - l2); s >= s1; --s)
        if (::wcsncmp(s, s2, l2) == 0)
            return s;

    return nullptr;
}
