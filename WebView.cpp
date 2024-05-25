
/** $VER: WebView.cpp (2024.05.25) P. Stuer - Creates the WebView. **/

#include "pch.h"

#include <pfc/string-conv-lite.h>
#include <pfc/pathUtils.h>

#include <wrl.h>
#include <wil/com.h>

#include "WebView2.h"

using namespace Microsoft::WRL;

#include "UIElement.h"

/// <summary>
/// Creates the WebView.
/// </summary>
void UIElement::CreateWebView(HWND hWnd) noexcept
{
    std::wstring UserDataFolderPath = pfc::wideFromUTF8(_ProfilePath).c_str();

    ::CreateCoreWebView2EnvironmentWithOptions(nullptr, UserDataFolderPath.c_str(), nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([hWnd, this](HRESULT hResult, ICoreWebView2Environment * environment) -> HRESULT
    {
        UNREFERENCED_PARAMETER(hResult);

        // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window.
        environment->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([hWnd, this](HRESULT hResult, ICoreWebView2Controller * controller) -> HRESULT
        {
            UNREFERENCED_PARAMETER(hResult);

            if (controller != nullptr)
            {
                _Controller = controller;
                _Controller->get_CoreWebView2(&_WebView);
            }

            // Add a few settings for the webview. The demo step is redundant since the values are the default settings.
            {
                wil::com_ptr<ICoreWebView2Settings> Settings;

                _WebView->get_Settings(&Settings);

                Settings->put_IsScriptEnabled(TRUE);
                Settings->put_AreDefaultScriptDialogsEnabled(FALSE);
                Settings->put_IsWebMessageEnabled(FALSE);
            }

            // Resize WebView to fit the bounds of the parent window
            {
                RECT Bounds;

                ::GetClientRect(hWnd, &Bounds);

                _Controller->put_Bounds(Bounds);
            }

            // Set a mapping between a virtual host name and a folder path to make available to web sites via that host name. (E,g, L"<img src=\"http://foo_vis_text.local/wv2.png\"/>")

            wil::com_ptr<ICoreWebView2_3> WebView23 = _WebView.try_query<ICoreWebView2_3>();

            hResult = WebView23->SetVirtualHostNameToFolderMapping(L"foo_vis_text.local", pfc::wideFromUTF8(_ProfilePath).c_str(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);

            StartTimer();

            return S_OK;
        }).Get());

        return S_OK;
    }).Get());
}
