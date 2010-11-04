/*-----------------------------------------------------------------------------
  example skin file (fennec, minimalization).
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "skin.h"
#include "resource.h"

/* declarations */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* data */

struct skin_data skin;
HDC              hdc;
HWND             wnd, hdlg;
HINSTANCE        hInst;
int              seekin = 0;

/* functions */

int skin_refresh(int inum, void *idata)
{

	return 1;
}

int skin_uninitialize(int inum, void *idata)
{
	ReleaseDC(wnd, hdc);
	return 1;
}

int fennec_skin_initialize(struct skin_data *indata, struct skin_data_return *outdata)
{
	memcpy(&skin, indata, sizeof(struct skin_data));

	outdata->refresh      = skin_refresh;
	outdata->uninitialize = skin_uninitialize;
	outdata->callback     = WindowProc;
	outdata->woptions     = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;

	wnd = indata->wnd;
	hdc = GetDC(wnd);

	SetWindowRgn(wnd, CreateRectRgn(0, 0, 0, 0), 1);

	Sleep(0);

	SetWindowPos(wnd, 0, 0, 0, 180, 140, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	ShowWindow(wnd, SW_SHOW);
	return 1;
}


/* Windows specific callbacks */


/*
 * startup.
 */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch(fdwReason) 
    { 
        case DLL_PROCESS_ATTACH:
			hInst = hinstDLL;
            break;
    }
    return 1;
}

/*
 * fennec main window.
 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mousedown = 0;

	switch(msg)
	{
	case WM_MOUSEMOVE:
		if(mousedown)
		{
			double pos;
			RECT   rct;
			int    x;

			GetClientRect(hwnd, &rct);

			x = LOWORD(lParam);

			if(x < 0)x = 0;
			if(x > rct.right)x = rct.right;
			
			pos = (double)x / (double)rct.right;

			skin.shared->audio.output.setposition(pos);
		}
		break;

	case WM_RBUTTONDOWN:
		mousedown = 1;
		SetCapture(hwnd);
		break;

	case WM_RBUTTONUP:
		mousedown = 0;
		ReleaseCapture();
		break;

	case WM_LBUTTONUP:
		skin.shared->audio.output.play();
		break;

	case WM_LBUTTONDBLCLK:
		skin.shared->simple.show_openfile();
		break;

	case WM_PAINT:
		{
			RECT rct;
			GetClientRect(hwnd, &rct);

			Rectangle(hdc, 0, 0, rct.right, rct.bottom);

			TextOut(hdc, 10, 10, "Play/Pause.", 11);
			TextOut(hdc, 10, 30, "Double click to load.", 21);
			TextOut(hdc, 10, 50, "Right-Drag to seek.", 19);
			TextOut(hdc, 10, 70, "Press F2 for settings.", 22);

		}
		break;
	}
	return 1;
}

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
