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
#include "ids.h"
#include "data/system/windows/resource.h"




/* prototypes ---------------------------------------------------------------*/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK    display_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void CALLBACK    seek_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

int __stdcall fennec_skin_initialize(struct skin_data *indata, struct skin_data_return *outdata);
int  adjust_colors(HBITMAP hbmp, int h, int s, int l);
int  skin_uninitialize(int inum, void *idata);
int  skin_refresh(int inum, void *idata);
void set_theme_colors(void);
int  skin_set_color(int hue, unsigned long combsl);
int  skin_get_themes(int id, string name, int osize);
int  skin_set_theme(int id);
int  skin_getdata(int id, void *rdata, int dsize);





/* defines ------------------------------------------------------------------*/

#define skin_main_button_play       0x1
#define skin_main_button_stop       0x2
#define skin_main_button_previous   0x3
#define skin_main_button_next       0x4
#define skin_main_button_open       0x5
#define skin_main_button_playlist   0x6
#define skin_main_button_equalizer  0x7
#define skin_main_button_minimize   0x8
#define skin_main_button_exit       0x9
#define skin_main_button_seek       0xa
#define skin_main_button_volume     0xb
#define skin_main_button_settings   0xc
#define skin_main_button_convert    0xd
#define skin_main_button_rip        0xe
#define skin_main_button_join       0xf
#define skin_main_button_visual     0x10
#define skin_main_button_video      0x11
#define skin_main_endl              0x11
#define skin_main_endr              0x12
#define skin_main_endt              0x13
#define skin_main_endb              0x14
#define skin_main_button_dsp        0x15
#define skin_main_button_lock       0x16


#define state_normal 0
#define state_hover  1
#define state_down   2





/* data ---------------------------------------------------------------------*/

struct skin_data        skin;
HDC                     hdc;
HWND                    wnd, hdlg;
HINSTANCE               instance_skin;
int                     seekin = 0;
int                     last_dx = 0, last_dy = 0;

HDC                     mdc_sheet = 0;
HBITMAP                 bmp_sheet = 0;
HBITMAP                 oldbmp_sheet;

HDC     				ldcmem = 0;
HBITMAP 				lbmp;

HFONT                   displayfont = 0;
HRGN                    cliprgn = 0;
HRGN                    wndmainrgn = 0;

letter                  artist[512];
letter                  title[512];
letter                  album[512];

int                     seekmode = 0;

local_skin_settings     skin_settings;
FILE                   *file_skin_settings;
 
COLORREF                display_normal  = RGB(204, 204, 255);
COLORREF                display_paused  = RGB(224, 194, 245);
COLORREF                display_stopped = RGB(154, 154, 205);

fn_vis_message          vis_message = 0;

struct fennec_audiotag  ctag; /* current tag */

unsigned int            timer_id_seek = 0, timer_id_display = 0;

WNDPROC                 wndproc_vis;

letter                  skin_sheet_bmp[v_sys_maxpath];
letter                  skin_sheet_table[v_sys_maxpath];
		extern HDC hdc_vid;


/* code ---------------------------------------------------------------------*/

/*
 * initialization callback.
 */
int __stdcall fennec_skin_initialize(struct skin_data *indata, struct skin_data_return *outdata)
{
	memcpy(&skin, indata, sizeof(struct skin_data));

	outdata->setcolor     = skin_set_color;
	outdata->getthemes    = skin_get_themes;
	outdata->settheme     = skin_set_theme;
	outdata->getdata      = skin_getdata;
	outdata->getinfo      = 0;
	outdata->refresh      = skin_refresh;
	outdata->uninitialize = skin_uninitialize;
	outdata->callback     = WindowProc;
	outdata->woptions     = WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MINIMIZEBOX;
	outdata->subs_get     = skin_subskins_get;
	outdata->subs_select  = skin_subskins_select;

	str_cpy(skin_sheet_bmp,   uni(""));
	str_cpy(skin_sheet_table, uni(""));


	wnd = indata->wnd;
	hdc = GetDC(wnd);

	fill_skin_coords();

	/* load settings */

	{
		int     i, er = 0;
		letter  tmp_buf[1024];

		GetModuleFileName(instance_skin, tmp_buf, sizeof(tmp_buf));

		i = (int)str_len(tmp_buf);

		while(i)
		{
			if(tmp_buf[i] == '/' || tmp_buf[i] == '\\')
			{
				tmp_buf[i + 1] = 0;
				break;
			}
			i--;
		}

		str_cat(tmp_buf, uni("settings - skin player.fsd"));

		file_skin_settings = _wfopen(tmp_buf, uni("rb+"));

		if(file_skin_settings)
		{
			if(fread(&skin_settings, sizeof(local_skin_settings), 1, file_skin_settings) != 1)
				er = 1;
		}else{
			file_skin_settings = _wfopen(tmp_buf, uni("wb+"));

			if(!file_skin_settings) /* try read only */
			{
				file_skin_settings = _wfopen(tmp_buf, uni("rb"));
				
				if(file_skin_settings)
				{
					if(!fread(&skin_settings, sizeof(local_skin_settings), 1, file_skin_settings))
						er = 1;
					fclose(file_skin_settings);
					file_skin_settings = 0;
				}
			}else{
				er = 1;
			}
		}

		if(er)
		{
			int px = 0, py = 200;

			coords.zoom              = 1.0;
			skin_settings.zoom       = 1.0;

			skin_settings.main_x     = px;
			skin_settings.main_y     = py;
			
			skin_settings.eq_x       = px;
			skin_settings.eq_y       = py + cr(coords.window_main.height);
			skin_settings.eq_d       = 1;
			skin_settings.eq_show    = 1;

			skin_settings.ml_show    = 1;
			skin_settings.ml_d       = 1;
			skin_settings.ml_x       = px;
			skin_settings.ml_y       = py - 200;
			skin_settings.ml_w       = cr(coords.window_main.width);
			skin_settings.ml_h       = 200;

			skin_settings.vis_show   = 0;
			skin_settings.vis_d      = 1;
			skin_settings.vis_x      = px;
			skin_settings.vis_y      = py + coords.window_eq.height + cr(coords.window_main.height);
			skin_settings.vis_w      = cr(coords.window_main.width);
			skin_settings.vis_h      = 200;

			skin_settings.vid_show   = 1;
			skin_settings.vid_d      = 1;
			skin_settings.vid_x      = px + cr(coords.window_main.width);
			skin_settings.vid_y      = py - 200;
			skin_settings.vid_w      = cr(coords.window_main.width);
			skin_settings.vid_h      = 200;

			skin_settings.use_color  = 1;
			skin_settings.hue        = 41;
			skin_settings.sat        = 23;
			skin_settings.light      = 68;

			skin_settings.sel_theme  = 0;
			skin_settings.theme_mode = 0;
			skin_settings.ml_font_size = 8;

			skin_settings.ml_sorted_column = -1;
			skin_settings.ml_sorted_mode = 0;

			skin_settings.ml_current_dir    = 0;
			skin_settings.ml_in_dir         = 0;
			skin_settings.ml_dir_sort_mode  = 0;

			skin_settings.mode_ml           = 0;
			skin_settings.skin_lock         = 1;

			str_cpy(skin_settings.font_display, uni("Tahoma"));
			str_cpy(skin_settings.skin_file_name, uni("<default>"));


			
			
			for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(int); i++)
					skin_settings.ml_pl_xoff[i] = -1;

			skin_settings.ml_pl_xoff[header_tag_artist] = 5;
			skin_settings.ml_pl_xoff[header_tag_title]  = 100;
		}

	}

	coords.zoom = skin_settings.zoom;

	skin_file_load(skin_settings.skin_file_name);
	
	ShowWindow(wnd, SW_SHOW);

	ml_create(skin.wnd);
	eq_create(skin.wnd);
	vis_create(skin.wnd);
	vid_create(skin.wnd);
	return 1;
}

void skin_recreate(void)
{
	if(wndmainrgn) DeleteObject(wndmainrgn);
	if(cliprgn) DeleteObject(cliprgn);
	if(bmp_sheet)
	{
		SelectObject(mdc_sheet, oldbmp_sheet);
		DeleteObject(bmp_sheet);
	}
	if(mdc_sheet) DeleteDC(mdc_sheet);
	
	if(timer_id_seek)   KillTimer(0, timer_id_seek);
	if(timer_id_display)KillTimer(0, timer_id_display);

	if(displayfont) DeleteObject(displayfont);


	SetWindowRgn(wnd, 0, 0);
	wndmainrgn = CreateRoundRectRgn(0, 0, cr(coords.window_main.width), cr(coords.window_main.height), cr(coords.window_main.window_edge), cr(coords.window_main.window_edge));
	SetWindowRgn(wnd, wndmainrgn, 1);

	setwinpos_clip(wnd, 0, skin_settings.main_x, skin_settings.main_y,
		cr(coords.window_main.width), cr(coords.window_main.height), SWP_NOZORDER);

	sys_pass();


	set_theme_colors();

	mdc_sheet = CreateCompatibleDC(hdc);
	bmp_sheet = load_skin_sheet_bitmap();

	if(skin_settings.use_color)adjust_colors(bmp_sheet, skin_settings.hue, skin_settings.sat, skin_settings.light);

	oldbmp_sheet = SelectObject(mdc_sheet, bmp_sheet);

	timer_id_seek    = (unsigned int) SetTimer(0, 0, 35,  (TIMERPROC) display_timer);
	timer_id_display = (unsigned int) SetTimer(0, 0, 100, (TIMERPROC) seek_timer);

	displayfont = CreateFont(-MulDiv(cr(8), GetDeviceCaps(hdc, LOGPIXELSY), 72),
                                0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, skin_settings.font_display);

	cliprgn = CreateRectRgn(cr(coords.window_main.display_region.x), cr(coords.window_main.display_region.y), cr(coords.window_main.display_region.w), cr(coords.window_main.display_region.h));


	/*setwinpos_clip(wnd, 0, skin_settings.main_x, skin_settings.main_y,
		cr(coords.window_main.width), cr(coords.window_main.height), SWP_NOZORDER);*/

	eq_skinchange();

	display_timer((HWND)-1, WM_USER + 221, 0, 0);
}


/*
 * uninitialize callback.
 */
int skin_uninitialize(int inum, void *idata)
{
	ml_close();
	eq_close();
	vis_close();

	ReleaseDC(wnd, hdc);

	KillTimer(0, timer_id_seek);
	KillTimer(0, timer_id_display);

	if(file_skin_settings)
	{
		fseek(file_skin_settings, 0, SEEK_SET);
		fwrite(&skin_settings, sizeof(local_skin_settings), 1, file_skin_settings);
		fclose(file_skin_settings);
	}
	return 1;
}


/*
 * refresh callback.
 */
int skin_refresh(int inum, void *idata)
{
	letter        fname[v_sys_maxpath];
	string        fpath;
	unsigned long id;

	artist[0] = 0;
	title[0]  = 0;
	album[0]  = 0;

	if(!skin.shared->audio.output.playlist.getcount())return 0;

	fpath = skin.shared->audio.output.playlist.getsource(skin.shared->audio.output.playlist.getcurrentindex());
	
	if(!fpath)return 0;

	id = skin.shared->audio.input.tagread(fpath, &ctag);

	if(ctag.tag_artist.tsize)
		str_ncpy(artist, ctag.tag_artist.tdata, sizeof(artist) / sizeof(letter));

	if(ctag.tag_album.tsize)
		str_ncpy(album, ctag.tag_album.tdata, sizeof(album) / sizeof(letter));

	if(ctag.tag_title.tsize)
		str_ncpy(title, ctag.tag_title.tdata, sizeof(title) / sizeof(letter));

	if(!ctag.tag_title.tsize)
	{
		_wsplitpath(fpath, 0, 0, fname, 0);
		str_ncpy(title, fname, sizeof(title) / sizeof(letter));
	}
	

	skin.shared->audio.input.tagread_known(id, 0, &ctag);

	display_timer(0, WM_USER + 222, 0, 0);

	ml_refresh(inum);
	eq_refresh(inum);

	vis_lyrics_refresh(inum);
	return 1;
}

HBITMAP load_skin_sheet_bitmap(void)
{
	letter  fpath[v_sys_maxpath];

	if(skin_sheet_bmp[0])
	{
		skin.shared->general.getskinspath(fpath, sizeof(fpath));
		str_cat(fpath, uni("/skin player data/"));
		str_cat(fpath, skin_sheet_bmp);
		return LoadImage(0, fpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	}else{ /* default */
		return LoadBitmap(instance_skin, MAKEINTRESOURCE(data_sheet));
	}
}

void show_tip(int dtid, string tiptext)
{
	POINT pt;
	GetCursorPos(&pt);
	skin.showtip(pt.x, pt.y, dtid, tiptext);
}

void show_tipex(int dtid, int dtaid, int tid, string conj)
{
	letter tp[512];
	POINT  pt;

	GetCursorPos(&pt);

	str_cpy(tp + tid, skin.shared->language_text[dtid]);
	str_cat(tp + tid, conj ? conj : uni("/"));
	str_cat(tp + tid, skin.shared->language_text[dtaid]);

	skin.showtip(pt.x, pt.y, 0, tp + tid);
}


/*
 * apply theme colors.
 */
void set_theme_colors(void)
{
	if(!skin_settings.use_color)
	{
		display_normal  = RGB(204, 204, 255);
		display_paused  = RGB(224, 194, 245);
		display_stopped = RGB(154, 154, 205);

	}else{
		unsigned char r, g, b;
		int           rv, gv, bv;
		int           lv;

		if(skin_settings.theme_mode == 1) /* light */
		{
			lv = skin_settings.light;
			color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(lv * 2.5), &r, &g, &b);
			rv = r; gv = g; bv = b;
			display_normal   = RGBx(rv + 140, gv + 140, bv + 140);

			lv = skin_settings.light;
			color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(lv * 2.5), &r, &g, &b);
			rv = r; gv = g; bv = b;
			display_paused   = RGBx(rv + 120, gv + 120, bv + 120);

			lv = skin_settings.light;
			color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5) + 40, (int)(skin_settings.sat * 2.5), (int)(lv * 2.5), &r, &g, &b);
			rv = r; gv = g; bv = b;
			display_stopped   = RGBx(rv + 100, gv + 100, bv + 100);


		}else{ /* middle/dark */


			lv = skin_settings.light;
			color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(lv * 2.5), &r, &g, &b);
			rv = r; gv = g; bv = b;
			display_normal   = RGBx(rv + 100, gv + 100, bv + 100);

			lv = skin_settings.light;
			color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(lv * 2.5), &r, &g, &b);
			rv = r; gv = g; bv = b;
			display_paused   = RGBx(rv + 40, gv + 40, bv + 40);

			lv = skin_settings.light;
			color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5) + 40, (int)(skin_settings.sat * 2.5), (int)(lv * 2.5), &r, &g, &b);
			rv = r; gv = g; bv = b;
			display_stopped   = RGBx(rv + 40, gv + 40, bv + 40);
		}
	}
}


/*
 * set colors (hue, combsel - saturation | lightness)
 */
int skin_set_color(int hue, unsigned long combsl)
{
	if(hue == -1) /* get */
	{
		switch(combsl)
		{
		case 0: return skin_settings.hue;
		case 1: return skin_settings.sat;
		case 2: return skin_settings.light;
		}

	}else{
		SelectObject(mdc_sheet, oldbmp_sheet);
		DeleteObject(bmp_sheet);

		bmp_sheet = load_skin_sheet_bitmap();

		oldbmp_sheet = SelectObject(mdc_sheet, bmp_sheet);

		if(hue == -2) /* reset */
		{
			skin_settings.use_color = 0;
		}else{
			skin_settings.use_color = 1;

			skin_settings.hue   = hue;
			skin_settings.sat   = (int)LOWORD(combsl);
			skin_settings.light = (int)HIWORD(combsl);

			adjust_colors(bmp_sheet, hue, skin_settings.sat, skin_settings.light);
		}

		set_theme_colors();

		display_timer(0, WM_USER + 221, 0, 0);
		ml_refresh(v_fennec_refresh_force_full);
		eq_refresh(v_fennec_refresh_force_full);
		vis_refresh();
	}
	return 1;
}


/*
 * get themes list, finally return 0.
 */
int skin_get_themes(int id, string name, int osize)
{
	/* max: maxpath or 260 letters */
	switch(id)
	{
	case 0:  str_cpy(name, uni("Default")); break;
	case 1:  str_cpy(name, uni("Theme Mode: Bright")); break;
	case 2:  str_cpy(name, uni("Theme Mode: Middle")); break;
	case 3:  str_cpy(name, uni("Theme Mode: Dark")); break;
	case 4:  str_cpy(name, uni("Theme Mode: Average")); break;
	case 5:  str_cpy(name, uni("")); break;
	case 6:  str_cpy(name, uni("Mission")); break;
	case 7:  str_cpy(name, uni("Shiny Ice")); break;
	case 8:  str_cpy(name, uni("Platinum")); break;
	case 9:  str_cpy(name, uni("Valentine")); break;
	case 10: str_cpy(name, uni("Copper")); break;
	case 11: str_cpy(name, uni("Metal")); break;
	case 12: str_cpy(name, uni("")); break;
	case 13: str_cpy(name, uni("Electronic")); break;
	case 14: str_cpy(name, uni("Vine")); break;
	case 15: str_cpy(name, uni("Classic Wood")); break;
	case 16: str_cpy(name, uni("Artificial")); break;
	case 17: str_cpy(name, uni("Soft & Bright")); break;
	case 18: str_cpy(name, uni("Mono")); break;
	default:
		return 0;
	}
	return 1;
}


/*
 * set/apply current theme id.
 */
int skin_set_theme(int id)
{
	switch(id)
	{
	case 0: /* default */
		skin_set_color(-2, 0); /* reset */
		break;
	case 1: /* blue */
		skin_settings.theme_mode = 1;
		skin_set_color(skin_settings.hue, MAKELONG(skin_settings.sat, skin_settings.light));
		break;

	case 2: /* green */
		skin_settings.theme_mode = 2;
		skin_set_color(skin_settings.hue, MAKELONG(skin_settings.sat, skin_settings.light));
		break;

	case 3: /* red */
		skin_settings.theme_mode = 3;
		skin_set_color(skin_settings.hue, MAKELONG(skin_settings.sat, skin_settings.light));
		break;

	case 4: /* average */
		skin_settings.theme_mode = 4;
		skin_set_color(skin_settings.hue, MAKELONG(skin_settings.sat, skin_settings.light));
		break;
		
	case 5: /* none */
		break;

	case 6: 
		skin_set_color(19, MAKELONG(41, 30)); break;
	case 7:
		skin_set_color(65, MAKELONG(16, 47)); break;
	case 8:
		skin_set_color(0, MAKELONG(0, 26)); break;
	case 9: 
		skin_set_color(0, MAKELONG(83, 39)); break;
	case 10: 
		skin_set_color(12, MAKELONG(97, 34)); break;
	case 11:
		skin_set_color(12, MAKELONG(13, 41)); break;


	case 12:
		/* none */
		break;

	case 13:
		skin_set_color(41, MAKELONG(23, 66)); break;
	case 14:
		skin_set_color(81, MAKELONG(14, 66)); break;
	case 15:
		skin_set_color(8, MAKELONG(50, 76)); break;
	case 16:
		skin_set_color(15, MAKELONG(18, 96)); break;
	case 17:
		skin_set_color(61, MAKELONG(13, 99)); break;
	case 18:
		skin_set_color(0, MAKELONG(0, 62)); break;

	default:
		return 0;
	}
	return 1;
}


/*
 * get data callback.
 */
int skin_getdata(int id, void *rdata, int dsize)
{
	RECT  rct;

	switch(id)
	{
	case get_visual:
		*((HWND*)rdata) = (vis_init ? window_vis : 0);
		return vis_init;

	case get_visual_dc:
		*((HDC*)rdata) = (vis_init ? hdc_vis : 0);
		return vis_init;

	case get_visual_x:
		if(vis_init)
		{
			vis_get_position(&rct);
			return rct.left;
		}else{
			return 0;
		}

	case get_visual_y:
		if(vis_init)
		{
			vis_get_position(&rct);
			return rct.top;
		}else{
			return 0;
		}

	case get_visual_w:
		if(vis_init)
		{
			vis_get_position(&rct);
			return rct.right;
		}else{
			return 0;
		}

	case get_visual_h:
		if(vis_init)
		{
			vis_get_position(&rct);
			return rct.bottom;
		}else{
			return 0;
		}

	case get_visual_winproc:
		if(vis_init)
		{
			wndproc_vis = callback_vis_window;
			*((WNDPROC*)rdata) = wndproc_vis;
		}
		return vis_init;

	case set_msg_proc:
		if(vis_init)
		{
			vis_message = (fn_vis_message)rdata;
			return vis_init;
		}else{
			return 0;
		}

	case get_color:
		switch(dsize) /* index */
		{
		case color_light:
		case color_normal:  return display_normal;
		case color_dark:    return display_paused;
		case color_shifted: return display_stopped;
		}
		return 0;

	case get_window_playlist:
		*((HWND*)rdata) = window_ml;
		return 1;

	case get_window_vis:
		*((HWND*)rdata) = window_vis;
		return 1;

	case get_window_eq:
		*((HWND*)rdata) = window_eq;
		return 1;

	case get_window_ml:
		*((HWND*)rdata) = window_ml;
		return 1;

	case get_window_misc:
		return 0;

	case get_window_options:
		return 0;
	
	case get_window_video:
		if(window_vid)
		{
			*((HWND*)rdata) = window_vid;
			return 1;
		}else{
			*((HWND*)rdata) = 0;
			return 0;
		}

	case get_window_video_dc:

		if(hdc_vid)
		{
			*((HDC*)rdata) = hdc_vid;
			return 1;
		}else{
			*((HDC*)rdata) = 0;
			return 0;
		}

	case get_window_video_rect:
		vid_get_position((RECT*)rdata);
		return 0;

	case get_window_video_state:
		return 0;

	case set_video_window:
		{
			int prevst = 0;

			if(rdata)
			{
				prevst = skin_settings.vid_show;

				skin_settings.vid_show = 1;
				vid_create(skin.wnd);

			}else{

				prevst = skin_settings.vid_show;

				skin_settings.vis_show = 0;
				vid_close();
			}
			return prevst;
		}

	}

	return 1;
}



/* skin interface  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




/*
 * get button index.
 */
int skin_get_button_index(int x, int y)
{

	if(incoordx(x, y, &coords.window_main.button_play    ))     return skin_main_button_play;
	if(incoordx(x, y, &coords.window_main.button_stop    ))     return skin_main_button_stop;
	if(incoordx(x, y, &coords.window_main.button_previous))     return skin_main_button_previous;
	if(incoordx(x, y, &coords.window_main.button_next    ))     return skin_main_button_next;
	if(incoordx(x, y, &coords.window_main.button_open    ))     return skin_main_button_open;
	if(incoordx(x, y, &coords.window_main.button_playlist))     return skin_main_button_playlist;
	if(incoordx(x, y, &coords.window_main.button_eq      ))     return skin_main_button_equalizer;
	if(incoordx(x, y, &coords.window_main.button_minimize))     return skin_main_button_minimize;
	if(incoordx(x, y, &coords.window_main.button_exit    ))     return skin_main_button_exit;

	if(incoordx(x, y, &coords.window_main.bar_seek       ))     return skin_main_button_seek;
	if(incoordx(x, y, &coords.window_main.bar_volume     ))     return skin_main_button_volume;

	if(incoordx(x, y, &coords.window_main.button_settings))     return skin_main_button_settings;
	if(incoordx(x, y, &coords.window_main.button_convert ))     return skin_main_button_convert;
	if(incoordx(x, y, &coords.window_main.button_rip     ))     return skin_main_button_rip;
	if(incoordx(x, y, &coords.window_main.button_join    ))     return skin_main_button_join;
	if(incoordx(x, y, &coords.window_main.button_vis     ))     return skin_main_button_visual;
	if(incoordx(x, y, &coords.window_main.button_video   ))     return skin_main_button_video;

	if(incoordx(x, y, &coords.window_main.button_dsp     ))     return skin_main_button_dsp;
	if(incoordx(x, y, &coords.window_main.button_lock    ))     return skin_main_button_lock;

	return 0;
}


/*
 * erase button actions (back to normal).
 */
int skin_draw_button_normal(int id, HDC dc)
{

#	define blt(x, y, w, h, sx, sy) (StretchBlt(dc, (x), (y), ((w) * 2), ((h) * 2), mdc_sheet, (sx), (sy), (w), (h), SRCCOPY))
	
	switch(id)
	{
	case skin_main_button_play:      blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_play);     break;
	case skin_main_button_stop:      blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_stop);     break;
	case skin_main_button_previous:  blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_previous); break;
	case skin_main_button_next:      blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_next);     break;
	case skin_main_button_open:      blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_open);     break;
	case skin_main_button_playlist:  blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_playlist); break;
	case skin_main_button_equalizer: blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_eq);       break;
	case skin_main_button_minimize:  blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_minimize); break;
	case skin_main_button_exit:      blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_exit);     break;

	case skin_main_button_settings:  blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_settings); break; 
	case skin_main_button_convert:   blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_convert);  break;
	case skin_main_button_rip:       blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_rip);      break;
	case skin_main_button_join:      blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_join);     break;
	case skin_main_button_visual:    blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_vis);      break;
	case skin_main_button_video:     blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_video);    break;
	case skin_main_button_dsp:       blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_dsp);      break;
	
	case skin_main_button_lock:
		if(skin_settings.skin_lock)
			blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_lock);
		else
			blt_coord(dc, mdc_sheet, 0, &coords.window_main.button_unlock);
		break;
	}

	return 1;
}


/*
 * draw bright button.
 */
int skin_draw_button_hover(int id, HDC dc)
{
	static int lid = -1;

	switch(id)
	{
	case skin_main_button_play:      blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_play);     show_tip(oooo_skins_play_pause,          0); break;
	case skin_main_button_stop:      blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_stop);     show_tip(oooo_skins_stop,                0); break;
	case skin_main_button_previous:  blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_previous); show_tipex(oooo_skins_previous, oooo_skins_rewind,        0, uni("\nRight: ")); break;
	case skin_main_button_next:      blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_next);     show_tipex(oooo_skins_next,     oooo_skins_fast_forward,  1, uni("\nRight: ")); break;
	case skin_main_button_open:      blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_open);     show_tipex(oooo_skins_open,     oooo_skins_add_files,     2, uni("\nRight: ")); break;
	case skin_main_button_playlist:  blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_playlist); show_tip(oooo_skins_show_playlist,       0); break;
	case skin_main_button_equalizer: blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_eq);       show_tip(oooo_skins_show_equalizer,      0); break;
	case skin_main_button_minimize:  blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_minimize); show_tip(oooo_skins_minimize,            0); break;
	case skin_main_button_exit:      blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_exit);     show_tip(oooo_skins_close,               0); break;
																									   
	case skin_main_button_settings:  blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_settings); show_tip(oooo_skins_show_preferences,    0); break;
	case skin_main_button_convert:   blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_convert);  show_tip(oooo_skins_show_conversion,     0); break;
	case skin_main_button_rip:       blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_rip);      show_tip(oooo_skins_show_ripping,        0); break;
	case skin_main_button_join:      blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_join);     show_tip(oooo_skins_show_joining,        0); break;
	case skin_main_button_visual:    blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_vis);      show_tip(oooo_skins_Show_visualizations, 0); break;
	case skin_main_button_video:     blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_video);    show_tip(0, uni("Show/Hide video window")); break;
	case skin_main_button_dsp:       blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_dsp);      show_tip(0, uni("DSP/Effects")); break;
	
	case skin_main_button_lock:
		if(skin_settings.skin_lock)
		{
			blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_lock); show_tip(0, uni("Turn off window lock")); break;
		}else{
			blt_coord(dc, mdc_sheet, 1, &coords.window_main.button_unlock); show_tip(0, uni("Turn on window lock")); break;
		}
		break;
	}

	if(id == 0 || id != lid)
		skin_draw_button_normal(lid, dc);

	lid = id;
	return 0;
}


/*
 * draw pressed button.
 */
int skin_draw_button_down(int id, HDC dc)
{
	static int lid = -1;

#	define blt(x, y, w, h, sx, sy) (StretchBlt(dc, (x), (y), ((w) * 2), ((h) * 2), mdc_sheet, (sx), (sy), (w), (h), SRCCOPY))
	
	switch(id)
	{
	case skin_main_button_play:      blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_play);     break;
	case skin_main_button_stop:      blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_stop);     break;
	case skin_main_button_previous:  blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_previous); break;
	case skin_main_button_next:      blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_next);     break;
	case skin_main_button_open:      blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_open);     break;
	case skin_main_button_playlist:  blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_playlist); break;
	case skin_main_button_equalizer: blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_eq);       break;
	case skin_main_button_minimize:  blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_minimize); break;
	case skin_main_button_exit:      blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_exit);     break;

	case skin_main_button_settings:  blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_settings); break; 
	case skin_main_button_convert:   blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_convert);  break;
	case skin_main_button_rip:       blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_rip);      break;
	case skin_main_button_join:      blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_join);     break;
	case skin_main_button_visual:    blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_vis);      break;
	case skin_main_button_video:     blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_video);    break;
	case skin_main_button_dsp:       blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_dsp);      break;
	
	case skin_main_button_lock:
		if(skin_settings.skin_lock)
			blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_lock);
		else
			blt_coord(dc, mdc_sheet, 2, &coords.window_main.button_unlock);
		break;
	}

	if(id == 0 || id != lid)
		skin_draw_button_normal(lid, dc);

	lid = id;
	return 0;
}

/*
 * move main window without requesting hwnd
 */

int skin_move_main_window(int dx, int dy)
{
	return skin_move_window(0, dx, dy);
}

/*
 * move main window.
 */
int skin_move_window(HWND hwnd, int dx, int dy)
{
	POINT        pt;
	static int   lpx, lpy;
	int          x, y;
	RECT         cposrct;
	RECT         rct;
	RECT         wrct;

	GetCursorPos(&pt);

	if(pt.x == lpx && pt.y == lpy)
	{
		return 0;
	}else{
		lpx = pt.x;
		lpy = pt.y;
	}

	if(hwnd)
		GetWindowRect(hwnd, &cposrct);
	else
		GetWindowRect(wnd, &cposrct);

	x = pt.x - dx;
	y = pt.y - dy;

	if(x < 10 && x > -10)x = 0;
	if(y < 10 && y > -10)y = 0;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &wrct, 0);

	if((x + cr(coords.window_main.width) < wrct.right + 10)  && (x + cr(coords.window_main.width) > wrct.right - 10))
		x = wrct.right - cr(coords.window_main.width);

	if((y + cr(coords.window_main.height) < wrct.bottom + 10) && (y + cr(coords.window_main.height) > wrct.bottom - 10))
		y = wrct.bottom - cr(coords.window_main.height);
	
	if(skin_settings.ml_d)
	{
		if((skin_settings.ml_y > y) && (y + skin_settings.ml_h + cr(coords.window_main.height) < wrct.bottom + 10) && (y + skin_settings.ml_h + cr(coords.window_main.height) > wrct.bottom - 10))
			y = wrct.bottom - cr(coords.window_main.height) - skin_settings.ml_h;

		if((skin_settings.ml_y + skin_settings.ml_y <= y) && (y - skin_settings.ml_h < 10) && (y - skin_settings.ml_h > -10))
			y = skin_settings.ml_h;

		if((skin_settings.ml_x > x) && (x + skin_settings.ml_w + cr(coords.window_main.width) < wrct.right + 10) && (x + skin_settings.ml_w + cr(coords.window_main.width) > wrct.right - 10))
			x = wrct.right - cr(coords.window_main.width) - skin_settings.ml_w;

		if((skin_settings.ml_x + skin_settings.ml_x <= x) && (x - skin_settings.ml_w < 10) && (x - skin_settings.ml_w > -10))
			x = skin_settings.ml_w;
	}

	if(skin_settings.eq_d)
	{
		if((skin_settings.eq_y > y) && (y + cr(coords.window_eq.height) + cr(coords.window_main.height) < wrct.bottom + 10) && (y + cr(coords.window_eq.height) + cr(coords.window_main.height) > wrct.bottom - 10))
			y = wrct.bottom - cr(coords.window_main.height) - cr(coords.window_eq.height);

		if((skin_settings.eq_y + skin_settings.eq_y <= y) && (y - cr(coords.window_eq.height) < 10) && (y - cr(coords.window_eq.height) > -10))
			y = cr(coords.window_eq.height);

		if((skin_settings.eq_x > x) && (x + cr(coords.window_eq.width) + cr(coords.window_main.width) < wrct.right + 10) && (x + cr(coords.window_eq.width) + cr(coords.window_main.width) > wrct.right - 10))
			x = wrct.right - cr(coords.window_main.width) - cr(coords.window_eq.width);

		if((skin_settings.eq_x + skin_settings.eq_x <= x) && (x - cr(coords.window_eq.width) < 10) && (x - cr(coords.window_eq.width) > -10))
			x = cr(coords.window_eq.width);
	}


	if(skin_settings.ml_d)
	{
		skin_settings.ml_x = x - (cposrct.left - skin_settings.ml_x);
		skin_settings.ml_y = y - (cposrct.top  - skin_settings.ml_y);

		if(skin_settings.ml_show)
			SetWindowPos(window_ml, 0, skin_settings.ml_x, skin_settings.ml_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		
		if(skin_settings.ml_show && GetWindowRect(window_ml, &rct))
		{
			skin_settings.ml_w = rct.right - rct.left;
			skin_settings.ml_h = rct.bottom - rct.top;
			skin_settings.ml_d = skin_settings.ml_d;
		}


	}

	if(skin_settings.eq_d)
	{
		int xc, yc;

		xc = x - (cposrct.left - skin_settings.eq_x);
		yc = y - (cposrct.top - skin_settings.eq_y);

		if(skin_settings.eq_show)
			SetWindowPos(window_eq, 0, xc, yc, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		skin_settings.eq_x = xc;
		skin_settings.eq_y = yc;
	}

	
	if(skin_settings.vis_d)
	{
		int xc, yc;

		xc = x - (cposrct.left - skin_settings.vis_x);
		yc = y - (cposrct.top - skin_settings.vis_y);

		if(skin_settings.vis_show)
			SetWindowPos(window_vis, 0, xc, yc, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		skin_settings.vis_x = xc;
		skin_settings.vis_y = yc;
	}

	if(skin_settings.vid_d)
	{
		int xc, yc;

		xc = x - (cposrct.left - skin_settings.vid_x);
		yc = y - (cposrct.top - skin_settings.vid_y);

		if(skin_settings.vid_show)
			SetWindowPos(window_vid, 0, xc, yc, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		skin_settings.vid_x = xc;
		skin_settings.vid_y = yc;
	}


	if(hwnd)
		SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	else
		SetWindowPos(wnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	skin_settings.main_x = x;
	skin_settings.main_y = y;

	sys_pass();

	
	SendMessage(hwnd, WM_PAINT, 0, 0);
	
	if(skin_settings.eq_show)
		SendMessage(window_eq, WM_PAINT, 0, 0);
	if(skin_settings.ml_show)
		SendMessage(window_ml, WM_PAINT, 0, 0);
	if(skin_settings.vis_show)
		SendMessage(window_vis, WM_PAINT, 0, 0);
	if(skin_settings.vid_show)
		SendMessage(window_vid, WM_PAINT, 0, 0);

	sys_pass();

	return 1;
}

void CALLBACK seek_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if(seekmode == 1)
	{
		double ps = 0.0;

		skin.shared->audio.output.getposition(&ps);
		ps += 0.01;
		if(ps > 1.0)ps = 1.0;
		skin.shared->audio.output.setposition(ps);

	}else if(seekmode == 2){

		double ps = 0.0;

		skin.shared->audio.output.getposition(&ps);
		ps -= 0.01;
		if(ps < 0.0)ps = 0.0;
		skin.shared->audio.output.setposition(ps);
	}
}

void CALLBACK display_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{

	int     v, p;
	letter  pbuf[10];
	double  d = 0.0, dl = 0.0, dr = 0.0;
	int     ps;


	static string  current_title_buf = 0;
	
	static int title_rev_stay = 0;
	static int title_rev = 0;
	static int hp = 0;
	static int cp = 0;
	static int mp = 0;
	static int left_pos = 0;
	static int left_max = 0;

	ml_pl_preview_display_timer();

	if(uMsg == WM_USER + 222)
	{
		mp = cp = hp = 0;
	}
	
	if(uMsg == WM_USER + 221 && (hwnd == (HWND)-1))
	{
		DeleteDC(ldcmem);
		DeleteObject(lbmp);
		ldcmem = 0;
		lbmp = 0;
	}


	if(!ldcmem)
	{
		ldcmem = CreateCompatibleDC(hdc);
		lbmp   = CreateCompatibleBitmap(hdc, cr(coords.window_main.width), cr(coords.window_main.height));

		SelectObject(ldcmem, lbmp);
		SelectObject(ldcmem, displayfont);
	}

	if(uMsg == (UINT)-1)
	{
		DeleteDC(ldcmem);
		DeleteObject(lbmp);
		ldcmem = 0;
		return;
	}

				
	SetStretchBltMode(ldcmem, HALFTONE);
	SetBrushOrgEx(ldcmem, 0, 0, 0);


	if(uMsg == WM_USER + 221)
		StretchBlt(ldcmem, 0, 0, cr(coords.window_main.width), cr(coords.window_main.height), mdc_sheet, 0, 0, coords.window_main.width, coords.window_main.height, SRCCOPY);
	else
	{
		blt_coord(ldcmem, mdc_sheet, 0, &coords.window_main.display_area);
	}
	if(!skin_settings.skin_lock)
		blt_coord(ldcmem, mdc_sheet, 0, &coords.window_main.button_unlock);

	
	ps = skin.shared->audio.output.getplayerstate();
	
	SetTextColor(hdc, display_normal);
	SetBkMode(ldcmem, TRANSPARENT);

	blt_coord(ldcmem, mdc_sheet, 0, &coords.window_main.display_text_area);

	p = (int)(skin.shared->audio.output.getduration_ms() / 1000);
	
	
	memset(pbuf, 0, sizeof(pbuf));
	
	if(p / 60 <= 60)
	{
		_itow(p / 60, pbuf, 10);
		pbuf[str_len(pbuf)] = uni(':');

		if((p % 60) < 10)
		{	
			pbuf[str_len(pbuf)] = uni('0');
			_itow(p % 60, pbuf + str_len(pbuf), 10);
		}else{
			_itow(p % 60, pbuf + str_len(pbuf), 10);
		}

	}else{
		_itow(p / 3600, pbuf, 10);
		str_cat(pbuf, uni("h"));
	}

	TextOut(ldcmem, cr(coords.window_main.dur_text_x), cr(coords.window_main.dur_text_y), pbuf, (int)str_len(pbuf));

	p = (int)(skin.shared->audio.output.getposition_ms() / 1000);

	memset(pbuf, 0, sizeof(pbuf));
	
	if(p / 60 <= 60)
	{
		_itow(p / 60, pbuf, 10);
		pbuf[str_len(pbuf)] = uni(':');
		
		if(p % 60 < 10)
		{	
			pbuf[str_len(pbuf)] = uni('0');
			_itow(p % 60, pbuf + str_len(pbuf), 10);
		}else{
			_itow(p % 60, pbuf + str_len(pbuf), 10);
		}

	}else{
		_itow(p / 3600, pbuf, 10);
		str_cat(pbuf, uni("h"));
	}


	TextOut(ldcmem, cr(coords.window_main.pos_text_x), cr(coords.window_main.pos_text_y), pbuf, (int)str_len(pbuf));

	SelectClipRgn(ldcmem, cliprgn);

	
	switch(ps)
	{
	case v_audio_playerstate_playingandbuffering:
	case v_audio_playerstate_playing:
		SetTextColor(ldcmem, display_normal);
		break;

	case v_audio_playerstate_paused:
		SetTextColor(ldcmem, display_paused);
		break;

	case v_audio_playerstate_stopped:
	case v_audio_playerstate_init:
	case v_audio_playerstate_loaded:
		SetTextColor(ldcmem, display_stopped);
		break;

	case v_audio_playerstate_buffering:
		SetTextColor(ldcmem, RGB(0, 192, 0));
		break;

	case v_audio_playerstate_notinit:
		SetTextColor(ldcmem, RGB(255, 0, 0));
		break;
	}

	if(title[0] && (!artist[0] && !album[0]))
	{
		mp = 0;
		cp = 0;
		hp = 0;
	}

	switch(mp)
	{
	case 0: current_title_buf = title;  break;
	case 1: current_title_buf = artist; break;
	case 2: current_title_buf = album;  break;
	}

	if(!current_title_buf)mp++;
	else if(!current_title_buf[0])mp++;

	TextOut(ldcmem, cr(coords.window_main.infotext_x) - left_pos, cr(coords.window_main.infotext_y) - cp, current_title_buf, (int)str_len(current_title_buf)); 

	SelectClipRgn(ldcmem, 0);


	if(uMsg != WM_USER + 221)
	{
		hp++;

		if(cp < 0)cp++;

		if(hp > 50)
		{
			cp++;
		}else if(cp == 0){
			if(!left_max)
			{
				SIZE  extsz;

					GetTextExtentPoint32(ldcmem, current_title_buf, (int)str_len(current_title_buf), &extsz);
					if(extsz.cx >  cr(coords.window_main.display_region.w))
						left_max = extsz.cx;
			}

			if(!title_rev)
			{
				if(left_max + 30> (left_pos + cr(coords.window_main.display_region.w)))
				{
					left_pos++;
					hp = 0;
				}else{
					if(left_max)
					{
						title_rev = 1;
						title_rev_stay = 20;
					}
				}
			}else{
				left_pos -= 5;
				
				if(left_pos <= 0)
				{
					left_pos = 0;
					title_rev_stay--;

					if(title_rev_stay <= 0)
					{
						left_max = 0;
						hp = 51;
						title_rev = 0;
						title_rev_stay = 0;
					}
				}
			}
		}

		if(cp > cr(coords.window_main.infotext_y + 2))
		{
			cp = -cr(coords.window_main.infotext_x + 2);
			hp = 0;
			mp++;
			
			if(mp >= ((artist[0] != 0) + (album[0] != 0) + (title[0] != 0)))
				mp = 0;
		}
	}


	skin.shared->audio.output.getposition(&d);
	if     (d > 1) d = 1;
	else if(d < 0) d = 0;


	v = (int)(d * coords.window_main.bar_seek.w);

	blt_coord(ldcmem, mdc_sheet, 0, &coords.window_main.bar_seek);
	blt_coord_ew(ldcmem, mdc_sheet, 1, &coords.window_main.bar_seek, v);

	skin.shared->audio.output.getvolume(&dl, &dr);

	if     (dl > 1) dl = 1;
	else if(dl < 0) dl = 0;

	if     (dr > 1) dr = 1;
	else if(dr < 0) dr = 0;

	v = (int)(((dl + dr) / 2) * coords.window_main.bar_volume.w);

	blt_coord(ldcmem, mdc_sheet, 0, &coords.window_main.bar_volume);
	blt_coord_ew(ldcmem, mdc_sheet, 1, &coords.window_main.bar_volume, v);

	if(uMsg == WM_USER + 221)
		BitBlt(hdc, 0, 0, cr(coords.window_main.width), cr(coords.window_main.height), ldcmem, 0, 0, SRCCOPY);
	else		
		blt_coord_nozoom(hdc, ldcmem, 0, &coords.window_main.display_area);



}


/* Windows specific callbacks -----------------------------------------------*/


/*
 * startup.
 */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch(fdwReason) 
    { 
        case DLL_PROCESS_ATTACH:
			instance_skin = hinstDLL;
            break;
    }
    return 1;
}

/*
 * fennec main window.
 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int downid = -1, dx = 0, dy = 0;

	switch(msg)
	{
	case WM_MOUSEMOVE:
		{
			int x = (int)(short)LOWORD(lParam), y = (int)(short)HIWORD(lParam);

			if((x > cr(coords.window_main.width) - 5 && x < cr(coords.window_main.width)) || downid == skin_main_endl)
				SetCursor(LoadCursor(0, IDC_SIZEWE));
			if((y > cr(coords.window_main.height) - 5 && y < cr(coords.window_main.height)) || downid == skin_main_endb)
				SetCursor(LoadCursor(0, IDC_SIZENS));

			if(!downid)
			{
				skin_move_window(wnd, dx, dy);
				break;
			}

			if(downid == skin_main_button_seek)
			{
				int v = x - cr(coords.window_main.bar_seek.x);
				double  pos;

				if(v < 0)   v = 0;
				if(v > cr(coords.window_main.bar_seek.w)) v = cr(coords.window_main.bar_seek.w);

				blt_coord(hdc, mdc_sheet, 0, &coords.window_main.bar_seek);
				blt_coord_ew(hdc, mdc_sheet, 1, &coords.window_main.bar_seek, (int)((float)v / coords.zoom));

				pos =  (((double)v) / ((double)coords.window_main.bar_seek.w)) / ((double)coords.zoom);
				if(pos <= 1.0)
					skin.shared->audio.output.setposition(pos);
				else
					skin.shared->audio.output.setposition(1.0);
				break;
			}

			if(downid == skin_main_button_volume)
			{
				double vv;
				int v = x - cr(coords.window_main.bar_volume.x);

				if(v < 0)   v = 0;
				if(v > cr(coords.window_main.bar_volume.w)) v = cr(coords.window_main.bar_volume.w);

				blt_coord(hdc, mdc_sheet, 0, &coords.window_main.bar_volume);
				blt_coord_ew(hdc, mdc_sheet, 1, &coords.window_main.bar_volume, (int)((float)v / coords.zoom));

				vv =  (((double)v) / ((double)coords.window_main.bar_volume.w)) / ((double)coords.zoom);
	
				skin.shared->audio.output.setvolume(vv, vv);
				break;
			}

			if(downid == skin_main_endr || downid == skin_main_endb)
			{
				if(downid == skin_main_endr)
					coords.zoom = (float)x / (float)coords.window_main.width;
				else
					coords.zoom = (float)y / (float)coords.window_main.height;

				if(coords.zoom < 0.2f)coords.zoom = 0.2f;
				if(coords.zoom > 0.98f && coords.zoom < 1.2f)coords.zoom = 1.0f;

				skin_settings.zoom = coords.zoom;

				SetWindowRgn(hwnd, 0, 0);
				if(wndmainrgn) DeleteObject(wndmainrgn);
				wndmainrgn = CreateRoundRectRgn(0, 0, cr(coords.window_main.width), cr(coords.window_main.height), cr(coords.window_main.window_edge), cr(coords.window_main.window_edge));
				SetWindowRgn(hwnd, wndmainrgn, 1);

				setwinpos_clip(hwnd, 0, skin_settings.main_x, skin_settings.main_y,
								cr(coords.window_main.width), cr(coords.window_main.height), SWP_NOZORDER);

				if(cliprgn)DeleteObject(cliprgn);
				cliprgn = CreateRectRgn(cr(coords.window_main.display_region.x), cr(coords.window_main.display_region.y), cr(coords.window_main.display_region.w), cr(coords.window_main.display_region.h));

				if(displayfont)DeleteObject(displayfont);

				displayfont = CreateFont(-MulDiv(cr(8), GetDeviceCaps(hdc, LOGPIXELSY), 72),
                            0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                            DEFAULT_PITCH, skin_settings.font_display);

				eq_skinchange();

				display_timer((HWND)-1, WM_USER + 221, 0, 0);
				
			}

			skin_draw_button_hover(skin_get_button_index(x, y), hdc);
		}
		break;

	case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam), y = HIWORD(lParam), id;

			id = skin_get_button_index(x, y);

			switch(id)
			{
			case skin_main_button_previous: 
				SetCapture(hwnd);
				seekmode = 2;
				break;

			case skin_main_button_next:
				SetCapture(hwnd);
				seekmode = 1;
				break;
			
			case 0:
				
				break;
			}
		}
		break;

	case WM_RBUTTONUP:
		{
			int x = LOWORD(lParam), y = HIWORD(lParam), id;

			if(incoordx(x, y, &coords.window_main.display_text_area))
			{
				skin.shared->general.show_tageditor(0, 0, 0);

			}else{
				id = skin_get_button_index(x, y);

				switch(id)
				{
				case 0:
					SendMessage(hwnd, (WM_USER + 123), 0, WM_RBUTTONUP);
					break;

				case skin_main_button_open: 
					{
						POINT pt;
						HMENU mc = user_create_menu(menu_open, 0);
						
						GetCursorPos(&pt);

						switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0))
						{
						case mid_addfiles:
							skin.shared->simple.show_addfile();
							break;

						case mid_adddirs:
							skin.shared->simple.show_addfolder();
							break;

						case mid_tray:
							skin.shared->general.sendcommand(uni(">eject"));
							break;

						case mid_seldrive:
							skin.shared->general.sendcommand(uni(">selectdrive"));
							break;

						case mid_loadtracks:
							skin.shared->general.sendcommand(uni(">loadtracks"));
							break;

						case mid_experimental:
							skin.shared->call_function(show_open_experimental, 0, 0, 0);
							break;
						}

						DestroyMenu(mc);
					}
					break;
				}
			}
		}

		ReleaseCapture();
		seekmode = 0;
		break;

	case WM_LBUTTONDOWN:

		dx = LOWORD(lParam), dy = HIWORD(lParam);
		
		downid = skin_get_button_index(dx, dy);

		if(dx > cr(coords.window_main.width)  - 5)downid = skin_main_endr;
		if(dy > cr(coords.window_main.height) - 5)downid = skin_main_endb;
	

		if(downid == 0 || downid == skin_main_button_seek || downid == skin_main_button_volume || downid == skin_main_endr || downid == skin_main_endb)
		{
			PostMessage(hwnd, WM_MOUSEMOVE, 0, lParam);
			SetCapture(hwnd);
		}

		if(downid != skin_main_endr)
			skin_draw_button_down(downid, hdc);

		break;

	case WM_LBUTTONUP:
		{
			int x = LOWORD(lParam), y = HIWORD(lParam), id;

			ReleaseCapture();
			downid = 1;

			id = skin_get_button_index(x, y);

			skin_draw_button_hover(id, hdc);

			switch(id)
			{
			case skin_main_button_play: 
				skin.shared->audio.output.play();
				break;

			case skin_main_button_stop: 
				skin.shared->audio.output.stop();
				break;

			case skin_main_button_previous: 
				skin.shared->audio.output.playlist.previous();
				break;

			case skin_main_button_next: 
				skin.shared->audio.output.playlist.next();
				break;

			case skin_main_button_open: 
				skin.shared->simple.show_openfile();
				break;

			case skin_main_button_minimize:
				ShowWindow(wnd, SW_MINIMIZE);
				break;

			case skin_main_button_exit:
				SendMessage(wnd, WM_DESTROY, 0, 0);
				break;

			case skin_main_button_playlist:
				if(!skin_settings.ml_show)
				{
					skin_settings.ml_show = 1;
					ml_create(hwnd);
					
				}else{
					ml_close();
					skin_settings.ml_show = 0;
				}
				break;

			case skin_main_button_equalizer:
				if(!skin_settings.eq_show)
				{
					skin_settings.eq_show = 1;
					eq_create(hwnd);
					
				}else{
					eq_close();
					skin_settings.eq_show = 0;
				}
				break;

			case skin_main_button_settings:
				skin.shared->general.show_settings(0, 0, 0);
				break;

			case skin_main_button_convert:
				skin.shared->general.show_conversion(0, 0, 0);
				break;

			case skin_main_button_rip:
				skin.shared->general.show_ripping(0, 0, 0);
				break;

			case skin_main_button_join:
				skin.shared->general.show_joining(0, 0, 0);
				break;

			case skin_main_button_visual:
				skin_settings.vis_show ^= 1;

				if(skin_settings.vis_show)
					vis_create(hwnd);
				else
					vis_close();
				break;

			case skin_main_button_video:
				skin_settings.vid_show ^= 1;

				if(skin_settings.vid_show)
					vid_create(hwnd);
				else
					vid_close();
				break;

			case skin_main_button_dsp:
				skin.shared->general.show_settings(0, 0, 11);
				break;

			case skin_main_button_lock:
				skin_settings.skin_lock ^= 1;
				break;
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if(incoord((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), 7, 17, 204, 11))
			skin.shared->general.show_tageditor(0, 0, 0);

		break;

	case WM_PAINT:
		{
			PAINTSTRUCT  ps;
		
			BeginPaint(hwnd, &ps);

			display_timer(0, WM_USER + 221, 0, 0);
			//BitBlt(hdc, 0, 0, cr(coords.window_main.width), cr(coords.window_main.height), mdc_sheet, skin_main_x, skin_main_y, SRCCOPY);
			
			EndPaint(hwnd, &ps);
		}
		break;

	}
	return 1;
}

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
