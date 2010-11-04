/*-----------------------------------------------------------------------------
  sample visualization for fennec (wave display).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include <math.h>
#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"
#include "main.h"


#define clip(a, l, h)   ((a) < (l) ? ((a) = (l)) : ((a) > (h)) ? ((a) = (h)) : (a))
#define clip_zero(a, h) ((a) < (0) ? ((a) = (0)) : ((a) > (h)) ? ((a) = (h)) : (a))

#define event_name "fennec.vis.audio analyzer.1.0.thread.1.terminate"

#define help_text \
"Keys:\n\
F1 - help\n\
1 - zoom in\n\
Q - zoom out\n\
2 - increase brightness\n\
W - decrease brightness\n\
3, E - frequency cut on/off\n\
4 - increase bpm (marker)\n\
R - decrease bpm (marker, zero to turn off)\n\
5 - increase delay\n\
T - decrease delay\n\
F2 - minimize to tray"

/* declarations -------------------------------------------------------------*/

int callc visualization_refresh(int idata, void* vdata);
int callc visualization_settings(void* vdata);
int callc visualization_about(void* vdata);
int callc visualization_uninitialize(int idata, void* vdata);
void set_info(const char *text, int iv);
void set_info_simple(const char *text);
unsigned long callc vis_message(int id, int mdata, int sdata);

DWORD WINAPI thread_draw(LPVOID lpParam);

/* data ---------------------------------------------------------------------*/

struct fennec sfennec;
struct general_visualization_data visdata;

HINSTANCE     hInst;
HWND          hdlg = 0, window_vis;
HDC           hdc = 0;
HDC           mdc = 0;
int           zoom_v = 1000;
int           brightness = 100;
int           placement_sel = 0;
int           buflen;
int           mode = 0;
int           steps = 0, color_hue_mode = 0;

char          stitle[1024];
char          sartist[1024];
char          salbum[1024];
int           freq_cut = 1;
int           bpm_mark = 0;
int           bpm_count = 0, misc_tap = 0;
int           bpm;

char          info_text[256];
int           info_active = 1;

int           thread_terminate = 0;

int           delay = 2;
float         block = 0.0f;

int           screen_w = 0, screen_h = 0, rscreen_w, rscreen_h;

int           bpm_back = 0;


HANDLE        event_thread_terminated;
CRITICAL_SECTION cs;
/* functions ----------------------------------------------------------------*/




int callc fennec_visualization_initialize(struct general_visualization_data *sdata, string plgtitle)
{
	DWORD tid;
	
	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));
	memcpy(&visdata, sdata, sizeof(struct general_visualization_data));

	sdata->uninitialize = visualization_uninitialize;
	sdata->refresh      = visualization_refresh;
	sdata->settings     = visualization_settings;
	sdata->about        = visualization_about;

	sdata->getdata(set_msg_proc, 0, vis_message, 0);

	str_cpy(plgtitle, uni("Audio Analyzer"));

	buflen = 1024;
	if(sdata->fsettings.plugin_settings_getnum("audio-analyzer", "length", &buflen, 0, 0))buflen = 1024;
	
	InitializeCriticalSection(&cs);
	CreateThread(0, 0, thread_draw, 0, 0, &tid);

	set_info_simple("Press F1 for help.");
	return 1;
}

int callc visualization_refresh(int idata, void* vdata)
{

}

unsigned long callc vis_message(int id, int mdata, int sdata)
{
	if(id == 1 /* keys, lparam */)
	{
		switch(mdata)
		{
		case '1': /* zoom in */
			zoom_v -= 50;
			clip_zero(zoom_v, 1000);
			set_info("Zoom level: ", 1000 - zoom_v);
			return 1;

		case 'Q': /* zoom out */
			zoom_v += 50;
			clip_zero(zoom_v, 1000);
			set_info("Zoom level: ", 1000 - zoom_v);
			return 1;

		case '2': /* increase brightness */
			brightness += 50;
			clip_zero(brightness, 1000);
			set_info("Brightness: ", 1000 - brightness);
			return 1;

		case 'W': /* decrease brightness */
			brightness -= 10;
			clip_zero(brightness, 1000);
			set_info("Brightness: ", 1000 - brightness);
			return 1;

		case '3': /* frequency cut (on) */
			freq_cut = 1;
			set_info("Frequency cut (20,000+): ", freq_cut);
			return 1;

		case 'E': /* frequency cut (off) */
			freq_cut = 0;
			set_info("Frequency cut (20,000+): ", freq_cut);
			return 1;

		case '4': /* increase bpm */
			bpm_mark += 10;
			clip_zero(bpm_mark, 5000);

			if(bpm_mark)
				bpm = 60000 / bpm_mark ;

			bpm_count = timeGetTime() + bpm;

			set_info("BPM (marker): ", bpm_mark);
			return 1;

		case 'R': /* dncrease bpm */
			bpm_mark -= 10;
			clip_zero(bpm_mark, 5000);
			
			if(bpm_mark)
				bpm = 60000 / bpm_mark;

			bpm_count = timeGetTime() + bpm;


			set_info("BPM (marker): ", bpm_mark);
			return 1;

		case '5':
			delay++;
			delay = clip(delay, 0, 100);

			set_info("Delay: ", delay);
			return 1;

		case 'T':
			delay--;
			delay = clip(delay, 0, 100);

			set_info("Delay: ", delay);
			return 1;

		case '0':
			block += 1000.0;
			block = clip_zero(block, 200000.0f);

			set_info("Block: ", (int)block);
			return 1;

		case 'P':
			block -= 1000.0;
			block = clip_zero(block, 200000.0f);

			set_info("Block: ", (int)block);
			return 1;

		case 'X':

			bpm_back ^= 1;
			return 1;

		case 'S':
			steps += 1;
			steps = clip_zero(steps, 10.0f);

			set_info("Steps: ", (int)steps);
			return 1;

		case 'D':
			steps -= 1;
			steps = clip_zero(steps, 10.0f);

			set_info("Steps: ", (int)steps);
			return 1;

		case 'Y':
		case 'Z':
		case 'M':
			color_hue_mode ++;
			if(color_hue_mode >= 3)color_hue_mode = 0;

			set_info("Color Mode: ", (int)color_hue_mode);
			return 1;

		case VK_TAB:
			mode = !mode;
			set_info("Mode: ", (int)mode);
			return 1;

		case VK_F1:
			MessageBox(0, help_text, "Help", MB_ICONINFORMATION);
			return 1;
		}
	}
	return 0;
}

int callc visualization_settings(void* vdata)
{
	MessageBox((HWND)vdata, "No Settings", "Settings", MB_ICONINFORMATION);
	return 1;
}

int callc visualization_about(void* vdata)
{
	MessageBox((HWND)vdata, "Audio Analyzer visualization for Fennec Player", "About", MB_ICONINFORMATION);
	return 1;
}

int callc visualization_uninitialize(int idata, void* vdata)
{

	visdata.getdata(set_msg_proc, 0, 0, 0);

	EnterCriticalSection(&cs);
	thread_terminate = 1;
	LeaveCriticalSection(&cs);
	sys_sleep(0);
	DeleteCriticalSection(&cs);

	return 1;
}

/* standard windows callbacks -----------------------------------------------*/

int SetPixelX(HDC dc, int x, int y, COLORREF c)
{
	static COLORREF lc = 0;
	int r, g, b;

	r = min((GetRValue(lc) + GetRValue(c)) / 2, 255);
	g = min((GetGValue(lc) + GetGValue(c)) / 2, 255);
	b = min((GetBValue(lc) + GetBValue(c)) / 2, 255);

	c = RGB(r, g, b);

	lc = c;

	switch(color_hue_mode)
	{
	case 0: c = RGB(r, g, b); break;
	case 1: c = RGB(r, b, g); break;
	case 2: c = RGB(g, b, r); break;
	}

	if(misc_tap)
		return SetPixelV(dc, x, y, 0xffFFff);
	else
		return SetPixelV(dc, x, y, c);
}

void set_info(const char *text, int iv)
{
	char buf[20];

	memset(buf, 0, sizeof(buf));

	itoa(iv, buf, 10);

	strcpy(info_text, text);
	strcat(info_text, buf);

	info_active = 200;
}

void set_info_simple(const char *text)
{
	strcpy(info_text, text);
	info_active = 200;
}

DWORD WINAPI thread_draw(LPVOID lpParam) 
{
	unsigned int  i, jj, v1, v2, z = 512, n, k;
	int           fq, ix, iy;
	float         x = 1.0, y;
	static HBRUSH blackbrush;
	static HBRUSH oldbrush;
	static HPEN   gpen;
	static HPEN   bpen;
	static float *buffer;
	static POINT *pts;
	static float *fftr;
	static float *ffti;
	float         v;
	HBITMAP       cbmp = 0;
	HBITMAP       obmp = 0;

	visdata.getdata(get_visual_dc, 0, &hdc, 0);

	gpen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	bpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	blackbrush = CreateSolidBrush(0x0);

	mdc = CreateCompatibleDC(hdc);

	rscreen_w = screen_w = GetDeviceCaps(hdc, HORZRES);
	rscreen_h = screen_h = GetDeviceCaps(hdc, VERTRES);
	cbmp = CreateCompatibleBitmap(hdc, screen_w, screen_h);
	
	obmp     = SelectObject(mdc, cbmp);
	oldbrush = SelectObject(mdc, blackbrush);

	SetTextColor(mdc, 0x00cc00);
	SetBkMode(mdc, TRANSPARENT);
	SetBkColor(mdc, 0x0);

	SelectObject(hdc, gpen);

	buffer = (float*) sys_mem_alloc(buflen * 2 *  sizeof(float));
	fftr   = (float*) sys_mem_alloc(buflen * sizeof(float));
	ffti   = (float*) sys_mem_alloc(buflen * sizeof(float));

	timeBeginPeriod(1);

point_start: 
	EnterCriticalSection(&cs);



	if(thread_terminate)
	{
		if(mdc)
		{
			SelectObject(mdc, obmp);
			SelectObject(mdc, oldbrush);
			DeleteDC(mdc);
		}
		
		if(cbmp)DeleteObject(cbmp);
		if(blackbrush)DeleteObject(blackbrush);
		if(gpen)DeleteObject(gpen);
		if(bpen)DeleteObject(bpen);
		if(buffer)sys_mem_free(buffer);
		if(fftr)sys_mem_free(fftr);
		if(ffti)sys_mem_free(ffti);
		
		thread_terminate = 2;
		LeaveCriticalSection(&cs);
		return 1;
	}
	

	if(sfennec.audio.output.getfloatbuffer(buffer, buflen, 1) != 1)
	{
		sys_sleep(1);
		LeaveCriticalSection(&cs);
		goto point_start;
	}


	visdata.getdata(get_visual, 0, &window_vis, 0);
	screen_w = visdata.getdata(get_visual_w, 0, 0, 0);
	screen_h = visdata.getdata(get_visual_h, 0, 0, 0);
	ix       = visdata.getdata(get_visual_x, 0, 0, 0);
	iy       = visdata.getdata(get_visual_y, 0, 0, 0);

	if(!screen_w || !screen_h)
	{
		sys_sleep(10);
		LeaveCriticalSection(&cs);
		goto point_start;
	}


	z = buflen / 4;
	x = 1.0;
	
	fft_float(buflen, 0, buffer, 0, fftr, ffti);

	y = ((float)zoom_v / 1000);

	if(freq_cut)
	{	
		/* calculate last frequency index */

		sfennec.audio.output.getdata(audio_v_data_frequency, &fq);
		if(fq > 20000)
		{
			z = (unsigned int)((20000.0f / (float)fq) * (buflen / 2));
		}
	}

	for(i = 0;  i < buflen/2;  i++)
	{
		x += 0.4f;
		fftr[i] *= x;
	}

	for(k = 1;  k < screen_h; k++)//(unsigned int) (mode ? screen_w : screen_h);  k ++)
	{
		// color calculation
		if(freq_cut)
			n = (unsigned int)(((k * z * y) / screen_h));
		else
			n = (unsigned int)(((k * (buflen / 2) * y) / screen_h));
		

		v = (float)sqrt((double)fftr[n] * (double)fftr[n] + (double)ffti[n] * (double)ffti[n]);


		v1 = abs((int)(v / (((float)brightness * ((float)buflen / 512.0f) + 1) * 0.005)));

		if(v1 > 200)
		{
			v2 = abs((int)(v / (((float)brightness * ((float)buflen / 512.0f) + 1) * 0.007)));
			v1 = 0;
		}else{
			v2 = 0;
		}

		if(v > (block * 200.0f))
		{
			if(mode)
			{
				if(v1)
				{
					SetPixelX(mdc, k, 0, RGB(min(v2, 255), min(v1, 255), 0));
				}else{
					SetPixelX(mdc, k, 0, RGB(min(v2, 255), min(v2, 255), min(v2, 255)));
				}
			}else{
				if(v1)
					SetPixelX(mdc, screen_w - 2, screen_h - k, RGB(min(v2, 255), min(v1, 255), 0));
				else								 
					SetPixelX(mdc, screen_w - 2, screen_h - k, RGB(min(v2, 255), min(v2, 255), min(v2, 255)));
			}

		}else{
			//SetPixel(mdc, screen_w - 2, screen_h - k, 0);
		}
	}

	if(mode)
	{
		for(jj=1; jj<steps+1; jj++)
		{
			BitBlt(mdc, 0, jj, screen_w, 1, mdc, 0, 0, SRCCOPY);
		}
	}

	if(mode)
	{
		BitBlt(mdc, 0, 0, screen_w, screen_h, mdc, 0, -(1 + steps), SRCCOPY);
	}else{
		BitBlt(mdc, 0, 0, screen_w, screen_h, mdc, 1 + steps, 0, SRCCOPY);
	}
	
	BitBlt(hdc, ix, iy, screen_w, screen_h, mdc, 0, 0, SRCCOPY);

	if(info_active != 0)
	{
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 0));
		TextOut(hdc, ix + 4, iy + 4, info_text, (int)strlen(info_text));
	}
	
	misc_tap = 0;

	/* after blitting */

	if(info_active > 0)info_active--;

	sys_sleep(delay + 1);
	//SendMessage(window_vis, WM_USER, 0, (LPARAM)&hdc);
	LeaveCriticalSection(&cs);
	goto point_start;
}