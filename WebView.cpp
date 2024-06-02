
/** $VER: WebView.cpp (2024.06.02) P. Stuer - Creates the WebView. **/

#include "pch.h"

#include "UIElement.h"
#include "Exceptions.h"

#include <pfc/string-conv-lite.h>
#include <pfc/pathUtils.h>

using namespace Microsoft::WRL;

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
        throw Win32Exception((DWORD) hResult, "Failed to initialize COM");

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

                    // Resize WebView to fit the bounds of the parent window
                    {
                        RECT Bounds;

                        ::GetClientRect(m_hWnd, &Bounds);

                        _Controller->put_Bounds(Bounds);
                    }

                    // Set a mapping between a virtual host name and a folder path to make it available to web sites via that host name. (E,g, L"<img src="http://foo_vis_text.local/wv2.png"/>")
                    {
                        wil::com_ptr<ICoreWebView2_3> WebView2_3 = _WebView.try_query<ICoreWebView2_3>();

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
                                HRESULT hResult = _WebView->AddHostObjectToScript(TEXT(STR_COMPONENT_BASENAME), &RemoteObject);

                                RemoteObject.pdispVal->Release();

                                return hResult;
                            }
                        ).Get(), &_NavigationStartingToken);
                    }

                    // Add custom context menu items.
                    {
                        wil::com_ptr<ICoreWebView2_11> WebView2_11 = _WebView.try_query<ICoreWebView2_11>();

                        if (WebView2_11 == nullptr)
                            return S_OK;

                        hResult = WebView2_11->add_ContextMenuRequested(Callback<ICoreWebView2ContextMenuRequestedEventHandler>
                        (
                            [this](ICoreWebView2 * sender, ICoreWebView2ContextMenuRequestedEventArgs * eventArgs)
                            {
                                wil::com_ptr<ICoreWebView2ContextMenuRequestedEventArgs> Args = eventArgs;

                                wil::com_ptr<ICoreWebView2ContextMenuTarget> Target;

                                HRESULT hr = Args->get_ContextMenuTarget(&Target);

                                if (!SUCCEEDED(hr))
                                    return S_OK;

                                COREWEBVIEW2_CONTEXT_MENU_TARGET_KIND TargetKind;

                                hr = Target->get_Kind(&TargetKind);

                                if (!SUCCEEDED(hr))
                                    return S_OK;

                                if (TargetKind != COREWEBVIEW2_CONTEXT_MENU_TARGET_KIND_PAGE)
                                    return S_OK;

                                wil::com_ptr<ICoreWebView2ContextMenuItemCollection> Items;

                                hr = Args->get_MenuItems(&Items);

                                if (!SUCCEEDED(hr))
                                    return S_OK;

                                UINT32 ItemCount;

                                hr = Items->get_Count(&ItemCount);

                                if (!SUCCEEDED(hr))
                                    return S_OK;

                                // Add the context menu item.
                                {
                                    // Custom items should be reused whenever possible.
                                    if (_ContextMenuItem == nullptr)
                                    {
                                        wil::com_ptr<ICoreWebView2Environment9> Environment9;

                                        hr = _Environment->QueryInterface(IID_PPV_ARGS(&Environment9));

                                        if (!SUCCEEDED(hr))
                                            return S_OK;

                                        hr = Environment9->CreateContextMenuItem(TEXT(STR_COMPONENT_NAME), nullptr, COREWEBVIEW2_CONTEXT_MENU_ITEM_KIND_SUBMENU, &_ContextMenuItem);

                                        if (!SUCCEEDED(hr))
                                            return S_OK;

                                        wil::com_ptr<ICoreWebView2ContextMenuItemCollection> Children;

                                        hr = _ContextMenuItem->get_Children(&Children);

                                        if (!SUCCEEDED(hr))
                                            return S_OK;

                                        wil::com_ptr<ICoreWebView2ContextMenuItem> MenuItem;
/*
                                        wil::com_ptr<IStream> IconStream;

                                        ::SHCreateStreamOnFileEx(L"small.ico", STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &IconStream);
*/
                                        hr = Environment9->CreateContextMenuItem(L"Preferences", nullptr, COREWEBVIEW2_CONTEXT_MENU_ITEM_KIND_COMMAND, &MenuItem);

                                        if (!SUCCEEDED(hr))
                                            return S_OK;

                                        hr = MenuItem->add_CustomItemSelected(Callback<ICoreWebView2CustomItemSelectedEventHandler>
                                        (
                                            [this, Target](ICoreWebView2ContextMenuItem * sender, IUnknown * args)
                                            {
                                                RunAsync([this] { ShowPreferences(); });

                                                return S_OK;
                                            }
                                        ).Get(), nullptr);

                                        if (!SUCCEEDED(hr))
                                            return S_OK;

                                        hr = Children->InsertValueAtIndex(0, MenuItem.get());
                                    }

                                    hr = Items->InsertValueAtIndex(ItemCount, _ContextMenuItem.get());
                                }

                                // Display the context menu.
                                return S_OK;
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
        throw Win32Exception((DWORD) hResult, "Failed to create WebView");
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
