
/** $VER: configuration_t.cpp (2024.07.03) P. Stuer **/

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

#pragma region Deprecated
static constexpr GUID FilePathGUID = { 0x341c4082, 0x255b, 0x4a38, { 0x81, 0x53, 0x55, 0x43, 0x5a, 0xd2, 0xe8, 0xa5 }};
cfg_string FilePathCfg(FilePathGUID, "");
#pragma endregion

/// <summary>
/// Initializes a new instance.
/// </summary>
configuration_t::configuration_t()
{
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

        HRESULT hResult = ::PathCchAppend(FilePath, _countof(FilePath), L"Template.html");

        if (SUCCEEDED(hResult))
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

    return *this;
}

/// <summary>
/// Reads this instance with the specified reader. (CUI version)
/// </summary>
void configuration_t::Read(stream_reader * reader, size_t size, abort_callback & abortHandler, bool isPreset) noexcept
{
    Reset();

    try
    {
        int32_t Version;

        size_t Size = reader->read(&Version, sizeof(Version), abortHandler);

        if ((Size != sizeof(Version)) || (Version > _CurrentVersion))
        {
            // One-time conversion from configuration variables to custom configuration handling.
            if ((_CurrentVersion == 1) && !FilePathCfg.isEmpty())
                _TemplateFilePath = pfc::wideFromUTF8(FilePathCfg.c_str());

            return;
        }

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
        }
    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to read configuration: %s", ex.what());

        Reset();
    }
}

/// <summary>
/// Writes this instance to the specified writer. (CUI version)
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
    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to write configuration: %s", ex.what());
    }
}
