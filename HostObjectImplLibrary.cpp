
/** $VER: HostObjectImplMetaDB.cpp (2024.12.27) P. Stuer **/

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

    return S_OK;
}

/// <summary>
/// Searches the Media Library for matching tracks.
/// </summary>
STDMETHODIMP HostObject::searchLibrary(BSTR query, __int64 * tracks)
{
    *tracks = 0;

    auto List = new metadb_handle_list();

/*
    ui_selection_manager::get()->get_selection(Selection);

    if (Selection.get_count() == 0)
        return E_FAIL;
*/
    library_manager::get()->get_all_items(*List);

    if ((query != nullptr) && (query[0] != '\0'))
    {
        try
        {
            static const auto SearchFilter = search_filter_manager_v2::get()->create_ex(::WideToUTF8(query).c_str(), fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);

            pfc::array_t<bool> Mask;

            Mask.set_size(List->get_count());

            SearchFilter->test_multi(*List, Mask.get_ptr());

            List->filter_mask(Mask.get_ptr());
        }
        catch (const pfc::exception & e)
        {
        }
    }

    *tracks = (__int64) List;

    return S_OK;
}

/// <summary>
/// Gets the number of items in the specified metadb handle list.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleListCount(__int64 list, __int64 * count)
{
    auto List = (const metadb_handle_list *) list;

    *count = (__int64) List->get_count();

    return S_OK;
}

/// <summary>
/// Gets the list item at the specified index.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleListItem(__int64 list, size_t index, __int64 * metaDBHandle)
{
    auto List = (const metadb_handle_list *) list;

    *metaDBHandle = (__int64) List->get_item(index).get_ptr();

    return S_OK;
}

/// <summary>
/// Releases the specified metadb handle list.
/// </summary>
STDMETHODIMP HostObject::releaseMetaDBHandleList(__int64 list)
{
    auto List = (const metadb_handle_list *) list;

    delete List;

    return S_OK;
}

/// <summary>
/// Gets the path of the specified metadb handle.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandlePath(__int64 metaDBHandle, BSTR * path)
{
    auto Handle = (metadb_handle *) metaDBHandle;

    const playable_location & Location = Handle->get_location();

    *path = ::SysAllocString(::UTF8ToWide(Location.get_path()).c_str());

    return S_OK;
}

/// <summary>
/// Gets the path of the specified metadb handle relative to the Media Library folder it is in.
/// </summary>
STDMETHODIMP HostObject::getMetaDBHandleRelativePath(__int64 metaDBHandle, BSTR * path)
{
    auto Handle = (metadb_handle *) metaDBHandle;

    pfc::string Path;

    if (library_manager::get()->get_relative_path(Handle, Path))
        *path = ::SysAllocString(::UTF8ToWide(Path.c_str()).c_str());
    else
        *path = ::SysAllocString(L"");

    return S_OK;
}

#pragma endregion
