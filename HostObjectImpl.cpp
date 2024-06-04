
/** $VER: HostObjectImpl.cpp (2024.06.03) P. Stuer **/

#include "pch.h"

#include "HostObjectImpl.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#include "Support.h"
#include "Resources.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/ui.h>
#include <SDK/contextmenu.h>

#include <pfc/string-conv-lite.h>

/// <summary>
/// Initializes a new instance
/// </summary>
HostObject::HostObject(HostObject::RunCallbackAsync runCallbackAsync) : _PropertyValue(L"Example Property String Value"), _RunCallbackAsync(runCallbackAsync)
{
}

/// <summary>
/// Gets the interpreted version of the specified text containing Title Formating.
/// </summary>
STDMETHODIMP HostObject::GetFormattedText(BSTR text, BSTR * formattedText)
{
    static_api_ptr_t<playlist_manager> _PlaylistManager;

    t_size PlaylistIndex = ~0u;
    t_size ItemIndex = ~0u;

    auto SelectionManager = ui_selection_manager::get();

    auto SelectionType = SelectionManager->get_selection_type();

    // Description as used in the Preferences dialog (Display / Selection Viewers).
    const bool PreferCurrentlyPlayingTrack = (SelectionType == contextmenu_item::caller_now_playing);
    const bool PreferCurrentSelection      = (SelectionType == contextmenu_item::caller_active_playlist_selection);

    bool IsTrackPlaying = false;

    if (PreferCurrentlyPlayingTrack)
        IsTrackPlaying = _PlaylistManager->get_playing_item_location(&PlaylistIndex, &ItemIndex);

    if (PreferCurrentSelection || !IsTrackPlaying)
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

STDMETHODIMP HostObject::get_Property(BSTR * value_)
{
    *value_ = ::SysAllocString(_PropertyValue.c_str());

    return S_OK;
}

STDMETHODIMP HostObject::put_Property(BSTR value_)
{
    _PropertyValue = value_;

    return S_OK;
}

STDMETHODIMP HostObject::get_IndexedProperty(INT index, BSTR * result)
{
    std::wstring Result(L"[");

    Result = Result + std::to_wstring(index) + L"] is " + _PropertyValues[index] + L".";

    *result = ::SysAllocString(Result.c_str());

    return S_OK;
}

STDMETHODIMP HostObject::put_IndexedProperty(INT index, BSTR value_)
{
    _PropertyValues[index] = value_;

    return S_OK;
}

STDMETHODIMP HostObject::get_DateProperty(DATE * value_)
{
    *value_ = _Date;

    return S_OK;
}

STDMETHODIMP HostObject::put_DateProperty(DATE value_)
{
    _Date = value_;

    SYSTEMTIME st;

    if (::VariantTimeToSystemTime(value, &st))
    {
        // Save the Date and Time as strings to be able to easily check that we are getting the correct values.

        ::GetDateFormatEx(LOCALE_NAME_INVARIANT, 0, &st, nullptr, _FormattedDate, _countof(_FormattedDate), nullptr);
        ::GetTimeFormatEx(LOCALE_NAME_INVARIANT, 0, &st, nullptr, _FormattedTime, _countof(_FormattedTime));
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

    _RunCallbackAsync
    (
        [callbackParameterForCapture]() -> void
        {
            DISPPARAMS DispParams = { };

            callbackParameterForCapture->Invoke(DISPID_UNKNOWN, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &DispParams, nullptr, nullptr, nullptr);
        }
    );

    return S_OK;
}

#pragma region IDispatch

STDMETHODIMP HostObject::GetTypeInfoCount(UINT * typeInfoCount)
{
    *typeInfoCount = 1;

    return S_OK;
}

STDMETHODIMP HostObject::GetTypeInfo(UINT typeInfoIndex, LCID lcid, ITypeInfo ** typeInfo)
{
    if (typeInfoIndex != 0)
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

    return _TypeLibrary->GetTypeInfoOfGuid(__uuidof(IHostObject), typeInfo);
}

STDMETHODIMP HostObject::GetIDsOfNames(REFIID riid, LPOLESTR * names, UINT nameCount, LCID lcid, DISPID * dispIds)
{
    wil::com_ptr<ITypeInfo> TypeInfo;

    RETURN_IF_FAILED(GetTypeInfo(0, lcid, &TypeInfo));

    return TypeInfo->GetIDsOfNames(names, nameCount, dispIds);
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

#pragma endregion
