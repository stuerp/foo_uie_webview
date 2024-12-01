
/** $VER: UIElement.cpp (2024.12.01) P. Stuer **/

#include "pch.h"

#include "UIElement.h"
#include "UIElementTracker.h"
#include "Encoding.h"
#include "Exceptions.h"
#include "Support.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/playback_control.h>
#include <SDK/ui.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
UIElement::UIElement() : m_bMsgHandled(FALSE)
{
    _PlaybackControl = playback_control::get();

    playlist_manager::get()->register_callback(this, (t_uint32) flag_all);
}

/// <summary>
/// Deletes this instance.
/// </summary>
UIElement::~UIElement()
{
    playlist_manager::get()->unregister_callback(this);
}

#pragma region User Interface

/// <summary>
/// Creates the window.
/// </summary>
LRESULT UIElement::OnCreate(LPCREATESTRUCT cs) noexcept
{
    _UIElementTracker.Add(this);

    std::wstring WebViewVersion;

    if (!GetWebViewVersion(WebViewVersion))
    {
        console::print(STR_COMPONENT_BASENAME " failed to find a compatible WebView component.");

        return 1;
    }

    console::printf(STR_COMPONENT_BASENAME " is using WebView %s.", WideToUTF8(WebViewVersion).c_str());

    _HostObject = Microsoft::WRL::Make<HostObject>
    (
        [this](std::function<void (void)> callback)
        {
            RunAsync(callback);
        }
    );

    Initialize();

    HRESULT hr = CreateWebView();

    if (!SUCCEEDED(hr))
        console::print(STR_COMPONENT_BASENAME " failed to create WebView control.");

    InitializeFileWatcher();

    // Create the visualisation stream.
    try
    {
        static_api_ptr_t<visualisation_manager> VisualisationManager;

        VisualisationManager->create_stream(_VisualisationStream, visualisation_manager::KStreamFlagNewFFT);

        _VisualisationStream->set_channel_mode(visualisation_stream_v2::channel_mode_default);
    }
    catch (std::exception &)
    {
        console::print(STR_COMPONENT_BASENAME " failed to create visualisation stream.");
    }

    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void UIElement::OnDestroy() noexcept
{
    StopTimer();

    _VisualisationStream.release();

    _FileWatcher.Stop();

    DeleteWebView();

    _HostObject = nullptr;

    _UIElementTracker.Remove(this);
}

/// <summary>
/// Handles the WM_SIZE message.
/// </summary>
void UIElement::OnSize(UINT type, CSize size) noexcept
{
    if (_Controller == nullptr)
        return;

    RECT Bounds;

    ::GetClientRect(m_hWnd, &Bounds);

    _Controller->put_Bounds(Bounds);
}

/// <summary>
/// Handles the WM_ERASEBKGND message.
/// </summary>
BOOL UIElement::OnEraseBackground(CDCHandle dc) noexcept
{
    RECT cr;

    GetClientRect(&cr);

    HBRUSH Brush = ::CreateSolidBrush(_BackgroundColor);

    ::FillRect(dc, &cr, Brush);

    ::DeleteObject(Brush);

    return TRUE;
}

/// <summary>
/// Handles the WM_PAINT message.
/// </summary>
void UIElement::OnPaint(CDCHandle dc) noexcept
{
    PAINTSTRUCT ps = { };

    BeginPaint(&ps);

    RECT cr;

    GetClientRect(&cr);

    HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_TEXTSTYLE);

    const DWORD Format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;

    if (hTheme != NULL)
    {
        DTTOPTS Options = { sizeof(Options) };

        Options.dwFlags = DTT_TEXTCOLOR;
        Options.crText = _ForegroundColor;

        ::DrawThemeTextEx(hTheme, ps.hdc, TEXT_BODYTEXT, 0, _Configuration._Name.c_str(), -1, Format, &cr, &Options);

        ::CloseThemeData(hTheme);
    }
    else
        ::DrawTextW(ps.hdc, _Configuration._Name.c_str(), -1, &cr, (UINT) Format);

    EndPaint(&ps);
}

/// <summary>
/// Handles a change to the template. Either the path name or the content changed.
/// </summary>
LRESULT UIElement::OnTemplateChanged(UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    try
    {
        InitializeWebView();
    }
    catch (std::exception & e)
    {
        console::error(e.what());
    }

    return 0;
}

/// <summary>
/// The WebView is ready.
/// </summary>
LRESULT UIElement::OnWebViewReady(UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    try
    {
        SetWebViewVisibility(IsWebViewVisible()); // Work-around for WebView not appearing after foobar2000 starts while being hosted in a hidden tab.

        InitializeWebView();
    }
    catch (std::exception & e)
    {
        console::error(e.what());
    }

    return 0;
}

/// <summary>
/// Handles a notification from the Preferences page that the template file path has changed.
/// </summary>
void UIElement::OnConfigurationChanged() noexcept
{
    _ExpandedTemplateFilePath = GetTemplateFilePath();

    try
    {
        InitializeFileWatcher();
        InitializeWebView();
    }
    catch (std::exception & e)
    {
        console::error(e.what());
    }
}

/// <summary>
/// Handles an async method call.
/// </summary>
LRESULT UIElement::OnAsync(UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    auto * task = reinterpret_cast<std::function<void()>*>(wParam);

    (*task)();

    delete task;

    return true;
}

/// <summary>
/// Handles a change of the user interface colors.
/// </summary>
void UIElement::OnColorsChanged()
{
    GetColors();

    RecreateWebView(); // There is no way (yet) to update the environment options of an existing WebView2 control so we delete and recreate it.

    if (IsWindow())
        Invalidate(TRUE);
}

/// <summary>
/// Initializes the component.
/// </summary>
void UIElement::Initialize()
{
    {
        if (!::PathFileExistsW(_Configuration._UserDataFolderPath.c_str()))
        {
            // Create the user data directory.
            if (!::CreateDirectoryW(_Configuration._UserDataFolderPath.c_str(), nullptr))
                console::print(::GetErrorMessage(::GetLastError(), ::FormatText(STR_COMPONENT_BASENAME " failed to create user data folder \"%s\"", ::WideToUTF8(_Configuration._UserDataFolderPath).c_str())).c_str());
        }
    }

    {
        _ExpandedTemplateFilePath = GetTemplateFilePath();

        if (::PathFileExistsW(_ExpandedTemplateFilePath.c_str()))
            return;
    }

    // Create a default template file.
    {
        wchar_t DefaultFilePath[MAX_PATH];

        HMODULE hModule = GetCurrentModule();

        if (hModule == NULL)
            return;

        if (::GetModuleFileNameW(hModule, DefaultFilePath, _countof(DefaultFilePath)) == 0)
            return;

        HRESULT hr = ::PathCchRemoveFileSpec(DefaultFilePath, _countof(DefaultFilePath));

        if (!SUCCEEDED(hr))
            return;

        hr = ::PathCchAppend(DefaultFilePath, _countof(DefaultFilePath), L"Default-Template.html");

        if (!SUCCEEDED(hr))
            return;

        if (!::CopyFileW(DefaultFilePath, _ExpandedTemplateFilePath.c_str(), TRUE))
            console::print(::GetErrorMessage(::GetLastError(), ::FormatText(STR_COMPONENT_BASENAME " failed to create default template file \"%s\"", ::WideToUTF8(_ExpandedTemplateFilePath).c_str())).c_str());
    }
}

/// <summary>
/// Initializes the file watcher.
/// </summary>
void UIElement::InitializeFileWatcher()
{
    _FileWatcher.Stop();

    try
    {
        _FileWatcher.Start(m_hWnd, _ExpandedTemplateFilePath);
    }
    catch (std::exception & e)
    {
        console::print(::FormatText(STR_COMPONENT_BASENAME " failed to start file system watcher: %s", e.what()).c_str());
    }
}

/// <summary>
/// Initializes the WebView.
/// </summary>
void UIElement::InitializeWebView()
{
    if (_WebView == nullptr)
        return;

    // Navigate to the template.
    _IsNavigationCompleted = false;

    HRESULT hr = _WebView->Navigate(_ExpandedTemplateFilePath.c_str());

    if (!SUCCEEDED(hr))
    {
        console::print(::GetErrorMessage(hr, ::FormatText(STR_COMPONENT_BASENAME " failed to navigate to template \"%s\"", ::WideToUTF8(_ExpandedTemplateFilePath).c_str())).c_str());

        hr = _WebView->Navigate(L"about:blank");

        if (!SUCCEEDED(hr))
            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to navigate to about:blank").c_str());
    }

    on_playback_new_track(nullptr);
}

/// <summary>
/// Shows or hides the WebView.
/// </summary>
void UIElement::SetWebViewVisibility(bool visible) noexcept
{
    // Hack: Don't use put_IsVisible(). It hides the host window. Reduce the WebView's size if we need to show the client area 'behind' the WebView f.e. in Layout Edit mode.
    RECT cr = { };

    if (visible)
        GetClientRect(&cr);

    if (_Controller != nullptr)
        _Controller->put_Bounds(cr);

    if (visible)
        on_playback_new_track(nullptr); // Forces a refresh when the WebView becomes visible again e.g. after exiting Layout Edit mode.
    else
        InvalidateRect(nullptr, TRUE);
}

/// <summary>
/// Gets the template file path with all environment variables expanded.
/// </summary>
std::wstring UIElement::GetTemplateFilePath() const noexcept
{
    wchar_t FilePath[MAX_PATH];

    if (::ExpandEnvironmentStringsW(_Configuration._TemplateFilePath.c_str(), FilePath, _countof(FilePath)) == 0)
        ::wcscpy_s(FilePath, _countof(FilePath), _Configuration._TemplateFilePath.c_str());

    return std::wstring(FilePath);
}

/// <summary>
/// Shows the preferences page.
/// </summary>
void UIElement::ShowPreferences() noexcept
{
    _UIElementTracker.SetCurrentElement(this);

    static constexpr GUID _GUID = GUID_PREFERENCES;

    static_api_ptr_t<ui_control> uc;

    uc->show_preferences(_GUID);
}

/// <summary>
/// Gets the window class definition.
/// </summary>
CWndClassInfo & UIElement::GetWndClassInfo()
{
    static ATL::CWndClassInfoW wci =
    {
        {
            sizeof(WNDCLASSEX),
            CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
            StartWindowProc,
            0, 0,
            NULL, // Instance,
            NULL, // Icon
            NULL, // Cursor
            NULL, // Background brush
            NULL, // Menu
            TEXT(STR_WINDOW_CLASS_NAME), // Class name
            NULL // Small Icon
        },
        NULL, NULL, IDC_ARROW, TRUE, 0, L""
    };

    return wci;
}

#pragma endregion

#pragma region play_callback_impl_base

/// <summary>
/// Called when playback is being initialized.
/// </summary>
void UIElement::on_playback_starting(play_control::t_track_command command, bool paused)
{
    if (_WebView == nullptr)
        return;

    static const wchar_t * CommandName = L"Unknown";

    if (command == play_control::t_track_command::track_command_play) CommandName = L"Play"; else
    if (command == play_control::t_track_command::track_command_next) CommandName = L"Next"; else       // Plays the next track from the current playlist according to the current playback order.
    if (command == play_control::t_track_command::track_command_prev) CommandName = L"Prev"; else       // Plays the previous track from the current playlist according to the current playback order.
    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Random"; else     // Plays a random track from the current playlist.

    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Set track"; else  // For internal use only, do not use.
    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Resume";          // For internal use only, do not use.

    HRESULT hr = _WebView->ExecuteScript(::FormatText(L"OnPlaybackStarting(\"%s\", %s)", CommandName, (paused ? L"true" : L"false")).c_str(), nullptr);

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to call OnPlaybackStarting()").c_str());
}

/// <summary>
/// Called when playback advances to a new track.
/// </summary>
void UIElement::on_playback_new_track(metadb_handle_ptr /*track*/)
{
    if (_WebView == nullptr)
        return;

    HRESULT hr = _WebView->ExecuteScript(L"OnPlaybackNewTrack()", nullptr);

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to call OnPlaybackNewTrack()").c_str());

    _LastPlaybackTime = 0.;
    _SampleRate = 44100; // Temporary until we get the sample rate from the chunk.

    StartTimer();
}

/// <summary>
/// Called when playback stops.
/// </summary>
void UIElement::on_playback_stop(play_control::t_stop_reason reason)
{
    StopTimer();

    _LastPlaybackTime = 0.;

    if (_WebView == nullptr)
        return;

    static const wchar_t * Reason = L"unknown";

    if (reason == play_control::t_stop_reason::stop_reason_user)                Reason = L"User"; else
    if (reason == play_control::t_stop_reason::stop_reason_eof)                 Reason = L"EOF"; else
    if (reason == play_control::t_stop_reason::stop_reason_starting_another)    Reason = L"Starting another"; else
    if (reason == play_control::t_stop_reason::stop_reason_shutting_down)       Reason = L"Shutting down";

    HRESULT hr = _WebView->ExecuteScript(::FormatText(L"OnPlaybackStop(\"%s\")", Reason).c_str(), nullptr);

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to call OnPlaybackStop()").c_str());
}

/// <summary>
/// Called when the user seeks to a specific time.
/// </summary>
void UIElement::on_playback_seek(double time)
{
    if (_WebView == nullptr)
        return;

    HRESULT hr = _WebView->ExecuteScript(::FormatText(L"OnPlaybackSeek(%f)", time).c_str(), nullptr);

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to call OnPlaybackSeek()").c_str());
}

/// <summary>
/// Called when playback pauses or resumes.
/// </summary>
void UIElement::on_playback_pause(bool paused)
{
    const std::wstring Script = ::FormatText(L"OnPlaybackPause(%s)", (paused ? L"true" : L"false"));

    ExecuteScript(Script);
}

/// <summary>
/// Called when the currently played file gets edited.
/// </summary>
void UIElement::on_playback_edited(metadb_handle_ptr hTrack)
{
    const std::wstring Script = L"OnPlaybackEdited()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when dynamic info (VBR bitrate etc...) changes.
/// </summary>
void UIElement::on_playback_dynamic_info(const file_info & fileInfo)
{
    const std::wstring Script = L"OnPlaybackDynamicInfo()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when the per-track dynamic info (stream track titles etc...) change. Happens less often than on_playback_dynamic_info().
/// </summary>
void UIElement::on_playback_dynamic_info_track(const file_info & fileInfo)
{
    const std::wstring Script = L"OnPlaybackDynamicTrackInfo()";

    ExecuteScript(Script);
}

/// <summary>
/// Called, every second, for time display.
/// </summary>
void UIElement::on_playback_time(double time)
{
    const std::wstring Script = ::FormatText(L"OnPlaybackTime(%f)", time);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the user changes the volume.
/// </summary>
void UIElement::on_volume_change(float newValue) // in dBFS
{
    const std::wstring Script = ::FormatText(L"OnVolumeChange(%f)", (double) newValue);

    ExecuteScript(Script);
}

#pragma endregion

#pragma region playlist_callback_single

/// <summary>
/// Called when items have been added to the active playlist.
/// </summary>
void UIElement::on_items_added(t_size playlistIndex, t_size startIndex, metadb_handle_list_cref data, const bit_array & selection)
{
    const std::wstring Script = ::FormatText(L"OnAddedPlaylistItems(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the items of the specified playlist have been reordered.
/// </summary>
void UIElement::on_items_reordered(t_size playlistIndex, const t_size * order, t_size count)
{
    const std::wstring Script = ::FormatText(L"OnReorderedPlaylistItems(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when removing items of the specified playlist.
/// </summary>
void UIElement::on_items_removing(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount)
{
    const std::wstring Script = ::FormatText(L"OnRemovingPlaylistItems(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when items of the active playlist have been removed.
/// </summary>
void UIElement::on_items_removed(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount)
{
    const std::wstring Script = ::FormatText(L"OnRemovedPlaylistItems(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the selected items changed.
/// </summary>
void UIElement::on_items_selection_change(t_size playlistIndex, const bit_array & affectedItems, const bit_array & state)
{
    const std::wstring Script = ::FormatText(L"OnPlaylistSelectedItemsChanged(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the focused item of a playlist changed.
/// </summary>
void UIElement::on_item_focus_change(t_size playlistIndex, t_size fromIndex, t_size toIndex)
{
    const std::wstring Script = ::FormatText(L"OnPlaylistFocusedItemChanged(%d, %d, %d)", (int) playlistIndex, (int) fromIndex, (int) toIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when some playlist items of the specified playlist have been modified.
/// </summary>
void UIElement::on_items_modified(t_size playlistIndex, const bit_array & mask)
{
    const std::wstring Script = ::FormatText(L"OnModifiedPlaylistItems(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when some playlist items of the specified playlist have been modified from playback.
/// </summary>
void UIElement::on_items_modified_fromplayback(t_size playlistIndex, const bit_array & mask, play_control::t_display_level displayLevel)
{
    const std::wstring Script = ::FormatText(L"OnModifiedPlaylistItemsFromPlayback(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when items of the specified playlist have been replaced.
/// </summary>
void UIElement::on_items_replaced(t_size playlistIndex, const bit_array & mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & replacedItems)
{
    const std::wstring Script = ::FormatText(L"OnReplacedPlaylistItems(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

void UIElement::on_item_ensure_visible(t_size playlistIndex, t_size itemIndex)
{
    const std::wstring Script = ::FormatText(L"OnEnsuredPlaylistItemIsVisible(%d, %d)", (int) playlistIndex, (int) itemIndex);

    ExecuteScript(Script);
}

void UIElement::on_playlist_activate(t_size oldPlaylistIndex, t_size newPlaylistIndex)
{
    const std::wstring Script = ::FormatText(L"OnChangedActivePlaylist(%d, %d)", (int) oldPlaylistIndex, (int) newPlaylistIndex);

    ExecuteScript(Script);
}

void UIElement::on_playlist_created(t_size playlistIndex, const char * name, t_size size)
{
    const std::wstring Script = ::FormatText(L"OnCreatedPlaylist(%d, \"%s\")", (int) playlistIndex, UTF8ToWide(name, size).c_str());

    ExecuteScript(Script);
}

void UIElement::on_playlists_reorder(const t_size * order, t_size count)
{
    const std::wstring Script = L"OnReorderedPlaylists()";

    ExecuteScript(Script);
}

void UIElement::on_playlists_removing(const bit_array & mask, t_size oldCount, t_size newCount)
{
    const std::wstring Script = L"OnRemovingPlaylists()";

    ExecuteScript(Script);
}

void UIElement::on_playlists_removed(const bit_array & mask, t_size oldCount, t_size newcount)
{
    const std::wstring Script = L"OnRemovedPlaylists()";

    ExecuteScript(Script);
}

void UIElement::on_playlist_renamed(t_size playlistIndex, const char * name, t_size size)
{
    const std::wstring Script = ::FormatText(L"OnRenamedPlaylist(%d, \"%s\")", (int) playlistIndex, UTF8ToWide(name, size).c_str());

    ExecuteScript(Script);
}

void UIElement::on_playlist_locked(t_size playlistIndex, bool isLocked)
{
    const std::wstring Script = ::FormatText(isLocked ? L"OnLockedPlaylist(%d)" : L"OnUnlockedPlaylist(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

void UIElement::on_default_format_changed()
{
    const std::wstring Script = L"OnChangedDefaultFormat()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when the playback order changed.
/// </summary>
void UIElement::on_playback_order_changed(t_size playbackOrderIndex)
{
    const std::wstring Script = ::FormatText(L"OnChangedPlaybackOrder(%d)", (int) playbackOrderIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Executes a script.
/// </summary>
void UIElement::ExecuteScript(const std::wstring & script) const noexcept
{
    if (_WebView == nullptr)
        return;

    HRESULT hr = _WebView->ExecuteScript(script.c_str(), nullptr);

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, FormatText(STR_COMPONENT_BASENAME " failed to call %s", WideToUTF8(script).c_str())).c_str());
}

#pragma endregion
