
/** $VER: HostObjectImpl.h (2024.11.20) P. Stuer **/

#pragma once

#include "pch.h"

#include <functional>
#include <map>
#include <string>

#include <wrl.h>
#include <wrl/client.h>
#include <wil/com.h>
#include <wil/resource.h>
#include <wil/result.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

#include "HostObject_h.h"

#include <SDK/playback_control.h>
#include <SDK/album_art.h>

class HostObject : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IHostObject, IDispatch>
{
public:
    HostObject(const HostObject &) = delete;
    HostObject(const HostObject &&) = delete;
    HostObject & operator=(const HostObject &) = delete;
    HostObject & operator=(HostObject &&) = delete;

    virtual ~HostObject() { };

    typedef std::function<void(void)> Callback;
    typedef std::function<void(Callback)> RunCallbackAsync;

    HostObject(RunCallbackAsync runCallbackAsync);

    #pragma region IHostObject

    STDMETHODIMP get_ComponentVersion(__int32 * version) override;
    STDMETHODIMP get_ComponentVersionText(BSTR * versionText) override;

    STDMETHODIMP Print(BSTR text) override;

    /* Playback control **/

    STDMETHODIMP Stop() override;
    STDMETHODIMP Play(BOOL paused) override;
    STDMETHODIMP Pause(BOOL paused) override;
    STDMETHODIMP Previous() override;
    STDMETHODIMP Next() override;
    STDMETHODIMP Random() override;

    STDMETHODIMP TogglePause() override;
    STDMETHODIMP ToggleMute() override;
    STDMETHODIMP ToggleStopAfterCurrent() override;

    STDMETHODIMP VolumeUp() override;
    STDMETHODIMP VolumeDown() override;

    STDMETHODIMP Seek(double time) override;
    STDMETHODIMP SeekDelta(double delta) override;

    STDMETHODIMP get_IsPlaying(BOOL * value) override;
    STDMETHODIMP get_IsPaused(BOOL * value) override;

    STDMETHODIMP get_StopAfterCurrent(BOOL * value) override;
    STDMETHODIMP put_StopAfterCurrent(BOOL value) override;

    STDMETHODIMP get_Length(double * value) override;
    STDMETHODIMP get_Position(double * value) override;
    STDMETHODIMP get_CanSeek(BOOL * value) override;

    STDMETHODIMP get_Volume(double * value) override;
    STDMETHODIMP put_Volume(double value) override;

    STDMETHODIMP get_IsMuted(BOOL * value) override;

    STDMETHODIMP GetFormattedText(BSTR text, BSTR * formattedText) override;

    STDMETHODIMP GetArtwork(BSTR type, BSTR * image) override;

    STDMETHODIMP ReadAllText(BSTR filePath, __int32 codePage, BSTR * text) override;

    #pragma endregion

    #pragma region IDispatch

    STDMETHODIMP GetTypeInfoCount(UINT * typeInfoCount) override;
    STDMETHODIMP GetTypeInfo(UINT typeInfoIndex, LCID lcid, ITypeInfo ** typeInfo) override;
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR * names, UINT nameCount, LCID lcid, DISPID * dispIds) override;
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * dispParams, VARIANT * result, EXCEPINFO * excepInfo, UINT * argErr) override;

    #pragma endregion

private:
    static HRESULT GetTrackIndex(t_size & playlistIndex, t_size & itemIndex) noexcept;
    static HRESULT GetTypeLibFilePath(std::wstring & filePath) noexcept;

private:
    wil::com_ptr<ITypeLib> _TypeLibrary;

    wil::com_ptr<IDispatch> _Callback;
    RunCallbackAsync _RunCallbackAsync;

    service_ptr_t<playback_control> _PlaybackControl;

    /// <summary>
    /// Represents an Album Art Manager configuration to allow overriding the default configuration in this component (see album_art_manager_v3::open_v3)
    /// </summary>
    class album_art_manager_config_t : public album_art_manager_config
    {
    public:
        album_art_manager_config_t() { };

        album_art_manager_config_t(const album_art_manager_config_t &) = delete;
        album_art_manager_config_t(const album_art_manager_config_t &&) = delete;
        album_art_manager_config_t & operator=(const album_art_manager_config_t &) = delete;
        album_art_manager_config_t & operator=(album_art_manager_config_t &&) = delete;

        virtual ~album_art_manager_config_t() { };

        virtual bool get_external_pattern(pfc::string_base & out, const GUID & albumArtType) override
        {
            return false;
        }

        virtual bool use_embedded_pictures() override
        {
            return true;
        }

        virtual bool use_fallbacks() override
        {
            return true;
        }
    };

    service_ptr_t<album_art_manager_config_t> _AlbumArtManagerConfig = new service_impl_t<album_art_manager_config_t>;
};
