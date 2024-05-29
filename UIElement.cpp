
/** $VER: UIElement.cpp (2024.05.29) P. Stuer **/

#include "pch.h"

#include "UIElement.h"
#include "Encoding.h"
#include "Exceptions.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/playback_control.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
UIElement::UIElement() : m_bMsgHandled(FALSE)
{
    pfc::string8 ProfilePath = pfc::io::path::combine(core_api::get_profile_path(), STR_COMPONENT_BASENAME);

    if (::_strnicmp(ProfilePath, "file://", 7) == 0)
        _ProfilePath = UTF8ToWide(ProfilePath.subString(7).c_str());

    _UserDataFolderPath = _ProfilePath.c_str();

    _FilePath = GetTemplateFilePath();
}

#pragma region User Interface

/// <summary>
/// Creates the window.
/// </summary>
LRESULT UIElement::OnCreate(LPCREATESTRUCT cs)
{
    _HostObject = Microsoft::WRL::Make<HostObject>
    (
        [this](std::function<void(void)> callback)
        {
            RunAsync(callback);
        }
    );

    CreateWebView();

    InitializeFileWatcher();

    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void UIElement::OnDestroy() noexcept
{
    _FileWatcher.Stop();

    DeleteWebView();
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
/// Handles a change to the template. Either the path name or the content changed.
/// </summary>
LRESULT UIElement::OnTemplateChanged(UINT msg, WPARAM wParam, LPARAM lParam)
{
    InitializeWebView();

    return 0;
}

/// <summary>
/// The WebView is ready.
/// </summary>
LRESULT UIElement::OnWebViewReady(UINT msg, WPARAM wParam, LPARAM lParam)
{
    InitializeWebView();

    return 0;
}

/// <summary>
/// Handles a notification from the Preferences page that the template file path has changed.
/// </summary>
void UIElement::OnTemplateFilePathChanged()
{
    _FilePath = GetTemplateFilePath();

    InitializeFileWatcher();
    InitializeWebView();
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
/// Initializes the file watcher.
/// </summary>
void UIElement::InitializeFileWatcher()
{
    _FileWatcher.Stop();

    try
    {
        _FileWatcher.Start(m_hWnd, _FilePath);
    }
    catch (std::exception & e)
    {
        throw ComponentException(::FormatText("Failed to start file system watcher: %s", e.what()));
    }
}

/// <summary>
/// Initializes the WebView.
/// </summary>
void UIElement::InitializeWebView()
{
    if (_WebView == nullptr)
        return;

    try
    {
        _TemplateText = ReadTemplate(_FilePath);
    }
    catch (...)
    {
        _TemplateText.clear();
    }

    // Navigate to the template content.
    HRESULT hResult = _WebView->NavigateToString(UTF8ToWide(_TemplateText).c_str());

    if (!SUCCEEDED(hResult))
        throw Win32Exception((DWORD) hResult, "Failed to navigate to template content");

    on_playback_new_track(nullptr);
}

/// <summary>
/// Formats the specified text using title formatting.
/// </summary>
bool UIElement::FormatText(const std::string & text, pfc::string & formattedText) noexcept
{
    static_api_ptr_t<playlist_manager> _PlaylistManager;

    t_size PlaylistIndex = ~0u;
    t_size ItemIndex = ~0u;

    if (!_PlaylistManager->get_playing_item_location(&PlaylistIndex, &ItemIndex))
    {
        PlaylistIndex = _PlaylistManager->get_active_playlist();

        if (PlaylistIndex == ~0u)
            return false;

        ItemIndex = _PlaylistManager->playlist_get_focus_item(PlaylistIndex);

        if (ItemIndex == ~0u)
            return false;
    }

    titleformat_object::ptr FormatObject;

    bool Success = titleformat_compiler::get()->compile(FormatObject, text.c_str());

    if (Success)
        _PlaylistManager->playlist_item_format_title(PlaylistIndex, ItemIndex, nullptr, formattedText, FormatObject, nullptr, playback_control::t_display_level::display_level_all);

    return Success;
}

/// <summary>
/// Reads the template text into a string.
/// </summary>
std::string UIElement::ReadTemplate(const std::wstring & filePath)
{
    HANDLE hFile = ::CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
        throw Win32Exception(::FormatText("Failed to open template file \"%s\" for read", ::WideToUTF8(filePath).c_str()));

    DWORD Size = ::GetFileSize(hFile, 0);

    char * Data = new char[Size + 1];

    DWORD BytesRead = 0;

    BOOL Success = ::ReadFile(hFile, Data, Size, &BytesRead, 0);

    if (!Success)
    {
        DWORD LastError = ::GetLastError();

        ::CloseHandle(hFile);

        throw Win32Exception(LastError, ::FormatText("Failed to read template file \"%s\"", ::WideToUTF8(filePath).c_str()));
    }

    ::CloseHandle(hFile);

    if (BytesRead == 0)
        return std::string();

    Data[BytesRead] = '\0';
 
    std::string Text;
    
    Text.resize((size_t) BytesRead + 1);

    ::memcpy(Text.data(), Data, (size_t) BytesRead + 1);

    delete[] Data;

    return Text;
}

/// <summary>
/// Gets the template file path with all environment variables expanded.
/// </summary>
std::wstring UIElement::GetTemplateFilePath() const noexcept
{
    wchar_t FilePath[MAX_PATH];

    if (::ExpandEnvironmentStringsW(pfc::wideFromUTF8(FilePathCfg.c_str()), FilePath, _countof(FilePath)) == 0)
        ::wcscpy_s(FilePath, _countof(FilePath), pfc::wideFromUTF8(FilePathCfg.c_str()));

    return std::wstring(FilePath);
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

    const std::wstring FunctionName = ::UTF8ToWide(OnPlaybackStartingCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    static const wchar_t * CommandName = L"Unknown";

    if (command == play_control::t_track_command::track_command_play) CommandName = L"Play"; else
    if (command == play_control::t_track_command::track_command_next) CommandName = L"Next"; else       // Plays the next track from the current playlist according to the current playback order.
    if (command == play_control::t_track_command::track_command_prev) CommandName = L"Prev"; else       // Plays the previous track from the current playlist according to the current playback order.
    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Random"; else     // Plays a random track from the current playlist.

    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Set track"; else  // For internal use only, do not use.
    if (command == play_control::t_track_command::track_command_rand) CommandName = L"Resume";          // For internal use only, do not use.

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s(\"%s\", %s)", FunctionName.c_str(), CommandName, (paused ? L"true" : L"false")).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_new_track failed");
}

/// <summary>
/// Called when playback advances to a new track.
/// </summary>
void UIElement::on_playback_new_track(metadb_handle_ptr /*track*/)
{
    if (_WebView == nullptr)
        return;

    const std::wstring FunctionName = ::UTF8ToWide(OnPlaybackNewTrackCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s()", FunctionName.c_str()).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_new_track failed");
}

/// <summary>
/// Called when playback stops.
/// </summary>
void UIElement::on_playback_stop(play_control::t_stop_reason reason)
{
    if (_WebView == nullptr)
        return;

    const std::wstring FunctionName = ::UTF8ToWide(OnPlaybackStopCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    static const wchar_t * Reason = L"unknown";

    if (reason == play_control::t_stop_reason::stop_reason_user)                Reason = L"User"; else
    if (reason == play_control::t_stop_reason::stop_reason_eof)                 Reason = L"EOF"; else
    if (reason == play_control::t_stop_reason::stop_reason_starting_another)    Reason = L"Starting another"; else
    if (reason == play_control::t_stop_reason::stop_reason_shutting_down)       Reason = L"Shutting down";

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s(\"%s\")", FunctionName.c_str(), Reason).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_stop failed");
}

/// <summary>
/// Called when the user seeks to a specific time.
/// </summary>
void UIElement::on_playback_seek(double time)
{
    if (_WebView == nullptr)
        return;

    const std::wstring FunctionName = ::UTF8ToWide(OnPlaybackSeekCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s(%f)", FunctionName.c_str(), time).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_seek failed");
}

/// <summary>
/// Called when playback pauses or resumes.
/// </summary>
void UIElement::on_playback_pause(bool paused)
{
    if (_WebView == nullptr)
        return;

    const std::wstring FunctionName = ::UTF8ToWide(OnPlaybackPauseCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s(%s)", FunctionName.c_str(), (paused ? L"true" : L"false")).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_pause failed");
}

/// <summary>
/// Called when the currently played file gets edited.
/// </summary>
void UIElement::on_playback_edited(metadb_handle_ptr hTrack)
{
}

/// <summary>
/// Called when dynamic info (VBR bitrate etc) changes.
/// </summary>
void UIElement::on_playback_dynamic_info(const file_info & fileInfo)
{
}

/// <summary>
/// Called when the per-track dynamic info (stream track titles etc.) change. Happens less often than on_playback_dynamic_info().
/// </summary>
void UIElement::on_playback_dynamic_info_track(const file_info & fileInfo)
{
}

/// <summary>
/// Called, every second, for time display.
/// </summary>
void UIElement::on_playback_time(double time)
{
    if (_WebView == nullptr)
        return;

    const std::wstring FunctionName = ::UTF8ToWide(OnPlaybackTimeCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s(%f)", FunctionName.c_str(), time).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_time failed");
}

/// <summary>
/// Called when the user changes the volume.
/// </summary>
void UIElement::on_volume_change(float newValue) // in dBFS
{
    if (_WebView == nullptr)
        return;

    const std::wstring FunctionName = ::UTF8ToWide(OnVolumeChangeCallbackCfg.c_str());

    if (FunctionName.empty())
        return;

    HRESULT hResult = _WebView->ExecuteScript(::FormatText(L"%s(%f)", FunctionName.c_str(), (double) newValue).c_str(), nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_volume_change failed");
}

#pragma endregion
