
/** $VER: HostObjectImpl.h (2024.06.03) P. Stuer **/

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

    // Demonstrate getting and setting a property.
    STDMETHODIMP get_Property(BSTR* stringResult) override;
    STDMETHODIMP put_Property(BSTR stringValue) override;
    STDMETHODIMP get_IndexedProperty(INT index, BSTR* stringResult) override;
    STDMETHODIMP put_IndexedProperty(INT index, BSTR stringValue) override;
    STDMETHODIMP get_DateProperty(DATE* dateResult) override;
    STDMETHODIMP put_DateProperty(DATE dateValue) override;
    STDMETHODIMP CreateNativeDate() override;

    // Demonstrate native calling back into JavaScript.
    STDMETHODIMP CallCallbackAsynchronously(IDispatch* callbackParameter) override;

    #pragma endregion

    #pragma region IDispatch

    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo) override;
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo ** ppTInfo) override;
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId) override;
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * dispParams, VARIANT * result, EXCEPINFO * excepInfo, UINT * argErr) override;

    #pragma endregion

    void SetFollowSelectedTrackMode(bool enabled) noexcept { _FollowSelectedTrack = enabled; }

private:
    wil::com_ptr<ITypeLib> _TypeLibrary;

    std::wstring _PropertyValue;
    std::map<INT, std::wstring> _PropertyValues;
    wil::com_ptr<IDispatch> _Callback;
    RunCallbackAsync _RunCallbackAsync;

    DATE _Date = 0.;
    WCHAR _FormattedTime[200] = {};
    WCHAR _FormattedDate[200] = {};

    bool _FollowSelectedTrack;
};
