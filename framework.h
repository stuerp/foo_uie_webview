
/** $VER: framework.h (2024.11.27) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <sdk/foobar2000-lite.h>
#include <sdk/autoplaylist.h>

#include <SDKDDKVer.h>

#include <Windows.h>
#include <pathcch.h>

#include <atlbase.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlcrack.h>

#include <algorithm>
#include <cmath>
#include <cassert>
#include <string>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_INSTANCE ((HINSTANCE) &__ImageBase)
