//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//

#include "tinyptc.h"

#ifdef __PTC_DDRAW__

#include "convert.h"
#define WIN32_LEAN_AND_MEAN
#include "ddraw.h"

PTC_CONVERTER convert;
static HMODULE library = 0;
static LPDIRECTDRAW lpDD = 0;
static LPDIRECTDRAWSURFACE lpDDS = 0;
static LPDIRECTDRAWSURFACE lpDDS_back;
static WNDCLASS wc;
static HWND wnd;
static int active;
static int dx;
static int dy;

extern WNDPROC window_vis_proc;

#ifdef __PTC_WINDOWED__
static LPDIRECTDRAWCLIPPER lpDDC = 0;
static LPDIRECTDRAWSURFACE lpDDS_secondary = 0;
#endif

#ifdef __PTC_SYSTEM_MENU__
static int original_window_width;
static int original_window_height;
static HMENU system_menu;
#endif


typedef HRESULT (WINAPI * DIRECTDRAWCREATE) (GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,IUnknown FAR *pUnkOuter);


#ifdef __PTC_WINDOWED__

static void ptc_paint_primary()
{
    RECT source;
    RECT destination;
    POINT point;

    // check
    if (lpDDS)
    {
        // setup source rectangle
        source.left = 0;
        source.top = 0;
        source.right = dx;
        source.bottom = dy;

        // get origin of client area
        point.x = 0;
        point.y = 0;
        ClientToScreen(wnd,&point);

        // get window client area
        GetClientRect(wnd,&destination);

        // offset destination rectangle
        destination.left += point.x;
        destination.top += point.y;
        destination.right += point.x;
        destination.bottom += point.y;

        // blt secondary to primary surface
        IDirectDrawSurface_Blt(lpDDS,&destination,lpDDS_secondary,&source,DDBLT_WAIT,0);
    } 
}

#endif


// menu option identifier
#define SC_ZOOM_MSK 0x400
#define SC_ZOOM_1x1 0x401
#define SC_ZOOM_2x2 0x402
#define SC_ZOOM_4x4 0x404

static LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    // result data
    int result = 0;

    // handle message
    switch (message)
    {
 #ifdef __PTC_WINDOWED__

        case WM_PAINT:
        {
            // paint primary
            ptc_paint_primary();

            // call default window painting
            return DefWindowProc(hWnd,message,wParam,lParam);
        }
        break;

 #else

        case WM_ACTIVATEAPP:
        {                                       
            // update active flag
            active = (BOOL) wParam;
        }
        break;

        case WM_SETCURSOR:
        {
            // hide cursor
            SetCursor(0);
        }
        break;

 #endif

        case WM_CLOSE:
        {
            #ifdef __PTC_ALLOW_CLOSE__

			    // close ptc
			    ptc_close();

            #endif
        }
        break;

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			{
				RECT rct, rctp;
				GetWindowRect(GetParent(hWnd), &rctp);
				GetWindowRect(hWnd, &rct);
				lParam = MAKELONG(LOWORD(lParam) + (rct.left - rctp.left), HIWORD(lParam) + (rct.top - rctp.top));

				if(window_vis_proc)
					result = (int)window_vis_proc(GetParent(hWnd), message, wParam, lParam);
			}
			break;

		case WM_KEYDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			if(window_vis_proc)
				result = (int)window_vis_proc(GetParent(hWnd), message, wParam, lParam);
			break;

        default:
        {
            // unhandled messages
			result = (int)DefWindowProc(hWnd,message,wParam,lParam);
        }
    }

    // finished
    return result;
}



int ptc_draw(void)
{
	return 0;
}

int ptc_open_ex(char *title, int width, int height, HWND hwnd, HDC hdc)
{
    int x;
    int y;
    RECT rect;
    //DEVMODE mode;
    DDPIXELFORMAT format;
    DDSURFACEDESC descriptor;
    DIRECTDRAWCREATE DirectDrawCreate;

    // setup data
    dx = width;
    dy = height;

    // load direct draw library
    library = (HMODULE) LoadLibrary("ddraw.dll");
    if (!library) return 0;

    // get directdraw create function address
    DirectDrawCreate = (DIRECTDRAWCREATE) GetProcAddress(library,"DirectDrawCreate");
    if (!DirectDrawCreate) return 0;

    // create directdraw interface
    if (FAILED(DirectDrawCreate(0,&lpDD,0))) return 0;

    // register window class
    wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = 0;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(0,IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = title;
    RegisterClass(&wc);

    // calculate window size
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,0);
    rect.right -= rect.left;
    rect.bottom -= rect.top;

    // let windows decide
    x = CW_USEDEFAULT;
    y = CW_USEDEFAULT;


    // create fixed window
    wnd = CreateWindow(title,title,WS_CHILD,x,y,rect.right,rect.bottom, hwnd,0,0,0);

    // show window
    ShowWindow(wnd,SW_NORMAL);

    // enter cooperative mode
    if (FAILED(IDirectDraw_SetCooperativeLevel(lpDD,wnd,DDSCL_NORMAL))) return 0;

    // primary with no back buffers
    descriptor.dwSize  = sizeof(descriptor);
    descriptor.dwFlags = DDSD_CAPS;
    descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
    if (FAILED(IDirectDraw_CreateSurface(lpDD,&descriptor,&lpDDS,0))) return 0;

    // create secondary surface
    descriptor.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    descriptor.dwWidth = width;
    descriptor.dwHeight = height;
    if (FAILED(IDirectDraw_CreateSurface(lpDD,&descriptor,&lpDDS_secondary,0))) return 0;
    
    // create clipper
    if (FAILED(IDirectDraw_CreateClipper(lpDD,0,&lpDDC,0))) return 0;

    // set clipper to window
    if (FAILED(IDirectDrawClipper_SetHWnd(lpDDC,0,wnd))) return 0;

    // attach clipper object to primary surface
    if (FAILED(IDirectDrawSurface_SetClipper(lpDDS,lpDDC))) return 0;
    
    // set back to secondary
    lpDDS_back = lpDDS_secondary;

    // get pixel format
    format.dwSize = sizeof(format);
    if (FAILED(IDirectDrawSurface_GetPixelFormat(lpDDS,&format))) return 0;

    // check that format is direct color
    if (!(format.dwFlags & DDPF_RGB)) return 0;
    
    // request converter function
    convert = ptc_request_converter(format.dwRGBBitCount,format.dwRBitMask,format.dwGBitMask,format.dwBBitMask);
    if (!convert) return 0;

#ifdef __PTC_DISABLE_SCREENSAVER__

    // disable screensaver while ptc is open
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, 0, 0);

#endif

    // success
    return 1;
}

HWND ptc_get_wnd(void)
{
	return wnd;
}

int ptc_update(void *buffer)
{
    int y;
    char8 *src;
    char8 *dst;
    int src_pitch;
    int dst_pitch;
    MSG message;
    DDSURFACEDESC descriptor;

    // process messages
    while (PeekMessage(&message,wnd,0,0,PM_REMOVE))
    {
        // translate and dispatch
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    #ifndef __PTC_WINDOWED__
    if (active)
    #endif
    {
        // restore surfaces
        IDirectDrawSurface_Restore(lpDDS);
        #ifdef __PTC_WINDOWED__
        IDirectDrawSurface_Restore(lpDDS_secondary);
        #endif

        // lock back surface
        descriptor.dwSize = sizeof descriptor;
        if (FAILED(IDirectDrawSurface_Lock(lpDDS_back,0,&descriptor,DDLOCK_WAIT,0))) return 0;
    
        // calculate pitches
        src_pitch = dx * 4;
        dst_pitch = descriptor.lPitch;

        // copy pixels to back surface
        src = (char8*) buffer;
        dst = (char8*) descriptor.lpSurface;
        for (y=0; y<dy; y++)
        {
            // convert line
            convert(src,dst,dx);
            src += src_pitch;
            dst += dst_pitch;
        }

        // unlock back surface
        IDirectDrawSurface_Unlock(lpDDS_back,descriptor.lpSurface);

        #ifndef __PTC_WINDOWED__
    
            // flip primary surface
            IDirectDrawSurface_Flip(lpDDS,0,DDFLIP_WAIT);

        #else

            // paint primary
            ptc_paint_primary();

        #endif

        // sleep
        Sleep(0);
    }
    #ifndef __PTC_WINDOWED__
    else
    {
        // sleep
        Sleep(1);
    }
    #endif

    // success
    return 1;
}


void ptc_close()
{
#ifdef __PTC_WINDOWED__

    // check secondary
    if (lpDDS_secondary)
    {
        // release secondary
        IDirectDrawSurface_Release(lpDDS_secondary);
        lpDDS_secondary = 0;
    }

#endif

    // check
    if (lpDDS)
    {
        // release primary
        IDirectDrawSurface_Release(lpDDS);
        lpDDS = 0;
    }

    // check
    if (lpDD)
    {
        // leave display mode
        IDirectDraw_RestoreDisplayMode(lpDD);

        // leave exclusive mode
        IDirectDraw_SetCooperativeLevel(lpDD,wnd,DDSCL_NORMAL);

        // free direct draw
        IDirectDraw_Release(lpDD);
        lpDD = 0;
    }

    // hide window
    DestroyWindow(wnd);

    // check
    if (library)
    {
        // free library
        FreeLibrary(library);
        library = 0;
    }

#ifdef __PTC_DISABLE_SCREENSAVER__

    // enable screensaver now that ptc is closed
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 1, 0, 0);

#endif
}


#endif