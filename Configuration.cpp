
/** $VER: configuration_t.cpp (2024.07.09) P. Stuer **/

#include "pch.h"

#include "Configuration.h"
#include "Encoding.h"
#include "Resources.h"

#include <SDK/file.h>
#include <SDK/advconfig_impl.h>

#include <pfc/string_conv.h>
#include <pfc/string-conv-lite.h>

using namespace pfc;
using namespace stringcvt;

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
configuration_t::configuration_t()
{
    {
        GUID Guid;

        (void) ::CoCreateGuid(&Guid);

        wchar_t ProfileName[64];

        ::swprintf_s(ProfileName, _countof(ProfileName), TEXT(STR_COMPONENT_BASENAME) L"-%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X", (int) Guid.Data1, (int) Guid.Data2, (int) Guid.Data3, Guid.Data4[0], Guid.Data4[1], Guid.Data4[2], Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);

        _ProfileName = ProfileName;
    }

    Reset();
}

/// <summary>
/// Resets this instance.
/// </summary>
void configuration_t::Reset() noexcept
{
    _Name = TEXT(STR_COMPONENT_NAME);

    pfc::string8 Path = pfc::io::path::combine(core_api::get_profile_path(), STR_COMPONENT_BASENAME);

    if (::_strnicmp(Path, "file://", 7) == 0)
        Path = Path.subString(7);

    {
        wchar_t FilePath[MAX_PATH];

        ::wcscpy_s(FilePath, _countof(FilePath), ::UTF8ToWide(Path.c_str()).c_str());

        HRESULT hr = ::PathCchAppend(FilePath, _countof(FilePath), L"Template.html");

        if (SUCCEEDED(hr))
            _TemplateFilePath = FilePath;
        else
            ::wcscpy_s(FilePath, _countof(FilePath), L"Template.html");
    }

    {
        _UserDataFolderPath = ::UTF8ToWide(Path.c_str());
    }

    _WindowSize = 100; // ms
    _WindowSizeUnit = WindowSizeUnit::Milliseconds;
    _ReactionAlignment = 0.25;

    _ClearOnStartup = ClearOnStartup::None;
    _InPrivateMode = false;
}

/// <summary>
/// Implements the = operator.
/// </summary>
configuration_t & configuration_t::operator=(const configuration_t & other)
{
    _Name = other._Name;
    _TemplateFilePath = other._TemplateFilePath;
    _UserDataFolderPath = other._UserDataFolderPath;

    _WindowSize = other._WindowSize;
    _WindowSizeUnit = other._WindowSizeUnit;
    _ReactionAlignment = other._ReactionAlignment;

    _ProfileName = other._ProfileName;

    _ClearOnStartup = other._ClearOnStartup;
    _InPrivateMode = other._InPrivateMode;
 
    return *this;
}

/// <summary>
/// Reads this instance with the specified reader.
/// </summary>
void configuration_t::Read(stream_reader * reader, size_t size, abort_callback & abortHandler, bool isPreset) noexcept
{
    Reset();

    try
    {
        int32_t Version;

        reader->read(&Version, sizeof(Version), abortHandler);

        pfc::string UTF8String;

        // Version 1, v0.1.4.0
        reader->read_string(UTF8String, abortHandler); _TemplateFilePath = pfc::wideFromUTF8(UTF8String);

        // Version 2, v0.1.5.0-alpha1
        if (Version >= 2)
        {
            reader->read_string(UTF8String, abortHandler); _Name               = pfc::wideFromUTF8(UTF8String);
            reader->read_string(UTF8String, abortHandler); _UserDataFolderPath = pfc::wideFromUTF8(UTF8String);
        }

        // Version 3, v0.1.5.0-alpha2
        if (Version >= 3)
        {
            reader->read_object_t(_WindowSize, abortHandler);
            uint32_t Value; reader->read_object_t(Value, abortHandler); _WindowSizeUnit = (WindowSizeUnit) Value;
            reader->read_object_t(_ReactionAlignment, abortHandler);
        }

        // Version 4, v0.1.5.6
        if (Version >= 4)
        {
            reader->read_string(UTF8String, abortHandler); _ProfileName = pfc::wideFromUTF8(UTF8String);
        }

        // Version 5, v0.1.6.0
        if (Version >= 5)
        {
            uint32_t Value; reader->read_object_t(Value, abortHandler); _ClearOnStartup = (ClearOnStartup) Value;
        }

        // Version 6, v0.1.6.3-alpha3
        if (Version >= 6)
        {
            reader->read_object_t(_InPrivateMode, abortHandler);
        }
    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to read configuration: %s", ex.what());

        Reset();
    }
}

/// <summary>
/// Writes this instance to the specified writer.
/// </summary>
void configuration_t::Write(stream_writer * writer, abort_callback & abortHandler, bool isPreset) const noexcept
{
    try
    {
        writer->write_object_t(_CurrentVersion, abortHandler);

        // Version 1, v0.1.4.0
        pfc::string UTF8String = pfc::utf8FromWide(_TemplateFilePath.c_str()); writer->write_string(UTF8String, abortHandler);

        // Version 2, v0.1.5.0-alpha1
        UTF8String = pfc::utf8FromWide(_Name.c_str());               writer->write_string(UTF8String, abortHandler);
        UTF8String = pfc::utf8FromWide(_UserDataFolderPath.c_str()); writer->write_string(UTF8String, abortHandler);

        // Version 3, v0.1.5.0-alpha2
        writer->write_object_t(_WindowSize, abortHandler);
        uint32_t Value = (uint32_t) _WindowSizeUnit; writer->write_object_t(Value, abortHandler);
        writer->write_object_t(_ReactionAlignment, abortHandler);

        // Version 4, v0.1.5.6
        UTF8String = pfc::utf8FromWide(_ProfileName.c_str()); writer->write_string(UTF8String, abortHandler);

        // Version 5, v0.1.6.0
        Value = (uint32_t) _ClearOnStartup; writer->write_object_t(Value, abortHandler);

        // Version 6, v0.1.6.3-alpha3
        writer->write_object_t(_InPrivateMode, abortHandler);
    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to write configuration: %s", ex.what());
    }
}
