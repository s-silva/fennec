/*-----------------------------------------------------------------------------
  example skin file (fennec, minimalization).
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include <windows.h>
#include <commctrl.h>
#include "../../include/settings.h"
#include "../../include/fennec.h"

struct skin_data
{
	WNDPROC        wproc;
	HWND           wnd;
	HINSTANCE      finstance;
	struct fennec *shared;
};

struct skin_data_return
{
	int (*uninitialize)(int, void*);
	int (*refresh)(int, void*);
	WNDPROC callback;
	LONG    woptions;
};

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
