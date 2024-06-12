
/** $VER: Configuration.h (2024.06.12) P. Stuer **/

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
    std::wstring _TemplateFilePath;

private:
    const int32_t _CurrentVersion = 1;
};

static const std::wstring OnPlaybackStartingCallback              = L"OnPlaybackStarting";
static const std::wstring OnPlaybackNewTrackCallback              = L"OnPlaybackNewTrack";
static const std::wstring OnPlaybackStopCallback                  = L"OnPlaybackStop";
static const std::wstring OnPlaybackSeekCallback                  = L"OnPlaybackSeek";
static const std::wstring OnPlaybackPauseCallback                 = L"OnPlaybackPause";
static const std::wstring OnPlaybackEditedCallback                = L"OnPlaybackEdited";
static const std::wstring OnPlaybackDynamicInfoCallback           = L"OnPlaybackDynamicInfo";
static const std::wstring OnPlaybackDynamicTrackInfoCallback      = L"OnPlaybackDynamicTrackInfo";
static const std::wstring OnPlaybackTimeCallback                  = L"OnPlaybackTime";
static const std::wstring OnVolumeChangeCallback                  = L"OnVolumeChange";
static const std::wstring OnPlaylistFocusedItemChangedCallback    = L"OnPlaylistFocusedItemChanged";
