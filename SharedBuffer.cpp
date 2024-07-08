
/** $VER: SharedBuffer.cpp (2024.07.03) P. Stuer **/

#include "pch.h"

#include "SharedBuffer.h"

#include "Encoding.h"

#pragma hdrstop

using namespace Microsoft::WRL;

/// <summary>
/// Ensures that a buffer of the correct size is posted to the WebView.
/// </summary>
HRESULT SharedBuffer::Ensure(wil::com_ptr<ICoreWebView2Environment> & environment, wil::com_ptr<ICoreWebView2> & webView, size_t sampleCount, uint32_t sampleRate, uint32_t channelCount, uint32_t channelConfig) noexcept
{
    if ((_Buffer != nullptr) && (_SampleCount == sampleCount) && (_SampleRate == sampleRate) && (_ChannelCount == channelCount) && (_ChannelConfig == channelConfig))
        return S_OK;

    Release();

    HRESULT hr = environment->QueryInterface(IID_PPV_ARGS(&_Environment12));

    if (!SUCCEEDED(hr))
        return hr;

    _WebView17 = webView.try_query<ICoreWebView2_17>();

    if (_WebView17 == nullptr)
        return E_NOINTERFACE;

    _Size = sizeof(double) * sampleCount * channelCount; // Don't use audio_sample.

    hr = _Environment12->CreateSharedBuffer(_Size, &_SharedBuffer);

    if (!SUCCEEDED(hr))
        return hr;

    hr = _SharedBuffer->get_Buffer(&_Buffer);

    if (!SUCCEEDED(hr))
        return hr;

    std::wstring AdditionalDataAsJson = ::FormatText(L"{\"SampleCount\":%d,\"SampleRate\":%d,\"ChannelCount\":%d,\"ChannelConfig\":%d}", (int) sampleCount, (int) sampleRate, (int) channelCount, (int) channelConfig);

    hr = _WebView17->PostSharedBufferToScript(_SharedBuffer.get(), COREWEBVIEW2_SHARED_BUFFER_ACCESS_READ_WRITE, AdditionalDataAsJson.c_str());

    if (!SUCCEEDED(hr))
        return hr;

    _SampleCount = sampleCount;
    _SampleRate = sampleRate;
    _ChannelCount = channelCount;
    _ChannelConfig = channelConfig;

    return S_OK;
}

/// <summary>
/// Releases the resources of this instance.
/// </summary>
void SharedBuffer::Release() noexcept
{
    _ChannelConfig = 0;
    _ChannelCount = 0;
    _SampleRate = 0;
    _SampleCount = 0;

    _Buffer = nullptr;
    _SharedBuffer = nullptr;
    _WebView17 = nullptr;
    _Environment12 = nullptr;
}

/// <summary>
/// Copies data to the buffer.
/// </summary>
void SharedBuffer::Copy(const BYTE * data, size_t size) noexcept
{
    if (_Buffer == nullptr)
        return;

    if (_Size < size)
        size = (size_t) _Size;

    ::memcpy(_Buffer, data, size);
}

/// <summary>
/// Fills the buffer with samples converted from 32-bit to 64-floats.
/// </summary>
void SharedBuffer::Convert(const float * sampleData, size_t sampleCount) noexcept
{
    if (_Buffer == nullptr)
        return;

    size_t Size = sizeof(double) * sampleCount * _ChannelCount;

    if (_Size < Size)
        sampleCount = _SampleCount;

    const float * p = sampleData;
    double * q = (double *) _Buffer;

    for (size_t i = 0; i < sampleCount * _ChannelCount; ++i)
        *q++ = (double) *p++;
}

/// <summary>
/// Deletes this instance.
/// </summary>
SharedBuffer::~SharedBuffer()
{
    Release();
}
