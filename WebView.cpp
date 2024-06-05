
/** $VER: WebView.cpp (2024.06.03) P. Stuer - Creates the WebView. **/

#include "pch.h"

#include "UIElement.h"
#include "Exceptions.h"

#include <pfc/string-conv-lite.h>
#include <pfc/pathUtils.h>

using namespace Microsoft::WRL;

static HRESULT CreateIconStream(const wchar_t * resourceName, wil::com_ptr<IStream> & stream);

/// <summary>
/// Returns true if a supported WebView version is available on this system.
/// </summary>
bool UIElement::GetWebViewVersion(std::wstring & versionString)
{
    LPWSTR VersionString = nullptr;

    HRESULT hResult = ::GetAvailableCoreWebView2BrowserVersionString(nullptr, &VersionString);

    if (!SUCCEEDED(hResult))
        return false;

    versionString = VersionString;

    ::CoTaskMemFree(VersionString);

    return true;
}

/// <summary>
/// Creates the WebView.
/// </summary>
void UIElement::CreateWebView()
{
    HRESULT hResult = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    if (!SUCCEEDED(hResult))
        throw Win32Exception(hResult, "Failed to initialize COM");

    hResult = ::CreateCoreWebView2EnvironmentWithOptions(nullptr, _UserDataFolderPath.c_str(), nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>
    (
        [this](HRESULT hResult, ICoreWebView2Environment * environment) -> HRESULT
        {
            UNREFERENCED_PARAMETER(hResult);

            _Environment = environment;

            // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window.
            environment->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
            (
                [this](HRESULT hResult, ICoreWebView2Controller * controller) -> HRESULT
                {
                    if (controller != nullptr)
                    {
                        _Controller = controller;
                        _Controller->get_CoreWebView2(&_WebView);
                    }

                    (void) SetDarkMode(_DarkMode); // Ignore result.

                    // Add a few settings.
                    {
                        wil::com_ptr<ICoreWebView2Settings> Settings;

                        hResult = _WebView->get_Settings(&Settings);

                        if (SUCCEEDED(hResult))
                        {
                            Settings->put_IsScriptEnabled(TRUE);
                            Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            Settings->put_IsWebMessageEnabled(TRUE);
                        }
                    }

                    // Resize WebView to fit the bounds of the parent window.
                    {
                        RECT Bounds;

                        ::GetClientRect(m_hWnd, &Bounds);

                        _Controller->put_Bounds(Bounds);
                    }

                    // Set a mapping between a virtual host name and a folder path to make it available to web sites via that host name. (E.g. L"<img src="http://foo_vis_text.local/wv2.png"/>")
                    {
                        wil::com_ptr<ICoreWebView2_3> WebView2_3 = _WebView.try_query<ICoreWebView2_3>();

                        if (WebView2_3 == nullptr)
                            return E_NOINTERFACE;

                        hResult = WebView2_3->SetVirtualHostNameToFolderMapping(_HostName, _ProfilePath.c_str(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
                    }

                    // Add an event handler to add the host object before navigation starts. That way the host object is available when the scripts start running.
                    {
                        _WebView->add_NavigationStarting(Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>
                        (
                            [this](ICoreWebView2 * webView, ICoreWebView2NavigationStartingEventArgs * args) -> HRESULT
                            {
                                VARIANT RemoteObject = {};

                                _HostObject.query_to<IDispatch>(&RemoteObject.pdispVal);

                                RemoteObject.vt = VT_DISPATCH;

                                // We can call AddHostObjectToScript multiple times in a row without calling RemoveHostObject first.
                                // This will replace the previous object with the new object. In our case this is the same object and everything is fine.
                                HRESULT hr = _WebView->AddHostObjectToScript(TEXT(STR_COMPONENT_BASENAME), &RemoteObject);

                                RemoteObject.pdispVal->Release();

                                return hr;
                            }
                        ).Get(), &_NavigationStartingToken);
                    }

                    // Add custom context menu items.
                    {
                        wil::com_ptr<ICoreWebView2_11> WebView2_11 = _WebView.try_query<ICoreWebView2_11>();

                        if (WebView2_11 == nullptr)
                            return E_NOINTERFACE;

                        hResult = WebView2_11->add_ContextMenuRequested(Callback<ICoreWebView2ContextMenuRequestedEventHandler>
                        (
                            [this](ICoreWebView2 * sender, ICoreWebView2ContextMenuRequestedEventArgs * eventArgs)
                            {
                                wil::com_ptr<ICoreWebView2ContextMenuRequestedEventArgs> Args = eventArgs;

                                wil::com_ptr<ICoreWebView2ContextMenuTarget> Target;

                                HRESULT hr = Args->get_ContextMenuTarget(&Target);

                                if (!SUCCEEDED(hr))
                                    return hr;

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
                    }

                    ::PostMessageW(m_hWnd, UM_WEB_VIEW_READY, 0, 0);

                    return S_OK;
                }
            ).Get());

            return S_OK;
        }
    ).Get());

    if (!SUCCEEDED(hResult))
        throw Win32Exception(hResult, "Failed to create WebView");
}

/// <summary>
/// Deletes the WebView.
/// </summary>
void UIElement::DeleteWebView() noexcept
{
    if (_WebView)
    {
        _WebView->RemoveHostObjectFromScript(TEXT(STR_COMPONENT_BASENAME));

        _WebView->remove_NavigationStarting(_NavigationStartingToken);

        _WebView = nullptr;
    }

    _Controller = nullptr;
}

/// <summary>
/// Enables or disables dark mode.
/// </summary>
HRESULT UIElement::SetDarkMode(bool enabled) const noexcept
{
    if (_WebView == nullptr)
        return E_ILLEGAL_METHOD_CALL;

    wil::com_ptr<ICoreWebView2_13> WebView2_13 = _WebView.try_query<ICoreWebView2_13>();

    if (WebView2_13 == nullptr)
        return E_NOINTERFACE;

    wil::com_ptr<ICoreWebView2Profile> Profile;

    HRESULT hr = WebView2_13->get_Profile(&Profile);

    if (!SUCCEEDED(hr))
        return hr;

    return Profile->put_PreferredColorScheme(enabled ? COREWEBVIEW2_PREFERRED_COLOR_SCHEME_DARK : COREWEBVIEW2_PREFERRED_COLOR_SCHEME_LIGHT);
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
