
/** $VER: Preferences.cpp (2024.05.28) P. Stuer **/

#include "pch.h"

#include <SDK/foobar2000-lite.h>
#include <SDK/cfg_var.h>
#include <SDK/preferences_page.h>

#include <helpers/advconfig_impl.h>
#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "UIElement.h"
#include "Preferences.h"
#include "Resources.h"

#pragma hdrstop

static constexpr GUID       FilePathGUID                        = { 0x341c4082, 0x255b, 0x4a38, { 0x81, 0x53, 0x55, 0x43, 0x5a, 0xd2, 0xe8, 0xa5 }};
static constexpr const char FilePathDefault[]                   = "Template.html";
cfg_string                  FilePathCfg(FilePathGUID, FilePathDefault);

static constexpr GUID       OnPlaybackStartingCallbackGUID      = { 0x8f77ab56, 0xcfa5, 0x4dab, { 0x9d, 0xa3, 0x74, 0x06, 0x9c, 0x75, 0x26, 0x07 } };
static constexpr const char OnPlaybackStartingCallbackDefault[] = "OnPlaybackStarting";
cfg_string                  OnPlaybackStartingCallbackCfg(OnPlaybackStartingCallbackGUID, OnPlaybackStartingCallbackDefault);

static constexpr GUID       OnPlaybackNewTrackCallbackGUID      = { 0xe82961ad, 0xaa0a, 0x47b9, { 0x9b, 0x3d, 0x71, 0xd0, 0x2a, 0xf5, 0x09, 0x67 } };
static constexpr const char OnPlaybackNewTrackCallbackDefault[] = "OnPlaybackNewTrack";
cfg_string                  OnPlaybackNewTrackCallbackCfg(OnPlaybackNewTrackCallbackGUID, OnPlaybackNewTrackCallbackDefault);

static constexpr GUID       OnPlaybackStopCallbackGUID          = { 0xdb62948a, 0x550a, 0x487c, { 0xac, 0xb0, 0x44, 0xf2, 0xdd, 0x5e, 0xd7, 0xa1 } };
static constexpr const char OnPlaybackStopCallbackDefault[]     = "OnPlaybackStop";
cfg_string                  OnPlaybackStopCallbackCfg(OnPlaybackStopCallbackGUID, OnPlaybackStopCallbackDefault);

static constexpr GUID       OnPlaybackSeekCallbackGUID          = { 0x6d28ff36, 0x08aa, 0x4ac6, { 0xa8, 0x68, 0x57, 0x68, 0x08, 0xa9, 0xcc, 0x3d } };
static constexpr const char OnPlaybackSeekCallbackDefault[]     = "OnPlaybackSeek";
cfg_string                  OnPlaybackSeekCallbackCfg(OnPlaybackSeekCallbackGUID, OnPlaybackSeekCallbackDefault);

static constexpr GUID       OnPlaybackPauseCallbackGUID         = { 0x12c5a8f7, 0x7521, 0x4cb9, { 0xa6, 0x7c, 0x9d, 0xf2, 0x26, 0xaa, 0xdf, 0x54 } };
static constexpr const char OnPlaybackPauseCallbackDefault[]    = "OnPlaybackPause";
cfg_string                  OnPlaybackPauseCallbackCfg(OnPlaybackPauseCallbackGUID, OnPlaybackPauseCallbackDefault);

static constexpr GUID       OnPlaybackTimeCallbackGUID         = { 0x0c5d244e2, 0xf6cc, 0x4b3f, { 0x99, 0xfe, 0x29, 0xbf, 0x2e, 0x03, 0x14, 0x04 } };
static constexpr const char OnPlaybackTimeCallbackDefault[]    = "OnPlaybackTime";
cfg_string                  OnPlaybackTimeCallbackCfg(OnPlaybackTimeCallbackGUID, OnPlaybackTimeCallbackDefault);

static constexpr GUID       OnVolumeChangeCallbackGUID         = { 0x0959697db, 0x64a4, 0x4726, { 0x85, 0x27, 0x64, 0x88, 0x14, 0x0c, 0xa6, 0xda } };
static constexpr const char OnVolumeChangeCallbackDefault[]    = "OnVolumeChange";
cfg_string                  OnVolumeChangeCallbackCfg(OnVolumeChangeCallbackGUID, OnVolumeChangeCallbackDefault);

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
        if (FilePathCfg != _FilePath)
        {
            FilePathCfg = _FilePath;

            for (auto && Panel : PanelTracker::instanceList())
            {
                Panel->OnTemplateFilePathChanged();
            }
        }

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
            {
                _FilePath = Text;
                break;
            }

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
