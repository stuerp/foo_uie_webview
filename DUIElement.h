
/** $VER: DUIElement.h (2024.07.03) P. Stuer - Default User Interface support **/

#pragma once

#include "framework.h"

#include "UIElement.h"

#include <helpers/BumpableElem.h>

/// <summary>
/// Implements a Default UI element.
/// </summary>
class DUIElement : public UIElement, public ui_element_instance
{
public:
    DUIElement(ui_element_config::ptr data, ui_element_instance_callback::ptr callback);

    DUIElement(const DUIElement &) = delete;
    DUIElement & operator=(const DUIElement &) = delete;
    DUIElement(DUIElement &&) = delete;
    DUIElement & operator=(DUIElement &&) = delete;

    virtual ~DUIElement() { };

    #pragma region ui_element_instance interface

    static void g_get_name(pfc::string_base & p_out);
    static const char * g_get_description();
    static GUID g_get_guid();
    static GUID g_get_subclass();
    static ui_element_config::ptr g_get_default_configuration();

    void initialize_window(HWND p_parent);
    virtual void set_configuration(ui_element_config::ptr p_data);
    virtual ui_element_config::ptr get_configuration();
    virtual void notify(const GUID & what, t_size param1, const void * param2, t_size param2Size);

    #pragma endregion

    virtual bool IsWebViewVisible() const noexcept
    {
        return !m_callback->is_edit_mode_enabled(); // Hide the WebView to allow the default foobar2000 context menu to appear in "Layout Edit" mode.
    }

    void GetColors() noexcept override;

protected:
    ui_element_instance_callback::ptr m_callback; // Don't rename this. BumpableElement uses it.
};
