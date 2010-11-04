/*-----------------------------------------------------------------------------
  sample visualization for fennec (wave display).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "../../../include/system.h"
#include "../../../include/fennec.h"
#include "goom.h"
#include "tinyptc.h"


/* declarations -------------------------------------------------------------*/

int callc visualization_refresh(int idata, void* vdata);
int callc visualization_settings(void* vdata);
int callc visualization_about(void* vdata);
int callc visualization_uninitialize(int idata, void* vdata);

/* data ---------------------------------------------------------------------*/

struct fennec sfennec;
struct general_visualization_data visdata;

unsigned int   buffer_length = 0;

WNDPROC   window_vis_proc;
HWND        window_vis;
HDC         hdc_dcdd = 0;
UINT_PTR    timer_id;

/* functions ----------------------------------------------------------------*/



void CALLBACK timer_display(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	static float     buffer[512];
	static float     buffer2[512];
	static short     sbuffer[2][512];
	static int       sbcleared = 0;
	int              i, cret = 1;

	if(sfennec.audio.output.getfloatbuffer(buffer, 512, 1) != 1)cret = 0;
	if(sfennec.audio.output.getfloatbuffer(buffer2, 512, 2) != 1)cret = 0;

	if(cret)
	{
		for(i=0; i<512; i++)sbuffer[0][i] = (short)(buffer[i]  * 32787.0f);
		for(i=0; i<512; i++)sbuffer[1][i] = (short)(buffer2[i] * 32787.0f);
		sbcleared = 0;

	}else{
		if(sbcleared)
		{
			for(i=0; i<512; i++)sbuffer[0][i] = 0;
			for(i=0; i<512; i++)sbuffer[1][i] = 0;
			sbcleared = 1;
		}
	}

	jeko_render_pcm(sbuffer);
	{
		HDC dc;

		dc = GetDC(ptc_get_wnd());
		SendMessage(window_vis, WM_USER, 0, (LPARAM)&dc);
		ReleaseDC(ptc_get_wnd(), dc);
	}
	return;
}


int callc fennec_visualization_initialize(struct general_visualization_data *sdata, string plgtitle)
{
	HWND hwnd;
	HDC  hdc;

	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));

	memcpy(&visdata, sdata, sizeof(struct general_visualization_data));

	sdata->uninitialize = visualization_uninitialize;
	sdata->refresh      = visualization_refresh;
	sdata->settings     = visualization_settings;
	sdata->about        = visualization_about;

	visdata.getdata(get_visual, 0, &hwnd, 0);
	window_vis = hwnd;
	visdata.getdata(get_visual_dc, 0, &hdc, 0);

	visdata.getdata(get_visual_winproc, 0, &window_vis_proc, 0);
	goom_main(hwnd, hdc);

	timer_id = SetTimer(0, 0, 30, (TIMERPROC)timer_display);
	return 1;
}

int callc visualization_refresh(int idata, void* vdata)
{
	int x, y, w, h;

	x  = visdata.getdata(get_visual_x, 0, 0, 0);
	y  = visdata.getdata(get_visual_y, 0, 0, 0);
	w  = visdata.getdata(get_visual_w, 0, 0, 0);
	h  = visdata.getdata(get_visual_h, 0, 0, 0);

	SetWindowPos(ptc_get_wnd(), 0, x, y, w, h, SWP_NOZORDER);
	return 1;
}

int callc visualization_settings(void* vdata)
{
	MessageBox((HWND)vdata, "No Settings", "Settings", MB_ICONINFORMATION);
	return 1;
}

int callc visualization_about(void* vdata)
{
	MessageBox((HWND)vdata, "Visualization test for Fennec Player", "About", MB_ICONINFORMATION);
	return 1;
}

int callc visualization_uninitialize(int idata, void* vdata)
{
	DestroyWindow(ptc_get_wnd());
	KillTimer(0, timer_id);
	return 1;
}
