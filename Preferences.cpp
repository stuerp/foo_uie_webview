
/** $VER: Preferences.cpp (2024.05.25) P. Stuer **/

#include "pch.h"

#include <SDK/foobar2000-lite.h>
#include <SDK/cfg_var.h>
#include <SDK/preferences_page.h>

#include <helpers/advconfig_impl.h>
#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "Resources.h"

#pragma hdrstop

static constexpr GUID FilePathGUID = {0x341c4082,0x255b,0x4a38,{0x81,0x53,0x55,0x43,0x5a,0xd2,0xe8,0xa5}}; // {341c4082-255b-4a38-8153-55435ad2e8a5}

static constexpr const char FilePathDefault[] = "Template.html";

cfg_string FilePathCfg(FilePathGUID, FilePathDefault);

/// <summary>
/// Implements the preferences page for the component.
/// </summary>
class Preferences : public CDialogImpl<Preferences>, public preferences_page_instance
{
public:
    Preferences(preferences_page_callback::ptr callback) : m_bMsgHandled(FALSE), _Callback(callback) { }

    virtual ~Preferences() { }

    enum
    {
        IDD = IDD_PREFERENCES
    };

    #pragma region preferences_page_instance

    /// <summary>
    /// Returns a combination of preferences_state constants.
    /// </summary>
    virtual t_uint32 get_state() final
    {
        t_uint32 State = preferences_state::resettable | preferences_state::dark_mode_supported;

        if (HasChanged())
            State |= preferences_state::changed;

        return State;
    }

    /// <summary>
    /// Applies the changes to the preferences.
    /// </summary>
    virtual void apply() final
    {
        pfc::string8 Text;

        ::uGetDlgItemText(m_hWnd, IDC_FILE_PATH, Text);

        FilePathCfg = Text;

        OnChanged();
    }

    /// <summary>
    /// Resets this page's content to the default values. Does not apply any changes - lets user preview the changes before hitting "apply".
    /// </summary>
    virtual void reset() final
    {
        ::uSetDlgItemText(m_hWnd, IDC_FILE_PATH, FilePathDefault);

        UpdateDialog();

        OnChanged();
    }

    #pragma endregion

    //WTL message map
    BEGIN_MSG_MAP_EX(Preferences)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_HANDLER_EX(IDC_FILE_PATH, EN_CHANGE, OnEditChange)
    END_MSG_MAP()

private:
    /// <summary>
    /// Initializes the dialog.
    /// </summary>
    BOOL OnInitDialog(CWindow, LPARAM) noexcept
    {
        _DarkModeHooks.AddDialogWithControls(*this);

        _FilePath = FilePathCfg;

        UpdateDialog();

        return FALSE;
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditChange(UINT code, int id, CWindow) noexcept
    {
        if (code != EN_CHANGE)
            return;

        pfc::string8 Text;

        ::uGetDlgItemText(m_hWnd, IDC_FILE_PATH, Text);

        switch (id)
        {
            case IDC_FILE_PATH:
                _FilePath = Text;
                break;

            default:
                return;
        }

        OnChanged();
    }

    /// <summary>
    /// Tells the host that our state has changed to enable/disable the apply button appropriately.
    /// </summary>
    void OnChanged() const noexcept
    {
        _Callback->on_state_changed();
    }

    /// <summary>
    /// Returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)
    /// </summary>
    bool HasChanged() noexcept
    {
        return _FilePath != FilePathCfg;
    }

    /// <summary>
    /// Updates the appearance of the dialog according to the values of the settings.
    /// </summary>
    void UpdateDialog() noexcept
    {
        ::uSetDlgItemText(m_hWnd, IDC_FILE_PATH, _FilePath);
    }

private:
    const preferences_page_callback::ptr _Callback;

    fb2k::CDarkModeHooks _DarkModeHooks;

    pfc::string8 _FilePath;
};

#pragma region PreferencesPage

/// <summary>
/// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
/// </summary>
class PreferencesPage : public preferences_page_impl<Preferences>
{
public:
    virtual ~PreferencesPage() { }

    const char * get_name()
    {
        return STR_COMPONENT_NAME;
    }

    GUID get_guid()
    {
        static constexpr GUID _GUID = {0xb18587e0,0x9c95,0x4ee3,{0x8e,0x9f,0xaa,0x8c,0x77,0xec,0x2f,0x85}}; // {b18587e0-9c95-4ee3-8e9f-aa8c77ec2f85}

        return _GUID;
    }

    GUID get_parent_guid()
    {
        return guid_display;
    }
};

static preferences_page_factory_t<PreferencesPage> _PreferencesPageFactory;

#pragma endregion
