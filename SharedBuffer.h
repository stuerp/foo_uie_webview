
/** $VER: SharedBuffer.h (2024.06.26) P. Stuer - Implements a buffer shared by the component and WebView. **/

#pragma once

#include "framework.h"

#include <wrl.h>
#include <wil/com.h>

#include <WebView2.h>

/// <summary>
/// Implements a shared buffer.
/// </summary>
class SharedBuffer
{
public:
    SharedBuffer() : _SampleCount(), _SampleRate(), _ChannelCount(), _ChannelConfig() { }

    virtual ~SharedBuffer();

    HRESULT Ensure(wil::com_ptr<ICoreWebView2Environment> & environment, wil::com_ptr<ICoreWebView2> & webView, size_t sampleCount, uint32_t sampleRate, uint32_t channelCount, uint32_t channelConfig) noexcept;
    void Release() noexcept;

    void Copy(const BYTE * data, size_t size) noexcept;
    void Convert(const float * sampleData, size_t sampleCount) noexcept;

private:
    size_t _SampleCount;
    uint32_t _SampleRate;
    uint32_t _ChannelCount;
    uint32_t _ChannelConfig;

    wil::com_ptr<ICoreWebView2Environment12> _Environment12;
    wil::com_ptr<ICoreWebView2_17> _WebView17;
    wil::com_ptr<ICoreWebView2SharedBuffer> _SharedBuffer;
    UINT64 _Size;
    BYTE * _Buffer;
};
