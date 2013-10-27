#ifndef __INCLUDE_FC__
#define __INCLUDE_FC__

#pragma comment( lib, "comctl32.lib" )
#pragma warning( disable : 4710 4711 )

#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlguid.h>
#include <shellapi.h>
#include <commctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <tl/tl.h>

namespace fc
{
#include "window.h"

#include "dialog.h"

#include "popupmenu.h"
#include "menu.h"

#include "statusbar.h"
#include "toolbar.h"

#include "editbox.h"
#include "spin.h"
#include "button.h"
#include "slider.h"
#include "progressbar.h"
#include "listbox.h"
#include "listview.h"
#include "treeview.h"
#include "combobox.h"
#include "tab.h"
#include "rebar.h"
#include "tooltip.h"
};
/*
#include "file.h"
#include "taskbar.h"

#include "static.h"
#include "ip.h"
#include "listview.h"

#include "menu.h"
#include "mouse.h"
#include "key.h"
*/
#endif
