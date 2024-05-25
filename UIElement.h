
/** $VER: UIElement.h (2024.05.25) P. Stuer **/

#pragma once

#include "framework.h"

#include "Resources.h"
#include "FileWatcher.h"

#include <SDK/coreDarkMode.h>
#include <SDK/playback_control.h>
#include <SDK/play_callback.h>
#include <SDK/cfg_var.h>
#include <pfc/string_conv.h>
#include <pfc/string-conv-lite.h>

extern cfg_string FilePathCfg;

#include <wrl.h>
#include <wil/com.h>

#include "WebView2.h"

using namespace Microsoft::WRL;

/// <summary>
/// Implements the UIElement and Playback interface.
/// </summary>
class UIElement : public CWindowImpl<UIElement>, private play_callback_impl_base
{
public:
    UIElement();

    UIElement(const UIElement &) = delete;
    UIElement & operator=(const UIElement &) = delete;
    UIElement(UIElement &&) = delete;
    UIElement & operator=(UIElement &&) = delete;

    #pragma region CWindowImpl

    static CWndClassInfo & GetWndClassInfo();

    #pragma endregion

protected:
    /// <summary>
    /// Retrieves the GUID of the element.
    /// </summary>
    static const GUID & GetGUID() noexcept
    {
        static const GUID guid = GUID_UI_ELEMENT;

        return guid;
    }

private:
    #pragma region Playback callback methods

    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) { }
    void on_playback_new_track(metadb_handle_ptr p_track);
    void on_playback_stop(play_control::t_stop_reason p_reason);
    void on_playback_seek(double p_time) { }
    void on_playback_pause(bool p_state);
    void on_playback_edited(metadb_handle_ptr p_track) { }
    void on_playback_dynamic_info(const file_info& p_info) { }
    void on_playback_dynamic_info_track(const file_info& p_info) { }
    void on_playback_time(double time);
    void on_volume_change(float p_new_val) { }

    #pragma endregion

    #pragma region CWindowImpl

    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct) noexcept;
    void OnDestroy() noexcept;
    void OnSize(UINT nType, CSize size) noexcept;
    void OnTimer(UINT_PTR timerId) noexcept;
    LRESULT OnTemplateFileChanged(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    BEGIN_MSG_MAP_EX(UIElement)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)

        MSG_WM_TIMER(OnTimer)
        MESSAGE_HANDLER_EX(UM_FILE_CHANGED, OnTemplateFileChanged)
    END_MSG_MAP()

    #pragma endregion

private:
    void CreateWebView(HWND hWnd) noexcept;
    void UpdateWebView() noexcept;

    bool FormatText(const std::string & text, pfc::string & formattedText) noexcept;
    std::string ReadTemplate(const std::wstring & filePath) noexcept;

    const UINT_PTR TimerId = 1;

    void StartTimer() noexcept
    {
        ::SetTimer(m_hWnd, TimerId, 500, nullptr);
    }

    void StopTimer() noexcept
    {
        ::KillTimer(m_hWnd, TimerId);
    }

    std::wstring GetTemplateFilePath() const noexcept
    {
        wchar_t FilePath[MAX_PATH];

        if (::ExpandEnvironmentStringsW(pfc::wideFromUTF8(FilePathCfg.c_str()), FilePath, _countof(FilePath)) == 0)
            ::wcscpy_s(FilePath, _countof(FilePath), pfc::wideFromUTF8(FilePathCfg.c_str()));

        return std::wstring(FilePath);
    }

private:
    pfc::string8 _ProfilePath;
    std::wstring _FilePath;

    wil::com_ptr<ICoreWebView2Controller> _Controller;
    wil::com_ptr<ICoreWebView2> _WebView;

    FileWatcher _FileWatcher;

    std::string _TemplateText;

    fb2k::CCoreDarkModeHooks _DarkMode;
};
