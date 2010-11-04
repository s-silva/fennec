/*-----------------------------------------------------------------------------
  sample visualization for fennec (cube).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include <math.h>
#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"
#include "main.h"



/* prototypes ---------------------------------------------------------------*/

int callc videoout_initialize(void);
int callc videoout_uninitialize(int idata, void* vdata);
int callc videoout_display(void *data, int mode, void *modedata);
int callc videoout_refresh(int rlevel);
int callc videoout_settings(void* vdata);
int callc videoout_about(void* vdata);
int callc videoout_pushtext(int textmode, int addmode, void *data, int x, int y, int w, int h);
int callc videoout_setinfo_ex(int id, void *data, int (*afunc)());
int callc videoout_setinfo(int frame_w, int frame_h, int frame_rate, int colors);
int callc videoout_setstate(int state, int *extradata);






/* data ---------------------------------------------------------------------*/

struct    fennec                  sfennec;
struct    general_videoout_data   vdata;
int       frame_w = 0, frame_h = 0;
void     *current_frame;
double    aspect_ratio = 0;
string    subtitle_text = 0;
string    subtitle_text_sec = 0;
HINSTANCE hinstance;

/* functions ----------------------------------------------------------------*/


int callc fennec_videoout_initialize(struct general_videoout_data *sdata, string plgtitle)
{
	HWND  hwndp;

	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));
	memcpy(&vdata, sdata, sizeof(struct general_videoout_data));

	sdata->initialize    = videoout_initialize;
	sdata->refresh       = videoout_refresh;
	sdata->pushtext      = videoout_pushtext;
	sdata->setinfo_ex    = videoout_setinfo_ex;
	sdata->setinfo       = videoout_setinfo;
	sdata->setstate      = videoout_setstate;
	sdata->display       = videoout_display;
	sdata->uninitialize  = videoout_uninitialize;


	vdata.getdata(get_window_video, 0, &hwndp, 0);
	if(IsWindow(hwndp))
	{
		video_init(hwndp);
		videoout_refresh(0);
	}
	
	return 1;
}

int callc videoout_initialize(void)
{
	HWND hwndp;

	vdata.getdata(get_window_video, 0, &hwndp, 0);
	if(IsWindow(hwndp))
	{
		video_init(hwndp);
		videoout_refresh(0);
	}
	return 1;
}

int callc videoout_uninitialize(int idata, void* vdata)
{

	video_uninit();

	return 1;
}

int callc videoout_display(void *data, int mode, void *modedata)
{
	current_frame = data;
	video_display();
	return 1;
}


int callc videoout_refresh(int rlevel)
{
	HDC    hdc;
	RECT   rct;
	int    rw, rh, rx = 0, ry = 0; /* relative sizes */
	int    ww, wh;                 /* video window sizes */
	float  sc;                     /* scale */

	vdata.getdata(get_window_video_rect, 0, &rct, 0);
	vdata.getdata(get_window_video_dc, 0, &hdc, 0);

	if(frame_w <= 0)frame_w = 256;
	if(frame_h <= 0)frame_h = 256;

	ww = rw = rct.right - rct.left;
	wh = rh = rct.bottom - rct.top;
#ifdef dfkdsfh
	if(w > h)
	{
		sc = (float)rw / (float)w;
	}else{
		sc = (float)rh / (float)h;
	}

	/* to keep aspect ratio, both width and height should be scaled equally */
	
	rw = (int)((float)w * sc);
	rh = (int)((float)h * sc);

	/* we gotta handle video's corners vertically and check if it's gonna surpass the parent window's borders */

	if(rh > wh)
		sc = (float)wh / (float)h;

	if(rw > ww)
		sc = (float)ww / (float)w;

	/* any changes? calculate'em again */

	rh = (int)((float)h * sc);
	rw = (int)((float)w * sc);

	/* calculate video position */

	rx = (ww - rw) / 2;
	ry = (wh - rh) / 2;
#endif
	SetWindowPos(video_window, 0, rct.left + rx, rct.top + ry, rw, rh, SWP_NOZORDER);
	
	/* draw border */

	BitBlt(hdc, rct.left, rct.top, ww, ry, 0, 0, 0, BLACKNESS);
	BitBlt(hdc, rct.left, rct.top + ry, ww, wh - ry, 0, 0, 0, BLACKNESS);
	
	BitBlt(hdc, rct.left, rct.top, rx, wh, 0, 0, 0, BLACKNESS);
	BitBlt(hdc, rct.left + rw, rct.top, ww - rw, wh, 0, 0, 0, BLACKNESS);

	/*screen_w = rct.right - rct.left;
	screen_h = rct.bottom - rct.top;*/
	return 1;
}

int callc videoout_pushtext(int textmode, int addmode, void *data, int x, int y, int w, int h)
{
	if(textmode == 0)
		subtitle_text = (string)data;
	else
		subtitle_text_sec = (string)data;
	return 1;
}

int callc videoout_setinfo_ex(int id, void *data, int (*afunc)())
{
	if(id == 1) /* aspect ratio */
	{
		aspect_ratio = *((double*)data);
	}
	return 1;
}

int callc videoout_setinfo(int f_w, int f_h, int frame_rate, int colors)
{
	int do_refresh = 0;

	if((frame_w != f_w) || (frame_h != f_h)) do_refresh = 1;

	frame_w = f_w;
	frame_h = f_h;

	if(do_refresh)
		videoout_refresh(0);
	return 1;
}

int callc videoout_setstate(int state, int *extradata)
{
	return 1;
}



int callc videoout_settings(void* vdata)
{
	MessageBox((HWND)vdata, uni("No Settings"), uni("Settings"), MB_ICONINFORMATION);
	return 1;
}

int callc videoout_about(void* vdata)
{
	MessageBox((HWND)vdata, uni("DirectX video output plug-in"), uni("About"), MB_ICONINFORMATION);
	return 1;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, unsigned long fdwReason, LPVOID lpReserved)
{
    switch(fdwReason) 
    { 
        case DLL_PROCESS_ATTACH:
			hinstance = hinstDLL;
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

