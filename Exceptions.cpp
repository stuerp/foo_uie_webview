
/** $VER: Exceptions.cpp (2024.05.27) P. Stuer **/

#include "pch.h"

#include "Exceptions.h"
#include "Encoding.h"

const char * strrstr(const char * __restrict s1, const char *__restrict s2) noexcept;

/// <summary>
/// Gets the error message of the specified error code.
/// </summary>
std::string GetErrorMessage(DWORD errorCode, const std::string & errorMessage) noexcept
{
    std::string Text;

    Text.resize(256);

    if (::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0, Text.data(), (DWORD) Text.size(), nullptr) != 0)
    {
        // Remove a trailing "\r\n".
        char * p = (char *) ::strrstr(Text.c_str(), "\r\n");

        if (p != nullptr)
            *p = '\0';

        // Remove a trailing period ('.').
        p = (char *) ::strrchr(Text.c_str(), '.');

        if (p != nullptr)
            *p = '\0';
    }
    else
        Text = ::FormatText("Failed to get error message for error code (0x%08X)", ::GetLastError());

    return FormatText("%s: %s (0x%08X)", errorMessage.c_str(), Text.c_str(), errorCode);
}

/// <summary>
/// Returns a pointer to the last occurance of a string.
/// </summary>
const char * strrstr(const char * __restrict s1, const char *__restrict s2) noexcept
{
    const size_t l1 = ::strlen(s1);
    const size_t l2 = ::strlen(s2);

    if (l2 > l1)
        return nullptr;

    for (const char * s = s1 + (l1 - l2); s >= s1; --s)
        if (::strncmp(s, s2, l2) == 0)
            return s;

    return nullptr;
}
