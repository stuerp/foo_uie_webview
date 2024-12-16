
/** $VER: Callbacks.h (2024.12.15) P. Stuer **/

#pragma once

#include "framework.h"

#include "UIElement.h"

#include <SDK/library_callbacks.h>

#pragma once

#pragma region library_callback

class LibraryCallback : public library_callback_v2
{
public:
    LibraryCallback() { };

    LibraryCallback(const LibraryCallback &) = delete;
    LibraryCallback & operator=(const LibraryCallback &) = delete;
    LibraryCallback(LibraryCallback &&) = delete;
    LibraryCallback & operator=(LibraryCallback &&) = delete;

    virtual ~LibraryCallback() { };

    void on_items_added(metadb_handle_list_cref data) override;
    void on_items_modified(metadb_handle_list_cref data) override;
    void on_items_removed(metadb_handle_list_cref data) override;

    void on_items_modified_v2(metadb_handle_list_cref items, metadb_io_callback_v2_data & data) override;
    void on_library_initialized() override;
};

#pragma endregion
