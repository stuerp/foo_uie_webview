
/** $VER: FileWatcher.cpp (2024.05.25) P. Stuer - Implements a file system watcher. **/

#include "pch.h"

#include "FileWatcher.h"
#include "Exceptions.h"
#include "Resources.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch")

/// <summary>
/// 
/// </summary>
void FileWatcher::Start(HWND hWnd, const std::wstring & filePath)
{
    _ThreadParameters.hWnd = hWnd;
    _ThreadParameters.FilePath = filePath;

    _hThread = ::CreateThread(nullptr, 0, ThreadProc, &_ThreadParameters, 0, nullptr);

    if (_hThread == NULL)
        throw Win32Exception(::GetLastError(), "Failed to create file system watcher thread");
}

/// <summary>
/// 
/// </summary>
void FileWatcher::Stop() noexcept
{
    if (_hThread != NULL)
    {
        ::CloseHandle(_hThread);
        _hThread = NULL;
    }
}

/// <summary>
/// Thread procedure
/// </summary>
DWORD WINAPI FileWatcher::ThreadProc(LPVOID lParam) noexcept
{
    auto Parameters = (FileWatcher::thread_parameters_t *) lParam;

    if (Parameters == nullptr)
        return 1;

    wchar_t DirectoryPath[MAX_PATH]; ::wcscpy_s(DirectoryPath, _countof(DirectoryPath), Parameters->FilePath.c_str());

    wchar_t * FileName = ::PathFindFileNameW(DirectoryPath);

    HRESULT hResult = ::PathCchRemoveFileSpec(DirectoryPath, _countof(DirectoryPath));

    if (!SUCCEEDED(hResult))
        return (DWORD) hResult;

    HANDLE hDirectory = ::CreateFileW(DirectoryPath, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

    if (hDirectory == INVALID_HANDLE_VALUE)
        return ::GetLastError();

    HANDLE hIoCompletionPort = ::CreateIoCompletionPort(hDirectory, NULL, 0, 1);

    if (hIoCompletionPort == NULL)
        return ::GetLastError();

    const DWORD NotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY;

    uint8_t Data[1024] = {};
    OVERLAPPED Overlapped = {};

    BOOL Success = ::ReadDirectoryChangesW(hDirectory, Data, sizeof(Data), FALSE, NotifyFilter, nullptr, &Overlapped, nullptr);

    while (Success)
    {
        DWORD BytesRead;
        ULONG_PTR CompletionKey;
        LPOVERLAPPED OverlappedComplete;

        Success = ::GetQueuedCompletionStatus(hIoCompletionPort, &BytesRead, &CompletionKey, &OverlappedComplete, INFINITE);

        auto fni = (FILE_NOTIFY_INFORMATION *) Data;

        for (;;)
        {
            if (::wcsncmp(FileName, fni->FileName, fni->FileNameLength / 2) == 0)
                ::PostMessageW(Parameters->hWnd, UM_FILE_CHANGED, 0, 0);

            if (fni->NextEntryOffset == 0)
                break;

            fni = (FILE_NOTIFY_INFORMATION *)(((uint8_t *) fni) + fni->NextEntryOffset);
        }

        Success = ::ReadDirectoryChangesW(hDirectory, Data, sizeof(Data), FALSE, NotifyFilter, nullptr, &Overlapped, nullptr);
    }

    return 0;
}
