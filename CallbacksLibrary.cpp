
/** $VER: CallbacksLibrary.cpp (2024.12.15) P. Stuer **/

#include "pch.h"

#include "Callbacks.h"
#include "UIElementTracker.h"

#include "Encoding.h"

#pragma hdrstop

#pragma region library_callback

/// <summary>
/// Called when items have been added to the library.
/// </summary>
void LibraryCallback::on_items_added(metadb_handle_list_cref data)
{
    const std::wstring Text = Stringify(ToJSON(data));

    const std::wstring Script = ::FormatText(L"onLibraryItemsAdded(\"%s\")", Text.c_str());

    auto CurrentElement = _UIElementTracker.GetCurrentElement();

    if (CurrentElement != nullptr)
        CurrentElement->ExecuteScript(Script);
}

/// <summary>
/// Called when library items have been modified.
/// </summary>
void LibraryCallback::on_items_modified(metadb_handle_list_cref data)
{
    const std::wstring Text = Stringify(ToJSON(data));

    const std::wstring Script = ::FormatText(L"onLibraryItemsModified(\"%s\")", Text.c_str());

    auto CurrentElement = _UIElementTracker.GetCurrentElement();

    if (CurrentElement != nullptr)
        CurrentElement->ExecuteScript(Script);
}

/// <summary>
/// Called when library items have been removed.
/// </summary>
void LibraryCallback::on_items_removed(metadb_handle_list_cref data)
{
    const std::wstring Text = Stringify(ToJSON(data));

    const std::wstring Script = ::FormatText(L"onLibraryItemsRemoved(\"%s\")", Text.c_str());

    auto CurrentElement = _UIElementTracker.GetCurrentElement();

    if (CurrentElement != nullptr)
        CurrentElement->ExecuteScript(Script);
}

/// <summary>
/// Called when library items have been modified.
/// </summary>
void LibraryCallback::on_items_modified_v2(metadb_handle_list_cref items, metadb_io_callback_v2_data & data)
{
}

/// <summary>
/// Called when the library has been initialized.
/// </summary>
void LibraryCallback::on_library_initialized()
{
/* Reserved: WebView is not instantiated yet when this callback happens.
    const std::wstring Script = L"onLibraryInitialized()";

    auto CurrentElement = _UIElementTracker.GetCurrentElement();

    if (CurrentElement != nullptr)
        CurrentElement->ExecuteScript(Script);
*/
}

#pragma endregion

namespace
{
    FB2K_SERVICE_FACTORY(LibraryCallback);
}
