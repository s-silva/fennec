/*-----------------------------------------------------------------------------
  sample visualization for fennec (wave display).
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

void draw_rect(int x, int y, int w, int h, COLORREF c);

#define color_adjust(c, v) (RGB(GetRValue(c) / (v), GetGValue(c) / (v), GetBValue(c) / (v)))



/* data ---------------------------------------------------------------------*/

struct fennec sfennec;
struct general_visualization_data visdata;
HDC    dc = 0;
HWND   window_vis;

int    bars_count = 0, peak_mode_hold = 1;
int    vwx, vwy, vww, vwh;
int    fallco = 1;
int    showfall = 1, showbars = 1;
int    intensecolors = 0;

COLORREF col_bar_fill, col_bar_line;

UINT_PTR  timer_id;


/* functions ----------------------------------------------------------------*/


int callc fennec_visualization_initialize(struct general_visualization_data *sdata, string plgtitle)
{
	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));
	memcpy(&visdata, sdata, sizeof(struct general_visualization_data));

	sdata->uninitialize = visualization_uninitialize;
	sdata->refresh      = visualization_refresh;
	sdata->settings     = visualization_settings;
	sdata->about        = visualization_about;

	sdata->fsettings.plugin_settings_getnum("vis.spectrum", "intense",   &intensecolors, 0, 0);
	sdata->fsettings.plugin_settings_getnum("vis.spectrum", "showfall",  &showfall, 0, 0);
	sdata->fsettings.plugin_settings_getnum("vis.spectrum", "showbars",  &showbars, 0, 0);
	sdata->fsettings.plugin_settings_getnum("vis.spectrum", "fallco",    &fallco, 0, 0);
	sdata->fsettings.plugin_settings_getnum("vis.spectrum", "barscount", &bars_count, 0, 0);

	str_cpy(plgtitle, uni("Spectrum Analyzer"));

	visdata.getdata(get_visual_dc, 0, &dc, 0);
	visdata.getdata(set_msg_proc, 0, vis_message, 0);
	visdata.getdata(get_visual, 0, &window_vis, 0);

	vwx = visdata.getdata(get_visual_x, 0, 0, 0);
	vwy = visdata.getdata(get_visual_y, 0, 0, 0);
	vww = visdata.getdata(get_visual_w, 0, 0, 0);
	vwh = visdata.getdata(get_visual_h, 0, 0, 0);

	draw_rect(vwx, vwy, vww, vwh, 0);

	timer_id = SetTimer(0, 0, 30, timer_display);

	col_bar_line = color_adjust(visdata.getdata(get_color, color_dark, 0, 0), 0.3);
	col_bar_fill = (COLORREF)visdata.getdata(get_color, color_dark, 0, 0);
	return 1;
}

unsigned long callc vis_message(int id, int mdata, int sdata)
{
	if(id == 1 /* keys, lparam */)
	{
		switch(mdata)
		{
		case VK_RIGHT: /* add bars */
			bars_count++;
			draw_rect(vwx, vwy, vww, vwh, 0);
			return 1;

		case VK_LEFT: /* rem bars */
			bars_count--;
			if(bars_count < 1)bars_count = 1;
			draw_rect(vwx, vwy, vww, vwh, 0);
			return 1;

		case VK_UP: /* add bars ++ */
			bars_count += 10;
			draw_rect(vwx, vwy, vww, vwh, 0);
			return 1;

		case VK_DOWN: /* rem bars ++ */
			bars_count -= 10;
			if(bars_count < 1)bars_count = 1;
			draw_rect(vwx, vwy, vww, vwh, 0);
			return 1;

		case VK_NEXT:
			fallco++;
			return 1;

		case VK_PRIOR:
			if(fallco > 1)
				fallco--;
			return 1;

		case VK_SPACE:
			peak_mode_hold ^= 1;
			draw_rect(vwx, vwy, vww, vwh, 0);
			return 1;

		case VK_HOME:
			showfall ^= 1;
			break;

		case VK_END:
			showbars ^= 1;
			break;

		case VK_CONTROL:
			intensecolors ^= 1;
			break;
		}
	}

	return 0;
}

int callc visualization_refresh(int idata, void* vdata)
{
	col_bar_line = color_adjust(visdata.getdata(get_color, color_dark, 0, 0), 1.3);
	col_bar_fill = (COLORREF)visdata.getdata(get_color, color_dark, 0, 0);
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
	visdata.fsettings.plugin_settings_setnum("vis.spectrum", "intense",   intensecolors);
	visdata.fsettings.plugin_settings_setnum("vis.spectrum", "showfall",  showfall);
	visdata.fsettings.plugin_settings_setnum("vis.spectrum", "showbars",  showbars);
	visdata.fsettings.plugin_settings_setnum("vis.spectrum", "fallco",    fallco);
	visdata.fsettings.plugin_settings_setnum("vis.spectrum", "barscount", bars_count);

	visdata.getdata(set_msg_proc, 0, 0, 0);
	KillTimer(0, timer_id);
	return 1;
}

void draw_rect(int x, int y, int w, int h, COLORREF c)
{
	HPEN   pn, po;
	HBRUSH br, bo;

	if(w == 1 && h == 1)
	{
		SetPixelV(dc, x, y, intensecolors ? c : col_bar_line);

	}else{

		if(!intensecolors)
		{
			if(c)
			{
				pn = CreatePen(PS_SOLID, 1, col_bar_line);
				br = CreateSolidBrush(col_bar_fill);
			}else{
				pn = CreatePen(PS_SOLID, 1, 0);
				br = CreateSolidBrush(0);
			}
		}else{
			if(w != 1)
				pn = CreatePen(PS_SOLID, 1, 0);
			else
				pn = CreatePen(PS_SOLID, 1, c );
			br = CreateSolidBrush(c);
		}
		
		po = (HPEN) SelectObject(dc, pn);
		bo = (HBRUSH) SelectObject(dc, br);

		Rectangle(dc, x, y, x + w, y + h);

		SelectObject(dc, po);
		SelectObject(dc, bo);

		DeleteObject(pn);
		DeleteObject(br);
	}
}


void CALLBACK timer_display(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	static float   wbuffer[1024];
	static float   ibuffer[1024];
	static float   rbuffer[1024];
	int            i, j, k, bar_width;
	static int     v[1024];
	static int     fc[1024]; /* falling coefficient */
	static int     fp[1024];
	static int     fpfc[1024];
	int            id, tv, nid;
	unsigned long  freq, mb;
	static int     firsttime = 1;
	double         tav;
	int            rbars_count;
	COLORREF       colorvalue;

	if(sfennec.audio.output.getfloatbuffer(wbuffer, 1024, 1) != 1) return;

	col_bar_line = color_adjust(visdata.getdata(get_color, color_dark, 0, 0), 1.3);
	col_bar_fill = (COLORREF)visdata.getdata(get_color, color_dark, 0, 0);

	sfennec.audio.output.getdata(audio_v_data_frequency, &freq);
	
	fft_float(1024, 0, wbuffer, 0, ibuffer, rbuffer);

	vwx = visdata.getdata(get_visual_x, 0, 0, 0);
	vwy = visdata.getdata(get_visual_y, 0, 0, 0);
	vww = visdata.getdata(get_visual_w, 0, 0, 0);
	vwh = visdata.getdata(get_visual_h, 0, 0, 0);

	if(bars_count <= 0)
		bars_count   = 8;

	bar_width    = bars_count;
	rbars_count  = vww / bar_width;

	if(firsttime)
	{
		memset(v, 0, sizeof(v));
		memset(fp, 0, sizeof(fp));
		firsttime = 0;
	}

	if(freq > (20000 * 2))
		mb = (512 * (20000 * 2)) / freq;
	else
		mb = 512;
	
	for(i=0; i<rbars_count; i++)
	{
		id  = (mb * i) / rbars_count;
		nid = (mb * (i + 1)) / rbars_count;

		k = (nid - id);
		
		for(j=0, tav=0.0f; j<=1/*k*/; j++)
		{
			nid = id + j;
			tav += fabs( sqrt(ibuffer[nid] * ibuffer[nid] + rbuffer[nid] * rbuffer[nid]) ) * ((pow(1.007, nid) - 0.4) / 80.0 * (double)vwh);
		}

		/*tav /= k;*/

		tv = (int)tav;

		
		if(tv > vwh)tv = vwh;


		if(tv > v[id])
		{
			v[id]    = tv;
			fc[id]   = 2;
		}else{
			v[id]  -= fc[id];
			fc[id] += fallco * (double)vwh / 100.0;
		}

		if(tv > fp[id])
		{
			fpfc[id] = -(tv - fp[id]) / 5;
			fp[id]   = tv;
		}else{
			if(fp[id] > 0)
			{
				fpfc[id] += 2;
				if(!peak_mode_hold || fpfc[id] > 0)
					fp[id]   -= fpfc[id];
			}
		}

		fp[id] = max(fp[id], v[id]);
		fp[id] = max(fp[id], 1);
		v[id]  = max(v[id], 0);
		
		if(fp[id] > vwh)fp[id] = vwh;

		colorvalue = RGB(min(((fp[id] *  255) / vwh) + 20, 255), 192, 125);
		
		draw_rect((i * bar_width) + vwx, vwy, max(bar_width, 1), max(vwh - v[id], 0), RGB(0, 0, 0));
		
		if(showbars)
			draw_rect((i * bar_width) + vwx, vwy + vwh, max(bar_width, 1), -min(v[id], vwh), colorvalue);
		else
			draw_rect((i * bar_width) + vwx, vwy + vwh, max(bar_width, 1), -min(v[id], vwh), 0);
		
		//

		if(showfall)
			draw_rect((i * bar_width) + vwx, vwy + max(vwh - fp[id], 0), max(bar_width , 1), 1, colorvalue);


		SendMessage(window_vis, WM_USER /* overlay */, 0, (LPARAM)&dc);

	}
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

