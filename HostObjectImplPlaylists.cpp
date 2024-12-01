
/** $VER: HostObjectImplPlaylists.cpp (2024.12.01) P. Stuer **/

#include "pch.h"

#include "HostObjectImpl.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

#pragma comment(lib, "crypt32")

#include "Support.h"
#include "Resources.h"
#include "Encoding.h"

#include "ProcessLocationsHandler.h"

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/ui.h>
#include <SDK/contextmenu.h>

#include <pfc/string-conv-lite.h>
#include <pfc/bit_array_impl.h>

#pragma region Playlists

/// <summary>
/// Gets the number of playlists.
/// </summary>
STDMETHODIMP HostObject::get_PlaylistCount(int * count)
{
    if (count == nullptr)
        return E_INVALIDARG;

    *count = (int) playlist_manager::get()->get_playlist_count();

    return S_OK;
}

/// <summary>
/// Gets the index of the active playlist.
/// </summary>
STDMETHODIMP HostObject::get_ActivePlaylist(int * playlistIndex)
{
    if (playlistIndex == nullptr)
        return E_INVALIDARG;

    *playlistIndex = (int) playlist_manager::get()->get_active_playlist();

    return S_OK;
}

/// <summary>
/// Sets the index of the active playlist.
/// </summary>
STDMETHODIMP HostObject::put_ActivePlaylist(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->set_active_playlist((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Gets the index of the playing playlist.
/// </summary>
STDMETHODIMP HostObject::get_PlayingPlaylist(int * playlistIndex)
{
    if (playlistIndex == nullptr)
        return E_INVALIDARG;

    *playlistIndex = (int) playlist_manager::get()->get_playing_playlist();

    return S_OK;
}

/// <summary>
/// Sets the index of the playing playlist.
/// </summary>
STDMETHODIMP HostObject::put_PlayingPlaylist(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->set_playing_playlist((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Gets the name of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::GetPlaylistName(int playlistIndex, BSTR * name)
{
    if (name == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    pfc::string Name;

    Manager->playlist_get_name((size_t) playlistIndex, Name);

    *name = ::SysAllocString(pfc::wideFromUTF8(Name).c_str());

    return S_OK;
}

/// <summary>
/// Gets the name of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::SetPlaylistName(int playlistIndex, BSTR name)
{
    if (name == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    pfc::string Name = pfc::utf8FromWide(name).c_str();

    Manager->playlist_rename((size_t) playlistIndex, Name.c_str(), Name.length());

    return S_OK;
}

/// <summary>
/// Finds the index of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::FindPlaylist(BSTR name, int * playlistIndex)
{
    if (name == nullptr)
        return E_INVALIDARG;

    pfc::string Name = pfc::utf8FromWide(name).c_str();

    *playlistIndex = (int) playlist_manager::get()->find_playlist(Name.c_str(), Name.length());

    return S_OK;
}

/// <summary>
/// Gets the number of items of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::GetPlaylistItemCount(int playlistIndex, int * itemCount)
{
    if (itemCount == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    *itemCount = (int) Manager->playlist_get_item_count((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Gets the number of selected items of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::GetSelectedPlaylistItemCount(int playlistIndex, int maxItems, int * itemCount)
{
    if (itemCount == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    *itemCount = (int) Manager->playlist_get_selection_count((size_t) playlistIndex, (size_t) maxItems);

    return S_OK;
}

/// <summary>
/// Gets the index of the focused playlist item.
/// </summary>
STDMETHODIMP HostObject::GetFocusedPlaylistItem(int playlistIndex, int * itemIndex)
{
    if (itemIndex == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    *itemIndex = (int) Manager->playlist_get_focus_item((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Gets the name of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::SetFocusedPlaylistItem(int playlistIndex, int itemIndex)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    Manager->playlist_set_focus_item((size_t) playlistIndex, (size_t) itemIndex);

    return S_OK;
}

/// <summary>
/// Ensures that the specified item in the specified playlist is visible.
/// </summary>
STDMETHODIMP HostObject::EnsurePlaylistItemVisible(int playlistIndex, int itemIndex)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    Manager->playlist_ensure_visible((size_t) playlistIndex, (size_t) itemIndex);

    return S_OK;
}

/// <summary>
/// Returns true if the specified item in the specified playlist is selected.
/// </summary>
STDMETHODIMP HostObject::IsPlaylistItemSelected(int playlistIndex, int itemIndex, VARIANT_BOOL * result)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    *result = Manager->playlist_is_item_selected((size_t) playlistIndex, (size_t) itemIndex);

    return S_OK;
}

/// <summary>
/// Execute the default action on the specified item in the specified playlist.
/// </summary>
STDMETHODIMP HostObject::ExecutePlaylistDefaultAction(int playlistIndex, int itemIndex)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    Manager->playlist_execute_default_action((size_t) playlistIndex, (size_t) itemIndex);

    return S_OK;
}

/// <summary>
/// Removes the specified item from the specified playlist.
/// </summary>
STDMETHODIMP HostObject::RemovePlaylistItem(int playlistIndex, int itemIndex)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    Manager->playlist_clear_selection((size_t) playlistIndex);
    Manager->playlist_set_selection_single((size_t) playlistIndex, (size_t) itemIndex, true);
    Manager->playlist_remove_selection((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Creates a new playlist at the specified index.
/// </summary>
STDMETHODIMP HostObject::CreatePlaylist(int playlistIndex, BSTR name, int * newPlaylistIndex)
{
    if (newPlaylistIndex == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager::get();

    if ((name != nullptr) && (*name != '\0'))
    {
        pfc::string Name = pfc::utf8FromWide(name).c_str();

        *newPlaylistIndex = (int) Manager->create_playlist(Name.c_str(), Name.length(), (size_t) playlistIndex);
    }
    else
        *newPlaylistIndex = (int) Manager->create_playlist_autoname((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Adds an item to the specified playlist after the specified item using a location and optionally selects it.
/// </summary>
STDMETHODIMP HostObject::AddPath(int playlistIndex, int itemIndex, BSTR filePath, VARIANT_BOOL selectAddedItem)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    pfc::string_list_impl LocationList;

    LocationList.add_item(WideToUTF8(filePath).c_str());

    playlist_incoming_item_filter_v2::get()->process_locations_async
    (
        LocationList,
        playlist_incoming_item_filter_v2::op_flag_no_filter | playlist_incoming_item_filter_v2::op_flag_delay_ui,
        nullptr,
        nullptr,
        nullptr,
        fb2k::service_new<ProcessLocationsHandler>(playlistIndex, itemIndex, selectAddedItem)
    );

    return S_OK;
}

/// <summary>
/// Duplicates the specified playlist.
/// </summary>
STDMETHODIMP HostObject::DuplicatePlaylist(int playlistIndex, BSTR name, int * newPlaylistIndex)
{
    if (newPlaylistIndex == nullptr)
        return E_INVALIDARG;

    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    pfc::string Name;

    if ((name != nullptr) && (*name != '\0'))
        Name = pfc::utf8FromWide(name).c_str();
    else
        (void) Manager->playlist_get_name((size_t) playlistIndex, Name);

    metadb_handle_list Items;

    Manager->playlist_get_all_items((size_t) playlistIndex, Items);

    stream_reader_dummy sr;

    *newPlaylistIndex = (int) Manager->create_playlist_ex(Name.c_str(), Name.length(), (size_t) playlistIndex + 1, Items, &sr, fb2k::noAbort);

    return S_OK;
}

/// <summary>
/// Clears the specified playlist.
/// </summary>
STDMETHODIMP HostObject::ClearPlaylist(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->playlist_clear((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Gets the items of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::GetPlaylistItems(int playlistIndex, BSTR * json)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    metadb_handle_list hItems;

    Manager->playlist_get_all_items((size_t) playlistIndex, hItems);

    *json = ::SysAllocString(ToJSON(hItems).c_str());

    return S_OK;
}

/// <summary>
/// Selects the specified item of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::SelectPlaylistItem(int playlistIndex, int itemIndex)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    Manager->playlist_set_selection_single((size_t) playlistIndex, (size_t) itemIndex, true);

    return S_OK;
}

/// <summary>
/// Deselects the specified item of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::DeselectPlaylistItem(int playlistIndex, int itemIndex)
{
    NormalizeIndexes(playlistIndex, itemIndex);

    auto Manager = playlist_manager_v4::get();

    Manager->playlist_set_selection_single((size_t) playlistIndex, (size_t) itemIndex, false);

    return S_OK;
}

/// <summary>
/// Gets the selected items of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::GetSelectedPlaylistItems(int playlistIndex, BSTR * json)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    metadb_handle_list hItems;

    Manager->playlist_get_selected_items((size_t) playlistIndex, hItems);

    *json = ::SysAllocString(ToJSON(hItems).c_str());

    return S_OK;
}

/// <summary>
/// Clears the selection of the specified playlist.
/// </summary>
STDMETHODIMP HostObject::ClearPlaylistSelection(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->playlist_clear_selection((size_t) playlistIndex);

    return S_OK;
}

/// <summary>
/// Removes the selected items in the specified playlist.
/// </summary>
STDMETHODIMP HostObject::RemoveSelectedPlaylistItems(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->playlist_remove_selection((size_t) playlistIndex, false);

    return S_OK;
}

/// <summary>
/// Removes the unselected items in the specified playlist.
/// </summary>
STDMETHODIMP HostObject::RemoveUnselectedPlaylistItems(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->playlist_remove_selection((size_t) playlistIndex, true);

    return S_OK;
}

/// <summary>
/// Deletes the specified playlist.
/// </summary>
STDMETHODIMP HostObject::DeletePlaylist(int playlistIndex)
{
    auto Manager = playlist_manager_v4::get();

    if (playlistIndex == -1)
        playlistIndex = (int) Manager->get_active_playlist();

    Manager->remove_playlist((size_t) playlistIndex);

    return S_OK;
}

#pragma endregion
