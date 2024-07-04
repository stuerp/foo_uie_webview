
/** $VER: Configuration.h (2024.07.04) P. Stuer **/

#pragma once

#include "pch.h"

enum WindowSizeUnit
{
    Milliseconds = 0,
    Samples,

    Count
};

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

    uint32_t _WindowSize;                                           // Milliseconds or samples
    WindowSizeUnit _WindowSizeUnit;
    double _ReactionAlignment;                                      // Like in Vizzy.io. Controls the delay between the actual playback and the visualization.
                                                                    // < 0: All samples are ahead the actual playback (with the first sample equal to the actual playback)
                                                                    //   0: The first half of samples are behind the actual playback and the second half are ahead of it (just like original foo_musical_spectrum and basically any get_spectrum_absolute() visualizations
                                                                    // > 0: All samples are behind the playback (similar to VST audio analyzer plugins like Voxengo SPAN) with the last sample equal to the actual playback.

private:
    const int32_t _CurrentVersion = 3;
};
