#include "skin.h"

#define media_window_class uni("fennec.skin.3.mediaeo")
#define media_scroll_class uni("fennec.skin.3.mediaeo.scroll")
#define user_msg_start_keep_cursor 0x1
#define user_msg_end_keep_cursor   0x2

LRESULT CALLBACK callback_media_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK callback_scroll_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void media_create_scrolls(HWND hwndp);

HWND   window_media;
HDC    hdc_media;
int    media_init = 0;
HBRUSH hbrush_searchbox = 0;

extern letter  full_view_text_search[512];
extern HWND    hwnd_searchtext;
extern WNDPROC searchtext_wndproc;
extern int     skin_closing;


void media_create(HWND hwndp)
{
	WNDCLASS wndc;
	RECT     wrect;

	if(media_init)return;

	wndc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndc.lpfnWndProc   = (WNDPROC)callback_media_window;
	wndc.cbClsExtra    = 0;
	wndc.cbWndExtra    = 0;
	wndc.hInstance     = instance_skin;;
	wndc.hIcon         = LoadIcon(skin.finstance, (LPCTSTR)0);
	wndc.hCursor       = LoadCursor(0, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndc.lpszMenuName  = 0;
	wndc.lpszClassName = media_window_class;

	RegisterClass(&wndc);

	ml_cache_init();
	covers_init();

	hbrush_searchbox = CreateSolidBrush(0x5e5e5e);
	

	/* create window */

	SystemParametersInfo(SPI_GETWORKAREA, 0, &wrect, 0);

	window_media = CreateWindowW(media_window_class, uni("mediawindow"), WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX, wrect.top, wrect.left, wrect.right - wrect.left, wrect.bottom - wrect.top, 0, 0, instance_skin, 0);

	SetWindowLong(hwndp, GWL_HWNDPARENT, (LONG)window_media);

	ShowWindow(window_media, SW_SHOW);
	UpdateWindow(window_media);

	media_init = 1;
}

void media_close(void)
{
	if(!media_init)return;

	SetWindowLong(wnd, GWL_HWNDPARENT, 0);


	if(!skin_closing)
	{
		ShowWindow(wnd, SW_HIDE);
		ShowWindow(wnd, SW_SHOW);
	}

	if(hdc_media)DeleteDC(hdc_media);
	DestroyWindow(window_media);

	media_init = 0;
	fullview_uninit();
	ml_cache_uninit();
	covers_uninit();

	if(hbrush_searchbox) DeleteObject(hbrush_searchbox);
}

void media_draw_background(void)
{
	fullview_drawgeneral();
}

int media_refresh(void)
{
	media_draw_background();
	return 1;
}

/*
 * this function returns the corner vectors,
 * (not like vis_get_position)
 */
int media_get_position(RECT *retp)
{

	return 1;
}

LRESULT CALLBACK callback_searchbox(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
	case WM_KEYDOWN:
		if(wParam == VK_RETURN)
		{
			GetWindowText(hwnd, full_view_text_search, 511);
		}
		break;

	default:

		if(searchtext_wndproc)
			return CallWindowProc(searchtext_wndproc, hwnd, msg, wParam, lParam);
		else
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK callback_media_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, dx, dy, lx, ly;


	switch(msg)
	{
	case WM_TIMER:
		{
			letter wtext[512];
			GetWindowText(wnd, wtext, 512);
			SetWindowText(hwnd, wtext);
		}
		break;

	case WM_MOUSEMOVE:
		fullview_mousemsg(LOWORD(lParam), HIWORD(lParam), mm_move);
		break;

	case WM_MOUSEWHEEL:
		fullview_mousemsg((int)((short)HIWORD(wParam)), 0, mm_wheel);
		break;

	case WM_CTLCOLOREDIT:
		SetTextColor((HDC)wParam, 0xdbdbdb);
		SetBkColor((HDC)wParam, 0x5e5e5e);
		return (LRESULT)hbrush_searchbox;

	case WM_KEYDOWN:
		fullview_keymsg((int)wParam);
		break;


	case WM_LBUTTONDBLCLK:
		fullview_mousemsg(LOWORD(lParam), HIWORD(lParam), mm_dbl_l);
		break;

	case WM_LBUTTONDOWN:
		fullview_mousemsg(LOWORD(lParam), HIWORD(lParam), mm_down_l);
		break;

	case WM_USER:

		break;

	case WM_LBUTTONUP:
		fullview_mousemsg(LOWORD(lParam), HIWORD(lParam), mm_up_l);
		break;

	case WM_RBUTTONDOWN:
		fullview_mousemsg(LOWORD(lParam), HIWORD(lParam), mm_down_r);
		break;

	case WM_RBUTTONUP:
		fullview_mousemsg(LOWORD(lParam), HIWORD(lParam), mm_up_r);
		break;

	case WM_CREATE:
		hdc_media = GetDC(hwnd);
		window_media = hwnd;
		fullview_init();
		SetTimer(hwnd, 1010, 100, 0);
		break;

	case WM_PAINT:
		media_draw_background();
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
