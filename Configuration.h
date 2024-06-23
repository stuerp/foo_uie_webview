
/** $VER: Configuration.h (2024.06.23) P. Stuer **/

#pragma once

#include "pch.h"

/// <summary>
/// Represents the configuration of the component.
/// </summary>
class configuration_t
{
public:
    configuration_t();

    configuration_t & operator=(const configuration_t & other);

    virtual ~configuration_t() { }

    void Reset() noexcept;

    void Read(stream_reader * reader, size_t size, abort_callback & abortHandler = fb2k::noAbort, bool isPreset = false) noexcept;
    void Write(stream_writer * writer, abort_callback & abortHandler = fb2k::noAbort, bool isPreset = false) const noexcept;

public:
    std::wstring _Name;
    std::wstring _TemplateFilePath;
    std::wstring _UserDataFolderPath;

private:
    const int32_t _CurrentVersion = 2;
};
