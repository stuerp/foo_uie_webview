
/** $VER: foobar2000.js (2024.12.31) P. Stuer **/

"use strict";

// @ts-check

/**
    @typedef {object} Foobar2000
    @description Provides access to the foobar2000 application.
    @property {boolean} canReadFiles - Gets whether the application can read files.
    @property {boolean} canReadDirectories - Gets whether the application can read directories.
    @property {boolean} canExecuteShellOperations - Gets whether the application can execute shell operations.
    @function execute - Performs the specified shell operation on a file. Mostly used to run applications. Check the [SHELLEXECUTEINFOW](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow) documentation for the possible values of the parameters.
    @kind class
 **/
class Foobar2000
{
    static
    {
    }

    /**
        @description Returns true if the component can read local files. Needs to be enabled by the user in the component preferences.
        @name canReadFiles
        @returns {boolean}
        @function
     **/
    static get canReadFiles()
    {
        return chrome.webview.hostObjects.sync.foo_uie_webview.canReadFiles;
    }

    /**
        @description Returns true if the component can read directories. Needs to be enabled by the user in the component preferences.
        @name canReadDirectories
        @returns {boolean}
        @function
     **/
    static get canReadDirectories()
    {
        return chrome.webview.hostObjects.sync.foo_uie_webview.canReadDirectories;
    }

    /**
        @description Returns true if the component can execute Shell operations on local files. Needs to be enabled by the user in the component preferences.
        @name canExecuteShellOperations
        @returns {boolean}
        @function
     **/
    static get canExecuteShellOperations()
    {
        return chrome.webview.hostObjects.sync.foo_uie_webview.canExecuteShellOperations;
    }

    /**
        @description Performs the specified shell operation on a file. Mostly used to run applications. Check the [SHELLEXECUTEINFOW](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow) documentation for the possible values of the parameters.
        @param {string} filePath - The path of the file.
        @param {string=} [parameters] - The parameters to pass to the operation.
        @param {string=} [directoryPath] - The directory path.
        @param {string=} [operation] - The shell operation to execute on the file.
        @param {number=} [showMode] - The show mode of the window.
        @returns {void}
        @function
    **/
    static execute(filePath, parameters, directoryPath, operation, showMode)
    {
        chrome.webview.hostObjects.foo_uie_webview.execute(filePath, parameters, directoryPath, operation, showMode);
    }
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

/**
    @typedef {object} MediaLibrary
    @description Provides access to the foobar2000 Media Library.
    @property {boolean} isEnabled
    @function showPreferences
    @function search
    @kind class
 **/
class MediaLibrary
{
    static
    {
    }

    /**
        @description Gets whether the Media Library is enabled.
        @name isEnabled
        @returns {boolean}
        @kind function
    **/
    static get isEnabled()
    {
        return chrome.webview.hostObjects.sync.foo_uie_webview.isLibraryEnabled;
    }

    /**
        @description Shows the Media Library preferences dialog.
        @returns {void}
        @kind function
    **/
    static showPreferences()
    {
        chrome.webview.hostObjects.foo_uie_webview.showLibraryPreferences();
    }

    /**
        @description Searches the Media Library for matching tracks.
        @param {string} query - The search query.
        @returns {number} - Handle to a metadb handle list.
        @kind function
    **/
    static search(query)
    {
        return chrome.webview.hostObjects.foo_uie_webview.searchLibrary(query);
    }
};

/**
    @typedef {Object} MetaDBHandle
    @property {number} Source - Returns the native handle value of the metadb handle
    @property {string} path - Returns the file path represented by the handle.
    @property {string} relativePath - Returns the file path relative to the Media Library folder it is in represented by the handle.
    @property {number} length - Returns the length of the track represented by the handle.
    @property {function} FormatTitle - Formats the title of the track represented by the handle.
    @kind class
 **/
/**
    @constructor
    @name MetaDBHandle
    @param {number} handle - The native handle value of the metadb handle
    @returns {void}
    @kind function
 **/
function MetaDBHandle(handle)
{
    if (handle === 0)
        throw new Error('Invalid argument');

    /**
        @description The native handle value of the metadb handle
        @type {number}
        @public
    **/
    this.Source = handle;

    /**
        @description Returns the file path represented by the handle.
        @returns {string}
        @kind function
    **/
    Object.defineProperty(this, 'path',
    {
        get()
        {
            if (this.Source === 0)
                throw new Error('Object is not bound to a handle');

            return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandlePath(this.Source);
        }
    });

    /**
        @description Returns the file path relative to the Media Library folder it is in represented by the handle.
        @returns {string}
        @kind function
    **/
    Object.defineProperty(this, 'relativePath',
    {
        get()
        {
            if (this.Source === 0)
                throw new Error('Object is not bound to a handle');

            return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandleRelativePath(this.Source);
        }
    });

    /**
        @description Returns the length of the track represented by the handle.
        @returns {number}
        @kind function
    **/
    Object.defineProperty(this, 'length',
    {
        get()
        {
            if (this.Source === 0)
                throw new Error('Object is not bound to a handle');

            return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandleLength(this.Source);
        }
    });

    /**
        @description Formats the title of the track represented by the handle.
        @param {string} text - The foobar2000 title formating to use. (https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Title_Formatting_Reference)
        @returns {string}
        @kind function
    **/
    this.formatTitle = function(text)
    {
        if (this.Source === 0)
            throw new Error('Object is not bound to a handle');

        return chrome.webview.hostObjects.sync.foo_uie_webview.formatTitleMetaDBHandle(this.Source, text);
    };
}

/**
    @typedef {Object} MetaDBHandleList
    @description Provides access to a list of MetaDB handles.
    @property {number} Source - The native handle value of the metadb handle list
    @property {number} count - Gets the number of items.
    @function release - Releases the metadb handle list.
    @kind class
 **/
/**
     @constructor
     @name MetaDBHandleList
     @param {object} handle - The native handle value of the metadb handle list
     @returns {void}
     @kind function
 **/
function MetaDBHandleList(handle)
{
    if (handle === 0)
        throw new Error('Invalid argument');

    /**
        @description The native handle value of the metadb handle list
        @type {number}
        @public
    **/
    this.Source = handle;

    /**
        @description Gets the number of items.
        @returns {number}
    **/
    Object.defineProperty(this, 'count',
    {
        get()
        {
            if (this.Source === 0)
                throw new Error('Object is not bound to a handle list');

            return chrome.webview.hostObjects.sync.foo_uie_webview.getMetaDBHandleListCount(this.Source);
        }
    });

    /**
        @description Releases the metadb handle list.
        @returns {void}
    **/
    this.release = function ()
    {
        if (this.Source === 0)
            throw new Error('Object is not bound to a handle list');

        chrome.webview.hostObjects.sync.foo_uie_webview.releaseMetaDBHandleList(this.Source);
        this.Source = 0;
    };

    // Indexer
    {
        if (this.Source === 0)
            throw new Error('Object is not bound to a handle list');

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
            const n = this.count;

            let index = 0;

            return
            {
                next: () =>
                (
                    {
                        done: index >= n,
                        value: this[index++]
                    }
                )
            };
        }
    });
}
