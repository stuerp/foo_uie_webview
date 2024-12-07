
# foo_uie_webview

[foo_uie_webview](https://github.com/stuerp/foo_uie_webview/releases) is a [foobar2000](https://www.foobar2000.org/) component that exposes the [Microsoft WebView2](https://learn.microsoft.com/en-us/microsoft-edge/webview2/) control as UI panel. The component started as foo_vis_text.

It takes an HTML file that receives playback notifications from foobar2000. The panel can react to those notifications and adjust its output using JavaScript code.

## Features

* Supports the Default User Interface (DUI) and the [Columns User Interface](https://yuo.be/columns-ui) (CUI).
* Supports dark mode.
* Supports foobar2000 2.0 and later (32-bit and 64-bit version).

## Requirements

* [foobar2000](https://www.foobar2000.org/download) v2.0 or later (32 or 64-bit). ![foobar2000](https://www.foobar2000.org/button-small.png)
* [Microsoft Edge WebView2](https://learn.microsoft.com/en-us/deployoffice/webview2-install)
* Tested on Microsoft Windows 10 and later.
* Tested with [Columns UI](https://yuo.be/columns-ui) 2.1.0.
* Tested with WebView2 126.0.2592.87.

## Getting started

* Double-click `foo_uie_webview.fbk2-component`.

or

* Import `foo_uie_webview.fbk2-component` into foobar2000 using the "*File / Preferences / Components / Install...*" menu item.

### First-run

When you add a WebView panel to the foobar2000 user interface the following things happen:

* The component tries to enable a supported WebView2 control. If no compatible WebView can be found the component will fail gracefully and report the error in the foobar2000 console.
* A subdirectory called `foo_uie_webview` gets created in your foobar2000 profile directory. It will contain various state information used by WebView2.
* The component looks by default for the HTML file `foo_uie_webview\Template.html` in your foobar2000 profile directory. The location of the template can be changed in the preference page of the component. A copy of the default template will be created in that location if the file does not exist when the component starts.
* A `WebView` preferences page is added to Preferences dialog in the `Display` category.

### Preferences

The preferences page allows you to specify the path of the template file that should be used. The path can contain environment variables e.g. `"%UserProfile%\Documents\Template.html"`.

The panel automatically rereads the template file when the file path or the contents changes.

The `Edit` button launches the editor that has been associated with the file type of the template in Windows.

### Tips

* The directory where the component is installed contains an example template file called `Default-Template.html`. Do not store your customized template file in this directory because it will be overwritten or removed when the component gets upgraded.
* The context menu of the WebView contains a `WebView` submenu that can take you directly to the preferences page.
* Check the foobar2000 console for lines marked with `foo_uie_webview` in case you encounter problems with the component.

## Developing

### Requirements

To build the code you need:

* [Microsoft Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/downloads/) or later
* [foobar2000 SDK](https://www.foobar2000.org/SDK) 2023-09-23
* [Windows Template Library (WTL)](https://github.com/Win32-WTL/WTL) 10.0.10320
* [Columns UI SDK](https://yuo.be/columns-ui-sdk) 7.0.0

To create the deployment package you need:

* [PowerShell 7.2](https://github.com/PowerShell/PowerShell) or later

### Setup

Create the following directory structure:

    3rdParty
        columns_ui-sdk
        WTL10_10320
    bin
        x86
    foo_uie_webview
    out
    sdk

* `3rdParty/columns_ui-sdk` contains the Columns UI SDK 7.0.0.
* `3rdParty/WTL10_10320` contains WTL 10.0.10320.
* `bin` contains a portable version of foobar2000 64-bit for debugging purposes.
* `bin/x86` contains a portable version of foobar2000 32-bit for debugging purposes.
* `foo_uie_webview` contains the [Git](https://github.com/stuerp/foo_uie_webview) repository.
* `out` receives a deployable version of the component.
* `sdk` contains the foobar2000 SDK.

### Building

Open `foo_uie_webview.sln` with Visual Studio and build the solution.

### Packaging

To create the component first build the x86 configuration and next the x64 configuration.

## Change Log

v0.2.0.0, 2024-12-07, *"I'm getting framed..."*

* New: Builds with foobar2000 SDK 2024-08-07.
* New: Updated WebView2 component to 1.0.2849.39.
* New: Templates can contain iframes. Take a look at the included Default-FrameTemplate.html example.
* New:
  * Methods
    * readAllText(filePath, codePage): Reads a file and returns it as a string. If codePage is 0 the file is assumed to be UTF-8 encoded.
    * readImage(filePath): Reads an image file and returns it as a Base64 string. (alpha2)
    * readDirectory(directoryPath, searchPattern): Reads a directory and returns the matching items as a JSON string. (alpha3)

    * getPlaylistName(playlistIndex): Gets the name of a playlist. (alpha2)
    * setPlaylistName(playlistIndex, name): Sets the name of a playlist. (alpha2)
    * findPlaylist(name): Returns the index of the playlist with the specified name. (alpha2)
    * getPlaylistItemCount(index): Gets the number of items in a platlist. (alpha2)
    * getFocusedPlaylistItem(playlistIndex): Gets the index of the focused item in a playlist. (alpha2)
    * setFocusedPlaylistItem(playlistIndex, name): Sets the index of the focused item in a playlist. (alpha2)
    * ensurePlaylistItemVisible(playlistIndex, itemIndex): Ensures the specified item of a playlist is visible. (alpha2)
    * isPlaylistItemSelected(playlistIndex, itemIndex): Returns true if the specified item of a playlist is selected. (alpha3)   
    * executePlaylistDefaultAction(playlistIndex, itemIndex): Executes the default action on the specified item of a playlist. (alpha2)

    * createPlaylist(playlistIndex, name): Creates a new playlist with the specified name and inserts it after the specified playlist. (alpha2)
    * addPath(playlistIndex, itemIndex, filePath, selectAddedItem): Adds an item to the specified playlist after the specified item using a file path and optionally selects it. (alpha3)

    * duplicatePlaylist(playlistIndex, name): Duplicates the specified playlist and sets its name. (alpha2)
    * getPlaylistItems(playlistIndex): Returns the items of the specified playlist as a JSON string. (alpha3)
    * selectPlaylistItem(playlistIndex, itemIndex): Selects the specified item in a playlist. (alpha3)
    * deselectPlaylistItem(playlistIndex, itemIndex): Deselects the specified item in the specified playlist. (alpha3)
    * getSelectedPlaylistItems(playlistIndex): Returns the selected items of the specified playlist as a JSON string. (alpha3)
    * clearPlaylistSelection(playlistIndex): Clears the selected items in the specified playlist. (alpha2)
    * removeSelectedPlaylistItems(playlistIndex): Removes the selected items from the specfied playlist. (alpha3)
    * removeUnselectedPlaylistItems(playlistIndex): Removes the unselected items from the specfied playlist. (alpha3)
   
    * removePlaylistItem(playlistIndex, itemIndex): Removes the specified item from the specified playlist. (alpha3)

    * clearPlaylist(playlistIndex): Removes all items from the specified playlist. (alpha2)
    * deletePlaylist(playlistIndex): Deletes the specified playlist. (alpha3)

    * createAutoPlaylist(playlistIndex, name, query, sort, flags): Creates an auto playlist. Possible values for flags are 0 = default, 1 = Keep sorted). (alpha2)
    * isAutoPlaylist(playlistIndex): Returns true if the specified playlist is an auto playlist. (alpha2)

  * Properties
    * playlistCount: Returns the number of playlists. (alpha2)
    * activePlaylist: Gets or sets index of the active playlist. (alpha2)
    * playingPlaylist: Gets or sets index of the playing playlist. (alpha2)
    * isAutoPlaylist: Returns true if the specified playlist is an auto playlist. (alpha2)
    * playbackOrder: Gets or sets the playback order (0 = default, 1 = repeat playlist, 2 = repeat track, 3 = random, 4 = shuffle tracks, 5 = shuffle albums, 6 = shuffle folders). (alpha2)

  * Callbacks
    * onPlaylistItemsAdded(playlistIndex, startindex, locations): Called when items have been added to the specified playlist. (alpha5)
    * onPlaylistItemsReordered(playlistIndex, items): Called when the items of the specified playlist have been reordered. (alpha5)
    * onPlaylistItemsRemoving(playlistIndex, removedItems, newCount): Called when removing items of the specified playlist. (alpha5)
    * onPlaylistItemsRemoved(playlistIndex, removedItems, newCount): Called when items of the specified playlist have been removed. (alpha5)
    * onPlaylistItemsModified(playlistIndex, items): Called when some playlist items of the specified playlist have been modified. (alpha5)
    * onPlaylistItemsModifiedFromPlayback(playlistIndex, items): Called when some playlist items of the specified playlist have been modified from playback. (alpha5)
    * onPlaylistItemsReplaced(playlistIndex, items): Called when items of the specified playlist have been replaced. (alpha5)

    * onPlaylistItemEnsureVisible(playlistIndex, itemIndex): Called when the specified item of a playlist was ensured to be visible. (alpha4)

    * onPlaylistCreated(playlistIndex, name): Called when the specified playlist has been created. (alpha4)
    * onPlaylistRenamed(playlistIndex, name): Called when the specified playlist has been renamed. (alpha4)
    * onPlaylistActivated(oldPlaylistIndex, newPlaylistIndex): Called when the active playlist is changed. (alpha4)
    * onPlaylistLocked(playlistIndex): Called when the specified playlist has been locked. (alpha4)
    * onPlaylistUnlocked(playlistIndex): Called when the specified playlist has been unlocked. (alpha4)
    * onPlaylistSelectedItemsChanged(playlistIndex, selectedItems): Called when the selected items of the specified playlist have been changed. (alpha5)
    * onPlaylistFocusedItemChanged(playlistIndex, fromItemIndex, toItemIndex): Called when the focused item of the specified playlist has been changed. (alpha4)

    * onPlaylistsReordered(playlistOrder): Called when the playlists have been reordered. (alpha5)
    * onPlaylistsRemoving(removedPlaylists, newCount): Called when playlists are being removed. (alpha5)
    * onPlaylistsRemoved(removedPlaylists, newCount): Called when playlists have been removed. (alpha5)

    * onDefaultFormatChanged(): Called when the default format has been changed. (alpha4)
    * onPlaybackOrderChanged(playbackOrderIndex): Called when the playback order has been changed. (alpha4)

  * Fixed: Booleans in objects are now parsed correctly. (alpha4)
  * Fixed: Boolean parameters and return values are true Javascript booleans now. (alpha4)
  * Improved: The searchPattern parameter is now optional in ReadDirectory() and defaults to "\*.\*". (alpha4)
  * Improved: The CreateAutoPlaylist() sort and flags parameter can be omitted. They default to "" and 0 respectively. (alpha4)
  * Changed: Returned JSON objects now use camelCase casing. (alpha4)
  * Changed: *Breaking Change* Callbacks follow the Category-Noun-Verb naming convention. (alpha5)
  * Changed: *Breaking Change* All properties, methods and callbacks to use camelCase convention. (alpha5)
  * Changed: *Breaking Change* The parameter list of most callbacks has been expanded. (alpha5)

v0.1.8.0, 2024-08-10

* New: The Fluent scrollbar style can be disabled.
  * The component needs to be restarted for the change to become active.
  * Be sure to use separate user data folders when you have multiple instances that use different styles.

v0.1.7.0, 2024-07-14

* New: In Private mode can be enabled in the Preferences dialog and is no longer enabled by default (alpha3).
* New:
  * Methods
    * getArtwork(): Gets the embedded artwork (front / back / disc / icon / artist) from the current playing item (alpha1).
      * Fixed support for other artwork types (alpha2, regression).
      * Added support for WebP images (alpha2).
      * Always returns an empty data URI in case of an error or if the specified artwork type is not available (alpha2).
      * The album art search patterns are used first. If no matching file can be found, the embedded artwork gets queried (alpha3).
* Changed: Updated the WebView2 SDK to the latest version (alpha3).

v0.1.6.0, 2024-07-09

* New: Added a setting to clear the browsing data on startup or not.

v0.1.5.6, 2024-07-08

* New: Each instance of the component can have a name to easier distinguish between them.
* New: The location of the WebView user data folder can be specified in the Preferences dialog.
  * Note: The existing folder will not be moved or deleted. The new location will only be used after restarting foobar2000.
* New: Made the sample chunks from the foobar2000 visualization stream available to JavaScript.
* New: The component uses the DUI and CUI foreground and background color (alpha2).
* New: The window size can be specified in milliseconds or samples (alpha2).
* New: The reaction alignment can be specified (alpha2) to position the window behind or ahead of the playback samples.
* New: Renamed the component to foo_uie_webview (alpha3).
  * The component GUID remains the same.
  * The settings are retained.
  * Delete the "EBWebView" sub-directory of your user data folder to prevent caching problems.
* New: Added methods and properties to chrome.webview.hostObjects.sync.foo_uie_webview (alpha5):
  * Properties
    * componentVersion and componentVersionText: The version of this component as packed integer and as text.
    * isPlaying: Gets whether playback is active.
    * isPaused: Gets whether playback is active and in paused state.
    * stopAfterCurrent: Gets or sets the stop-after-current-track option state.
    * length: Gets the length of the currently playing item, in seconds.
    * position: Gets the playback position within the currently playing item, in seconds.
    * canSeek: Gets whether currently playing track is seekable. If it's not, Seek/SeekDelta calls will be ignored.
    * volume: Gets or sets the playback volume in dBFS. Use 0 for full volume.
    * isMuted: Gets whether playback is muted.
  * Methods
    * print(text): Prints text from JavaScript on the foobar2000 console.
    * stop(): Stops playback.
    * play(paused): Starts playback, paused or unpaused. If playback is already active, existing process is stopped first.
    * pause(paused): Pauses or resumes playback.
    * previous(): Plays the previous track from the current playlist according to the current playback order.
    * next(): Plays the next track from the current playlist according to the current playback order.
    * random(): Plays a random track from the current playlist (aka Shuffle).
    * togglePause(): Toggles the pause status.
    * toggleMute(): Toggles playback mute state.
    * toggleStopAfterCurrent(): Toggles the stop-after-current mode.
    * volumeUp(): Increases the volume with one step.
    * volumeDown(): Decreases the volume with one step.
    * seek(time): Seeks in the currently playing track to the specified time, in seconds.
    * seekDelta(delta): Seeks in the currently playing track forward or backwards by the specified delta time, in seconds.
* New: Each instance of the component uses its own browser profile.
* Improved: WebView is a Utility panel again and can be shown as a popup (alpha5).
* Improved: Enabled more options in WebView2 to better support dark mode (alpha3).
* Fixed: A last-minute change (never a good thing) broke the support for multiple instances of the Preferences dialog box.
* Fixed: Javascript example code (alpha2)
* Fixed: Support for tracks with more than 2 channels was not implemented correctly (alpha2).
* Fixed: The background color of the WebView is now correctly and dynamically set (alpha3).
* Fixed: WebView not appearing in CUI tabs (alpha4, Regression).

v0.1.4.0, 2024-06-12

* New: Support for multiple instances. Each instance can have its own configuration.
* New: Added onPlaybackEdited() callback.
* Improved: The name of the component will be displayed in Layout Edit mode.
* Fixed: Work-around for WebView not appearing after foobar2000 starts while being hosted in a hidden tab.

v0.1.3.1, 2024-06-05

* Fixed: "Layout Editing" mode now works when the panel is part of a Tab control.

v0.1.3.0, 2024-06-04, *"It's getting dark in here..."*

* New: Added dark mode support to WebView.
* New: Added "Follow selected track" mode. Supports the "Display / Selection Viewers" user preference ("Prefer current selection" vs. "Prefer currently playing track").
* Fixed: Added support for "Layout Editing" mode. The foobar2000 context menu is available again.

v0.1.2.0, 2024-06-03, *"Listening to user feedback"*

* New: Added onPlaybackDynamicInfo() and onPlaybackDynamicTrackInfo() callback. Useful to display meta data from streamed audio.
* Improved: A copy of the default template file is created when the component starts the first time.
* Improved: The context menu contains a menu item to access the preferences page.

v0.1.1.0, 2024-06-02, *"Down, boy..."*

* Fixed: Error handling is more tolerant during a first-run situation.

v0.1.0.0, 2024-06-02, *"Scratchin' the itch"*

* Initial release.

## Acknowledgements / Credits

* Peter Pawlowski for the [foobar2000](https://www.foobar2000.org/) audio player. ![foobar2000](https://www.foobar2000.org/button-small.png)

## Reference Material

* https://learn.microsoft.com/en-us/microsoft-edge/webview2/get-started/win32
* https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/user-data-folder?tabs=win32
* https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/?view=webview2-1.0.2478.35
* https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2_3?view=webview2-1.0.2478.35#setvirtualhostnametofoldermapping
* https://learn.microsoft.com/en-us/microsoft-edge/webview2/how-to/hostobject?tabs=win32
* https://learn.microsoft.com/en-us/microsoft-edge/webview2/how-to/javascript

## Links

* Home page: [https://github.com/stuerp/foo_uie_webview](https://github.com/stuerp/foo_uie_webview)
* Repository: [https://github.com/stuerp/foo_uie_webview.git](https://github.com/stuerp/foo_uie_webview.git)
* Issue tracker: [https://github.com/stuerp/foo_uie_webview/issues](https://github.com/stuerp/foo_uie_webview/issues)
* Wiki: [https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Components/WebView_(foo_uie_webview)](https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Components/WebView_(foo_uie_webview)).

## License

![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)
