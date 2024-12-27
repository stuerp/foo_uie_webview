
/** $VER: foo_uie_webview.js (2024.12.27) P. Stuer **/

const webView =
{
    /**
        Executes an operation on the specified file.
        @argument {string} filePath - The path of the file.
        @argument {string} [parameters] - The parameters to pass to the operation.
        @argument {string} [directoryPath] - The directory path.
        @argument {string} [operation] - The shell operation to execute on the file.
        @argument {int} [showMode] - The show mode of the window.
        @description Performs the specified shell operation on a file. Mostly used to run applications. Check the [SHELLEXECUTEINFOW](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow) documentation for the possible values of the parameters.
    **/
    execute: function(filePath, parameters, directoryPath, operation, showMode)
    {
        chrome.webview.hostObjects.foo_uie_webview.execute(filePath, parameters, directoryPath, operation, showMode);
    }
/*
    canExecuteShellOperations: fuction()
    {
        return chrome.webview.hostObjects.sync.foo_uie_webview.canExecuteShellOperations;
    }*/
};

const SW_HIDE = 0;              // Hides the window and activates another window.
const SW_SHOWNORMAL = 1;        // Activates and displays a window. If the window is minimized, maximized, or arranged, the system restores it to its original size and position. An application should specify this flag when displaying the window for the first time.
const SW_SHOWMINIMIZED = 2;     // Activates the window and displays it as a minimized window.
const SW_SHOWMAXIMIZED = 3;     // Activates the window and displays it as a maximized window.
const SW_SHOWNOACTIVATE = 4;    // Displays a window in its most recent size and position. This value is similar to SW_SHOWNORMAL, except that the window is not activated.
const SW_SHOW = 5;              // Activates the window and displays it in its current size and position.
const SW_MINIMIZE = 6;          // Minimizes the specified window and activates the next top-level window in the Z order.
const SW_SHOWMINNOACTIVE = 7;   // Displays the window as a minimized window. This value is similar to SW_SHOWMINIMIZED, except the window is not activated.
const SW_SHOWNA = 8;            // Displays the window in its current size and position. This value is similar to SW_SHOW, except that the window is not activated.
const SW_RESTORE = 9;           // Activates and displays the window. If the window is minimized, maximized, or arranged, the system restores it to its original size and position. An application should specify this flag when restoring a minimized window.
const SW_SHOWDEFAULT = 10;      // Sets the show state based on the SW_ value specified in the STARTUPINFO structure passed to the CreateProcess function by the program that started the application.

const mediaLibrary =
{
    /**
        Gets whether the Media Library is enabled.
        @name isEnabled
        @kind function
        @returns {boolean}
    **/
    isEnabled: function()
    {
        return chrome.webview.hostObjects.sync.foo_uie_webview.isLibraryEnabled;
    },

    /**
        Shows the Media Library preferences dialog.
    **/
    showPreferences: function()
    {
        chrome.webview.hostObjects.foo_uie_webview.showLibraryPreferences();
    },

    /**
        Searches the Media Library for matching tracks.
        @param {string} query - The search query.
        @returns {MetaDBHandleList}
    **/
    search: function(query)
    {
        return chrome.webview.hostObjects.foo_uie_webview.searchLibrary(query);
    }
};

/**
     @constructor
     @name MetaDBHandle
     @param {object} [arg] - MetaDB handle
     @returns {void}
 **/
function MetaDBHandle(arg)
{
    this.Source = arg;

    /**
        Gets the path.
        @name path
        @returns {string}
    **/
    Object.defineProperty(this, 'path',
    {
        get() { return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandlePath(this.Source); }
    });

    /**
        Gets the path relative to the Media Library folder it is in.
        @returns {string}
    **/
    Object.defineProperty(this, 'relativePath',
    {
        get() { return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandleRelativePath(this.Source); }
    });
}

/**
     @constructor
     @name MetaDBHandleList
     @param {object} [arg] - MetaDB handle list
     @returns {void}
 **/
function MetaDBHandleList(arg)
{
    this.Source = arg;

    /**
        Gets the number of items.
        @returns {int}
    **/
    Object.defineProperty(this, 'count',
    {
        get() { return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandleListCount(this.Source); }
    });

    /**
        Releases the metadb handle list.
        @returns {void}
    **/
    this.release = function ()
    {
        chrome.webview.hostObjects.sync.foo_uie_webview.releaseMetaDBHandleList(this.Source);
    };

    // Indexer
    {
        const n = this.count;

        for (let i = 0; i < n; ++i)
        {
            Object.defineProperty(this, i,
            {
                get() { return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandleListItem(this.Source, i); }
            });
        }
    };

    // Iterator
    Object.defineProperty(this, Symbol.iterator,
    {
        value: function ()
        {
            let index = 0;

            const n = this.count;

            return
            {
                next: () =>
                (
                    {
                        done: index >= n,
                        value: new MetaDBHandle(this[index++])
                    }
                )
            };
        }
    });
}
