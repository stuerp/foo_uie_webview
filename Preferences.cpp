
/** $VER: Preferences.cpp (2024.06.26) P. Stuer **/

#include "pch.h"

#include <SDK/foobar2000-lite.h>
#include <SDK/cfg_var.h>
#include <SDK/preferences_page.h>

#include <helpers/advconfig_impl.h>
#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "UIElement.h"
#include "UIElementTracker.h"
#include "Exceptions.h"
#include "Encoding.h"
#include "Resources.h"

#pragma hdrstop

/// <summary>
/// Implements the preferences page for the component.
/// </summary>
class Preferences : public CDialogImpl<Preferences>, public preferences_page_instance
{
public:
    Preferences(preferences_page_callback::ptr callback) : m_bMsgHandled(FALSE), _Callback(callback)
    {
        UIElement * CurrentElement = _UIElementTracker.GetCurrentElement();

        if (CurrentElement != nullptr)
            _Configuration = CurrentElement->GetConfiguration();
    }

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
        wchar_t Text[MAX_PATH];

        {
            GetDlgItemTextW(IDC_NAME, Text, _countof(Text));

            if (_Configuration._Name != Text)
                _Configuration._Name = Text;
        }

        {
            GetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, Text, _countof(Text));

            if (_Configuration._UserDataFolderPath != Text)
                _Configuration._UserDataFolderPath = Text;
        }

        {
            GetDlgItemTextW(IDC_FILE_PATH, Text, _countof(Text));

            if (_Configuration._TemplateFilePath != Text)
                _Configuration._TemplateFilePath = Text;
        }

        UIElement * CurrentElement = _UIElementTracker.GetCurrentElement();

        if (CurrentElement != nullptr)
            CurrentElement->SetConfiguration(_Configuration);

        OnChanged(); // The flags have been updated.
    }

    /// <summary>
    /// Resets this page's content to the default values. Does not apply any changes - lets user preview the changes before hitting "apply".
    /// </summary>
    virtual void reset() final
    {
        _Configuration.Reset();

        SetDlgItemTextW(IDC_NAME,                  _Configuration._Name.c_str());
        SetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, _Configuration._UserDataFolderPath.c_str());
        SetDlgItemTextW(IDC_FILE_PATH,             _Configuration._TemplateFilePath.c_str());

        OnChanged();
    }

    #pragma endregion

    // WTL message map
    BEGIN_MSG_MAP_EX(Preferences)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_HANDLER_EX(IDC_NAME, EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_USER_DATA_FOLDER_PATH, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_USER_DATA_FOLDER_PATH_SELECT, BN_CLICKED, OnButtonClicked)

        COMMAND_HANDLER_EX(IDC_FILE_PATH, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_FILE_PATH_SELECT, BN_CLICKED, OnButtonClicked)
        COMMAND_HANDLER_EX(IDC_FILE_PATH_EDIT, BN_CLICKED, OnButtonClicked)
    END_MSG_MAP()

private:
    /// <summary>
    /// Initializes the dialog.
    /// </summary>
    BOOL OnInitDialog(CWindow, LPARAM) noexcept
    {
        _DarkModeHooks.AddDialogWithControls(*this);

        SetDlgItemTextW(IDC_NAME, _Configuration._Name.c_str());
        SetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, _Configuration._UserDataFolderPath.c_str());
        SetDlgItemTextW(IDC_FILE_PATH, _Configuration._TemplateFilePath.c_str());

        return FALSE;
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditChange(UINT code, int id, CWindow) noexcept
    {
        if (code != EN_CHANGE)
            return;

        OnChanged();
    }

    /// <summary>
    /// Handles a click on a button.
    /// </summary>
    void OnButtonClicked(UINT, int id, CWindow) noexcept
    {
        switch (id)
        {
            case IDC_USER_DATA_FOLDER_PATH_SELECT:
            {
                char ExpandedDirectoryPath[MAX_PATH];

                if (::ExpandEnvironmentStringsA(::WideToUTF8(_Configuration._UserDataFolderPath).c_str(), ExpandedDirectoryPath, _countof(ExpandedDirectoryPath)) == 0)
                    ::strcpy_s(ExpandedDirectoryPath, _countof(ExpandedDirectoryPath), ::WideToUTF8(_Configuration._UserDataFolderPath).c_str());

                pfc::string8 DirectoryPath = ExpandedDirectoryPath;

                DirectoryPath.truncate_filename();

                if (::uBrowseForFolder(m_hWnd, "Locate the WebView user data folder...", DirectoryPath))
                {
                    SetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, ::UTF8ToWide(DirectoryPath.c_str()).c_str());

                    UpdateDialog();
                    OnChanged();
                }
                break;
            }

            case IDC_FILE_PATH_SELECT:
            {
                char ExpandedFilePath[MAX_PATH];

                if (::ExpandEnvironmentStringsA(::WideToUTF8(_Configuration._TemplateFilePath).c_str(), ExpandedFilePath, _countof(ExpandedFilePath)) == 0)
                    ::strcpy_s(ExpandedFilePath, _countof(ExpandedFilePath), ::WideToUTF8(_Configuration._TemplateFilePath).c_str());

                pfc::string8 DirectoryPath = ExpandedFilePath;

                DirectoryPath.truncate_filename();

                pfc::string8 FilePath = ExpandedFilePath;

                if (::uGetOpenFileName(m_hWnd, "Template files|*.htm;*.html;*.txt", 0, "html", "Choose a template...", DirectoryPath, FilePath, FALSE))
                {
                    SetDlgItemTextW(IDC_FILE_PATH, ::UTF8ToWide(FilePath.c_str()).c_str());

                    UpdateDialog();
                    OnChanged();
                }
                break;
            }

            case IDC_FILE_PATH_EDIT:
            {
                char ExpandedFilePath[MAX_PATH];

                if (::ExpandEnvironmentStringsA(::WideToUTF8(_Configuration._TemplateFilePath).c_str(), ExpandedFilePath, _countof(ExpandedFilePath)) == 0)
                    ::strcpy_s(ExpandedFilePath, _countof(ExpandedFilePath), ::WideToUTF8(_Configuration._TemplateFilePath).c_str());

                pfc::string8 DirectoryPath = ExpandedFilePath;

                DirectoryPath.truncate_filename();

                INT_PTR Result = (INT_PTR) ::ShellExecuteA(m_hWnd, "edit", ExpandedFilePath, nullptr, DirectoryPath.c_str(), SW_NORMAL);

                if (Result <= 32)
                    MessageBoxW(::UTF8ToWide(::GetErrorMessage(::GetLastError(), "Failed to launch editor")).c_str(), TEXT(STR_COMPONENT_NAME), MB_OK);
                break;
            }

            default:
                break;
        }
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
        wchar_t Text[MAX_PATH];

        GetDlgItemTextW(IDC_NAME, Text, _countof(Text));

        if (_Configuration._Name != Text)
            return true;

        GetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, Text, _countof(Text));

        if (_Configuration._UserDataFolderPath != Text)
            return true;

        GetDlgItemTextW(IDC_FILE_PATH, Text, _countof(Text));

        return _Configuration._TemplateFilePath != Text;
    }

    /// <summary>
    /// Updates the appearance of the dialog according to the values of the settings.
    /// </summary>
    void UpdateDialog() noexcept
    {
    }

private:
    const preferences_page_callback::ptr _Callback;

    fb2k::CDarkModeHooks _DarkModeHooks;

    configuration_t _Configuration;
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
        static constexpr GUID _GUID = GUID_PREFERENCES;

        return _GUID;
    }

    GUID get_parent_guid()
    {
        return guid_display;
    }
};

static preferences_page_factory_t<PreferencesPage> _Factory;

#pragma endregion
