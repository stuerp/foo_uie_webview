
/** $VER: Support.cpp (2024.05.28) P. Stuer **/

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
