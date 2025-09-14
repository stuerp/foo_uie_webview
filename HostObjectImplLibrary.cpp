
/** $VER: HostObjectImplLibrary.cpp (2025.01.06) P. Stuer **/

#include "pch.h"

#include "HostObjectImpl.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#include "Support.h"
#include "Resources.h"
#include "Encoding.h"

#include <SDK/search_tools.h>
#include <SDK/library_callbacks.h>

#include <pfc/string-conv-lite.h>
#include <pfc/bit_array_impl.h>

/// <summary>
/// Gets whether the Media Library is enabled.
/// </summary>
STDMETHODIMP HostObject::get_isLibraryEnabled(VARIANT_BOOL * isEnabled)
{
    if (isEnabled == nullptr)
        return E_INVALIDARG;

    *isEnabled = library_manager::get()->is_library_enabled() ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

/// <summary>
/// Shows the Media Library preferences dialog.
/// </summary>
STDMETHODIMP HostObject::showLibraryPreferences()
{
    library_manager::get()->show_preferences();

    return SetLastError(S_OK);
}

#pragma region metadb_handle_list

/// <summary>
/// Searches the Media Library for matching tracks.
/// </summary>
STDMETHODIMP HostObject::searchLibrary(BSTR query, __int64 * list)
{
    if (list == nullptr)
        return E_INVALIDARG;

    *list = 0;

    auto List = new metadb_handle_list();

    library_manager::get()->get_all_items(*List);

    if ((query != nullptr))// && (query[0] != '\0'))
    {
        try
        {
            static const auto SearchFilter = search_filter_manager_v2::get()->create_ex(::WideToUTF8(query).c_str(), fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);

            pfc::array_t<bool> Mask;

            Mask.set_size(List->get_count());

            SearchFilter->test_multi(*List, Mask.get_ptr());

            List->filter_mask(Mask.get_ptr());
        }
        catch (const pfc::exception &)
        {
        }
    }

    *list = (__int64) (size_t) List;

    return S_OK;
}

/// <summary>
/// Shows the Media Library search UI.
/// </summary>
STDMETHODIMP HostObject::showSearchUI(BSTR query)
{
    pfc::string Query;

    if (query != nullptr)
        Query = pfc::utf8FromWide(query).c_str();

    library_search_ui::get()->show(Query);

    return S_OK;
}

/// <summary>
/// Gets the number of items in the specified metadb handle list.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleListCount(__int64 list, __int64 * count)
{
    if ((list == 0) || (count == nullptr))
        return E_INVALIDARG;

    auto List = (const metadb_handle_list *) list;

    *count = (__int64) List->get_count();

    return S_OK;
}

/// <summary>
/// Gets the list item at the specified index.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleListItem(__int64 list,  __int64 index, __int64 * metaDBHandle)
{
    if ((list == 0) || (metaDBHandle == nullptr))
        return E_INVALIDARG;

    auto List = (const metadb_handle_list *) list;

    *metaDBHandle = (__int64) (size_t) List->get_item((t_size) index).get_ptr();

    return S_OK;
}

/// <summary>
/// Clones the specified metadb handle list.
/// </summary>
STDMETHODIMP HostObject::cloneMetaDBHandleList(__int64 list, __int64 * clone)
{
    if ((list == 0) || (clone == nullptr))
        return E_INVALIDARG;

    auto List = (const metadb_handle_list *) list;

    *clone = (__int64) (size_t) new metadb_handle_list(*List);

    return S_OK;
}

/// <summary>
/// Clears the specified metadb handle list.
/// </summary>
STDMETHODIMP HostObject::clearMetaDBHandleList(__int64 list)
{
    if (list == 0)
        return E_INVALIDARG;

    auto List = (metadb_handle_list *) list;

    List->remove_all();

    return S_OK;
}

/// <summary>
/// Adds the items of a metadb handle list to another.
/// </summary>
STDMETHODIMP HostObject::addMetaDBHandleList(__int64 from, __int64 to)
{
    if ((from == 0) || (to == 0))
        return E_INVALIDARG;

    auto From = (metadb_handle_list *) from;
    auto To = (metadb_handle_list *) to;

    To->add_items(*From);

    return S_OK;
}

/// <summary>
/// Releases the specified metadb handle list.
/// </summary>
STDMETHODIMP HostObject::releaseMetaDBHandleList(__int64 list)
{
    if (list == 0)
        return E_INVALIDARG;

    auto List = (const metadb_handle_list *) list;

    delete List;

    return S_OK;
}

/// <summary>
/// Sorts the metadb handle list by the specified title format.
/// </summary>
STDMETHODIMP HostObject::sortMetaDBHandleListByFormat(__int64 list, BSTR format)
{
    if ((list == 0) || (format == nullptr))
        return E_INVALIDARG;

    auto List = (metadb_handle_list *) list;

    List->sort_by_format(pfc::utf8FromWide(format).c_str(), nullptr);

    return S_OK;
}

/// <summary>
/// Sorts the metadb handle list by the path.
/// </summary>
STDMETHODIMP HostObject::sortMetaDBHandleListByPath(__int64 list)
{
    if (list == 0)
        return E_INVALIDARG;

    auto List = (metadb_handle_list *) list;

    List->sort_by_path();

    return S_OK;
}

/// <summary>
/// Sorts the metadb handle list by the relative path.
/// </summary>
STDMETHODIMP HostObject::sortMetaDBHandleListByRelativePath(__int64 list)
{
    if (list == 0)
        return E_INVALIDARG;

    auto List = (metadb_handle_list *) list;

    List->sort_by_relative_path();

    return S_OK;
}

/// <summary>
/// Removes duplicate items from a metadb handle list.
/// </summary>
STDMETHODIMP HostObject::removeMetaDBHandleListDuplicates(__int64 list)
{
    if (list == 0)
        return E_INVALIDARG;

    auto List = (metadb_handle_list *) list;

    List->remove_duplicates();

    return S_OK;
}

/// <summary>
/// Calculates the total duration of a metadb handle list.
/// </summary>
STDMETHODIMP HostObject::calculateMetaDBHandleListDuration(__int64 list, double * duration)
{
    if ((list == 0) || (duration == nullptr))
        return E_INVALIDARG;

    auto List = (metadb_handle_list *) list;

    *duration = metadb_handle_list_helper::calc_total_duration_v2(*List, std::thread::hardware_concurrency(), fb2k::noAbort);

    return S_OK;
}

#pragma endregion

#pragma region metadb_handle_ptr

/// <summary>
/// Creates a metadb handle pointer from the specified path.
/// </summary>
STDMETHODIMP HostObject::createMetaDBHandlePtr(BSTR path, unsigned __int32 subSongIndex, __int64 * metaDBHandlePtr)
{
    if ((path == nullptr) || (metaDBHandlePtr == nullptr))
        return SetLastError(E_INVALIDARG);

    *metaDBHandlePtr = (__int64)(size_t) new metadb_handle_ptr(metadb::get()->handle_create(pfc::stringcvt::string_utf8_from_wide(path), subSongIndex));

    return SetLastError(S_OK);
}

/// <summary>
/// Deletes a metadb handle pointer.
/// </summary>
STDMETHODIMP HostObject::deleteMetaDBHandlePtr(__int64 metaDBHandlePtr)
{
    if (metaDBHandlePtr == 0)
        return SetLastError(E_INVALIDARG);

    delete (metadb_handle_ptr *) metaDBHandlePtr;

    return SetLastError(S_OK);
}

/// <summary>
/// Gets a metadb handle from a pointer.
/// </summary>
STDMETHODIMP HostObject::getHandleFromMetaDBHandlePtr(__int64 metaDBHandlePtr, __int64 * metaDBHandle)
{
    if (metaDBHandlePtr == 0)
        return SetLastError(E_INVALIDARG);

    auto mhp = (metadb_handle_ptr *) metaDBHandlePtr;

    *metaDBHandle = (__int64)(size_t) mhp->get_ptr();

    return SetLastError(S_OK);
}

#pragma endregion

#pragma region metadb_handle

/// <summary>
/// Gets the path of the specified metadb handle.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandlePath(__int64 metaDBHandle, BSTR * path)
{
    if ((metaDBHandle == 0) || (path == nullptr))
        return SetLastError(E_INVALIDARG);

    auto Handle = (metadb_handle *) metaDBHandle;

    const playable_location & Location = Handle->get_location();

    *path = ::SysAllocString(::UTF8ToWide(Location.get_path()).c_str());

    return SetLastError(S_OK);
}

/// <summary>
/// Gets the path of the specified metadb handle relative to the Media Library folder it is in.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleRelativePath(__int64 metaDBHandle, BSTR * path)
{
    if ((metaDBHandle == 0) || (path == nullptr))
        return E_INVALIDARG;

    auto Handle = (metadb_handle *) metaDBHandle;

    pfc::string Path;

    if (library_manager::get()->get_relative_path(Handle, Path))
        *path = ::SysAllocString(::UTF8ToWide(Path.c_str()).c_str());
    else
        *path = ::SysAllocString(L"");

    return S_OK;
}

/// <summary>
/// Gets the length of the specified metadb handle.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleLength(__int64 metaDBHandle, double * length)
{
    if ((metaDBHandle == 0) || (length == nullptr))
        return E_INVALIDARG;

    auto Handle = (metadb_handle *) metaDBHandle;

    *length = Handle->get_length();

    return S_OK;
}

/// <summary>
/// Formats the title of a Media Library item.
/// </summary>
STDMETHODIMP HostObject::formatMetaDBHandleTitle(__int64 metaDBHandle, BSTR text, BSTR * formattedText)
{
    if ((metaDBHandle == 0) || (text == nullptr) || (formattedText == nullptr))
        return E_INVALIDARG;

    auto Handle = (metadb_handle *) metaDBHandle;

    titleformat_object::ptr FormatObject;
    pfc::string8 Text = pfc::utf8FromWide(text);

    bool Success = titleformat_compiler::get()->compile(FormatObject, Text);

    if (!Success)
    {
        *formattedText = ::SysAllocString(L"");

        return E_INVALIDARG;
    }

    pfc::string8 FormattedText;

    Success = Handle->format_title(nullptr, FormattedText, FormatObject, nullptr);

    *formattedText = ::SysAllocString(pfc::wideFromUTF8(FormattedText).c_str());

    return S_OK;
}

#pragma endregion

/*
    ui_selection_manager::get()->get_selection(Selection);

    if (Selection.get_count() == 0)
        return E_FAIL;
*/
