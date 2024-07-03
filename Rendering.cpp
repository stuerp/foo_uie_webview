
/** $VER: Rendering.cpp (2024.07.03) P. Stuer **/

#include "pch.h"

#include "UIElement.h"
#include "Encoding.h"
#include "Exceptions.h"
#include "Support.h"

#pragma hdrstop

/// <summary>
/// Starts the timer.
/// </summary>
void UIElement::StartTimer() noexcept
{
    ::SetTimer(m_hWnd, (UINT_PTR) this, 1000 / (DWORD) 50, (TIMERPROC) TimerCallback);
}

/// <summary>
/// Stops the timer.
/// </summary>
void UIElement::StopTimer() noexcept
{
    ::KillTimer(m_hWnd, (UINT_PTR) this);
}

/// <summary>
/// Handles a timer tick.
/// </summary>
void CALLBACK UIElement::TimerCallback(HWND hWnd, UINT msg, UINT_PTR timerId, DWORD time) noexcept
{
    ((UIElement *) timerId)->OnTimer();
}

/// <summary>
/// Handles a timer tick.
/// </summary>
void UIElement::OnTimer() noexcept
{
    if (_IsFrozen || _IsHidden || ::IsIconic(core_api::get_main_window()) || (_WebView == nullptr) || !_IsNavigationCompleted)
        return;

    double PlaybackTime; // in seconds

    if (!_VisualisationStream.is_valid() || !_VisualisationStream->get_absolute_time(PlaybackTime) || (PlaybackTime == _LastPlaybackTime))
        return;

    _LastPlaybackTime = PlaybackTime;

    audio_chunk_impl Chunk;

    size_t SampleCount = Chunk.get_sample_count();
    uint32_t SampleRate = Chunk.get_sample_rate();
    uint32_t ChannelCount = Chunk.get_channel_count();
    uint32_t ChannelConfig = Chunk.get_channel_config();

    const double WindowSize = _Configuration._WindowSize / ((_Configuration._WindowSizeUnit == WindowSizeUnit::Milliseconds) ? 1000. : (double) SampleRate); // in seconds
    const double Offset     = PlaybackTime - (WindowSize * (0.5 + _Configuration._ReactionAlignment));

    if (!_VisualisationStream->get_chunk_absolute(Chunk, Offset, WindowSize))
        return;

    const audio_sample * Samples = Chunk.get_data();

    HRESULT hr = PostChunk(Samples, SampleCount, SampleRate, ChannelCount, ChannelConfig);

    if (!SUCCEEDED(hr))
        return;

    {
        hr = _WebView->ExecuteScript(::FormatText(L"OnTimer(%d, %d, %d, %d)", SampleCount, SampleRate, ChannelCount, ChannelConfig).c_str(), nullptr); // Silently continue

        if (!SUCCEEDED(hr))
        {
            console::printf(GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to call OnTimer()").c_str());

            StopTimer();
        }
    }
}

/// <summary>
/// Posts a chunk to the script via a shared buffer.
/// </summary>
HRESULT UIElement::PostChunk(const audio_sample * samples, size_t sampleCount, uint32_t sampleRate, uint32_t channelCount, uint32_t channelConfig) noexcept
{
    HRESULT hr = _SharedBuffer.Ensure(_Environment, _WebView, sampleCount, sampleRate, channelCount, channelConfig);

    if (SUCCEEDED(hr))
        if (audio_sample_size == 64)
            _SharedBuffer.Copy((const BYTE *) samples, sizeof(audio_sample) * sampleCount * channelCount);
        else
            _SharedBuffer.Convert((const float *) samples, sampleCount);

    return S_OK;
}
