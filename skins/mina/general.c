/**----------------------------------------------------------------------------

 Fennec Skin - Player 1
 Copyright (C) 2007 Chase <c-h@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/

#include "skin.h"




/* defines ------------------------------------------------------------------*/

#define within(a, b, d)(((a) > (b) - (d)) && ((a) < (b) + (d)))
#define inside(c, l, h)(((c) > (l)) && ((c) < (h)))
#define mindef 10




/* types --------------------------------------------------------------------*/

typedef struct window_point
{
	int   x, y, ex, ey;
	HWND  wnd;

}window_point;





/* code ---------------------------------------------------------------------*/


void local_get_windows(window_point *wpt)
{
	RECT rct;

	wpt[0].wnd = skin.wnd;
	if(!GetWindowRect(wpt[0].wnd, &rct))wpt[0].wnd = 0;
	wpt[0].x = rct.left;
	wpt[0].y = rct.top;
	wpt[0].ex = rct.right;
	wpt[0].ey = rct.bottom;

	wpt[1].wnd = window_eq;
	if(!GetWindowRect(wpt[1].wnd, &rct))wpt[1].wnd = 0;
	wpt[1].x = rct.left;
	wpt[1].y = rct.top;
	wpt[1].ex = rct.right;
	wpt[1].ey = rct.bottom;

	wpt[2].wnd = window_ml;
	if(!GetWindowRect(wpt[2].wnd, &rct))wpt[2].wnd = 0;
	wpt[2].x = rct.left;
	wpt[2].y = rct.top;
	wpt[2].ex = rct.right;
	wpt[2].ey = rct.bottom;

	wpt[3].wnd = window_vis;
	if(!GetWindowRect(wpt[3].wnd, &rct))wpt[3].wnd = 0;
	wpt[3].x = rct.left;
	wpt[3].y = rct.top;
	wpt[3].ex = rct.right;
	wpt[3].ey = rct.bottom;

	wpt[4].wnd = window_vid;
	if(!GetWindowRect(wpt[4].wnd, &rct))wpt[4].wnd = 0;
	wpt[4].x = rct.left;
	wpt[4].y = rct.top;
	wpt[4].ex = rct.right;
	wpt[4].ey = rct.bottom;
}


int move_docking_window(int window_id, int x, int y)
{
	RECT          wrect;
	window_point  awindows[window_id_count];
	int           i, rx = x, ry = y, w, h, dmain = 0;

	if(skin_settings.skin_lock && window_id != 0)
	{
		int cwindocked = 0;

		switch(window_id)
		{
		case window_id_eq:
			if(skin_settings.eq_d)cwindocked = 1;
			break;

		case window_id_lib:
			if(skin_settings.ml_d)cwindocked = 1;
			break;

		case window_id_vis:
			if(skin_settings.vis_d)cwindocked = 1;
			break;

		case window_id_vid:
			if(skin_settings.vid_d)cwindocked = 1;
			break;
		}

		if(cwindocked)
		{
			POINT pt;
			GetCursorPos(&pt);
			skin_move_main_window(last_dx, last_dy);
			return 1;
		}
	}

	local_get_windows(awindows);

	w = awindows[window_id].ex - awindows[window_id].x;
	h = awindows[window_id].ey - awindows[window_id].y;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &wrect, 0);

	for(i=0; i<window_id_count; i++)
	{
		if(i == window_id)continue;
		if(!awindows[i].wnd)continue;

		if(within(x, awindows[i].ex, mindef) && (y + h > awindows[i].y && y < awindows[i].ey))
		{
			rx = awindows[i].ex;
			if(i == window_id_main)dmain = 1;
			if((i == window_id_eq) && skin_settings.eq_d)  dmain = 1;
			if((i == window_id_lib) && skin_settings.ml_d) dmain = 1;
			if((i == window_id_vis) && skin_settings.vis_d)dmain = 1;
		}

		if(within(x + w, awindows[i].x, mindef) && (y + h > awindows[i].y && y < awindows[i].ey))
		{
			rx = awindows[i].x - w;
			if(i == window_id_main)dmain = 1;
			if((i == window_id_eq) && skin_settings.eq_d)  dmain = 1;
			if((i == window_id_lib) && skin_settings.ml_d) dmain = 1;
			if((i == window_id_vis) && skin_settings.vis_d)dmain = 1;
		}

		if(within(y, awindows[i].ey, mindef) && (x + w > awindows[i].x && x < awindows[i].ex))
		{
			ry = awindows[i].ey;
			if(i == window_id_main)dmain = 1;
			if((i == window_id_eq) && skin_settings.eq_d)  dmain = 1;
			if((i == window_id_lib) && skin_settings.ml_d) dmain = 1;
			if((i == window_id_vis) && skin_settings.vis_d)dmain = 1;
		}

		if(within(y + h, awindows[i].y, mindef) && (x + w > awindows[i].x && x < awindows[i].ex))
		{
			ry = awindows[i].y - h;
			if(i == window_id_main)dmain = 1;
			if((i == window_id_eq) && skin_settings.eq_d)  dmain = 1;
			if((i == window_id_lib) && skin_settings.ml_d) dmain = 1;
			if((i == window_id_vis) && skin_settings.vis_d)dmain = 1;
		}


		if(within(x, wrect.left, mindef))
			rx = wrect.left;

		if(within(x + w, wrect.right, mindef))
			rx = wrect.right - w;

		if(within(y, wrect.top, mindef))
			ry = wrect.top;

		if(within(y + h, wrect.bottom, mindef))
			ry = wrect.bottom - h;
		
	}

	if(window_id == window_id_eq && skin_settings.eq_d && within(rx, awindows[window_id_main].x, 10))
		rx = awindows[window_id_main].x;

	if(window_id == window_id_eq && skin_settings.eq_d && within(ry, awindows[window_id_main].y, 10))
		ry = awindows[window_id_main].y;

	
	if(window_id == window_id_lib && skin_settings.ml_d && within(rx, awindows[window_id_main].x, 10))
		rx = awindows[window_id_main].x;

	if(window_id == window_id_lib && skin_settings.ml_d && within(ry, awindows[window_id_main].y, 10))
		ry = awindows[window_id_main].y;
	
	
	if(window_id == window_id_vis && skin_settings.vis_d && within(rx, awindows[window_id_main].x, 10))
		rx = awindows[window_id_main].x;

	if(window_id == window_id_vis && skin_settings.vis_d && within(ry, awindows[window_id_main].y, 10))
		ry = awindows[window_id_main].y;

		
	if(window_id == window_id_vid && skin_settings.vid_d && within(rx, awindows[window_id_main].x, 10))
		rx = awindows[window_id_main].x;

	if(window_id == window_id_vid && skin_settings.vid_d && within(ry, awindows[window_id_main].y, 10))
		ry = awindows[window_id_main].y;


	switch(window_id)
	{
	case window_id_eq:
		skin_settings.eq_x = rx;
		skin_settings.eq_y = ry;
		skin_settings.eq_d = dmain;
		break;

	case window_id_lib:
		skin_settings.ml_x = rx;
		skin_settings.ml_y = ry;
		skin_settings.ml_d = dmain;
		break;

	case window_id_vis:
		skin_settings.vis_x = rx;
		skin_settings.vis_y = ry;
		skin_settings.vis_d = dmain;
		break;

	case window_id_vid:
		skin_settings.vid_x = rx;
		skin_settings.vid_y = ry;
		skin_settings.vid_d = dmain;
		break;
	}


	if(window_id != window_id_main)
		SetWindowPos(awindows[window_id].wnd, 0, rx, ry, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	return 1;
}


/*
 * simplification of the Rectangle function; to minimize GUI bugs.
 */
void drawrect(HDC dc, int x, int y, int w, int h, COLORREF c)
{
	HPEN pn   = CreatePen(PS_SOLID, 1, c), po;
	HBRUSH br = CreateSolidBrush(c), bo;
	
	po = SelectObject(dc, pn);
	bo = SelectObject(dc, br);

	Rectangle(dc, x, y, x + w, y + h);

	SelectObject(dc, po);
	SelectObject(dc, bo);

	DeleteObject(pn);
	DeleteObject(br);
}


/*
 * hsv to rgb conversion.
 */
void color_hsv_2_rgb_fullint(int h, int s, int v, unsigned char* r, unsigned char* g, unsigned char* b)
{
	unsigned char _a, _b, _c, pc;
	unsigned int osc;

	/* optimized edition (only fixed-point maths used). */

	pc  = (unsigned char)((int)h / 60);
	osc = (((h * 100) / 6)) - ((h / 60) * 1000);

	_a = (unsigned char)(((255 - s) * v) / 255);
	_b = (unsigned char)(((255 - ((s * osc) / 1000)) * v) / 255);
	_c = (unsigned char)(((255 - ((s * (1000 - osc)) / 1000)) * v) / 255);

	switch(pc)
	{
	case 0: *r = (unsigned char)  v; *g = (unsigned char)_c; *b = (unsigned char)_a; break;
	case 1: *r = (unsigned char) _b; *g = (unsigned char) v; *b = (unsigned char)_a; break;
	case 2: *r = (unsigned char) _a; *g = (unsigned char) v; *b = (unsigned char)_c; break;
	case 3: *r = (unsigned char) _a; *g = (unsigned char)_b; *b = (unsigned char) v; break;
	case 4: *r = (unsigned char) _c; *g = (unsigned char)_a; *b = (unsigned char) v; break;
	case 5: *r = (unsigned char)  v; *g = (unsigned char)_a; *b = (unsigned char)_b; break;
	}
}


/*
 * adjust hsv/l of the skin-sheet.
 */
int adjust_colors(HBITMAP hbmp, int h, int s, int l)
{
    HDC              dc;
	BITMAPINFO       bmi;
	BITMAP           bmp;
	COLORREF         c;

	int              x, y;
	unsigned long   *bmem;
	unsigned char    ar, ag, ab, rr, rg, rb;

	unsigned short   rh;
	unsigned char    rs, rl;

	unsigned long    i;


	/* get bitmap info/size */

    GetObject(hbmp, sizeof(BITMAP), &bmp);

	/* allocate memory */

    bmem = (unsigned long*)sys_mem_alloc(sizeof(unsigned long) * bmp.bmWidth * bmp.bmHeight);

    memset(&bmi, 0, sizeof(bmi));

    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth       = bmp.bmWidth;
    bmi.bmiHeader.biHeight      = -bmp.bmHeight;

    dc = CreateCompatibleDC(0);
    GetDIBits(dc, hbmp, 0, bmp.bmHeight, bmem, &bmi, DIB_RGB_COLORS);

	/* adjust colors */

	rh = (unsigned short)(h * 3.5);
	rs = (unsigned char) (s * 2.5);

	for(y=0; y<bmp.bmHeight; y++)
	{
		for(x=0; x<bmp.bmWidth; x++)
		{
			i = coords_map(x, y, bmp.bmWidth);
			
			c = bmem[i];

			rr = GetRValue(c);
			rg = GetGValue(c);
			rb = GetBValue(c);

			if(skin_settings.theme_mode == 1)
				rl = (unsigned char) (wrap_color( (((l + 50) * 2) * rr) / 255 ));

			else if(skin_settings.theme_mode == 2)
				rl = (unsigned char) (wrap_color( (((l + 50) * 2) * rg) / 255 ));
			
			else if(skin_settings.theme_mode == 3)
				rl = (unsigned char) (wrap_color( (((l + 50) * 2) * rb) / 255 ));
			
			else if(skin_settings.theme_mode == 4)
				rl = (unsigned char) (wrap_color( (((l + 50) * 2) * ((rr + rg + rb) / 3)) / 255 ));
			
			else
				rl = (unsigned char) (wrap_color( (((l + 50) * 2) * rg) / 255 ));

			color_hsv_2_rgb_fullint(rh, rs, rl, &ab, &ag, &ar);


			ar = (unsigned char)wrap_color( ((int)ar * 15) / 10 /* x1.5 */ );
			ag = (unsigned char)wrap_color( ((int)ag * 15) / 10 /* x1.5 */ );
			ab = (unsigned char)wrap_color( ((int)ab * 15) / 10 /* x1.5 */ );

			/*
			color_blend_1 = rb / 255.0f;
			color_blend_2 = 1.0f - color_blend_1;

			bmem[i] = RGB((int)(((ar * color_blend_1) + (rr * color_blend_2))), (int)(((ag * color_blend_1) + (rg * color_blend_2))), (int)(((ab * color_blend_1) + (rb * color_blend_2))));
			*/

			bmem[i] = RGB(ar, ag, ab);
		}
	}

	SetDIBits(dc, hbmp, 0, bmp.bmHeight, bmem, &bmi, DIB_RGB_COLORS);

	DeleteDC(dc);
	sys_mem_free(bmem);
	return 1;
}



void gr_init(graphic_context *gr)
{
	if(!gr) return;
	gr->dc = 0;
	gr->lpen = 0;
	gr->lbrush = 0;
	gr->ltext_mode = 0;
	gr->lbrush_color = 0;
	gr->lpen_color = 0;
	gr->ltext_color = 0;
	gr->firstbrush = 0;
	gr->firstpen = 0;
	gr->lfont = 0;
}

void gr_delete(graphic_context *gr)
{

} 

 #define color_convertorder_rgb(rgb)((((BYTE) (rgb)) << 16) | (((WORD) (rgb)) << 8) | ((BYTE) ((rgb) >> 16)) )

void gr_setcolor(graphic_context *gr, uint32_t pencolor, uint32_t brushcolor)
{
	if(!gr) return; if(!gr->dc) return;

	if(!gr->lbrush || gr->lbrush_color != brushcolor)
	{
		HBRUSH nbrush = CreateSolidBrush(color_convertorder_rgb(brushcolor));
	
		if(!gr->lbrush)
		{
			gr->firstbrush = SelectObject(gr->dc, nbrush);
		}else{
			SelectObject(gr->dc, nbrush);
			DeleteObject(gr->lbrush);
		}

		gr->lbrush = nbrush;
		gr->lbrush_color = brushcolor;
	}

	if(!gr->lpen || gr->lpen_color != pencolor)
	{
		HPEN npen = CreatePen(PS_SOLID, 1, color_convertorder_rgb(pencolor));
	
		if(!gr->lpen)
		{
			gr->firstbrush = SelectObject(gr->dc, npen);
		}else{
			SelectObject(gr->dc, npen);
			DeleteObject(gr->lpen);
		}

		gr->lpen = npen;
		gr->lpen_color = pencolor;
	}
}

void gr_settextcolor(graphic_context *gr, uint32_t fcolor, uint32_t bkcolor, int bkmode)
{
	if(!gr) return; if(!gr->dc) return;

	SetTextColor(gr->dc, color_convertorder_rgb(fcolor));
}


void gr_rect(graphic_context *gr, uint32_t color, int x, int y, int w, int h)
{
	if(!gr) return; if(!gr->dc) return;

	gr_setcolor(gr, color, color);

	Rectangle(gr->dc, x, y, x + w, y + h);
}

void gr_roundrect(graphic_context *gr, int cornor, uint32_t color, int x, int y, int w, int h)
{
	
}

void gr_circle(graphic_context *gr, uint32_t color, int x, int y, int w, int h)
{
	
}

void gr_line(graphic_context *gr, int size, uint32_t color, int x1, int y1, int x2, int y2)
{
	gr_setcolor(gr, color, gr->lbrush_color);
	MoveToEx(gr->dc, x1, y1, (LPPOINT) NULL);
	LineTo(gr->dc, x2, y2);
}

void gr_text(graphic_context *gr, int mode, const string text, int x, int y, int w, int h)
{
	if(w == 0 && h == 0)
	{
		TextOut(gr->dc, x, y, text, (int)str_len(text));
	}
}

void gr_setfont(graphic_context *gr, const string fontface, int size, int bold, int italic, int underlined, int extramode)
{
	HFONT nfont;

	if(!gr) return; if(!gr->dc) return;

	if(gr->lfont)
	{
		DeleteObject(gr->lfont);
		gr->lfont = 0;
	}

	nfont = CreateFont(-MulDiv(size, GetDeviceCaps(gr->dc, LOGPIXELSY), 72),
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, fontface);

	SelectObject(gr->dc, nfont);
	gr->lfont = nfont;
}

void gr_blit(graphic_context *grdst, int sx, int sy, int dx, int dy, int w, int h)
{

}

void gr_blitto(graphic_context *grsrc, int sx, int sy, int dx, int dy, int w, int h)
{

}

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
