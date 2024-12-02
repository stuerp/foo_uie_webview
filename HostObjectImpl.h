
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

    STDMETHODIMP get_componentVersion(__int32 * version) override;
    STDMETHODIMP get_componentVersionText(BSTR * versionText) override;

    STDMETHODIMP print(BSTR text) override;

    /* Playback control **/

    STDMETHODIMP stop() override;
    STDMETHODIMP play(VARIANT_BOOL paused) override;
    STDMETHODIMP pause(VARIANT_BOOL paused) override;
    STDMETHODIMP previous() override;
    STDMETHODIMP next() override;
    STDMETHODIMP random() override;

    STDMETHODIMP togglePause() override;
    STDMETHODIMP toggleMute() override;
    STDMETHODIMP toggleStopAfterCurrent() override;

    STDMETHODIMP volumeUp() override;
    STDMETHODIMP volumeDown() override;

    STDMETHODIMP seek(double time) override;
    STDMETHODIMP seekDelta(double delta) override;

    STDMETHODIMP get_isPlaying(VARIANT_BOOL * value) override;
    STDMETHODIMP get_isPaused(VARIANT_BOOL * value) override;

    STDMETHODIMP get_stopAfterCurrent(VARIANT_BOOL * value) override;
    STDMETHODIMP put_stopAfterCurrent(VARIANT_BOOL value) override;

    STDMETHODIMP get_length(double * value) override;
    STDMETHODIMP get_position(double * value) override;
    STDMETHODIMP get_canSeek(VARIANT_BOOL * value) override;

    STDMETHODIMP get_volume(double * value) override;
    STDMETHODIMP put_volume(double value) override;

    STDMETHODIMP get_isMuted(VARIANT_BOOL * value) override;

    STDMETHODIMP getFormattedText(BSTR text, BSTR * formattedText) override;

    STDMETHODIMP getArtwork(BSTR type, BSTR * image) override;

    /* Files */

    STDMETHODIMP readAllText(BSTR filePath, __int32 codePage, BSTR * text) override;
    STDMETHODIMP readImage(BSTR filePath, BSTR * image) override;
    STDMETHODIMP readDirectory(BSTR filePath, BSTR searchPattern, BSTR * json) override;

    /* Playlists */

    STDMETHODIMP get_playlistCount(int * playlistCount) override;

    STDMETHODIMP get_activePlaylist(int * playlistIndex) override;
    STDMETHODIMP put_activePlaylist(int playlistIndex) override;

    STDMETHODIMP get_playingPlaylist(int * playlistIndex) override;
    STDMETHODIMP put_playingPlaylist(int playlistIndex) override;

    STDMETHODIMP getPlaylistName(int playlistIndex, BSTR * name) override;
    STDMETHODIMP setPlaylistName(int playlistIndex, BSTR name) override;

    STDMETHODIMP findPlaylist(BSTR name, int * playlistIndex) override;

    STDMETHODIMP getPlaylistItemCount(int playlistIndex, int * itemCount) override;
    STDMETHODIMP getSelectedPlaylistItemCount(int playlistIndex, int maxItems, int * itemCount) override;

    STDMETHODIMP getFocusedPlaylistItem(int playlistIndex, int * itemIndex) override;
    STDMETHODIMP setFocusedPlaylistItem(int playlistIndex, int itemIndex) override;
    STDMETHODIMP ensurePlaylistItemVisible(int playlistIndex, int itemIndex) override;
    STDMETHODIMP executePlaylistDefaultAction(int playlistIndex, int itemIndex) override;
    STDMETHODIMP isPlaylistItemSelected(int playlistIndex, int itemIndex, VARIANT_BOOL * result) override;

    STDMETHODIMP createPlaylist(int playlistIndex, BSTR name, int * newPlaylistIndex) override;
    STDMETHODIMP addPath(int playlistIndex, int itemIndex, BSTR filePath, VARIANT_BOOL selectAddedItems) override;

    STDMETHODIMP duplicatePlaylist(int playlistIndex, BSTR name, int * newPlaylistIndex) override;
    STDMETHODIMP clearPlaylist(int playlistIndex) override;
    STDMETHODIMP getPlaylistItems(int playlistIndex, BSTR * json) override;

    STDMETHODIMP selectPlaylistItem(int playlistIndex, int itemIndex) override;
    STDMETHODIMP deselectPlaylistItem(int playlistIndex, int itemIndex) override;
    STDMETHODIMP getSelectedPlaylistItems(int playlistIndex, BSTR * json) override;
    STDMETHODIMP clearPlaylistSelection(int playlistIndex) override;
    STDMETHODIMP removeSelectedPlaylistItems(int playlistIndex) override;
    STDMETHODIMP removeUnselectedPlaylistItems(int playlistIndex) override;

    STDMETHODIMP removePlaylistItem(int playlistIndex, int itemIndex) override;

    STDMETHODIMP deletePlaylist(int playlistIndex) override;

    /* Auto Playlists */

    STDMETHODIMP createAutoPlaylist(int playlistIndex, BSTR name, BSTR query, BSTR sort, uint32_t flags, int * newPlaylistIndex) override;
    STDMETHODIMP isAutoPlaylist(int playlistIndex, VARIANT_BOOL * result) override;

    /* Playback Order */

    STDMETHODIMP get_playbackOrder(int * playlistIndex) override;
    STDMETHODIMP put_playbackOrder(int playlistIndex) override;

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
extern const std::wstring Stringify(const std::wstring & s);

extern std::wstring ToJSON(const metadb_handle_list & hItems);
extern std::wstring ToJSON(const bit_array & mask, t_size count);
extern std::wstring ToJSON(const t_size * array, t_size count);
