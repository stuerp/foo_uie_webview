
/** $VER: HostObjectImpl.cpp (2024.05.27) P. Stuer **/

#include "pch.h"

#include "HostObjectImpl.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#include "Resources.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>

#include <pfc/string-conv-lite.h>

HostObject::HostObject(HostObject::RunCallbackAsync runCallbackAsync) : m_propertyValue(L"Example Property String Value"), m_runCallbackAsync(runCallbackAsync)
{
}

STDMETHODIMP HostObject::GetFormattedText(BSTR text, BSTR * formattedText)
{
    static_api_ptr_t<playlist_manager> _PlaylistManager;

    t_size PlaylistIndex = ~0u;
    t_size ItemIndex = ~0u;

    if (!_PlaylistManager->get_playing_item_location(&PlaylistIndex, &ItemIndex))
    {
        PlaylistIndex = _PlaylistManager->get_active_playlist();

        if (PlaylistIndex == ~0u)
            return E_FAIL;

        ItemIndex = _PlaylistManager->playlist_get_focus_item(PlaylistIndex);

        if (ItemIndex == ~0u)
            return E_FAIL;
    }

    titleformat_object::ptr FormatObject;

    bool Success = titleformat_compiler::get()->compile(FormatObject, pfc::utf8FromWide(text));

    if (!Success)
        return E_FAIL;

    pfc::string8 FormattedText;

    _PlaylistManager->playlist_item_format_title(PlaylistIndex, ItemIndex, nullptr, FormattedText, FormatObject, nullptr, playback_control::t_display_level::display_level_all);

    *formattedText = ::SysAllocString(pfc::wideFromUTF8(FormattedText).c_str());

    return S_OK;
}

STDMETHODIMP HostObject::get_Property(BSTR * stringResult)
{
    *stringResult = ::SysAllocString(m_propertyValue.c_str());

    return S_OK;
}

STDMETHODIMP HostObject::put_Property(BSTR stringValue)
{
    m_propertyValue = stringValue;

    return S_OK;
}

STDMETHODIMP HostObject::get_IndexedProperty(INT index, BSTR * stringResult)
{
    std::wstring result(L"[");
    result = result + std::to_wstring(index) + L"] is " + m_propertyValues[index] + L".";
    *stringResult = SysAllocString(result.c_str());
    return S_OK;
}

STDMETHODIMP HostObject::put_IndexedProperty(INT index, BSTR stringValue)
{
    m_propertyValues[index] = stringValue;
    return S_OK;
}

STDMETHODIMP HostObject::get_DateProperty(DATE * dateResult)
{
    *dateResult = m_date;
    return S_OK;
}

STDMETHODIMP HostObject::put_DateProperty(DATE dateValue)
{
    m_date = dateValue;

    SYSTEMTIME systemTime;

    if (VariantTimeToSystemTime(dateValue, &systemTime))
    {
        // Save the Date and Time as strings to be able to easily check that we are getting the correct values.
        GetDateFormatEx(LOCALE_NAME_INVARIANT, 0 /*flags*/, &systemTime, NULL /*format*/, m_formattedDate, ARRAYSIZE(m_formattedDate), NULL /*reserved*/);
        GetTimeFormatEx(LOCALE_NAME_INVARIANT, 0 /*flags*/, &systemTime, NULL /*format*/, m_formattedTime, ARRAYSIZE(m_formattedTime));
    }

    return S_OK;
}

STDMETHODIMP HostObject::CreateNativeDate()
{
    SYSTEMTIME systemTime;

    ::GetSystemTime(&systemTime);

    DATE date;

    if (SystemTimeToVariantTime(&systemTime, &date))
        return put_DateProperty(date);

    return E_UNEXPECTED;
}

STDMETHODIMP HostObject::CallCallbackAsynchronously(IDispatch * callbackParameter)
{
    wil::com_ptr<IDispatch> callbackParameterForCapture = callbackParameter;

    m_runCallbackAsync([callbackParameterForCapture]() -> void
    {
        DISPPARAMS DispParams = { };

        callbackParameterForCapture->Invoke(DISPID_UNKNOWN, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &DispParams, nullptr, nullptr, nullptr);
    });

    return S_OK;
}

STDMETHODIMP HostObject::GetTypeInfoCount(UINT * pctinfo)
{
    *pctinfo = 1;

    return S_OK;
}

/// <summary>
/// Gets the handle of the module that contains the executing code.
/// </summary>
inline HMODULE GetCurrentModule() noexcept
{
    HMODULE hModule = NULL;

    ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR) GetCurrentModule, &hModule);

    return hModule;
}

STDMETHODIMP HostObject::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo ** ppTInfo)
{
    if (iTInfo != 0)
        return TYPE_E_ELEMENTNOTFOUND;

    if (_TypeLibrary == nullptr)
    {
        wchar_t FilePath[MAX_PATH];

        HMODULE hModule = GetCurrentModule();;

        if (hModule == NULL)
            return HRESULT_FROM_WIN32(::GetLastError());

        if (::GetModuleFileNameW(hModule, FilePath, _countof(FilePath)) == 0)
            return HRESULT_FROM_WIN32(::GetLastError());

        RETURN_IF_FAILED(::PathCchRemoveFileSpec(FilePath, _countof(FilePath)));

        ::PathCchAppend(FilePath, _countof(FilePath), TEXT(STR_COMPONENT_BASENAME) L".tlb");

        RETURN_IF_FAILED(LoadTypeLib(FilePath, &_TypeLibrary));
    }

    return _TypeLibrary->GetTypeInfoOfGuid(__uuidof(IHostObject), ppTInfo);
}

STDMETHODIMP HostObject::GetIDsOfNames(REFIID riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId)
{
    wil::com_ptr<ITypeInfo> TypeInfo;

    RETURN_IF_FAILED(GetTypeInfo(0, lcid, &TypeInfo));

    return TypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispId);
}

/// <summary>
/// Provides access to the properties and methods exposed by this object.
/// </summary>
STDMETHODIMP HostObject::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * dispParams, VARIANT * result, EXCEPINFO * excepInfo, UINT * argErr)
{
    wil::com_ptr<ITypeInfo> TypeInfo;

    RETURN_IF_FAILED(GetTypeInfo(0, lcid, &TypeInfo));

    return TypeInfo->Invoke(this, dispIdMember, flags, dispParams, result, excepInfo, argErr);
}
