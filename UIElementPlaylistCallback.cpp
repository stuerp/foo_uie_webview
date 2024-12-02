
/** $VER: UIElement.cpp (2024.12.02) P. Stuer **/

#include "pch.h"

#include "UIElement.h"
#include "UIElementTracker.h"
#include "Encoding.h"
#include "Exceptions.h"
#include "Support.h"

#include <pathcch.h>

#pragma comment(lib, "pathcch")

#include <SDK/titleformat.h>
#include <SDK/playlist.h>
#include <SDK/ui.h>

#pragma hdrstop

#pragma region playlist_callback

/// <summary>
/// Called when items have been added to the specified playlist.
/// </summary>
void UIElement::on_items_added(t_size playlistIndex, t_size startIndex, metadb_handle_list_cref data, const bit_array & selection)
{
    const std::wstring Text = Stringify(ToJSON(data));

    const std::wstring Script = ::FormatText(L"onPlaylistItemsAdded(%d, %d, \"%s\")", (int) playlistIndex, (int) startIndex, Text.c_str());

    ExecuteScript(Script);
}

/// <summary>
/// Called when the items of the specified playlist have been reordered.
/// </summary>
void UIElement::on_items_reordered(t_size playlistIndex, const t_size * order, t_size count)
{
    const std::wstring Text = ToJSON(order, count);

    const std::wstring Script = ::FormatText(L"onPlaylistItemsReordered(%d, \"%s\")", (int) playlistIndex, Text.c_str());

    ExecuteScript(Script);
}

/// <summary>
/// Called when items of the specified playlist are being removed.
/// </summary>
void UIElement::on_items_removing(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount)
{
    const std::wstring Text = ToJSON(mask, oldCount);

    const std::wstring Script = ::FormatText(L"onPlaylistItemsRemoving(%d, \"%s\", %d)", (int) playlistIndex, Text.c_str(), (int) newCount);

    ExecuteScript(Script);
}

/// <summary>
/// Called when items of the specified playlist have been removed.
/// </summary>
void UIElement::on_items_removed(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount)
{
    const std::wstring Text = ToJSON(mask, oldCount);

    const std::wstring Script = ::FormatText(L"onPlaylistItemsRemoved(%d, \"%s\", %d)", (int) playlistIndex, Text.c_str(), (int) newCount);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the selected items changed.
/// </summary>
void UIElement::on_items_selection_change(t_size playlistIndex, const bit_array & affectedItems, const bit_array & state)
{
    const std::wstring Text = ToJSON(affectedItems, ~0);

    const std::wstring Script = ::FormatText(L"onPlaylistSelectedItemsChanged(%d, \"%s\")", (int) playlistIndex, Text.c_str());

    ExecuteScript(Script);
}

/// <summary>
/// Called when the focused item of a playlist changed.
/// </summary>
void UIElement::on_item_focus_change(t_size playlistIndex, t_size fromIndex, t_size toIndex)
{
    const std::wstring Script = ::FormatText(L"onPlaylistFocusedItemChanged(%d, %d, %d)", (int) playlistIndex, (int) fromIndex, (int) toIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the specified item of a playlist has been ensured to be visible.
/// </summary>
void UIElement::on_item_ensure_visible(t_size playlistIndex, t_size itemIndex)
{
    const std::wstring Script = ::FormatText(L"onPlaylistItemEnsureVisible(%d, %d)", (int) playlistIndex, (int) itemIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when some playlist items of the specified playlist have been modified.
/// </summary>
void UIElement::on_items_modified(t_size playlistIndex, const bit_array & mask)
{
    const std::wstring Script = ::FormatText(L"onPlaylistItemsModified(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when some playlist items of the specified playlist have been modified from playback.
/// </summary>
void UIElement::on_items_modified_fromplayback(t_size playlistIndex, const bit_array & mask, play_control::t_display_level displayLevel)
{
    const std::wstring Script = ::FormatText(L"onPlaylistItemsModifiedFromPlayback(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when items of the specified playlist have been replaced.
/// </summary>
void UIElement::on_items_replaced(t_size playlistIndex, const bit_array & mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & replacedItems)
{
    const std::wstring Script = ::FormatText(L"onPlaylistItemsReplaced(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the active playlist changes.
/// </summary>
void UIElement::on_playlist_activate(t_size oldPlaylistIndex, t_size newPlaylistIndex)
{
    const std::wstring Script = ::FormatText(L"onPlaylistActivated(%d, %d)", (int) oldPlaylistIndex, (int) newPlaylistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when a new playlist has been created.
/// </summary>
void UIElement::on_playlist_created(t_size playlistIndex, const char * name, t_size size)
{
    const std::wstring Script = ::FormatText(L"onPlaylistCreated(%d, \"%s\")", (int) playlistIndex, ::UTF8ToWide(name, size).c_str());

    ExecuteScript(Script);
}

/// <summary>
/// Called when the playlists have beenn reordered.
/// </summary>
void UIElement::on_playlists_reorder(const t_size * order, t_size count)
{
    const std::wstring Script = L"onPlaylistsReordered()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when playlists are being removed.
/// </summary>
void UIElement::on_playlists_removing(const bit_array & mask, t_size oldCount, t_size newCount)
{
    const std::wstring Script = L"onPlaylistsRemoving()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when playlists have been removed.
/// </summary>
void UIElement::on_playlists_removed(const bit_array & mask, t_size oldCount, t_size newcount)
{
    const std::wstring Script = L"onPlaylistsRemoved()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when the specified playlist has been renamed.
/// </summary>
void UIElement::on_playlist_renamed(t_size playlistIndex, const char * name, t_size size)
{
    const std::wstring Script = ::FormatText(L"onPlaylistRenamed(%d, \"%s\")", (int) playlistIndex, ::UTF8ToWide(name, size).c_str());

    ExecuteScript(Script);
}

/// <summary>
/// Called when the specified playlist has been locked or unlocked.
/// </summary>
void UIElement::on_playlist_locked(t_size playlistIndex, bool isLocked)
{
    const std::wstring Script = ::FormatText(isLocked ? L"onPlaylistLocked(%d)" : L"onPlaylistUnlocked(%d)", (int) playlistIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Called when the default format has been changed.
/// </summary>
void UIElement::on_default_format_changed()
{
    const std::wstring Script = L"onDefaultFormatChanged()";

    ExecuteScript(Script);
}

/// <summary>
/// Called when the playback order changed.
/// </summary>
void UIElement::on_playback_order_changed(t_size playbackOrderIndex)
{
    const std::wstring Script = ::FormatText(L"onPlaybackOrderChanged(%d)", (int) playbackOrderIndex);

    ExecuteScript(Script);
}

/// <summary>
/// Executes a script.
/// </summary>
void UIElement::ExecuteScript(const std::wstring & script) const noexcept
{
    if (_WebView == nullptr)
        return;

    HRESULT hr = _WebView->ExecuteScript(script.c_str(), nullptr);

    if (!SUCCEEDED(hr))
        console::print(::GetErrorMessage(hr, ::FormatText(STR_COMPONENT_BASENAME " failed to call %s", ::WideToUTF8(script).c_str())).c_str());
}

#pragma endregion
