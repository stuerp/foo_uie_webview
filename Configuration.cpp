
/** $VER: configuration_t.cpp (2024.06.12) P. Stuer **/

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
    pfc::string8 Path = pfc::io::path::combine(core_api::get_profile_path(), STR_COMPONENT_BASENAME);

    if (::_strnicmp(Path, "file://", 7) == 0)
        Path = Path.subString(7);

    wchar_t FilePath[MAX_PATH];

    ::wcscpy_s(FilePath, _countof(FilePath), ::UTF8ToWide(Path.c_str()).c_str());

    HRESULT hResult = ::PathCchAppend(FilePath, _countof(FilePath), L"Template.html");

    if (SUCCEEDED(hResult))
        _TemplateFilePath = FilePath;
    else
        ::wcscpy_s(FilePath, _countof(FilePath), L"Template.html");
}

/// <summary>
/// Implements the = operator.
/// </summary>
configuration_t & configuration_t::operator=(const configuration_t & other)
{
    _TemplateFilePath = other._TemplateFilePath;

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

        // Version 1, v0.1.4.0
        pfc::string FilePath; reader->read_string(FilePath, abortHandler); _TemplateFilePath = pfc::wideFromUTF8(FilePath);
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
        pfc::string FilePath = pfc::utf8FromWide(_TemplateFilePath.c_str()); writer->write_string(FilePath, abortHandler);
    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to write configuration: %s", ex.what());
    }
}
