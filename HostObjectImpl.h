
/** $VER: HostObjectImpl.h (2024.12.02) P. Stuer **/

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
    STDMETHODIMP Play(VARIANT_BOOL paused) override;
    STDMETHODIMP Pause(VARIANT_BOOL paused) override;
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

    STDMETHODIMP get_IsPlaying(VARIANT_BOOL * value) override;
    STDMETHODIMP get_IsPaused(VARIANT_BOOL * value) override;

    STDMETHODIMP get_StopAfterCurrent(VARIANT_BOOL * value) override;
    STDMETHODIMP put_StopAfterCurrent(VARIANT_BOOL value) override;

    STDMETHODIMP get_Length(double * value) override;
    STDMETHODIMP get_Position(double * value) override;
    STDMETHODIMP get_CanSeek(VARIANT_BOOL * value) override;

    STDMETHODIMP get_Volume(double * value) override;
    STDMETHODIMP put_Volume(double value) override;

    STDMETHODIMP get_IsMuted(VARIANT_BOOL * value) override;

    STDMETHODIMP GetFormattedText(BSTR text, BSTR * formattedText) override;

    STDMETHODIMP GetArtwork(BSTR type, BSTR * image) override;

    /* Files */

    STDMETHODIMP ReadAllText(BSTR filePath, __int32 codePage, BSTR * text) override;
    STDMETHODIMP ReadImage(BSTR filePath, BSTR * image) override;
    STDMETHODIMP ReadDirectory(BSTR filePath, BSTR searchPattern, BSTR * json) override;

    /* Playlists */

    STDMETHODIMP get_PlaylistCount(int * playlistCount) override;

    STDMETHODIMP get_ActivePlaylist(int * playlistIndex) override;
    STDMETHODIMP put_ActivePlaylist(int playlistIndex) override;

    STDMETHODIMP get_PlayingPlaylist(int * playlistIndex) override;
    STDMETHODIMP put_PlayingPlaylist(int playlistIndex) override;

    STDMETHODIMP GetPlaylistName(int playlistIndex, BSTR * name) override;
    STDMETHODIMP SetPlaylistName(int playlistIndex, BSTR name) override;

    STDMETHODIMP FindPlaylist(BSTR name, int * playlistIndex) override;

    STDMETHODIMP GetPlaylistItemCount(int playlistIndex, int * itemCount) override;
    STDMETHODIMP GetSelectedPlaylistItemCount(int playlistIndex, int maxItems, int * itemCount) override;

    STDMETHODIMP GetFocusedPlaylistItem(int playlistIndex, int * itemIndex) override;
    STDMETHODIMP SetFocusedPlaylistItem(int playlistIndex, int itemIndex) override;
    STDMETHODIMP EnsurePlaylistItemVisible(int playlistIndex, int itemIndex) override;
    STDMETHODIMP ExecutePlaylistDefaultAction(int playlistIndex, int itemIndex) override;
    STDMETHODIMP IsPlaylistItemSelected(int playlistIndex, int itemIndex, VARIANT_BOOL * result) override;

    STDMETHODIMP CreatePlaylist(int playlistIndex, BSTR name, int * newPlaylistIndex) override;
    STDMETHODIMP AddPath(int playlistIndex, int itemIndex, BSTR filePath, VARIANT_BOOL selectAddedItems) override;

    STDMETHODIMP DuplicatePlaylist(int playlistIndex, BSTR name, int * newPlaylistIndex) override;
    STDMETHODIMP ClearPlaylist(int playlistIndex) override;
    STDMETHODIMP GetPlaylistItems(int playlistIndex, BSTR * json) override;

    STDMETHODIMP SelectPlaylistItem(int playlistIndex, int itemIndex) override;
    STDMETHODIMP DeselectPlaylistItem(int playlistIndex, int itemIndex) override;
    STDMETHODIMP GetSelectedPlaylistItems(int playlistIndex, BSTR * json) override;
    STDMETHODIMP ClearPlaylistSelection(int playlistIndex) override;
    STDMETHODIMP RemoveSelectedPlaylistItems(int playlistIndex) override;
    STDMETHODIMP RemoveUnselectedPlaylistItems(int playlistIndex) override;

    STDMETHODIMP RemovePlaylistItem(int playlistIndex, int itemIndex) override;

    STDMETHODIMP DeletePlaylist(int playlistIndex) override;

    /* Auto Playlists */

    STDMETHODIMP CreateAutoPlaylist(int playlistIndex, BSTR name, BSTR query, BSTR sort, uint32_t flags, int * newPlaylistIndex) override;
    STDMETHODIMP IsAutoPlaylist(int playlistIndex, VARIANT_BOOL * result) override;

    /* Playback Order */

    STDMETHODIMP get_PlaybackOrder(int * playlistIndex) override;
    STDMETHODIMP put_PlaybackOrder(int playlistIndex) override;

    #pragma endregion

    #pragma region IDispatch

    STDMETHODIMP GetTypeInfoCount(UINT * typeInfoCount) override;
    STDMETHODIMP GetTypeInfo(UINT typeInfoIndex, LCID lcid, ITypeInfo ** typeInfo) override;
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR * names, UINT nameCount, LCID lcid, DISPID * dispIds) override;
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * dispParams, VARIANT * result, EXCEPINFO * excepInfo, UINT * argErr) override;

    #pragma endregion

private:
    static HRESULT GetTrackIndex(size_t & playlistIndex, size_t & itemIndex) noexcept;

    static HRESULT GetTypeLibFilePath(std::wstring & filePath) noexcept;

    static void NormalizeIndexes(int & playlistIndex, int & itemIndex) noexcept
    {
        auto Manager = playlist_manager_v4::get();

        if (playlistIndex == -1)
            playlistIndex = (int) Manager->get_active_playlist();

        if (itemIndex == -1)
            itemIndex = (int) Manager->playlist_get_item_count((size_t) playlistIndex) - 1;
    }

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

extern void ToBase64(const BYTE * data, DWORD size, BSTR * base64);
extern const std::string Stringify(const char * s);
extern std::wstring ToJSON(const metadb_handle_list & hItems);
