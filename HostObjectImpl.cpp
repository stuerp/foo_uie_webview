
/** $VER: HostObjectImpl.cpp (2024.07.09) P. Stuer **/

#include "pch.h"

#include "HostObjectImpl.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#include "Support.h"
#include "Resources.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/ui.h>
#include <SDK/contextmenu.h>
#include <SDK/album_art.h>

#include <pfc/string-conv-lite.h>

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
STDMETHODIMP HostObject::get_ComponentVersion(__int32 * version)
{
    if (version == nullptr)
        return E_INVALIDARG;

    *version = (NUM_PRODUCT_MAJOR << 24) | (NUM_PRODUCT_MINOR << 16) | (NUM_PRODUCT_PATCH << 8) | NUM_PRODUCT_PRERELEASE;

    return S_OK;
}

/// <summary>
/// Gets the version of the component as a string.
/// </summary>
STDMETHODIMP HostObject::get_ComponentVersionText(BSTR * versionText)
{
    if (versionText == nullptr)
        return E_INVALIDARG;

    *versionText = ::SysAllocString(TEXT(STR_COMPONENT_VERSION));

    return S_OK;
}

/// <summary>
/// Prints the specified text on the foobar2000 console.
/// </summary>
STDMETHODIMP HostObject::Print(BSTR text)
{
    if (text == nullptr)
        return E_INVALIDARG;

    console::print(pfc::utf8FromWide((const wchar_t *) text).c_str());

    return S_OK;
}

/// <summary>
/// Stops playback.
/// </summary>
STDMETHODIMP HostObject::Stop()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->stop();

    return S_OK;
}

/// <summary>
/// Starts playback, paused or unpaused. If playback is already active, existing process is stopped first.
/// </summary>
STDMETHODIMP HostObject::Play(BOOL paused)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_play, paused);

    return S_OK;
}

/// <summary>
/// Pauses or resumes playback.
/// </summary>
STDMETHODIMP HostObject::Pause(BOOL paused)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->pause(paused);

    return S_OK;
}

/// <summary>
/// Plays the previous track from the current playlist according to the current playback order.
/// </summary>
STDMETHODIMP HostObject::Previous()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_prev);

    return S_OK;
}

/// <summary>
/// Plays the next track from the current playlist according to the current playback order.
/// </summary>
STDMETHODIMP HostObject::Next()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_next);

    return S_OK;
}

/// <summary>
/// Plays a random track from the current playlist (aka Shuffle).
/// </summary>
STDMETHODIMP HostObject::Random()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->start(playback_control::track_command_rand);

    return S_OK;
}

/// <summary>
/// Toggles the pause status.
/// </summary>
STDMETHODIMP HostObject::TogglePause()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->pause(!_PlaybackControl->is_paused());

    return S_OK;
}

/// <summary>
/// Toggles playback mute state.
/// </summary>
STDMETHODIMP HostObject::ToggleMute()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->volume_mute_toggle();

    return S_OK;
}

/// <summary>
/// Toggles the stop-after-current mode.
/// </summary>
STDMETHODIMP HostObject::ToggleStopAfterCurrent()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->set_stop_after_current(!_PlaybackControl->get_stop_after_current());

    return S_OK;
}

/// <summary>
/// Increases the volume with one step.
/// </summary>
STDMETHODIMP HostObject::VolumeUp()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->volume_up();

    return S_OK;
}

/// <summary>
/// Decreases the volume with one step.
/// </summary>
STDMETHODIMP HostObject::VolumeDown()
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->volume_down();

    return S_OK;
}

/// <summary>
/// Seeks in the currently playing track to the specified time, in seconds.
/// </summary>
STDMETHODIMP HostObject::Seek(double time)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->playback_seek(time);

    return S_OK;
}

/// <summary>
/// Seeks in the currently playing track forward or backwards by the specified delta time, in seconds.
/// </summary>
STDMETHODIMP HostObject::SeekDelta(double delta)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->playback_seek_delta(delta);

    return S_OK;
}

/// <summary>
/// Gets whether playback is active.
/// </summary>
STDMETHODIMP HostObject::get_IsPlaying(BOOL * isPlaying)
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
STDMETHODIMP HostObject::get_IsPaused(BOOL * isPaused)
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
STDMETHODIMP HostObject::get_StopAfterCurrent(BOOL * stopAfterCurrent)
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
STDMETHODIMP HostObject::put_StopAfterCurrent(BOOL stopAfterCurrent)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->set_stop_after_current(stopAfterCurrent);

    return S_OK;
}

/// <summary>
/// Gets the length of the currently playing item, in seconds.
/// </summary>
STDMETHODIMP HostObject::get_Length(double * length)
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
STDMETHODIMP HostObject::get_Position(double * position)
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
STDMETHODIMP HostObject::get_CanSeek(BOOL * canSeek)
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
STDMETHODIMP HostObject::get_Volume(double * volume)
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
STDMETHODIMP HostObject::put_Volume(double volume)
{
    if (_PlaybackControl == nullptr)
        return E_UNEXPECTED;

    _PlaybackControl->set_volume((float) volume);

    return S_OK;
}

/// <summary>
/// Gets whether playback is muted.
/// </summary>
STDMETHODIMP HostObject::get_IsMuted(BOOL * isMuted)
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
STDMETHODIMP HostObject::GetFormattedText(BSTR text, BSTR * formattedText)
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

/// <summary>
/// Gets the index of the active playlist and the focused item, taking into account the user preferences.
/// </summary>
HRESULT HostObject::GetTrackIndex(t_size & playlistIndex, t_size & itemIndex) noexcept
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
/// Gets the specified artwork of the currently selected item in the current playlist.
/// </summary>
STDMETHODIMP HostObject::GetArtwork(BSTR type, BSTR * image)
{
    if (type == nullptr)
        return E_INVALIDARG;
/*
    auto aanm = now_playing_album_art_notify_manager::get();

    if (aanm != nullptr)
    {
        album_art_data_ptr aad = aanm->current();

        if (aad.is_valid())
            hr = _Artwork.Initialize((uint8_t *) aad->data(), aad->size());
    }
*/
    if (::_wcsicmp(type, L"front") == 0)
    {
    }
    else
    if (::_wcsicmp(type, L"back") == 0)
    {
    }
    else
    if (::_wcsicmp(type, L"disc") == 0)
    {
    }
    else
    if (::_wcsicmp(type, L"icon") == 0)
    {
    }
    else
    if (::_wcsicmp(type, L"artist") == 0)
    {
    }
    else
        return E_INVALIDARG;

    const wchar_t * Text = LR"({ "width": 2, "height": 2, "bytes": [ 1, 2, 3, 4 ] })";

    *image = ::SysAllocString(Text);

    return S_OK;
}

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
