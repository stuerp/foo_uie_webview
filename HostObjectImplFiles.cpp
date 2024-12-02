
/** $VER: HostObjectImplFiles.cpp (2024.12.02) P. Stuer **/

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

/// <summary>
/// Gets the specified artwork of the currently selected item in the current playlist.
/// </summary>
STDMETHODIMP HostObject::GetArtwork(BSTR type, BSTR * image)
{
    *image = ::SysAllocString(L""); // Return an empty string by default and in case of an error.

    if (type == nullptr)
        return E_INVALIDARG;

    // Verify the requested artwork type.
    GUID AlbumArtId;

    if (::_wcsicmp(type, L"front") == 0)
    {
        AlbumArtId = album_art_ids::cover_front;
    }
    else
    if (::_wcsicmp(type, L"back") == 0)
    {
        AlbumArtId = album_art_ids::cover_back;
    }
    else
    if (::_wcsicmp(type, L"disc") == 0)
    {
        AlbumArtId = album_art_ids::disc;
    }
    else
    if (::_wcsicmp(type, L"icon") == 0)
    {
        AlbumArtId = album_art_ids::icon;
    }
    else
    if (::_wcsicmp(type, L"artist") == 0)
    {
        AlbumArtId = album_art_ids::artist;
    }
    else
        return S_OK;

    metadb_handle_ptr Handle;

    if (!_PlaybackControl->get_now_playing(Handle))
        return S_OK;

    static_api_ptr_t<album_art_manager_v3> Manager;

    album_art_data::ptr aad;

    try
    {
        album_art_extractor_instance_v2::ptr Extractor = Manager->open_v3(pfc::list_single_ref_t<metadb_handle_ptr>(Handle), pfc::list_single_ref_t<GUID>(AlbumArtId), nullptr, fb2k::noAbort);

        if (Extractor.is_empty())
            return S_OK;

        // Query the external search patterns first.
        try
        {
            album_art_path_list::ptr Paths = Extractor->query_paths(AlbumArtId, fb2k::noAbort);

            if (Paths.is_valid())
            {
                for (size_t i = 0; i < Paths->get_count(); ++i)
                {
                    pfc::string Extension = pfc::io::path::getFileExtension(Paths->get_path(i));

                    if (!Extension.isEmpty() && ((::_stricmp(Extension.c_str(), ".jpg") == 0) || (::_stricmp(Extension.c_str(), ".png") == 0) || (::_stricmp(Extension.c_str(), ".webp") == 0) || (::_stricmp(Extension.c_str(), ".gif") == 0)))
                    {
                        ::SysFreeString(*image); // Free the empty string.

                        *image = ::SysAllocString(::UTF8ToWide(Paths->get_path(i)).c_str());
                
                        return S_OK;
                    }
                }
            }
        }
        catch (...)
        {
        }

        // Query the embedded art.
        if (!Extractor->query(AlbumArtId, aad, fb2k::noAbort))
        {
            // Query the stub the stub path.
            try
            {
                Extractor = Manager->open_stub(fb2k::noAbort);

                if (!Extractor->query(AlbumArtId, aad, fb2k::noAbort))
                    return S_OK;
            }
            catch (std::exception & e)
            {
                console::print(STR_COMPONENT_BASENAME " failed to query album art stub: ", e.what());
            }
        }
    }
    catch (...)
    {
        // Query the stub the stub path.
        try
        {
            album_art_extractor_instance_v2::ptr Extractor = Manager->open_stub(fb2k::noAbort);

            if (!Extractor->query(AlbumArtId, aad, fb2k::noAbort))
                return S_OK;
        }
        catch (std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to query album art stub: ", e.what());
        }
    }

    if (!aad.is_empty())
        ToBase64((const BYTE *) aad->data(), (DWORD) aad->size(), image);

    return S_OK;
}

#pragma region Files

/// <summary>
/// Reads the specified file and returns it as a string.
/// </summary>
STDMETHODIMP HostObject::ReadAllText(BSTR filePath, __int32 codePage, BSTR * text)
{
    if ((filePath == nullptr) || (text == nullptr))
        return E_INVALIDARG;

    if (codePage == 0)
        codePage = 65001;

    HANDLE hFile = ::CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(::GetLastError());

    LARGE_INTEGER FileSize;

    HRESULT hr = S_OK;

    if (::GetFileSizeEx(hFile, &FileSize))
    {
        std::string Text;

        Text.resize((size_t) FileSize.LowPart + 2);

        DWORD BytesRead;

        if (::ReadFile(hFile, (void *) Text.c_str(), FileSize.LowPart, &BytesRead, nullptr) && (BytesRead == FileSize.LowPart))
            *text = ::SysAllocString(::CodePageToWide((uint32_t) codePage, Text).c_str());
        else
            hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    else
        hr = HRESULT_FROM_WIN32(::GetLastError());

    ::CloseHandle(hFile);

    return hr;
}

/// <summary>
/// Reads the specified file and returns it as a string.
/// </summary>
STDMETHODIMP HostObject::ReadDirectory(BSTR directoryPath, BSTR searchPattern, BSTR * json)
{
    if ((directoryPath == nullptr) || (searchPattern == nullptr) || (json == nullptr))
        return E_INVALIDARG;

    *json = ::SysAllocString(L""); // Return an empty string by default and in case of an error.

    WCHAR PathName[MAX_PATH];

    if (!SUCCEEDED(::PathCchCombineEx(PathName, _countof(PathName), directoryPath, searchPattern, PATHCCH_ALLOW_LONG_PATHS)))
        return HRESULT_FROM_WIN32(::GetLastError());

    WIN32_FIND_DATA fd = {};

    HANDLE hFind = ::FindFirstFileW(PathName, &fd);

    if (hFind == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(::GetLastError());

    BOOL Success = TRUE;

    if (::wcscmp(fd.cFileName, L".") == 0)
    {
        Success = ::FindNextFileW(hFind, &fd);

        if (Success && ::wcscmp(fd.cFileName, L"..") == 0)
            Success = ::FindNextFileW(hFind, &fd);
    }

    std::wstring Result = L"[";
    bool IsFirstItem = true;

    while (Success)
    {
        if (!IsFirstItem)
            Result.append(L",");

        uint64_t FileSize = (((uint64_t) fd.nFileSizeHigh) << 32) + fd.nFileSizeLow;

        Result.append(FormatText(LR"({"name": "%s", "size": %lu, "isDirectory": %s})", fd.cFileName, FileSize, ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? L"true" : L"false")).c_str());

        IsFirstItem = false;

        Success = ::FindNextFileW(hFind, &fd);
    }

    Result.append(L"]");

    ::FindClose(hFind);

    *json = ::SysAllocString(Result.c_str());

    return S_OK;
}

/// <summary>
/// Reads the specified file and returns it as a string.
/// </summary>
STDMETHODIMP HostObject::ReadImage(BSTR filePath, BSTR * image)
{
    *image = ::SysAllocString(L""); // Return an empty string by default and in case of an error.

    if ((filePath == nullptr) || (image == nullptr))
        return E_INVALIDARG;

    HANDLE hFile = ::CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(::GetLastError());

    LARGE_INTEGER FileSize;

    HRESULT hr = S_OK;

    if (::GetFileSizeEx(hFile, &FileSize))
    {
        BYTE * Data = new BYTE[FileSize.LowPart];

        if (Data != nullptr)
        {
            DWORD BytesRead;

            if (::ReadFile(hFile, Data, FileSize.LowPart, &BytesRead, nullptr) && (BytesRead == FileSize.LowPart))
                ToBase64(Data, FileSize.LowPart, image);
            else
                hr = HRESULT_FROM_WIN32(::GetLastError());

            delete[] Data;
        }
    }
    else
        hr = HRESULT_FROM_WIN32(::GetLastError());

    ::CloseHandle(hFile);

    return S_OK;
}
