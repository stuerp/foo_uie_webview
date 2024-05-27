
/** $VER: UIElement.cpp (2024.05.27) P. Stuer **/

#include "pch.h"

#include "UIElement.h"
#include "Encoding.h"
#include "Exceptions.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
UIElement::UIElement() : m_bMsgHandled(FALSE)
{
    pfc::string8 ProfilePath = pfc::io::path::combine(core_api::get_profile_path(), STR_COMPONENT_BASENAME);

    if (::_strnicmp(ProfilePath, "file://", 7) == 0)
        _ProfilePath = UTF8ToWide(ProfilePath.subString(7).c_str());

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

    CreateWebView(m_hWnd);

    _TemplateText = ReadTemplate(_FilePath);

    try
    {
        _FileWatcher.Start(m_hWnd, _FilePath);
    }
    catch (std::exception & e)
    {
        const char * What = e.what();

        throw ComponentException(::FormatText("Failed to start file system watcher: %s", What));
    }

    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void UIElement::OnDestroy() noexcept
{
    _FileWatcher.Stop();
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
/// Handles a change to the template file.
/// </summary>
LRESULT UIElement::OnTemplateFileChanged(UINT msg, WPARAM wParam, LPARAM lParam)
{
    _TemplateText = ReadTemplate(_FilePath);

    InitializeWebView();

    return 0;
}

/// <summary>
/// The WebView is ready.
/// </summary>
LRESULT UIElement::OnWebViewReady(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (_WebView == nullptr)
        return 1;

    InitializeWebView();

    return 0;
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
/// Initializes the WebView.
/// </summary>
void UIElement::InitializeWebView()
{
    if (_WebView == nullptr)
        return;

    // Navigate to the template content.
    std::string Text = ReadTemplate(_FilePath);

    HRESULT hResult = _WebView->NavigateToString(UTF8ToWide(Text).c_str());

    if (!SUCCEEDED(hResult))
        throw Win32Exception((DWORD) hResult, "Failed to navigate to template content");
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
/// Reads the specified file into a string.
/// </summary>
std::string UIElement::ReadTemplate(const std::wstring & filePath) noexcept
{
    HANDLE hFile = ::CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD LastError = ::GetLastError();

        std::wstring Text;

        Text.resize(256);

        ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, LastError, 0, Text.data(), (DWORD) Text.size(), nullptr);
        ::OutputDebugStringW(Text.c_str());::OutputDebugStringW(L"\n");

        return std::string();
    }

    DWORD Size = ::GetFileSize(hFile, 0);

    char * Data = new char[Size + 1];

    DWORD BytesRead = 0;

    BOOL Success = ::ReadFile(hFile, Data, Size, &BytesRead, 0);

    ::CloseHandle(hFile);

    if (!Success || (BytesRead == 0))
        return std::string();

    Data[BytesRead] = '\0';
 
    std::string Text;
    
    Text.resize((size_t) BytesRead + 1);

    ::memcpy(Text.data(), Data, (size_t) BytesRead + 1);

    delete[] Data;

    return Text;
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

void UIElement::on_playback_starting(play_control::t_track_command command, bool paused)
{
}

/// <summary>
/// Playback advanced to new track.
/// </summary>
void UIElement::on_playback_new_track(metadb_handle_ptr track)
{
    if (_WebView == nullptr)
        return;

    HRESULT hResult = _WebView->ExecuteScript(L"Refresh()", nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_new_track failed");
}

/// <summary>
/// Playback stopped.
/// </summary>
void UIElement::on_playback_stop(play_control::t_stop_reason reason)
{
    if (_WebView == nullptr)
        return;

    HRESULT hResult = _WebView->ExecuteScript(L"Refresh()", nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_stop failed");
}

void UIElement::on_playback_seek(double time)
{
}

/// <summary>
/// Playback paused/resumed.
/// </summary>
void UIElement::on_playback_pause(bool)
{
    if (_WebView == nullptr)
        return;

    HRESULT hResult = _WebView->ExecuteScript(L"Refresh()", nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_pause failed");
}

void UIElement::on_playback_edited(metadb_handle_ptr hTrack)
{
}

void UIElement::on_playback_dynamic_info(const file_info & fileInfo)
{
}

void UIElement::on_playback_dynamic_info_track(const file_info & fileInfo)
{
}

/// <summary>
/// Called every second, for time display.
/// </summary>
void UIElement::on_playback_time(double time)
{
    if (_WebView == nullptr)
        return;

    HRESULT hResult = _WebView->ExecuteScript(L"Refresh()", nullptr);

    if (!SUCCEEDED(hResult))
        throw std::exception("on_playback_time failed");
}

void UIElement::on_volume_change(float newValue)
{
}

#pragma endregion
