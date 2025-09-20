
/** $VER: CUIElement.cpp (2024.07.03) P. Stuer **/

#include "pch.h"

#include "CUIElement.h"

#pragma hdrstop

namespace uie
{
#pragma region CUIElement

/// <summary>
/// Initializes a new instance.
/// </summary>
CUIElement::CUIElement()
{
    GetColors();
}

/// <summary>
/// Destroys this instance.
/// </summary>
CUIElement::~CUIElement()
{
}

/// <summary>
/// Creates or transfers the window.
/// </summary>
HWND CUIElement::create_or_transfer_window(HWND hParent, const window_host_ptr & newHost, const ui_helpers::window_position_t & position)
{
    _hParent = hParent;

    if (*this == nullptr)
    {
        _Host = newHost;

        CRect r;

        position.convert_to_rect(r);

        Create(hParent, r, 0, WS_CHILD, 0);
    }
    else
    {
        ShowWindow(SW_HIDE);
        SetParent(hParent);

        _Host->relinquish_ownership(*this);
        _Host = newHost;

        SetWindowPos(NULL, position.x, position.y, (int) position.cx, (int) position.cy, SWP_NOZORDER);
    }

    CUIColorClient::Register(this);

    return *this;
}

/// <summary>
/// Destroys the window.
/// </summary>
void CUIElement::destroy_window()
{
    CUIColorClient::Unregister(this);

    ::DestroyWindow(*this);

    _Host.release();
}

/// <summary>
/// Gets the colors.
/// </summary>
void CUIElement::GetColors() noexcept
{
    cui::colours::helper Helper(pfc::guid_null);

    _ForegroundColor = Helper.get_colour(cui::colours::colour_text);
    _BackgroundColor = Helper.get_colour(cui::colours::colour_background);
}

/// <summary>
/// Toggles borderless fullscreen mode.
/// </summary>
void CUIElement::ToggleFullScreen() noexcept
{
    if (!_IsFullscreen)
    {
        RECT r{};
        const HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        if (hMonitor)
        {
            MONITORINFOEX mix = {{sizeof(mix)}};
            if (::GetMonitorInfo(hMonitor, &mix))
            {
                r = mix.rcMonitor;
            }
        }
        const int w = max(0, r.right - r.left);
        const int h = max(0, r.bottom - r.top);
        const HWND z = HWND_TOP;

        const HWND window = m_hWnd;
        _PreviousWP = {sizeof(_PreviousWP)};
        ::GetWindowPlacement(window, &_PreviousWP);
        _PreviousStyle = ::GetWindowLongPtr(window, GWL_STYLE);
        _PreviousExStyle = ::GetWindowLongPtr(window, GWL_EXSTYLE);
        UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED;

        ::SetWindowLongPtr(window, GWL_STYLE, (_PreviousStyle & ~(WS_CHILD | WS_OVERLAPPEDWINDOW)) | WS_POPUP);
        ::SetWindowLongPtr(window, GWL_EXSTYLE, _PreviousExStyle & ~WS_EX_TOPMOST);
        ::SetParent(window, NULL);
        ::SetWindowPos(window, z, r.left, r.top, w, h, flags);

        _IsFullscreen = true;
    }
    else
    {
        const HWND window = m_hWnd;
        ::SetWindowLongPtr(window, GWL_STYLE, _PreviousStyle);
        ::SetWindowLongPtr(window, GWL_EXSTYLE, _PreviousExStyle);
        ::SetParent(window, _hParent);
        ::SetWindowPlacement(window, &_PreviousWP);
        ::SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ::SetFocus(core_api::get_main_window());

        _IsFullscreen = false;
    }
}

static uie::window_factory<CUIElement> _WindowFactory;

#pragma endregion

#pragma region CUIColorClient

void CUIColorClient::on_colour_changed(uint32_t changed_items_mask) const
{
    for (auto Iter : _Elements)
        Iter->OnColorsChanged();
}

void CUIColorClient::on_bool_changed(uint32_t changed_items_mask) const
{
    for (auto Iter : _Elements)
        Iter->OnColorsChanged();
}

static cui::colours::client::factory<CUIColorClient> _CUIColorClientFactory;

#pragma endregion
}
