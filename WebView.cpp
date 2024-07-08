
/** $VER: WebView.cpp (2024.07.08) P. Stuer - Creates the WebView. **/

#include "pch.h"

#include "UIElement.h"
#include "Exceptions.h"
#include "Encoding.h"

#include <WebView2EnvironmentOptions.h>

#include <pfc/string-conv-lite.h>
#include <pfc/pathUtils.h>

using namespace Microsoft::WRL;

static HRESULT CreateIconStream(const wchar_t * resourceName, wil::com_ptr<IStream> & stream);
static std::string GetWebViewErrorMessage(COREWEBVIEW2_WEB_ERROR_STATUS status, const std::string & errorMessage) noexcept;

/// <summary>
/// Returns true if a supported WebView version is available on this system.
/// </summary>
bool UIElement::GetWebViewVersion(std::wstring & versionString)
{
    LPWSTR VersionString = nullptr;

    HRESULT hr = ::GetAvailableCoreWebView2BrowserVersionString(nullptr, &VersionString);

    if (!SUCCEEDED(hr))
        return false;

    versionString = VersionString;

    ::CoTaskMemFree(VersionString);

    return true;
}

/// <summary>
/// Creates the WebView.
/// </summary>
HRESULT UIElement::CreateWebView()
{
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    if (!SUCCEEDED(hr))
    {
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to initialize COM").c_str());

        return hr;
    }

    // Create the environment options.
    auto EnvironmentOptions = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

    if (EnvironmentOptions == nullptr)
    {
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to create environment options").c_str());

        return E_UNEXPECTED;
    }

    if (_DarkMode)
    {
        hr = EnvironmentOptions->put_AdditionalBrowserArguments(L"--enable-features=WebContentsForceDark:inversion_method/cielab_based/image_behavior/none");

        if (!SUCCEEDED(hr))
            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to set additional browser arguments").c_str());
    }

    {
        Microsoft::WRL::ComPtr<ICoreWebView2EnvironmentOptions8> EnvironmentOptions8;

        hr = EnvironmentOptions.As(&EnvironmentOptions8);

        if (SUCCEEDED(hr))
        {
            const COREWEBVIEW2_SCROLLBAR_STYLE Style = COREWEBVIEW2_SCROLLBAR_STYLE_FLUENT_OVERLAY;

            hr = EnvironmentOptions8->put_ScrollBarStyle(Style); // See https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2environmentoptions8?view=webview2-1.0.2592.51

            if (!SUCCEEDED(hr))
                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to set scroll bar style").c_str());
        }
        else
            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to get ICoreWebView2EnvironmentOptions8 interface").c_str());
    }

    hr = ::CreateCoreWebView2EnvironmentWithOptions(nullptr, _Configuration._UserDataFolderPath.c_str(), EnvironmentOptions.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>
    (
        [this](HRESULT hr, ICoreWebView2Environment * environment) -> HRESULT
        {
            if (!SUCCEEDED(hr) || (environment == nullptr))
            {
                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to create environment").c_str());

                return E_INVALIDARG;
            }

            _Environment = environment;

            wil::com_ptr<ICoreWebView2Environment10> Environment10 = _Environment.try_query<ICoreWebView2Environment10>();

            if (Environment10 == nullptr)
            {
                console::print(::GetErrorMessage(E_NOINTERFACE, STR_COMPONENT_BASENAME " failed to get ICoreWebView2Environment10 interface").c_str());

                return E_NOINTERFACE;
            }

            // Create the controller options.
            wil::com_ptr<ICoreWebView2ControllerOptions> ControllerOptions;

            {
                hr = Environment10->CreateCoreWebView2ControllerOptions(&ControllerOptions);

                if (!SUCCEEDED(hr))
                {
                    console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to create controller options").c_str());

                    return hr;
                }

                // Enable In Private mode.
                {
                    hr = ControllerOptions->put_IsInPrivateModeEnabled(TRUE);

                    if (!SUCCEEDED(hr))
                        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to enable In Private mode").c_str());
                }

                {
                    hr = ControllerOptions->put_ProfileName(_Configuration._ProfileName.c_str());

                    if (!SUCCEEDED(hr))
                        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to set profile name").c_str());
                }

                {
                    LPWSTR ProfileName = nullptr;

                    hr = ControllerOptions->get_ProfileName(&ProfileName);

                    if (SUCCEEDED(hr))
                    {
                        console::print(::FormatText(STR_COMPONENT_BASENAME " is using profile \"%s\".", ::WideToUTF8(ProfileName).c_str()).c_str());

                        ::CoTaskMemFree(ProfileName);
                    }
                }
            }

            // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window.
            hr = Environment10->CreateCoreWebView2ControllerWithOptions(m_hWnd, ControllerOptions.get(), Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
            (
                [this](HRESULT hr, ICoreWebView2Controller * controller) -> HRESULT
                {
                    if (!SUCCEEDED(hr) || (controller == nullptr))
                    {
                        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to create controller").c_str());

                        return E_INVALIDARG;
                    }

                    _Controller = controller;

                    {
                        hr = _Controller->get_CoreWebView2(&_WebView);

                        if (!SUCCEEDED(hr))
                        {
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to get ICoreWebView2 interface from controller").c_str());

                            return hr;
                        }
                    }

                    _Controller->put_IsVisible(TRUE); // Required for the brain-dead CUI.

                    {
                        hr = ClearBrowserData();

                        if (!SUCCEEDED(hr))
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to clear browser data").c_str());

                        hr = SetDefaultBackgroundColor();

                        if (!SUCCEEDED(hr))
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to set background color").c_str());

                        hr = SetDarkMode(_DarkMode);

                        if (!SUCCEEDED(hr))
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to set theme colors").c_str());
                    }

                    // Add a few settings.
                    {
                        wil::com_ptr<ICoreWebView2Settings> Settings;

                        hr = _WebView->get_Settings(&Settings);

                        if (SUCCEEDED(hr))
                        {
                            (void) Settings->put_IsScriptEnabled(TRUE);
                            (void) Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            (void) Settings->put_IsWebMessageEnabled(TRUE);
                        }
                        else
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to get settings").c_str());
                    }

                    // Resize WebView to fit the bounds of the parent window.
                    {
                        RECT Bounds;

                        ::GetClientRect(m_hWnd, &Bounds);

                        (void) _Controller->put_Bounds(Bounds);
                    }

                    // Set a mapping between a virtual host name and a folder path to make it available to web sites via that host name. (E.g. L"<img src="http://foo_uie_webview.local/wv2.png"/>")
                    {
                        wil::com_ptr<ICoreWebView2_3> WebView03 = _WebView.try_query<ICoreWebView2_3>();

                        if (WebView03 != nullptr)
                        {
                            hr = WebView03->SetVirtualHostNameToFolderMapping(_HostName, _Configuration._UserDataFolderPath.c_str(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);

                            if (!SUCCEEDED(hr))
                                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to set virtual host name").c_str());
                        }
                        else
                            console::print(::GetErrorMessage(E_NOINTERFACE, STR_COMPONENT_BASENAME " failed to get ICoreWebView2_3 interface").c_str());
                    }

                    // Add an event handler to add the host object before navigation starts. That way the host object is available when the scripts start running.
                    {
                        hr = _WebView->add_NavigationStarting(Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>
                        (
                            [this](ICoreWebView2 * webView, ICoreWebView2NavigationStartingEventArgs * eventArgs) -> HRESULT
                            {
                                VARIANT RemoteObject = {};

                                _HostObject.query_to<IDispatch>(&RemoteObject.pdispVal);

                                if (RemoteObject.pdispVal == nullptr)
                                {
                                    console::print(::GetErrorMessage(E_NOINTERFACE, STR_COMPONENT_BASENAME " failed to get IDispatch interface from host object").c_str());

                                    return E_NOINTERFACE;
                                }

                                RemoteObject.vt = VT_DISPATCH;

                                // We can call AddHostObjectToScript multiple times in a row without calling RemoveHostObject first.
                                // This will replace the previous object with the new object. In our case this is the same object and everything is fine.
                                HRESULT hr = _WebView->AddHostObjectToScript(TEXT(STR_COMPONENT_BASENAME), &RemoteObject);

                                if (!SUCCEEDED(hr))
                                    console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add host object to script").c_str());

                                RemoteObject.pdispVal->Release();

                                return hr;
                            }
                        ).Get(), &_NavigationStartingToken);

                        if (!SUCCEEDED(hr))
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add NavigationStarting event handler").c_str());
                    }

                    // Add an event handler to know when navigation has completed.
                    {
                        hr = _WebView->add_NavigationCompleted(Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>
                        (
                            [this](ICoreWebView2 * webView, ICoreWebView2NavigationCompletedEventArgs * eventArgs) -> HRESULT
                            {
                                BOOL Success = TRUE;

                                HRESULT hr = eventArgs->get_IsSuccess(&Success);

                                if (!SUCCEEDED(hr))
                                {
                                    console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to get event status").c_str());
                                    return hr;
                                }

                                if (!Success)
                                {
                                    COREWEBVIEW2_WEB_ERROR_STATUS WebErrorStatus;

                                    hr = eventArgs->get_WebErrorStatus(&WebErrorStatus);

                                    if (SUCCEEDED(hr))
                                        console::print(::GetWebViewErrorMessage(WebErrorStatus, STR_COMPONENT_BASENAME " failed to navigate").c_str());
                                    else
                                        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to get web error status").c_str());
                                }

                                _IsNavigationCompleted = true;

                                return S_OK;
                            }
                        ).Get(), &_NavigationCompletedToken);

                        if (!SUCCEEDED(hr))
                            console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add NavigationCompleted event handler").c_str());
                    }

                    // Add custom context menu items.
                    {
                        wil::com_ptr<ICoreWebView2_11> WebView11 = _WebView.try_query<ICoreWebView2_11>();

                        if (WebView11 != nullptr)
                        {
                            hr = WebView11->add_ContextMenuRequested(Callback<ICoreWebView2ContextMenuRequestedEventHandler>
                            (
                                [this](ICoreWebView2 * sender, ICoreWebView2ContextMenuRequestedEventArgs * eventArgs)
                                {
                                    wil::com_ptr<ICoreWebView2ContextMenuRequestedEventArgs> Args = eventArgs;

                                    wil::com_ptr<ICoreWebView2ContextMenuTarget> Target;

                                    HRESULT hr = Args->get_ContextMenuTarget(&Target);

                                    if (!SUCCEEDED(hr))
                                    {
                                        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to get context menu target").c_str());
                                        return hr;
                                    }

                                    COREWEBVIEW2_CONTEXT_MENU_TARGET_KIND TargetKind;

                                    hr = Target->get_Kind(&TargetKind);

                                    if (!SUCCEEDED(hr))
                                        return hr;

                                    if (TargetKind != COREWEBVIEW2_CONTEXT_MENU_TARGET_KIND_PAGE)
                                        return hr;

                                    wil::com_ptr<ICoreWebView2ContextMenuItemCollection> Items;

                                    hr = Args->get_MenuItems(&Items);

                                    if (!SUCCEEDED(hr))
                                        return hr;

                                    UINT32 ItemCount;

                                    hr = Items->get_Count(&ItemCount);

                                    if (!SUCCEEDED(hr))
                                        return hr;

                                    hr = CreateContextMenu(TEXT(STR_COMPONENT_NAME), MAKEINTRESOURCE(IDR_CONTEXT_MENU_ICON));

                                    if (!SUCCEEDED(hr))
                                        return hr;

                                    hr = Items->InsertValueAtIndex(ItemCount, _ContextSubMenu.get());

                                    return hr;
                                }
                            ).Get(), &_ContextMenuRequestedToken);

                            if (!SUCCEEDED(hr))
                                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add ContextMenuRequested event handler").c_str());
                        }
                        else
                            console::print(::GetErrorMessage(E_NOINTERFACE, STR_COMPONENT_BASENAME " failed to get ICoreWebView2_11 interface").c_str());
                    }

                    // Tell the host we're fully initialized.
                    ::PostMessageW(m_hWnd, UM_WEB_VIEW_READY, 0, 0);

                    return S_OK;
                }
            ).Get());

            return hr;
        }
    ).Get());

    return hr;
}

/// <summary>
/// Deletes the WebView.
/// </summary>
void UIElement::DeleteWebView() noexcept
{
    ClearBrowserData();

    if (_WebView)
    {
        wil::com_ptr<ICoreWebView2_11> WebView11 = _WebView.try_query<ICoreWebView2_11>();

        if (WebView11 != nullptr)
            WebView11->remove_ContextMenuRequested(_ContextMenuRequestedToken);

        _WebView->remove_NavigationCompleted(_NavigationCompletedToken);

        _WebView->RemoveHostObjectFromScript(TEXT(STR_COMPONENT_BASENAME));

        _WebView->remove_NavigationStarting(_NavigationStartingToken);

        _WebView = nullptr;
    }

    _Controller = nullptr;

    if (_Environment != nullptr)
    {
        wil::com_ptr<ICoreWebView2Environment5> Environment5;

        HRESULT hr = _Environment->QueryInterface(IID_PPV_ARGS(&Environment5));

        if (SUCCEEDED(hr))
        {
            // Adds an event handler that gets called after all browser processes have terminated, after all resources have been released (including the user data folder).
            hr = Environment5->add_BrowserProcessExited(Microsoft::WRL::Callback<ICoreWebView2BrowserProcessExitedEventHandler>
            (
                [this, Environment5](ICoreWebView2Environment * environment, ICoreWebView2BrowserProcessExitedEventArgs * eventArgs) -> HRESULT
                {
                    HRESULT hr = Environment5->remove_BrowserProcessExited(_BrowserProcessExitedToken);

                    if (!SUCCEEDED(hr))
                        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to remove BrowserProcessExited event handler").c_str());

                    console::print(STR_COMPONENT_BASENAME " has terminated all browser processes.");

                    return S_OK;
                }
            ).Get(), &_BrowserProcessExitedToken);

            if (!SUCCEEDED(hr))
                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add BrowserProcessExited event handler").c_str());
        }

        _Environment = nullptr;
    }
}

/// <summary>
/// Recreates the WebView.
/// </summary>
HRESULT UIElement::RecreateWebView() noexcept
{
    if (_Environment == nullptr)
        return E_ILLEGAL_METHOD_CALL;

    wil::com_ptr<ICoreWebView2Environment5> Environment5;

    HRESULT hr = _Environment->QueryInterface(IID_PPV_ARGS(&Environment5));

    if (!SUCCEEDED(hr))
        return hr;

    // Adds an event handler that gets called after all browser processes have terminated, after all resources have been released (including the user data folder).
    hr = Environment5->add_BrowserProcessExited(Microsoft::WRL::Callback<ICoreWebView2BrowserProcessExitedEventHandler>
    (
        [this, Environment5](ICoreWebView2Environment * environment, ICoreWebView2BrowserProcessExitedEventArgs * eventArgs) -> HRESULT
        {
            HRESULT hr = Environment5->remove_BrowserProcessExited(_BrowserProcessExitedToken);

            if (!SUCCEEDED(hr))
                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to remove BrowserProcessExited event handler").c_str());

            CreateWebView();

            return S_OK;
        }
    ).Get(), &_BrowserProcessExitedToken);

    if (!SUCCEEDED(hr))
    {
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add BrowserProcessExited event handler").c_str());

        return hr;
    }

    DeleteWebView();

    return S_OK;
}

/// <summary>
/// Enables or disables dark mode.
/// </summary>
HRESULT UIElement::SetDarkMode(bool enabled) const noexcept
{
    if (_WebView == nullptr)
        return E_ILLEGAL_METHOD_CALL;

    wil::com_ptr<ICoreWebView2_13> WebView13 = _WebView.try_query<ICoreWebView2_13>();

    if (WebView13 == nullptr)
        return E_NOINTERFACE;

    wil::com_ptr<ICoreWebView2Profile> Profile;

    HRESULT hr = WebView13->get_Profile(&Profile);

    if (!SUCCEEDED(hr))
        return hr;

    return Profile->put_PreferredColorScheme(enabled ? COREWEBVIEW2_PREFERRED_COLOR_SCHEME_DARK : COREWEBVIEW2_PREFERRED_COLOR_SCHEME_LIGHT);
}

/// <summary>
/// Sets the default background color.
/// </summary>
HRESULT UIElement::SetDefaultBackgroundColor() const noexcept
{
    if (_Controller == nullptr)
        return E_ILLEGAL_METHOD_CALL;

    wil::com_ptr<ICoreWebView2Controller2> Controller2 = _Controller.try_query<ICoreWebView2Controller2>();

    if (Controller2 == nullptr)
        return E_NOINTERFACE;

    // Note: Semi-transparent background colors are not yet supported. (https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller2?view=webview2-1.0.2592.51#put_defaultbackgroundcolor)
    return Controller2->put_DefaultBackgroundColor({ 255, GetRValue(_BackgroundColor), GetGValue(_BackgroundColor), GetBValue(_BackgroundColor) });
}

/// <summary>
/// Creates the context menu.
/// </summary>
HRESULT UIElement::CreateContextMenu(const wchar_t * itemLabel, const wchar_t * iconName) noexcept
{
    if (itemLabel == nullptr)
        return E_INVALIDARG;

    if (_ContextSubMenu != nullptr)
        return S_FALSE; // Custom items should be reused whenever possible.

    wil::com_ptr<ICoreWebView2Environment9> Environment9;

    HRESULT hr = _Environment->QueryInterface(IID_PPV_ARGS(&Environment9));

    if (!SUCCEEDED(hr))
        return hr;

    // Create the sub menu.
    {
        wil::com_ptr<IStream> IconStream;

        (void) CreateIconStream(iconName, IconStream); // Ignore any error.

        hr = Environment9->CreateContextMenuItem(itemLabel, IconStream.get(), COREWEBVIEW2_CONTEXT_MENU_ITEM_KIND_SUBMENU, &_ContextSubMenu);

        if (!SUCCEEDED(hr))
            return hr;

        wil::com_ptr<ICoreWebView2ContextMenuItemCollection> Children;

        hr = _ContextSubMenu->get_Children(&Children);

        if (!SUCCEEDED(hr))
            return hr;

        wil::com_ptr<ICoreWebView2ContextMenuItem> ContextMenuItem;

        // Creates a menu item.
        {
            hr = Environment9->CreateContextMenuItem(L"Preferences", nullptr, COREWEBVIEW2_CONTEXT_MENU_ITEM_KIND_COMMAND, &ContextMenuItem);

            if (!SUCCEEDED(hr))
                return hr;

            hr = ContextMenuItem->add_CustomItemSelected(Callback<ICoreWebView2CustomItemSelectedEventHandler>
            (
                [this](ICoreWebView2ContextMenuItem * sender, IUnknown * args)
                {
                    RunAsync([this] { ShowPreferences(); });

                    return S_OK;
                }
            ).Get(), nullptr);

            if (!SUCCEEDED(hr))
                return hr;
        }

        hr = Children->InsertValueAtIndex(0, ContextMenuItem.get());
    }

    return hr;
}

/// <summary>
/// Clears all browser data.
/// </summary>
HRESULT UIElement::ClearBrowserData() const noexcept
{
    if (_WebView == nullptr)
        return E_ILLEGAL_METHOD_CALL;

    wil::com_ptr<ICoreWebView2_13> WebView13 = _WebView.try_query<ICoreWebView2_13>();

    if (WebView13 == nullptr)
        return E_NOINTERFACE;

    wil::com_ptr<ICoreWebView2Profile> Profile;

    HRESULT hr = WebView13->get_Profile(&Profile);

    if (!SUCCEEDED(hr))
        return hr;

    auto Profile2 = Profile.try_query<ICoreWebView2Profile2>();

    if (Profile2 == nullptr)
        return E_NOINTERFACE;

    hr = Profile2->ClearBrowsingDataAll(Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>
    (
        [](HRESULT hr) -> HRESULT
        {
            if (SUCCEEDED(hr))
                console::print(STR_COMPONENT_BASENAME " has cleared all browsing data.");
            else
                console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to clear browsing data").c_str());

            return S_OK;
        }
    ).Get());

    return hr;
}

/// <summary>
/// Requests the deletion of the browser profile.
/// </summary>
HRESULT UIElement::RequestBrowserProfileDeletion() const noexcept
{
    if (_WebView == nullptr)
        return E_ILLEGAL_METHOD_CALL;

    wil::com_ptr<ICoreWebView2_13> WebView13 = _WebView.try_query<ICoreWebView2_13>();

    if (WebView13 == nullptr)
        return E_NOINTERFACE;

    wil::com_ptr<ICoreWebView2Profile> Profile;

    HRESULT hr = WebView13->get_Profile(&Profile);

    if (!SUCCEEDED(hr))
        return hr;

    auto Profile8 = Profile.try_query<ICoreWebView2Profile8>();

    if (Profile8 == nullptr)
        return E_NOINTERFACE;

    hr = Profile8->add_Deleted(Microsoft::WRL::Callback<ICoreWebView2ProfileDeletedEventHandler>
    (
        [Profile8](ICoreWebView2Profile * sender, IUnknown * args)
        {
            LPWSTR ProfilePath = nullptr;

            (void) Profile8->get_ProfilePath(&ProfilePath);

            console::printf(STR_COMPONENT_BASENAME " has marked the browser profile in \"%s\" for deletion", (ProfilePath != nullptr) ? ::WideToUTF8(ProfilePath).c_str() : "");

            if (ProfilePath)
                ::CoTaskMemFree(ProfilePath);

            return S_OK;
        }).Get(), nullptr
    );

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, STR_COMPONENT_BASENAME " failed to add Deleted event handler").c_str());

    hr = Profile8->Delete();

    return hr;
}

/// <summary>
/// Creates a stream that contains the complete binary data of a Windows icon. The resource type is RT_RCDATA instead of RT_GROUP_ICON or RT_ICON.
/// </summary>
HRESULT CreateIconStream(const wchar_t * resourceName, wil::com_ptr<IStream> & stream)
{
    HRSRC hResource = ::FindResourceW(THIS_INSTANCE, resourceName, RT_RCDATA);

    if (hResource == NULL)
        return HRESULT_FROM_WIN32(::GetLastError());

    DWORD ResourceSize = ::SizeofResource(THIS_INSTANCE, hResource);

    if (ResourceSize == 0)
        return HRESULT_FROM_WIN32(::GetLastError());

    HGLOBAL hGlobal = ::LoadResource(THIS_INSTANCE, hResource);

    if (hGlobal == NULL)
        return HRESULT_FROM_WIN32(::GetLastError());

    BYTE * ResourceData = (BYTE *) ::LockResource(hGlobal);

    if (ResourceData == nullptr)
        return HRESULT_FROM_WIN32(::GetLastError());

    IStream * Stream = ::SHCreateMemStream(ResourceData, ResourceSize);

    if (Stream == nullptr)
        return HRESULT_FROM_WIN32(::GetLastError());

    stream = Stream;

    return S_OK;
}

/// <summary>
/// Gets the description of a web error status.
/// </summary>
std::string GetWebViewErrorMessage(COREWEBVIEW2_WEB_ERROR_STATUS status, const std::string & errorMessage) noexcept
{
    const char * Messages[] =
    {
        /* Unknown */                                   "An unknown error occurred.",
        /* CertificateCommonNameIsIncorrect */          "The SSL certificate common name does not match the web address.",
        /* CertificateExpired */                        "The SSL certificate has expired."
        /* ClientCertificateContainsErrors */           "The SSL client certificate contains errors.",
        /* CertificateRevoked */                        "The SSL certificate has been revoked.",
        /* CertificateIsInvalid */                      "The SSL certificate is not valid.",
        /* ServerUnreachable */                         "The host is unreachable.",
        /* Timeout */                                   "The connection has timed out.",
        /* ErrorHttpInvalidServerResponse */            "The server returned an invalid or unrecognized response.",
        /* ConnectionAborted */                         "The connection was stopped.",
        /* ConnectionReset */                           "The connection was reset.",
        /* Disconnected */                              "The Internet connection has been lost.",
        /* CannotConnect */                             "A connection to the destination was not established.",
        /* HostNameNotResolved */                       "The provided host name was not able to be resolved.",
        /* OperationCanceled */                         "The operation was canceled.",
        /* RedirectFailed */                            "The request redirect failed.",
        /* UnexpectedError */                           "An unexpected error occurred.",
        /* ValidAuthenticationCredentialsRequired */    "The user is prompted with a login, waiting on user action.",
        /* ValidProxyAuthenticationRequired */          "The user lacks proper authentication credentials for a proxy server.",
    };

    return ::FormatText("%s: %s (%d)", errorMessage.c_str(), (((size_t) status  < _countof(Messages)) ? Messages[(size_t) status] : "Invalid web error status"), (int) status);
}
