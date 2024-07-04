
/** $VER: Preferences.cpp (2024.07.04) P. Stuer **/

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
        _CurrentElement = _UIElementTracker.GetCurrentElement();

        if (_CurrentElement != nullptr)
            _Configuration = _CurrentElement->GetConfiguration();
    }

    virtual ~Preferences()
    {
        if (_CurrentElement != nullptr)
            _CurrentElement->SetConfiguration(_Configuration);
    }

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

            _Configuration._Name = Text;
        }

        {
            GetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, Text, _countof(Text));

            _Configuration._UserDataFolderPath = Text;
        }

        {
            GetDlgItemTextW(IDC_FILE_PATH, Text, _countof(Text));

            _Configuration._TemplateFilePath = Text;
        }

        {
            GetDlgItemTextW(IDC_WINDOW_SIZE, Text, _countof(Text));

            _Configuration._WindowSize = (uint32_t) ::_wtoi(Text);
        }

        {
            _Configuration._WindowSizeUnit = (WindowSizeUnit) ((CComboBox) GetDlgItem(IDC_WINDOW_SIZE_UNIT)).GetCurSel();
        }

        {
            GetDlgItemTextW(IDC_REACTION_ALIGNMENT, Text, _countof(Text));

            _Configuration._ReactionAlignment = ::_wtof(Text);
        }

        UIElement * CurrentElement = _UIElementTracker.GetCurrentElement();

        if (CurrentElement != nullptr)
            CurrentElement->SetConfiguration(_Configuration);

        OnChanged();
    }

    /// <summary>
    /// Resets this page's content to the default values. Does not apply any changes - lets user preview the changes before hitting "apply".
    /// </summary>
    virtual void reset() final
    {
        _Configuration.Reset();

        InitializeControls();

        OnChanged();
    }

    #pragma endregion

    // WTL message map
    BEGIN_MSG_MAP_EX(Preferences)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_CODE_HANDLER_EX(CBN_SELCHANGE, OnSelectionChanged) // This also handles LBN_SELCHANGE

        COMMAND_HANDLER_EX(IDC_USER_DATA_FOLDER_PATH_SELECT, BN_CLICKED, OnButtonClicked)

        COMMAND_HANDLER_EX(IDC_FILE_PATH_SELECT, BN_CLICKED, OnButtonClicked)
        COMMAND_HANDLER_EX(IDC_FILE_PATH_EDIT, BN_CLICKED, OnButtonClicked)

        COMMAND_HANDLER_EX(IDC_WINDOW_SIZE, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_REACTION_ALIGNMENT, EN_CHANGE, OnEditChange)
    END_MSG_MAP()

private:
    /// <summary>
    /// Initializes the dialog.
    /// </summary>
    BOOL OnInitDialog(CWindow, LPARAM) noexcept
    {
        _DarkModeHooks.AddDialogWithControls(*this);

        InitializeControls();

        return FALSE;
    }

    /// <summary>
    /// Initializes the controls.
    /// </summary>
    void InitializeControls()
    {
        SetDlgItemTextW(IDC_NAME,                  _Configuration._Name.c_str());
        SetDlgItemTextW(IDC_USER_DATA_FOLDER_PATH, _Configuration._UserDataFolderPath.c_str());
        SetDlgItemTextW(IDC_FILE_PATH,             _Configuration._TemplateFilePath.c_str());

        SetDlgItemTextW(IDC_WINDOW_SIZE, pfc::wideFromUTF8(pfc::format_int(_Configuration._WindowSize)));

        {
            auto w = (CComboBox) GetDlgItem(IDC_WINDOW_SIZE_UNIT);

            w.ResetContent();

            const WCHAR * Labels[] = { L"ms", L"samples" };

            assert(((size_t) WindowSizeUnit::Count == _countof(Labels)));

            for (auto Label : Labels)
                w.AddString(Label);

            w.SetCurSel((int) _Configuration._WindowSizeUnit);
        }

        SetDlgItemTextW(IDC_REACTION_ALIGNMENT, pfc::wideFromUTF8(pfc::format_float(_Configuration._ReactionAlignment, 0, 2)));
    }

    /// <summary>
    /// Handles an update of the selected item of a combo box.
    /// </summary>
    void OnSelectionChanged(UINT, int, CWindow) noexcept
    {
        OnChanged();
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditChange(UINT, int, CWindow) noexcept
    {
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
    void OnChanged() noexcept
    {
        WCHAR Text[16];

        GetDlgItemTextW(IDC_WINDOW_SIZE, Text, _countof(Text));
        uint32_t WindowSize = (uint32_t) ::_wtoi(Text);

        GetDlgItemTextW(IDC_REACTION_ALIGNMENT, Text, _countof(Text));
        double ReactionAlignment = ::_wtof(Text);

        int32_t WindowOffset = (int32_t) (WindowSize * (0.5 + ReactionAlignment));

        auto w = (CComboBox) GetDlgItem(IDC_WINDOW_SIZE_UNIT);

        const WCHAR * Format = (w.GetCurSel() == (int) WindowSizeUnit::Milliseconds) ? L"%dms %s playback" : L"%d samples %s playback";

        SetDlgItemTextW(IDC_WINDOW_OFFSET, ::FormatText(Format, ::abs(WindowOffset), (WindowOffset > 0) ? L"behind" : L"ahead of").c_str());

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

        if (_Configuration._TemplateFilePath != Text)
            return true;

        GetDlgItemTextW(IDC_WINDOW_SIZE, Text, _countof(Text));

        if (_Configuration._WindowSize != (uint32_t) ::_wtoi(Text))
            return true;

        auto w = (CComboBox) GetDlgItem(IDC_WINDOW_SIZE_UNIT);

        if (_Configuration._WindowSizeUnit != w.GetCurSel())
            return true;

        GetDlgItemTextW(IDC_REACTION_ALIGNMENT, Text, _countof(Text));

        if (_Configuration._ReactionAlignment != ::_wtof(Text))
            return true;

        return false;
    }

private:
    const preferences_page_callback::ptr _Callback;

    fb2k::CDarkModeHooks _DarkModeHooks;

    UIElement * _CurrentElement;
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
