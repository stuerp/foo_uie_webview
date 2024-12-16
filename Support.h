
/** $VER: Support.h (2024.12.16) P. Stuer **/

#pragma once

#include "pch.h"

extern HMODULE GetCurrentModule() noexcept;
extern const std::wstring ExpandEnvironmentStrings(const wchar_t * src) noexcept;
