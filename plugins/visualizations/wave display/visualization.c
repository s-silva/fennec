/*-----------------------------------------------------------------------------
  sample visualization for fennec (wave display).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "../../../include/system.h"
#include "../../../include/fennec.h"



/* declarations -------------------------------------------------------------*/

int callc visualization_refresh(int idata, void* vdata);
int callc visualization_settings(void* vdata);
int callc visualization_about(void* vdata);
int callc visualization_uninitialize(int idata, void* vdata);

/* data ---------------------------------------------------------------------*/

struct fennec sfennec;
struct general_visualization_data visdata;

unsigned int   buffer_length = 0;


int        s_bmp_w, s_bmp_h;
uint32_t  *s_bmp_pixels = 0;
HBITMAP    s_bmph = 0;
HDC        s_dc = 0;
HWND       window_vis;
UINT_PTR   timer_uid_ex, timer_uid_normal;
int        vis_mode = 1; /* extended */

#define set_pix(ax, ay, ac)(s_bmp_pixels[(s_bmp_w * (ay) ) + (ax)] = ac)


/* functions ----------------------------------------------------------------*/

void screenmap_init(int w, int h)
{
	int i;

	s_bmp_w = w;
	s_bmp_h = h;

	s_bmp_pixels = sys_mem_realloc(s_bmp_pixels, w * h * sizeof(uint32_t));

	if(s_bmph)
		DeleteObject(s_bmph);

	s_bmph = CreateBitmap(w, h, 1, 32, s_bmp_pixels);

	if(s_dc)
		DeleteDC(s_dc);

	for(i=0; i<w * h; i++)
	{
		s_bmp_pixels[i] = 0;
	}

	s_dc = CreateCompatibleDC(0);
	SelectObject(s_dc, s_bmph);
}

void lineout(int sx, int sy, int ex, int ey, int c)
{
	int i, j;

	c = RGB(GetBValue(c), GetGValue(c), GetRValue(c));

	if(sy < ey)
	{
		for(j=sy; j<=ey; j++)
		{
			set_pix(ex, j, c);
		}
	}else{
		for(j=ey; j<=sy; j++)
		{
			set_pix(ex, j, c);
		}
	}

}

COLORREF color_setbrightness(COLORREF c, float ca)
{
	float cr = GetRValue(c), cg = GetGValue(c), cb = GetBValue(c);
	cr *= ca; cg *= ca; cb *= ca;
	return RGB((unsigned char)min(cr, 255), (unsigned char)min(cg, 255), (unsigned char)min(cb, 255));
}


void CALLBACK timer_display(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	unsigned int     x, y, w, h, i, j;
	HDC              hdc;
	static float     buffer[1024];
	static float     buffer2[1024];
	static COLORREF  c, c2;
	static int       ly;


	if(uMsg == 1234)
	{
		DeleteObject(s_bmph);
		DeleteDC(s_dc);
		KillTimer(0, timer_uid_ex);
		return;
	}

	visdata.getdata(get_visual_dc, 0, &hdc, 0);
	visdata.getdata(get_visual, 0, &window_vis, 0);
	
	x  = 0;
	y  = 0;
	w  = s_bmp_w;
	h  = s_bmp_h;

	if(sfennec.audio.output.getfloatbuffer(buffer, 1024, 1) != 1)return;
	if(sfennec.audio.output.getfloatbuffer(buffer2, 1024, 2) != 1)return;
	
	for(i=0; i<h; i++)
		for(j=0; j<w; j++)
		{
			int iv = (i * s_bmp_w) + j;
			int mc = (s_bmp_pixels[iv]);
			int cr = GetRValue(mc), cg = GetGValue(mc), cb = GetBValue(mc);
			int cm = 1000;
			
			cr *= 900; cg *= 900; cb *= 900;
			cr /= cm; cg /= cm; cb /= cm;

			mc = RGB(cr, cg, cb);

			s_bmp_pixels[iv] = mc;
		}

	c = (COLORREF)visdata.getdata(get_color, color_dark, 0, 0);
	//c = RGB(GetBValue(c), GetGValue(c), GetRValue(c));

	c2 = (COLORREF)visdata.getdata(get_color, color_shifted, 0, 0);
	//c2 = RGB(GetBValue(c), GetGValue(c), GetRValue(c));

	for(i=1; i<w; i++)
	{
		if     (buffer[i] >  0.99f)buffer[i] =  0.99f;
		else if(buffer[i] < -0.99f)buffer[i] = -0.99f;

		if     (buffer2[i] >  0.99f)buffer2[i] =  0.99f;
		else if(buffer2[i] < -0.99f)buffer2[i] = -0.99f;

		j = h/2 + (int)(buffer[i] * (float)h/2.0f);
		lineout(i, h/2, i, j, c);

		j = h/2 + (int)(buffer2[i] * (float)h/2.0f);
		lineout(i, h/2, i, j, c2);
	}

	/*c = (COLORREF)visdata.getdata(get_color, color_normal, 0, 0);
	//c = RGB(GetBValue(c), GetGValue(c), GetRValue(c));

	c2 = (COLORREF)visdata.getdata(get_color, color_normal, 0, 0);
	//c2 = RGB(GetBValue(c), GetGValue(c), GetRValue(c));
	
	for(i=1; i<w; i++)
	{
		if     (buffer[i] >  0.99f)buffer[i] =  0.99f;
		else if(buffer[i] < -0.99f)buffer[i] = -0.99f;

		if     (buffer2[i] >  0.99f)buffer2[i] =  0.99f;
		else if(buffer2[i] < -0.99f)buffer2[i] = -0.99f;

		j = h/2 + (int)(buffer[i] * (float)h/2.0f);
		lineout(i, h/2, i, j, c);

		j = h/2 + (int)(buffer2[i] * (float)h/2.0f);
		lineout(i, h/2, i, j, c2);
	}
	*/

	SetBitmapBits(s_bmph, s_bmp_w * s_bmp_h * sizeof(uint32_t), s_bmp_pixels); 

	x  = visdata.getdata(get_visual_x, 0, 0, 0);
	y  = visdata.getdata(get_visual_y, 0, 0, 0);
	w  = visdata.getdata(get_visual_w, 0, 0, 0);
	h  = visdata.getdata(get_visual_h, 0, 0, 0);


	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchBlt(hdc, x, y, w, h, s_dc, 0, 0, s_bmp_w, s_bmp_h, SRCCOPY);
	SendMessage(window_vis, WM_USER, 0, (LPARAM)&hdc);

	return;
}

void CALLBACK timer_display_normal(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	static HPEN     gpen = 0;
	static HPEN     bpen = 0;
	static float   *buffer = 0;
	static POINT   *pts = 0;
	static COLORREF last_color = 0;
	COLORREF        newcolor;

	unsigned int   i, x, y, w, h;
	HDC            hdc;
	int            rv; 

	
	if(uMsg == 1234)
	{
		if(buffer)sys_mem_free(buffer);
		if(pts)sys_mem_free(pts);
		KillTimer(0, timer_uid_normal);
		return;
	}

	newcolor = (COLORREF)visdata.getdata(get_color, color_dark, 0, 0);
	
	if(newcolor != last_color)
	{
		if(gpen)
		{
			DeleteObject(gpen);
			gpen = 0;
		}
	}

	if(!gpen)gpen = CreatePen(PS_SOLID, 1, newcolor);
	if(!bpen)bpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	rv = visdata.getdata(get_visual_dc, 0, &hdc, 0);
	x  = visdata.getdata(get_visual_x, 0, 0, 0);
	y  = visdata.getdata(get_visual_y, 0, 0, 0);
	w  = visdata.getdata(get_visual_w, 0, 0, 0);
	h  = visdata.getdata(get_visual_h, 0, 0, 0);

	if(w != buffer_length)
	{
		

		buffer = (float*) sys_mem_realloc(buffer, w * sizeof(float));
		pts    = (POINT*) sys_mem_realloc(pts, w * sizeof(POINT));
		
		if(w > buffer_length)
		{
			unsigned int i, k = y + (h / 2);

			for(i=buffer_length; i<w; i++)
			{
				buffer[i] = 0.0f;
				pts[i].x = x;
				pts[i].y = k;
			}
		}
		
		buffer_length = w;
	}


	if(!rv)return;

			
	if(sfennec.audio.output.getfloatbuffer(buffer, buffer_length, 1) != 1)return;

	SelectObject(hdc, bpen);
	Polyline(hdc, pts, min(buffer_length, w));
	SelectObject(hdc, gpen);


	for(i=0; i<buffer_length; i++)
	{
		pts[i].x = i + x;
		pts[i].y = (long)(buffer[i] * (h / 2)) + (y + (h / 2));
	}

	Polyline(hdc, pts, min(buffer_length, w));
	
	SendMessage(window_vis, WM_USER, 0, (LPARAM)&hdc);
}


int callc fennec_visualization_initialize(struct general_visualization_data *sdata, string plgtitle)
{
	int k;

	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));

	memcpy(&visdata, sdata, sizeof(struct general_visualization_data));

	sdata->uninitialize = visualization_uninitialize;
	sdata->refresh      = visualization_refresh;
	sdata->settings     = visualization_settings;
	sdata->about        = visualization_about;

	plgtitle = 0;

	if(visdata.fsettings.plugin_settings_getnum("vis-wavedisplay", "mode", &k, 0, 0))k = 1;
	vis_mode = k;

	if(vis_mode)
	{
		if(visdata.fsettings.plugin_settings_getnum("vis-wavedisplay", "quality", &k, 0, 0))k = 0;
	
		if(k)
			screenmap_init(800, 600);
		else
			screenmap_init(320, 240);

		timer_uid_ex = SetTimer(0, 0, 30, (TIMERPROC)timer_display);
	}else{
		timer_uid_normal = SetTimer(0, 0, 30, (TIMERPROC)timer_display_normal);
	}
	return 1;
}

int callc visualization_refresh(int idata, void* vdata)
{

	return 1;
}

int callc visualization_settings(void* vdata)
{
	if(MessageBox((HWND)vdata, "Use extended visualization? (default - Yes)", "Setting - 1", MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		visdata.fsettings.plugin_settings_setnum("vis-wavedisplay", "mode", 1);
	}else{
		visdata.fsettings.plugin_settings_setnum("vis-wavedisplay", "mode", 0);
	}

	if(MessageBox((HWND)vdata, "Use high quality display? (default - No)", "Setting - 2", MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		visdata.fsettings.plugin_settings_setnum("vis-wavedisplay", "quality", 1);
	}else{
		visdata.fsettings.plugin_settings_setnum("vis-wavedisplay", "quality", 0);
	}
	return 1;
}

int callc visualization_about(void* vdata)
{
	MessageBox((HWND)vdata, "Visualization test for Fennec Player", "About", MB_ICONINFORMATION);
	return 1;
}

int callc visualization_uninitialize(int idata, void* vdata)
{
	if(vis_mode)
	{
		timer_display(0, 1234, 0, 0);
	}else{
		timer_display_normal(0, 1234, 0, 0);
	}
	return 1;
}
