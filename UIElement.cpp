
/** $VER: UIElement.cpp (2024.12.02) P. Stuer **/

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

    console::printf(STR_COMPONENT_BASENAME " is using WebView %s.", ::WideToUTF8(WebViewVersion).c_str());

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

    GetClientRect(&Bounds);

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
    static const wchar_t * CommandName = L"Unknown";

    if (command == play_control::t_track_command::track_command_play) CommandName = L"Play"; else
    if (command == play_control::t_track_command::track_command_next) CommandName = L"Next"; else           // Plays the next track from the current playlist according to the current playback order.
    if (command == play_control::t_track_command::track_command_prev) CommandName = L"Prev"; else           // Plays the previous track from the current playlist according to the current playback order.
    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Random"; else         // Plays a random track from the current playlist.

    if (command == play_control::t_track_command::track_command_settrack) CommandName = L"Set track"; else  // For internal use only, do not use.
    if (command == play_control::t_track_command::track_command_resume) CommandName = L"Resume";            // For internal use only, do not use.

    const std::wstring Script = ::FormatText(L"onPlaybackStarting(\"%s\", %s)", CommandName, (paused ? L"true" : L"false"));

    ExecuteScript(Script);
}

/// <summary>
/// Called when playback advances to a new track.
/// </summary>
void UIElement::on_playback_new_track(metadb_handle_ptr /*track*/)
{
    const std::wstring Script = L"onPlaybackNewTrack()";

    ExecuteScript(Script);

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

    static const wchar_t * Reason = L"unknown";

    if (reason == play_control::t_stop_reason::stop_reason_user)                Reason = L"User"; else
    if (reason == play_control::t_stop_reason::stop_reason_eof)                 Reason = L"EOF"; else
    if (reason == play_control::t_stop_reason::stop_reason_starting_another)    Reason = L"Starting another"; else
    if (reason == play_control::t_stop_reason::stop_reason_shutting_down)       Reason = L"Shutting down";

    const std::wstring Script = ::FormatText(L"onPlaybackStop(\"%s\")", Reason);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the user seeks to a specific time.
/// </summary>
void UIElement::on_playback_seek(double time)
{
    const std::wstring Script = ::FormatText(L"onPlaybackSeek(%f)", time);

    ExecuteScript(Script);
}

/// <summary>
/// Called when playback pauses or resumes.
/// </summary>
void UIElement::on_playback_pause(bool paused)
{
    const std::wstring Script = ::FormatText(L"onPlaybackPause(%s)", (paused ? L"true" : L"false"));

    ExecuteScript(Script);
}

/// <summary>
/// Called when the currently played file gets edited.
/// </summary>
void UIElement::on_playback_edited(metadb_handle_ptr hTrack)
{
    const std::wstring Script = L"onPlaybackEdited()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when dynamic info (VBR bitrate etc...) changes.
/// </summary>
void UIElement::on_playback_dynamic_info(const file_info & fileInfo)
{
    const std::wstring Script = L"onPlaybackDynamicInfo()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when the per-track dynamic info (stream track titles etc...) change. Happens less often than on_playback_dynamic_info().
/// </summary>
void UIElement::on_playback_dynamic_info_track(const file_info & fileInfo)
{
    const std::wstring Script = L"onPlaybackDynamicTrackInfo()";

    ExecuteScript(Script);
}

/// <summary>
/// Called, every second, for time display.
/// </summary>
void UIElement::on_playback_time(double time)
{
    const std::wstring Script = ::FormatText(L"onPlaybackTime(%f)", time);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the user changes the volume.
/// </summary>
void UIElement::on_volume_change(float newValue) // in dBFS
{
    const std::wstring Script = ::FormatText(L"onVolumeChange(%f)", (double) newValue);

    ExecuteScript(Script);
}

#pragma endregion
