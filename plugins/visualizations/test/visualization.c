/*-----------------------------------------------------------------------------
  sample visualization for fennec (wave display).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "../../../include/system.h"
#include "../../../include/fennec.h"



/* defines ------------------------------------------------------------------*/

#define buffer_length   100
#define speaker_sizer   200
#define clip(v, h, l) ((v) > (h) ? (h) : ((v) < (l) ? (l) : (v)))


/* declarations -------------------------------------------------------------*/

int callc visualization_refresh(int idata, void* vdata);
int callc visualization_settings(void* vdata);
int callc visualization_about(void* vdata);
int callc visualization_uninitialize(int idata, void* vdata);

void draw_rect(int x, int y, int w, int h, COLORREF c);




/* data ---------------------------------------------------------------------*/

struct fennec sfennec;
struct general_visualization_data visdata;

UINT_PTR       id_timer_display;
HINSTANCE      dll_inst;
HBITMAP        b_speaker;
HDC            dc_speaker;




/* functions ----------------------------------------------------------------*/

void vis_init(void)
{
	b_speaker = LoadBitmap(dll_inst, (LPCTSTR) "speaker");
	dc_speaker = CreateCompatibleDC(0);

	SelectObject(dc_speaker, b_speaker);
}


void CALLBACK timer_display(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	unsigned int   i, x, y, w, h;
	HDC            hdc;
	int            rv, v;
	double         s_sum;
	float          buffer[buffer_length];
	int            lb = 0, rb = 0;


	rv = visdata.getdata(get_visual_dc, 0, &hdc, 0);
	if(!rv)return;

	x  = visdata.getdata(get_visual_x, 0, 0, 0);
	y  = visdata.getdata(get_visual_y, 0, 0, 0);
	w  = visdata.getdata(get_visual_w, 0, 0, 0);
	h  = visdata.getdata(get_visual_h, 0, 0, 0);

	if(sfennec.audio.output.getfloatbuffer(buffer, buffer_length, 1) == 1)
	{
		static int     lv = 0, mv = 0, mf = 0;

		s_sum = 0;
		for(i=0; i<buffer_length; i++)
			s_sum += (double)buffer[i];

		s_sum /= buffer_length;
		if(s_sum < 0)s_sum = -s_sum;

		v = s_sum * w;

		mv -= mf;
		mv = max(mv, 0);
		if(v < mv)
		{
			v  = mv;
			mf += 20;
		}else{
			mv = v;
			mf = 0;
		}

		{
			int speaker_size = h / 2;
			int spz = (v / 2) + speaker_size;
			int xp, yp;

			if(spz > h) spz = h;

			xp = x + (w / 2 - spz / 2);
			yp = y + (h / 2 - spz / 2);

			draw_rect(hdc, x, y, xp - x, h, 0, 0, 0);
			draw_rect(hdc, xp + spz, y, w - (xp + spz) + x, h, 0, 0, 0);
			draw_rect(hdc, x, y, w, yp - y, 0, 0, 0);
			draw_rect(hdc, x, yp + spz, w, h - (yp + spz) + y, 0, 0, 0);

			
			StretchBlt(hdc, xp, yp, spz, spz, dc_speaker, 0, 0, speaker_sizer, speaker_sizer, SRCCOPY);
		}
			//draw_rect(hdc, 0, 0, v           , h / 2, RGB(0, 200, 0), x, y);
		//draw_rect(hdc, v, 0, (lv - v) + x, h / 2, RGB(0, 0, 0)  , x, y);
		lb = 1;

		lv = v;
	}

	/*
	if(sfennec.audio.output.getfloatbuffer(buffer, buffer_length, 2) == 1)
	{
		static int     lv = 0, mv = 0, mf = 0;

		s_sum = 0;
		for(i=0; i<buffer_length; i++)
			s_sum += (double)buffer[i];

		s_sum /= buffer_length;
		if(s_sum < 0)s_sum = -s_sum;

		v = s_sum * w;

		mv -= mf;
		mv = max(mv, 0);
		if(v < mv)
		{
			v  = mv;
			mf += 2;
		}else{
			mv = v;
			mf = 0;
		}

		//draw_rect(hdc, 0, h / 2 + 1, v           , h / 2, RGB(0, 200, 0), x, y);
		//draw_rect(hdc, v, h / 2 + 1, (lv - v) + x, h / 2, RGB(0, 0, 0)  , x, y);
		rb = 1;

		lv = v;
	}

	*/

	
	

}

/* local - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void draw_rect(HDC dc, int x, int y, int w, int h, COLORREF c, int dx, int dy)
{
	HPEN   pn, po;
	HBRUSH br, bo;

	if(w == 1 && h == 1)
	{
		SetPixelV(dc, x, y, c);

	}else{

		pn = CreatePen(PS_SOLID, 1, c);
		br = CreateSolidBrush(c);
		
		po = (HPEN) SelectObject(dc, pn);
		bo = (HBRUSH) SelectObject(dc, br);

		Rectangle(dc, dx + x, dy + y, dx + x + w, dy + y + h);

		SelectObject(dc, po);
		SelectObject(dc, bo);

		DeleteObject(pn);
		DeleteObject(br);
	}
}


/* plug-in - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


int callc fennec_visualization_initialize(struct general_visualization_data *sdata, string plgtitle)
{
	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));

	memcpy(&visdata, sdata, sizeof(struct general_visualization_data));

	sdata->uninitialize = visualization_uninitialize;
	sdata->refresh      = visualization_refresh;
	sdata->settings     = visualization_settings;
	sdata->about        = visualization_about;

	plgtitle = 0;

	id_timer_display = SetTimer(0, 0, 30, (TIMERPROC)timer_display);

	vis_init();
	return 1;
}


int callc visualization_refresh(int idata, void* vdata)
{

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
	KillTimer(0, id_timer_display);
	return 1;
}

BOOL WINAPI DllMain(HINSTANCE hdll, DWORD dreason, void* resv)
{
	if(dreason == DLL_PROCESS_ATTACH)
		dll_inst = hdll;
	return 1;
}

