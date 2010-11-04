/*-----------------------------------------------------------------------------
  sample visualization for fennec (cube).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include <math.h>
#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"
#include "main.h"




/* declarations -------------------------------------------------------------*/

int callc visualization_refresh(int idata, void* vdata);
int callc visualization_settings(void* vdata);
int callc visualization_about(void* vdata);
int callc visualization_uninitialize(int idata, void* vdata);

unsigned long callc vis_message(int id, int mdata, int sdata);
void CALLBACK timer_display(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);




/* data ---------------------------------------------------------------------*/

struct    fennec sfennec;
struct    general_visualization_data visdata;
HDC       dc = 0;
HWND      window_vis;
UINT_PTR  timer_id;
WNDPROC   window_vis_proc;

float     wave_buffer[2][1024];
float     fft_buffer[2][1024];
float     bars_buffer[2][1024];
float     bars_fallspeed = 0.08f;

/* functions ----------------------------------------------------------------*/


int callc fennec_visualization_initialize(struct general_visualization_data *sdata, string plgtitle)
{
	int i;

	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));
	memcpy(&visdata, sdata, sizeof(struct general_visualization_data));

	sdata->uninitialize = visualization_uninitialize;
	sdata->refresh      = visualization_refresh;
	sdata->settings     = visualization_settings;
	sdata->about        = visualization_about;

	str_cpy(plgtitle, uni("Spectrum Analyzer"));

	visdata.getdata(get_visual_dc, 0, &dc, 0);
	visdata.getdata(set_msg_proc, 0, vis_message, 0);
	visdata.getdata(get_visual, 0, &window_vis, 0);

	visdata.getdata(get_visual_winproc, 0, &window_vis_proc, 0);

	for(i=0; i<1024; i++)
			bars_buffer[0][i] = 0.0f;

	glvis_init(window_vis);

	timer_id = SetTimer(0, 0, 30, timer_display);
	return 1;
}

unsigned long callc vis_message(int id, int mdata, int sdata)
{
	if(id == 1 /* keys, lparam */)
	{
		switch(mdata)
		{
		case VK_NEXT:
			z += 4.0f;
			return 1;

		case VK_PRIOR:
			z -= 4.0f;
			return 1;

		case VK_SPACE:
			gl_lights ^= 1;
			return 1;

		case VK_TAB:
			if(values[0] == 0.0f)values[0] = 1.0f;
			else values[0] = 0.0f;
			return 1;

		case VK_UP:
			values[1] += 0.1f;
			if(values[1] > 1.0f)values[1] = 1.0f;
			return 1;

		case VK_DOWN:
			values[1] -= 0.1f;
			if(values[1] < 0.0f)values[1] = 0.0f;
			return 1;

		case VK_RIGHT:
			values[2] += 0.1f;
			if(values[2] > 20.0f)values[2] = 20.0f;
			return 1;

		case VK_LEFT:
			values[2] -= 0.1f;
			if(values[2] < 0.0f)values[2] = 0.0f;
			return 1;

		case VK_F3:
			values[3] += 0.1f;
			if(values[3] > 1.0f)values[3] = 1.0f;
			return 1;

		case VK_F4:
			values[3] -= 0.1f;
			if(values[3] < 0.0f)values[3] = 0.0f;
			return 1;

		case VK_F5:
			bars_fallspeed += 0.01f;
			if(bars_fallspeed > 0.2f)bars_fallspeed = 0.2f;
			return 1;

		case VK_F6:
			bars_fallspeed -= 0.01f;
			if(bars_fallspeed < 0.0f)bars_fallspeed = 0.0f;
			return 1;

		case VK_F7:
			values[4] += 0.1f;
			if(values[4] > 1.0f)values[4] = 1.0f;
			return 1;

		case VK_F8:
			values[4] -= 0.1f;
			if(values[4] < 0.0f)values[4] = 0.0f;
			return 1;
		}
	}
	return 0;
}

int callc visualization_refresh(int idata, void* vdata)
{
	int x, y, w, h;

	x  = visdata.getdata(get_visual_x, 0, 0, 0);
	y  = visdata.getdata(get_visual_y, 0, 0, 0);
	w  = visdata.getdata(get_visual_w, 0, 0, 0);
	h  = visdata.getdata(get_visual_h, 0, 0, 0);

	SetWindowPos(window_vis_in, 0, x, y, w, h, SWP_NOZORDER);
	return 1;
}

int callc visualization_settings(void* vdata)
{
	MessageBox((HWND)vdata, uni("No Settings"), uni("Settings"), MB_ICONINFORMATION);
	return 1;
}

int callc visualization_about(void* vdata)
{
	MessageBox((HWND)vdata, uni("Cube visualization for Fennec Player"), uni("About"), MB_ICONINFORMATION);
	return 1;
}

int callc visualization_uninitialize(int idata, void* vdata)
{
	glvis_uninit();
	visdata.getdata(set_msg_proc, 0, 0, 0);
	KillTimer(0, timer_id);
	return 1;
}

void CALLBACK timer_display(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	static float   ibuffer[1024];
	static float   rbuffer[1024];
	register int   i;

	if(sfennec.audio.output.getfloatbuffer(wave_buffer[0], 1024, 1))
	{
		fft_float(1024, 0, wave_buffer[0], 0, ibuffer, rbuffer);
	
		for(i=0; i<1024; i++)
			fft_buffer[0][i] = sqrt(ibuffer[i] * ibuffer[i] + rbuffer[i] * rbuffer[i]) * ((pow(1.007, i) - 0.8) / 80.0 * (double)1.0);;
	
		for(i=0; i<1024; i++)
		{
			if(fft_buffer[0][i] > bars_buffer[0][i])
				                                      bars_buffer[0][i] = fft_buffer[0][i];
			else                                      bars_buffer[0][i] -= bars_fallspeed;
			if(bars_buffer[0][i] < 0.0f)bars_buffer[0][i] = 0.0f;
		}

	}


	glvis_display();
}

int visualization_get_rel_path(string buf, const string relpath, int csize)
{
	visdata.shared->general.getpluginspath(buf, csize);

	str_cat(buf, relpath);
	return 1;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

