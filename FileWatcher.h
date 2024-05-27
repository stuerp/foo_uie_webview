
/** $VER: FileWatcher.h (2024.05.27) P. Stuer - Implements a file system watcher. **/

#pragma once

#include "framework.h"

/// <summary>
/// Implements a file system watcher.
/// </summary>
class FileWatcher
{
public:
    FileWatcher() : _ThreadParameters(), _hThread() { }

    void Start(HWND hWnd, const std::wstring & filePath);
    void Stop() noexcept;

private:
    static DWORD WINAPI ThreadProc(LPVOID lParam) noexcept;

    struct thread_parameters_t
    {
        HWND hWnd;
        std::wstring FilePath;
    } _ThreadParameters;

    HANDLE _hThread;
};
