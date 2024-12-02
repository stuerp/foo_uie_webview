
/** $VER: HostObjectImpl.cpp (2024.12.02) P. Stuer **/

#include "pch.h"

#include "HostObjectImpl.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#pragma comment(lib, "crypt32")

#include "Support.h"
#include "Resources.h"
#include "Encoding.h"

#include "ProcessLocationsHandler.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/ui.h>
#include <SDK/contextmenu.h>

#include <pfc/string-conv-lite.h>
#include <pfc/bit_array_impl.h>

/// <summary>
/// Initializes a new instance
/// </summary>
HostObject::HostObject(HostObject::RunCallbackAsync runCallbackAsync) : _RunCallbackAsync(runCallbackAsync)
{
    _PlaybackControl = playback_control::get();
}

/// <summary>
/// Gets the version of the component as packed integer.
/// </summary>
STDMETHODIMP HostObject::get_componentVersion(__int32 * version)
{
    if (version == nullptr)
        return E_INVALIDARG;

    *version = (NUM_PRODUCT_MAJOR << 24) | (NUM_PRODUCT_MINOR << 16) | (NUM_PRODUCT_PATCH << 8) | NUM_PRODUCT_PRERELEASE;

    return S_OK;
}

/// <summary>
/// Gets the version of the component as a string.
/// </summary>
STDMETHODIMP HostObject::get_componentVersionText(BSTR * versionText)
{
    if (versionText == nullptr)
        return E_INVALIDARG;

    *versionText = ::SysAllocString(TEXT(STR_COMPONENT_VERSION));

    return S_OK;
}

/// <summary>
/// Prints the specified text on the foobar2000 console.
/// </summary>
STDMETHODIMP HostObject::print(BSTR text)
{
    if (text == nullptr)
        return E_INVALIDARG;

    console::print(pfc::utf8FromWide((const wchar_t *) text).c_str());

    return S_OK;
}

/// <summary>
/// Stops playback.
/// </summary>
STDMETHODIMP HostObject::stop()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->stop();

    return S_OK;
}

/// <summary>
/// Starts playback, paused or unpaused. If playback is already active, existing process is stopped first.
/// </summary>
STDMETHODIMP HostObject::play(VARIANT_BOOL paused)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_play, (bool) paused);

    return S_OK;
}

/// <summary>
/// Pauses or resumes playback.
/// </summary>
STDMETHODIMP HostObject::pause(VARIANT_BOOL paused)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->pause((bool) paused);

    return S_OK;
}

/// <summary>
/// Plays the previous track from the current playlist according to the current playback order.
/// </summary>
STDMETHODIMP HostObject::previous()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_prev);

    return S_OK;
}

/// <summary>
/// Plays the next track from the current playlist according to the current playback order.
/// </summary>
STDMETHODIMP HostObject::next()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_next);

    return S_OK;
}

/// <summary>
/// Plays a random track from the current playlist (aka Shuffle).
/// </summary>
STDMETHODIMP HostObject::random()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_rand);

    return S_OK;
}

/// <summary>
/// Toggles the pause status.
/// </summary>
STDMETHODIMP HostObject::togglePause()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->pause(!_PlaybackControl->is_paused());

    return S_OK;
}

/// <summary>
/// Toggles playback mute state.
/// </summary>
STDMETHODIMP HostObject::toggleMute()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->volume_mute_toggle();

    return S_OK;
}

/// <summary>
/// Toggles the stop-after-current mode.
/// </summary>
STDMETHODIMP HostObject::toggleStopAfterCurrent()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->set_stop_after_current(!_PlaybackControl->get_stop_after_current());

    return S_OK;
}

/// <summary>
/// Increases the volume with one step.
/// </summary>
STDMETHODIMP HostObject::volumeUp()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->volume_up();

    return S_OK;
}

/// <summary>
/// Decreases the volume with one step.
/// </summary>
STDMETHODIMP HostObject::volumeDown()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->volume_down();

    return S_OK;
}

/// <summary>
/// Seeks in the currently playing track to the specified time, in seconds.
/// </summary>
STDMETHODIMP HostObject::seek(double time)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->playback_seek(time);

    return S_OK;
}

/// <summary>
/// Seeks in the currently playing track forward or backwards by the specified delta time, in seconds.
/// </summary>
STDMETHODIMP HostObject::seekDelta(double delta)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->playback_seek_delta(delta);

    return S_OK;
}

/// <summary>
/// Gets whether playback is active.
/// </summary>
STDMETHODIMP HostObject::get_isPlaying(VARIANT_BOOL * isPlaying)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (isPlaying == nullptr)
        return E_INVALIDARG;

    *isPlaying = _PlaybackControl->is_playing();

    return S_OK;
}

/// <summary>
/// Gets whether playback is active and in paused state.
/// </summary>
STDMETHODIMP HostObject::get_isPaused(VARIANT_BOOL * isPaused)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (isPaused == nullptr)
        return E_INVALIDARG;

    *isPaused = _PlaybackControl->is_paused();

    return S_OK;
}

/// <summary>
/// Gets the stop-after-current-track option state.
/// </summary>
STDMETHODIMP HostObject::get_stopAfterCurrent(VARIANT_BOOL * stopAfterCurrent)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (stopAfterCurrent == nullptr)
        return E_INVALIDARG;

    *stopAfterCurrent = _PlaybackControl->get_stop_after_current();

    return S_OK;
}

/// <summary>
/// Sets the stop-after-current-track option state.
/// </summary>
STDMETHODIMP HostObject::put_stopAfterCurrent(VARIANT_BOOL stopAfterCurrent)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->set_stop_after_current((bool) stopAfterCurrent);

    return S_OK;
}

/// <summary>
/// Gets the length of the currently playing item, in seconds.
/// </summary>
STDMETHODIMP HostObject::get_length(double * length)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (length == nullptr)
        return E_INVALIDARG;

    *length = _PlaybackControl->playback_get_length_ex();

    return S_OK;
}

/// <summary>
/// Gets the playback position within the currently playing track, in seconds.
/// </summary>
STDMETHODIMP HostObject::get_position(double * position)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (position == nullptr)
        return E_INVALIDARG;

    *position = _PlaybackControl->playback_get_position();

    return S_OK;
}

/// <summary>
/// Gets whether currently played track is seekable. If it's not, playback_seek/playback_seek_delta calls will be ignored.
/// </summary>
STDMETHODIMP HostObject::get_canSeek(VARIANT_BOOL * canSeek)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (canSeek == nullptr)
        return E_INVALIDARG;

    *canSeek = _PlaybackControl->playback_can_seek();

    return S_OK;
}

/// <summary>
/// Gets the playback volume.
/// </summary>
STDMETHODIMP HostObject::get_volume(double * volume)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (volume == nullptr)
        return E_INVALIDARG;

    *volume = (double) _PlaybackControl->get_volume();

    return S_OK;
}

/// <summary>
/// Sets the playback volume.
/// </summary>
STDMETHODIMP HostObject::put_volume(double volume)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->set_volume((float) volume);

    return S_OK;
}

/// <summary>
/// Gets whether playback is muted.
/// </summary>
STDMETHODIMP HostObject::get_isMuted(VARIANT_BOOL * isMuted)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    if (isMuted == nullptr)
        return E_INVALIDARG;

    *isMuted = _PlaybackControl->is_muted();

    return S_OK;
}

/// <summary>
/// Gets the interpreted version of the specified text containing Title Formating instructions.
/// </summary>
STDMETHODIMP HostObject::getFormattedText(BSTR text, BSTR * formattedText)
{
    if ((text == nullptr) || (formattedText == nullptr))
        return E_INVALIDARG;

    t_size PlaylistIndex = ~0u;
    t_size ItemIndex = ~0u;

    GetTrackIndex(PlaylistIndex, ItemIndex);

    titleformat_object::ptr FormatObject;
    pfc::string8 Text = pfc::utf8FromWide(text);

    bool Success = titleformat_compiler::get()->compile(FormatObject, Text);

    if (!Success)
    {
        console::printf(STR_COMPONENT_NAME " failed to compile \"%s\".", Text.c_str());

        return E_INVALIDARG;
    }

    static_api_ptr_t<playlist_manager> PlaylistManager;
    pfc::string8 FormattedText;

    PlaylistManager->playlist_item_format_title(PlaylistIndex, ItemIndex, nullptr, FormattedText, FormatObject, nullptr, playback_control::t_display_level::display_level_all);

    *formattedText = ::SysAllocString(pfc::wideFromUTF8(FormattedText).c_str());

    return S_OK;
}

#pragma endregion

#pragma region Auto Playlists

/// <summary>
/// Creates a new auto playlist at the specified index.
/// </summary>
STDMETHODIMP HostObject::createAutoPlaylist(int playlistIndex, BSTR name, BSTR query, BSTR sort, uint32_t flags, int * newPlaylistIndex)
{
    HRESULT hr = createPlaylist(playlistIndex, name, newPlaylistIndex);

    if (!SUCCEEDED(hr))
        return hr;

    try
    {
        pfc::string Query = pfc::utf8FromWide(query).c_str();
        pfc::string Sort = pfc::utf8FromWide(sort).c_str();

        autoplaylist_manager::get()->add_client_simple(Query.c_str(), Sort.c_str(), (size_t) *newPlaylistIndex, flags);

        return S_OK;
    }
    catch (const pfc::exception &)
    {
        playlist_manager::get()->remove_playlist((size_t) *newPlaylistIndex);

        return E_FAIL;
    }
}

STDMETHODIMP HostObject::isAutoPlaylist(int playlistIndex, VARIANT_BOOL * result)
{
    if (result == nullptr)
        return E_INVALIDARG;

    if (playlistIndex == -1)
        playlistIndex = (int) playlist_manager_v4::get()->get_active_playlist();

    *result = autoplaylist_manager::get()->is_client_present((size_t) playlistIndex);

    return S_OK;
}

#pragma endregion

#pragma region Playback Order

/// <summary>
/// Gets the playback order (0 = default, 1 = repeat playlist, 2 = repeat track, 3 = random, 4 = shuffle tracks, 5 = shuffle albums, 6 = shuffle folders).
/// </summary>
STDMETHODIMP HostObject::get_playbackOrder(int * playlistIndex)
{
    if (playlistIndex == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager::get();

    *playlistIndex = (int) Manager->playback_order_get_active();

    return S_OK;
}

/// <summary>
/// Sets the playback order (0 = default, 1 = repeat playlist, 2 = repeat track, 3 = random, 4 = shuffle tracks, 5 = shuffle albums, 6 = shuffle folders).
/// </summary>
STDMETHODIMP HostObject::put_playbackOrder(int playlistIndex)
{
    auto Manager = playlist_manager::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->playback_order_set_active((size_t) playlistIndex);

    return S_OK;
}

#pragma endregion

#pragma region IDispatch

/// <summary>
/// Retrieves the number of type information interfaces that an object provides (either 0 or 1).
/// </summary>
STDMETHODIMP HostObject::GetTypeInfoCount(UINT * typeInfoCount)
{
    *typeInfoCount = 1;

    return S_OK;
}

/// <summary>
/// Retrieves the type information for an object, which can then be used to get the type information for an interface.
/// </summary>
STDMETHODIMP HostObject::GetTypeInfo(UINT typeInfoIndex, LCID lcid, ITypeInfo ** typeInfo)
{
    if (typeInfoIndex != 0)
        return TYPE_E_ELEMENTNOTFOUND;

    if (_TypeLibrary == nullptr)
    {
        std::wstring TypeLibFilePath;

        HRESULT hr = GetTypeLibFilePath(TypeLibFilePath);

        if (!SUCCEEDED(hr))
            return hr;

        hr = ::LoadTypeLib(TypeLibFilePath.c_str(), &_TypeLibrary);

        if (!SUCCEEDED(hr))
            return hr;
    }

    return _TypeLibrary->GetTypeInfoOfGuid(__uuidof(IHostObject), typeInfo);
}

/// <summary>
/// Maps a single member and an optional set of argument names to a corresponding set of integer DISPIDs, which can be used on subsequent calls to Invoke.
/// </summary>
STDMETHODIMP HostObject::GetIDsOfNames(REFIID riid, LPOLESTR * names, UINT nameCount, LCID lcid, DISPID * dispIds)
{
    wil::com_ptr<ITypeInfo> TypeInfo;

    HRESULT hr = GetTypeInfo(0, lcid, &TypeInfo);

    if (!SUCCEEDED(hr))
        return hr;

    return TypeInfo->GetIDsOfNames(names, nameCount, dispIds);
}

/// <summary>
/// Provides access to the properties and methods exposed by this object.
/// </summary>
STDMETHODIMP HostObject::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * dispParams, VARIANT * result, EXCEPINFO * excepInfo, UINT * argErr)
{
    wil::com_ptr<ITypeInfo> TypeInfo;

    HRESULT hr = GetTypeInfo(0, lcid, &TypeInfo);

    if (!SUCCEEDED(hr))
        return hr;

    return TypeInfo->Invoke(this, dispIdMember, flags, dispParams, result, excepInfo, argErr);
}

/// <summary>
/// Gets the complete file path of our type library.
/// </summary>
HRESULT HostObject::GetTypeLibFilePath(std::wstring & filePath) noexcept
{
    HMODULE hModule = GetCurrentModule();

    if (hModule == NULL)
        return HRESULT_FROM_WIN32(::GetLastError());

    wchar_t FilePath[MAX_PATH];

    if (::GetModuleFileNameW(hModule, FilePath, _countof(FilePath)) == 0)
        return HRESULT_FROM_WIN32(::GetLastError());
/*
    HRESULT hr = ::PathCchRemoveFileSpec(FilePath, _countof(FilePath));

    if (!SUCCEEDED(hr))
        return hr;

    ::PathCchAppend(FilePath, _countof(FilePath), TEXT(STR_COMPONENT_BASENAME) L".tlb");

    if (!SUCCEEDED(hr))
        return hr;
*/
    filePath = FilePath;

    return S_OK;
}

#pragma endregion

/// <summary>
/// Gets the index of the active playlist and the focused item, taking into account the user preferences.
/// </summary>
HRESULT HostObject::GetTrackIndex(size_t & playlistIndex, size_t & itemIndex) noexcept
{
    auto SelectionManager = ui_selection_manager::get();

    auto SelectionType = SelectionManager->get_selection_type();

    // Description as used in the Preferences dialog (Display / Selection Viewers).
    const bool PreferCurrentlyPlayingTrack = (SelectionType == contextmenu_item::caller_now_playing);
    const bool PreferCurrentSelection      = (SelectionType == contextmenu_item::caller_active_playlist_selection);

    bool IsTrackPlaying = false;

    static_api_ptr_t<playlist_manager> PlaylistManager;

    if (PreferCurrentlyPlayingTrack)
        IsTrackPlaying = PlaylistManager->get_playing_item_location(&playlistIndex, &itemIndex);

    if (PreferCurrentSelection || !IsTrackPlaying)
    {
        playlistIndex = PlaylistManager->get_active_playlist();

        if (playlistIndex == ~0u)
            return E_FAIL;

        itemIndex = PlaylistManager->playlist_get_focus_item(playlistIndex);

        if (itemIndex == ~0u)
            return E_FAIL;
    }

    return S_OK;
}

/// <summary>
/// Converts the specified data to a JavaScript data URI.
/// </summary>
void ToBase64(const BYTE * data, DWORD size, BSTR * base64)
{
    // Convert the binary image data into a JavaScript data URI.
    const WCHAR * MIMEType = nullptr;

    const BYTE * p = data;

    // Determine the MIME type of the image data.
    if ((size > 2) && p[0] == 0xFF && p[1] == 0xD8)
        MIMEType = L"image/jpeg";
    else
    if ((size > 15) && (p[0] == 'R' && p[1] == 'I' && p[2] == 'F' && p[3] == 'F') && (::memcmp(p + 8, "WEBPVP8", 7) == 0))
        MIMEType = L"image/webp";
    else
    if ((size > 4) && p[0] == 0x89 && p[1] == 0x50 && p[2] == 0x4E && p[3] == 0x47)
        MIMEType = L"image/png";
    else
    if ((size > 3) && p[0] == 0x47 && p[1] == 0x49 && p[2] == 0x46)
        MIMEType = L"image/gif";

    if (MIMEType == nullptr)
        return;

    // Convert the image data to base64.
    const DWORD Flags = CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF;

    DWORD Size = 0;

    if (!::CryptBinaryToStringW(p, size, Flags, nullptr, &Size))
        return;

    Size += 16 + (DWORD) ::wcslen(MIMEType);

    WCHAR * Base64 = new WCHAR[Size];

    if (Base64 == nullptr)
        return;

    ::swprintf_s(Base64, Size, L"data:%s;base64,", MIMEType);

    // Create the result.
    if (::CryptBinaryToStringW(p, size, Flags, Base64 + ::wcslen(Base64), &Size))
    {
        ::SysFreeString(*base64); // Free the empty string.

        *base64 = ::SysAllocString(Base64);
    }

    delete[] Base64;
}

/// <summary>
/// Escapes all restricted JSON characters in a string.
/// </summary>
const std::string Stringify(const char * s)
{
    std::string t;

    t.reserve(::strlen(s));

    for (; *s; ++s)
    {
        switch (*s)
        {
            case '"':  t += '\\'; t += '\"'; break;
            case '\\': t += '\\'; t += '\\'; break;
            case '\b': t += '\\'; t += 'b'; break;
            case '\t': t += '\\'; t += 't'; break;
            case '\n': t += '\\'; t += 'n'; break;
            case '\f': t += '\\'; t += 'f'; break;
            case '\r': t += '\\'; t += 'r'; break;
            default:
                if ('\x00' <= *s && *s <= '\x1f')
                    t += ::FormatText("\\u04x", *s).c_str();
                else
                    t += *s;
        }
    }

    return t;
}

/// <summary>
/// Converts a metadb handle list to a JSON string.
/// </summary>
std::wstring ToJSON(const metadb_handle_list & hItems)
{
    std::wstring Result = L"[";
    bool IsFirstItem = true;

    for (const auto & hItem : hItems)
    {
        if (!IsFirstItem)
            Result.append(L",");

        const playable_location & Location = hItem->get_location();

        std::string Path = Stringify(Location.get_path());

        Result.append(::FormatText(LR"({"path": "%s", "subsong": %u})", ::UTF8ToWide(Path).c_str(), Location.get_subsong_index()).c_str());

        IsFirstItem = false;
    }

    Result.append(L"]");

    return Result;
}
