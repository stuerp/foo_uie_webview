
/** $VER: Resources.h (2024.07.10) P. Stuer **/

#pragma once

#define TOSTRING_IMPL(x) #x
#define TOSTRING(x) TOSTRING_IMPL(x)

#define NUM_FILE_MAJOR          0
#define NUM_FILE_MINOR          1
#define NUM_FILE_PATCH          6
#define NUM_FILE_PRERELEASE     3

#define NUM_PRODUCT_MAJOR       0
#define NUM_PRODUCT_MINOR       1
#define NUM_PRODUCT_PATCH       6
#define NUM_PRODUCT_PRERELEASE  3

/** Component specific **/

#define STR_COMPONENT_NAME          "WebView"
#define STR_COMPONENT_VERSION       TOSTRING(NUM_FILE_MAJOR) "." TOSTRING(NUM_FILE_MINOR) "." TOSTRING(NUM_FILE_PATCH) "." TOSTRING(NUM_FILE_PRERELEASE) "-alpha3"
#define STR_COMPONENT_BASENAME      "foo_uie_webview"
#define STR_COMPONENT_FILENAME      STR_COMPONENT_BASENAME ".dll"
#define STR_COMPONENT_COMPANY_NAME  ""
#define STR_COMPONENT_COPYRIGHT     "Copyright (c) 2024 P. Stuer. All rights reserved."
#define STR_COMPONENT_COMMENTS      ""
#define STR_COMPONENT_DESCRIPTION   "A WebView2 wrapper for foobar2000"
#define STR_COMPONENT_COMMENT       ""

/** Generic **/

#define STR_COMPANY_NAME        TEXT(STR_COMPONENT_COMPANY_NAME)
#define STR_INTERNAL_NAME       TEXT(STR_COMPONENT_NAME)
#define STR_COMMENTS            TEXT(STR_COMPONENT_COMMENTS)
#define STR_COPYRIGHT           TEXT(STR_COMPONENT_COPYRIGHT)

#define STR_FILE_NAME           TEXT(STR_COMPONENT_FILENAME)
#define STR_FILE_VERSION        TOSTRING(NUM_FILE_MAJOR) TEXT(".") TOSTRING(NUM_FILE_MINOR) TEXT(".") TOSTRING(NUM_FILE_PATCH) TEXT(".") TOSTRING(NUM_FILE_PRERELEASE)
#define STR_FILE_DESCRIPTION    TEXT(STR_COMPONENT_DESCRIPTION)

#define STR_PRODUCT_NAME        STR_INTERNAL_NAME
#define STR_PRODUCT_VERSION     TOSTRING(NUM_PRODUCT_MAJOR) TEXT(".") TOSTRING(NUM_PRODUCT_MINOR) TEXT(".") TOSTRING(NUM_PRODUCT_PATCH) TEXT(".") TOSTRING(NUM_PRODUCT_PRERELEASE)

#define STR_ABOUT_NAME          STR_INTERNAL_NAME
#define STR_ABOUT_WEB           TEXT("https://github.com/stuerp/") STR_COMPONENT_BASENAME
#define STR_ABOUT_EMAIL         TEXT("mailto:peter.stuer@outlook.com")

/** Window **/

#define GUID_UI_ELEMENT         {0xdabae3e2, 0xd31d, 0x4faa, { 0x88, 0xae, 0xf1, 0x50, 0xd6, 0x80, 0x33, 0x85}};
#define GUID_PREFERENCES        {0xb18587e0, 0x9c95, 0x4ee3, { 0x8e, 0x9f, 0xaa, 0x8c, 0x77, 0xec, 0x2f, 0x85}};
#define STR_WINDOW_CLASS_NAME   STR_COMPONENT_BASENAME "_{A1D51583-D8B7-40CF-88EC-B4C0AB194140}"

/** Messages **/

#define UM_TEMPLATE_CHANGED     WM_USER + 100
#define UM_WEB_VIEW_READY       WM_USER + 101
#define UM_ASYNC                WM_USER + 102

/** Configuration **/

#define IDD_PREFERENCES                      101

#define IDC_NAME                            1000

#define IDC_USER_DATA_FOLDER_PATH           1002
#define IDC_USER_DATA_FOLDER_PATH_SELECT    1004

#define IDC_FILE_PATH                       1006
#define IDC_FILE_PATH_SELECT                1008
#define IDC_FILE_PATH_EDIT                  1010

#define IDC_WINDOW_SIZE                     1020
#define IDC_WINDOW_SIZE_UNIT                1022

#define IDC_REACTION_ALIGNMENT              1030
#define IDC_WINDOW_OFFSET                   1032

#define IDC_CLEAR_BROWSING_DATA             1040
#define IDC_IN_PRIVATE_MODE                 1042

#define IDC_WARNING                         9999

#define IDR_CONTEXT_MENU_ICON               2000
