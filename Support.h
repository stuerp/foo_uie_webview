
/** $VER: Support.h (2024.12.31) P. Stuer **/

#pragma once

#include "pch.h"

extern HMODULE GetCurrentModule() noexcept;
extern const std::wstring ExpandEnvironmentStrings(const wchar_t * src) noexcept;
extern std::wstring GetErrorMessage(DWORD errorCode) noexcept;

