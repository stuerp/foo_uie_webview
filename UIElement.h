
/** $VER: UIElement.h (2024.12.02) P. Stuer **/

#pragma once

#include "framework.h"

#include "Resources.h"
#include "FileWatcher.h"
#include "Configuration.h"

#include <SDK/cfg_var.h>
#include <SDK/coreDarkMode.h>
#include <SDK/playback_control.h>
#include <SDK/play_callback.h>
#include <SDK/playlist.h>
#include <SDK/ui_element.h>
#include <SDK/vis.h>

#include <pfc/string_conv.h>
#include <pfc/string-conv-lite.h>

#include <wrl.h>
#include <wil/com.h>

#include <WebView2.h>

#include "HostObjectImpl.h"
#include "SharedBuffer.h"

using namespace Microsoft::WRL;

/// <summary>
/// Implements the UIElement and Playback interface.
/// </summary>
class UIElement : public CWindowImpl<UIElement>, public playlist_callback, private play_callback_impl_base
{
public:
    UIElement();

    UIElement(const UIElement &) = delete;
    UIElement & operator=(const UIElement &) = delete;
    UIElement(UIElement &&) = delete;
    UIElement & operator=(UIElement &&) = delete;

    virtual ~UIElement();

    #pragma region CWindowImpl

    static CWndClassInfo & GetWndClassInfo();

    void OnColorsChanged();

    #pragma endregion

    #pragma region WebView

    ICoreWebView2Controller * GetWebViewController()
    {
        return _Controller.get();
    }

    ICoreWebView2 * GetWebView()
    {
        return _WebView.get();
    }

    const wchar_t * const _HostName = TEXT(STR_COMPONENT_BASENAME) L".local";

    #pragma endregion

    #pragma region HostObject

    void RunAsync(std::function<void()> callback) noexcept
    {
        auto * Task = new std::function<void()>(std::move(callback));

        PostMessage(UM_ASYNC, reinterpret_cast<WPARAM>(Task), 0);
    }

    void MessageBoxAsync(std::wstring message, std::wstring title)
    {
        RunAsync
        (
            [this, message = std::move(message), title = std::move(title)]
            {
                MessageBox(message.c_str(), title.c_str(), MB_OK);
            }
        );
    }

    #pragma endregion

    #pragma region Configuration

    const configuration_t & GetConfiguration() const noexcept
    {
        return _Configuration;
    }

    void SetConfiguration(const configuration_t & configuration) noexcept
    {
        _Configuration = configuration;

        OnConfigurationChanged();
    }

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

    virtual void GetColors() noexcept = 0;

    virtual bool IsWebViewVisible() const noexcept = 0;

    virtual void SetWebViewVisibility(bool visible) noexcept;

private:
    #pragma region Playback callback methods

    void on_playback_starting(play_control::t_track_command command, bool paused);
    void on_playback_new_track(metadb_handle_ptr hTrack);
    void on_playback_stop(play_control::t_stop_reason reason);
    void on_playback_seek(double time);
    void on_playback_pause(bool state);
    void on_playback_edited(metadb_handle_ptr hTrack);
    void on_playback_dynamic_info(const file_info & fileInfo);
    void on_playback_dynamic_info_track(const file_info & fileInfo);
    void on_playback_time(double time);
    void on_volume_change(float newValue);

    #pragma endregion

public:
    #pragma region playlist_callback

    void on_items_added(t_size playlistIndex, t_size startIndex, metadb_handle_list_cref data, const bit_array & selection);
    void on_items_reordered(t_size playlistIndex, const t_size * order, t_size count);
    void on_items_removing(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount);
    void on_items_removed(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount);

    void on_items_selection_change(t_size playlistIndex, const bit_array & affectedItems, const bit_array & state);

    void on_items_modified(t_size playlistIndex, const bit_array & mask);
    void on_items_modified_fromplayback(t_size playlistIndex, const bit_array & mask, play_control::t_display_level displayLevel);
    void on_items_replaced(t_size playlistIndex, const bit_array & mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & data);

    void on_item_focus_change(t_size playlistIndex, t_size oldItemIndex, t_size newItemIndex);
    void on_item_ensure_visible(t_size playlistIndex, t_size itemIndex);

    void on_playlist_activate(t_size oldPlaylistIndex, t_size newPlaylistIndex);
    void on_playlist_created(t_size playlistIndex, const char * name, t_size size);
    void on_playlists_reorder(const t_size * order, t_size count);
    void on_playlists_removing(const bit_array & mask, t_size oldCount, t_size newCount);
    void on_playlists_removed(const bit_array & mask, t_size oldCount, t_size newCount);
    void on_playlist_renamed(t_size playlistIndex, const char * name, t_size size);

    void on_playlist_locked(t_size playlistIndex, bool isLocked);

    void on_default_format_changed() override;

    void on_playback_order_changed(t_size playbackOrderIndex);

    #pragma endregion

private:
    void ExecuteScript(const std::wstring & script) const noexcept;

    #pragma region CWindowImpl

    LRESULT OnCreate(LPCREATESTRUCT cs) noexcept;
    void OnDestroy() noexcept;
    void OnSize(UINT nType, CSize size) noexcept;
    BOOL OnEraseBackground(CDCHandle dc) noexcept;
    void OnPaint(CDCHandle dc) noexcept;
    LRESULT OnTemplateChanged(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT OnWebViewReady(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT OnAsync(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    BEGIN_MSG_MAP_EX(UIElement)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_ERASEBKGND(OnEraseBackground)
        MSG_WM_PAINT(OnPaint)

        MESSAGE_HANDLER_EX(UM_TEMPLATE_CHANGED, OnTemplateChanged)
        MESSAGE_HANDLER_EX(UM_WEB_VIEW_READY, OnWebViewReady)
        MESSAGE_HANDLER_EX(UM_ASYNC, OnAsync)
    END_MSG_MAP()

    #pragma endregion

    void Initialize();

    HRESULT PostChunk(const audio_sample * samples, size_t sampleCount, uint32_t sampleRate, uint32_t channelCount, uint32_t channelConfig) noexcept;

private:
    bool GetWebViewVersion(std::wstring & versionInfo);

    HRESULT CreateWebView();
    HRESULT RecreateWebView() noexcept;
    void DeleteWebView() noexcept;

    HRESULT SetDarkMode(bool enabled) const noexcept;
    HRESULT SetDefaultBackgroundColor() const noexcept;

    HRESULT CreateContextMenu(const wchar_t * itemLabel, const wchar_t * iconName) noexcept;
    HRESULT ClearBrowserData() const noexcept;
    HRESULT RequestBrowserProfileDeletion() const noexcept;

    void InitializeFileWatcher();
    void InitializeWebView();

    std::wstring GetTemplateFilePath() const noexcept;

    void ShowPreferences() noexcept;

    void OnConfigurationChanged() noexcept;

    void StartTimer() noexcept;
    void StopTimer() noexcept;

    static void CALLBACK TimerCallback(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4) noexcept;

    void OnTimer() noexcept;

protected:
    configuration_t _Configuration;

    COLORREF _ForegroundColor;
    COLORREF _BackgroundColor;

private:
    fb2k::CCoreDarkModeHooks _DarkMode;
    playback_control::ptr _PlaybackControl;

    std::wstring _ExpandedTemplateFilePath;

    wil::com_ptr<ICoreWebView2Environment> _Environment;
    wil::com_ptr<ICoreWebView2Controller> _Controller;
    wil::com_ptr<ICoreWebView2> _WebView;
    wil::com_ptr<ICoreWebView2ContextMenuItem> _ContextSubMenu;

    EventRegistrationToken _NavigationStartingToken = {};
    EventRegistrationToken _NavigationCompletedToken = {};
    EventRegistrationToken _FrameCreatedToken = {};
    EventRegistrationToken _ContextMenuRequestedToken = {};
    EventRegistrationToken _BrowserProcessExitedToken = {};

    wil::com_ptr<HostObject> _HostObject;

    bool _IsNavigationCompleted;

    FileWatcher _FileWatcher;

    PTP_TIMER _ThreadPoolTimer;
    bool _IsFrozen;
    bool _IsHidden;

    visualisation_stream_v2::ptr _VisualisationStream;
    double _LastPlaybackTime;
    uint32_t _SampleRate;

    SharedBuffer _SharedBuffer;
};
