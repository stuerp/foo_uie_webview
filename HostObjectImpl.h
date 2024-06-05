
/** $VER: HostObjectImpl.h (2024.06.05) P. Stuer **/

#pragma once

#include "pch.h"

#include <functional>
#include <map>
#include <string>

#include <wrl.h>
#include <wrl/client.h>
#include <wil/com.h>
#include <wil/resource.h>
#include <wil/result.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

#include "HostObject_h.h"

class HostObject : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IHostObject, IDispatch>
{
public:
    HostObject(const HostObject &) = delete;
    HostObject(const HostObject &&) = delete;
    HostObject & operator=(const HostObject &) = delete;
    HostObject & operator=(HostObject &&) = delete;

    virtual ~HostObject() { };

    typedef std::function<void(void)> Callback;
    typedef std::function<void(Callback)> RunCallbackAsync;

    HostObject(RunCallbackAsync runCallbackAsync);

    #pragma region IHostObject

    STDMETHODIMP GetFormattedText(BSTR text, BSTR * formattedText) override;

    #pragma endregion

    #pragma region IDispatch

    STDMETHODIMP GetTypeInfoCount(UINT * typeInfoCount) override;
    STDMETHODIMP GetTypeInfo(UINT typeInfoIndex, LCID lcid, ITypeInfo ** typeInfo) override;
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR * names, UINT nameCount, LCID lcid, DISPID * dispIds) override;
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * dispParams, VARIANT * result, EXCEPINFO * excepInfo, UINT * argErr) override;

    #pragma endregion

private:
    static HRESULT GetTrackIndex(t_size & playlistIndex, t_size & itemIndex) noexcept;
    static HRESULT GetTypeLibFilePath(std::wstring & filePath) noexcept;

private:
    wil::com_ptr<ITypeLib> _TypeLibrary;

    wil::com_ptr<IDispatch> _Callback;
    RunCallbackAsync _RunCallbackAsync;
};
