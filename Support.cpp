
/** $VER: Support.cpp (2024.12.16) P. Stuer **/

#include "pch.h"

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
