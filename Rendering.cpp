
/** $VER: Rendering.cpp (2024.06.23) P. Stuer **/

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
    ::SetTimer(m_hWnd, (UINT_PTR) this, 1000 / (DWORD) 50, TimerCallback);
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

    const double WindowSize = 0.05; // in seconds
    const double Offset     = PlaybackTime - (WindowSize / 2.);

    if (!_VisualisationStream->get_chunk_absolute(Chunk, Offset, WindowSize))
        return;

    const audio_sample * Samples = Chunk.get_data();

    size_t SampleCount = (uint32_t) Chunk.get_sample_count();
    uint32_t ChannelCount = Chunk.get_channel_count();
    uint32_t SampleRate = Chunk.get_sample_rate();

    HRESULT hr = PostChunk(Samples, SampleCount, ChannelCount, SampleRate);

    if (!SUCCEEDED(hr))
        return;

    {
        hr = _WebView->ExecuteScript(::FormatText(L"OnTimer(%d, %d, %d)", SampleCount, ChannelCount, SampleRate).c_str(), nullptr); // Silently continue

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
HRESULT UIElement::PostChunk(const audio_sample * samples, size_t sampleCount, uint32_t channelCount, uint32_t sampleRate) noexcept
{
    const UINT64 Size = sizeof(audio_sample) * sampleCount;

    if (_SharedBuffer == nullptr)
    {
        wil::com_ptr<ICoreWebView2Environment12> Environment;

        HRESULT hr = _Environment->QueryInterface(IID_PPV_ARGS(&Environment));

        if (!SUCCEEDED(hr))
            return hr;

        hr = Environment->CreateSharedBuffer(Size, &_SharedBuffer);

        if (!SUCCEEDED(hr))
            return hr;

        hr = _SharedBuffer->get_Buffer(&_Buffer);

        if (!SUCCEEDED(hr))
            return hr;

        _WebView17 = _WebView.try_query<ICoreWebView2_17>();

        if (_WebView17 == nullptr)
            return E_NOINTERFACE;

        std::wstring AdditionalDataAsJson = ::FormatText(L"{\"SampleCount\":%d,\"ChannelCount\":%d,\"SampleRate\":%d}", (int) sampleCount, (int) channelCount, (int) sampleRate);

        hr = _WebView17->PostSharedBufferToScript(_SharedBuffer.get(), COREWEBVIEW2_SHARED_BUFFER_ACCESS_READ_WRITE, AdditionalDataAsJson.c_str());

        if (!SUCCEEDED(hr))
            return hr;
    }

    if (_Buffer != nullptr)
        ::memcpy(_Buffer, samples, Size);

    return S_OK;
}
