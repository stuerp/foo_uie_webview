
/** $VER: CUIElement.cpp (2024.05.23) P. Stuer **/

#include "pch.h"

#include "CUIElement.h"

#include <ui_extension.h>

#pragma hdrstop

namespace uie
{
static cui::colours::client::factory<CUIColorClient> _CUIColorClientFactory;

/// <summary>
/// Initializes a new instance.
/// </summary>
CUIElement::CUIElement()
{
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
    if (*this == nullptr)
    {
        _Host = newHost;

        CRect r;

        position.convert_to_rect(r);

        Create(hParent, r, 0, WS_CHILD, 0);

        _hParent = hParent;
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

static uie::window_factory<CUIElement> _WindowFactory;

void CUIColorClient::on_colour_changed(uint32_t changed_items_mask) const
{
}

void CUIColorClient::on_bool_changed(uint32_t changed_items_mask) const
{
}

}
