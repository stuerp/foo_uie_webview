
/** $VER: Callbacks.h (2024.12.08) P. Stuer **/

#pragma once

#include "framework.h"

#include "UIElement.h"

#pragma once

#pragma region library_callback

class LibraryCallback : public library_callback
{
public:
    void on_items_added(metadb_handle_list_cref p_data) override;
    void on_items_modified(metadb_handle_list_cref p_data) override;
    void on_items_removed(metadb_handle_list_cref p_data) override;
};

#pragma endregion
