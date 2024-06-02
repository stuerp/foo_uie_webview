
/** $VER: WebView.cpp (2024.06.02) P. Stuer - Creates the WebView. **/

#include "pch.h"

#include "UIElement.h"
#include "Exceptions.h"

#include <pfc/string-conv-lite.h>
#include <pfc/pathUtils.h>

using namespace Microsoft::WRL;

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

            // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window.
            environment->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
            (
                [this](HRESULT hResult, ICoreWebView2Controller * controller) -> HRESULT
                {
                    UNREFERENCED_PARAMETER(hResult);

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

                    {
                        // Add an event handler to add the host object before navigation starts. That way the host object is available when the scripts start running.
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

                        ::PostMessageW(m_hWnd, UM_WEB_VIEW_READY, 0, 0);
                    }

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
