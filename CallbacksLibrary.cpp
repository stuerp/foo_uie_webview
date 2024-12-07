
/** $VER: CallbacksLibrary.cpp (2024.12.07) P. Stuer **/

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
/// Called when items have been added to the library.
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
/// Called when items have been added to the library.
/// </summary>
void LibraryCallback::on_items_removed(metadb_handle_list_cref data)
{
    const std::wstring Text = Stringify(ToJSON(data));

    const std::wstring Script = ::FormatText(L"onLibraryItemsRemoved(\"%s\")", Text.c_str());

    auto CurrentElement = _UIElementTracker.GetCurrentElement();

    if (CurrentElement != nullptr)
        CurrentElement->ExecuteScript(Script);
}

#pragma endregion

namespace
{
    FB2K_SERVICE_FACTORY(LibraryCallback);
}
