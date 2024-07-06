
# foo_uie_webview

[foo_uie_webview](https://github.com/stuerp/foo_uie_webview/releases) is a [foobar2000](https://www.foobar2000.org/) component that exposes the [Microsoft Edge WebView2](https://learn.microsoft.com/en-us/microsoft-edge/webview2/) control as UI panel. The component started as foo_vis_text.

It takes an HTML file that receives playback notifications from foobar2000. The panel can react to those notifications and adjust its output using Javascript code.

## Features

* Supports the Default User Interface (DUI) and the [Columns User Interface](https://yuo.be/columns-ui) (CUI).
* Supports dark mode.
* Supports foobar2000 2.0 and later (32-bit and 64-bit version).

## Requirements

* [foobar2000](https://www.foobar2000.org/download) v2.0 or later (32 or 64-bit). ![foobar2000](https://www.foobar2000.org/button-small.png)
* [Microsoft Edge WebView2](https://learn.microsoft.com/en-us/deployoffice/webview2-install)
* Tested on Microsoft Windows 10 and later.
* Tested with [Columns UI](https://yuo.be/columns-ui) 2.1.0.

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

v0.1.5.0-alpha4, 2024-07-06

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
* Improved: Enabled more options in WebView2 to better support dark mode (alpha3).
* Fixed: A last-minute change (never a good thing) broke the support for multiple instances of the Preferences dialog box.
* Fixed: Javascript example code (alpha2)
* Fixed: Support for tracks with more than 2 channels was not implemented correctly (alpha2).
* Fixed: The background color of the WebView is now correctly and dynamically set (alpha3).
* Fixed: WebView not appearing in CUI tabs (alpha4, Regression).

v0.1.4.0, 2024-06-12

* New: Support for multiple instances. Each instance can have its own configuration.
* New: Added OnPlaybackEdited() callback.
* Improved: The name of the component will be displayed in Layout Edit mode.
* Fixed: Work-around for WebView not appearing after foobar2000 starts while being hosted in a hidden tab.

v0.1.3.1, 2024-06-05

* Fixed: "Layout Editing" mode now works when the panel is part of a Tab control.

v0.1.3.0, 2024-06-04, *"It's getting dark in here..."*

* New: Added dark mode support to WebView.
* New: Added "Follow selected track" mode. Supports the "Display / Selection Viewers" user preference ("Prefer current selection" vs. "Prefer currently playing track").
* Fixed: Added support for "Layout Editing" mode. The foobar2000 context menu is available again.

v0.1.2.0, 2024-06-03, *"Listening to user feedback"*

* New: Added OnPlaybackDynamicInfo() and OnPlaybackDynamicTrackInfo() callback. Useful to display meta data from streamed audio.
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
* Wiki: [https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Components/Text_Visualizer_(foo_uie_webview)](https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Components/Text_Visualizer_(foo_uie_webview)).

## License

![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)
