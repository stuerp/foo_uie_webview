
/** $VER: ProcessLocationsHandler.h (2024.11.27) P. Stuer **/

#pragma once

#include "pch.h"

/// <summary>
/// Handles notifications while processing locations.
/// </summary>
class ProcessLocationsHandler : public process_locations_notify
{
public:
    ProcessLocationsHandler(size_t playlistIndex, size_t itemIndex, bool selectAddedItems) : _PlayListIndex(playlistIndex), _ItemIndex(itemIndex), _SelectAddedItems(selectAddedItems)
    {
    }

    ProcessLocationsHandler(const ProcessLocationsHandler &) = delete;
    ProcessLocationsHandler & operator=(const ProcessLocationsHandler &) = delete;
    ProcessLocationsHandler(ProcessLocationsHandler &&) = delete;
    ProcessLocationsHandler & operator=(ProcessLocationsHandler &&) = delete;

    virtual ~ProcessLocationsHandler() {}

    void on_completion(metadb_handle_list_cref items) override
    {
        auto Manager = playlist_manager::get();

        const size_t PlayListIndex = (_PlayListIndex == (size_t) -1) ? Manager->get_active_playlist() : _PlayListIndex;

        if (PlayListIndex >= Manager->get_playlist_count() || (Manager->playlist_lock_get_filter_mask(PlayListIndex) & playlist_lock::filter_add))
            return;

        pfc::bit_array_val selection(_SelectAddedItems);

        Manager->playlist_insert_items(PlayListIndex, _ItemIndex, items, selection);

        if (_SelectAddedItems)
            Manager->playlist_set_focus_item(PlayListIndex, _ItemIndex);
    }

    void on_aborted() override
    {
    }

private:
    const size_t _PlayListIndex;
    const size_t _ItemIndex;
    const bool _SelectAddedItems;
};
