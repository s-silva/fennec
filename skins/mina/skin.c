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
#include <math.h>
#include "skin settings.h"
#include <shlobj.h>
#include <userenv.h>

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
int  skin_create_images(void);






/* defines ------------------------------------------------------------------*/


#define  mask_id_play      1
#define  mask_id_display   2
#define  mask_id_close     3
#define  mask_id_eject     4
#define  mask_id_maximize  5
#define  mask_id_position  10
#define  mask_id_volume    15



/* data ---------------------------------------------------------------------*/

extern HWND  window_media;
extern HWND  window_search;

struct skin_data        skin;
HDC                     hdc;
HDC                     mdc;
HWND                    wnd, hdlg;
HINSTANCE               instance_skin;

HDC                     background_dc;
HDC                     play_hover_dc;
HDC                     main_vis_dc;
HDC                     wheel_pos_dc;
HDC                     wheel_vol_dc;
HDC                     mask_dc;
HDC                     basic_icons_dc;
HDC                     main_icons_dc;
HDC                     main_panel_dc;
HDC                     sparkles_dc;
HDC                     menuicons_dc;
HDC                     windowsheet_dc;

int                     main_w, main_h;
HDC                     ndc;
HBITMAP                 mbmp;

int                     mouse_down_controller = 0;
int                     mouse_over_controller = 0;
int                     mouse_hover_val = 0;

letter                  artist[512];
letter                  title[512];
letter                  album[512];
letter                  year[512];
letter                  album_year[512];
letter                  position[512];


letter                  next_artist[512];
letter                  next_title[512];
letter                  next_album[512];
letter                  next_year[512];
letter                  next_album_year[512];
letter                  next_position[512];


int                     dpoffset_artist = 0, dpoffset_max_artist = 0, dpoffset_rollback_artist = 0;
int                     dpoffset_title = 0, dpoffset_max_title = 0, dpoffset_rollback_title = 0;
int                     dpoffset_album_year = 0, dpoffset_max_album_year = 0, dpoffset_rollback_album_year = 0;
int						dpoffset_update = 0;

int                     display_content_x = 0, display_content_y = 0;
unsigned int            timer_id_seek = 0;

HRGN					display_clip;

int                     png_play_w, png_play_h;
int                     png_playh_w, png_playh_h;
int                     png_vis_w, png_vis_h;
int                     png_wheel_pos_w, png_wheel_pos_h;
int                     png_wheel_vol_w, png_wheel_vol_h;
int                     basic_icons_w, basic_icons_h;
int                     main_icons_w, main_icons_h;
int                     main_panel_w, main_panel_h;
int                     sparkles_w, sparkles_h;
int                     menuicons_w, menuicons_h;

int                     play_button_position_set = 0;
int                     delay_counter = 0;
int                     main_panel_active = 0;
int                     main_panel_display_time = 0;
int                     skin_panel_display_time = 0;
int                     main_panel_slide = 0;

float                   beat_level = 0.0f;

COLORREF                text_color[2];

int                     skin_color_part = 1; /* 1 base, 2 control, 3 text */

int                     sleep_timeout = 0;
int                     sleep_alpha   = 0;

int						tag_available_albumandyear = 0;
int						tag_available_artist       = 0;

int                     enable_transparency = 0;
int                     window_transparency_amount = 50;

int                     skin_mode = mode_display;
int                     eq_channel = -1; /* universal */
int                     eq_wait = 0;
int                     eq_current_bar = 0; /* -1 = preamp */
int                     down_button = 0;
int                     menu_selected_item = -1;
int                     menu_position = 0, menu_button_shift = 0;

fn_vis_message          vis_message = 0;
int                     vis_used = 0, vis_active = 0;

int                     button_video_transparency = 0;
int						button_video_transparency_add = 5;

void vis_update();
void make_initial_dirs(void);

int                     is_video_playing = 0;
int                     last_video_dsize = 0;
int                     skin_closing = 0;

/* code ---------------------------------------------------------------------*/

/*
 * initialization callback.
 */
int __stdcall fennec_skin_initialize(struct skin_data *indata, struct skin_data_return *outdata)
{
	HRGN      wndmainrgn;
	
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

	wnd = indata->wnd;
	hdc = GetDC(wnd);

	/* load settings */

	sset_load();
	enable_transparency        = settings_data.main.enable_transparency;
	window_transparency_amount = settings_data.main.transparency_amount;

	/* create main stuff */

	skin_create_images();
	typo_create_fonts();


	/* create needed dirs */

	make_initial_dirs();

	/* region */

	SetWindowRgn(wnd, 0, 0);
	wndmainrgn = CreateRoundRectRgn(0, 0, main_w, main_h, 4, 4);
	SetWindowRgn(wnd, wndmainrgn, 1);

	/* set position/size */

	SetWindowPos(wnd, 0, settings_data.main.x, settings_data.main.y, main_w, main_h, SWP_NOZORDER);


	display_clip = CreateRectRgn(115, 7, 415, 7 + 106);

	timer_id_seek = (unsigned int)SetTimer(0, 0, 35,  (TIMERPROC) display_timer);

	if(enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetClassLong(wnd, GCL_STYLE, GetClassLong(wnd, GCL_STYLE) | CS_DROPSHADOW);

	if(settings_data.vis.show_vis)
	{
		vid_create(wnd, 16.0 / 9.0);
		vis_used = 1;
		vis_active = 1;
	}

	ShowWindow(wnd, SW_SHOW);

	if(settings_data.playlist.visible) media_create(wnd);
	return 1;
}

void make_initial_dirs(void)
{
	letter     tmp[1024];

	memset(tmp, 0, sizeof(tmp));
	SHGetSpecialFolderPath(0, tmp, CSIDL_PERSONAL, FALSE);
	str_cat(tmp, uni("\\Fennec"));
	CreateDirectory(tmp, 0);

	SetFileAttributes(tmp, FILE_ATTRIBUTE_HIDDEN);

	memset(tmp, 0, sizeof(tmp));
	SHGetSpecialFolderPath(0, tmp, CSIDL_PERSONAL, FALSE);
	str_cat(tmp, uni("\\Fennec\\Thumbnails"));
	CreateDirectory(tmp, 0);

	memset(tmp, 0, sizeof(tmp));
	SHGetSpecialFolderPath(0, tmp, CSIDL_PERSONAL, FALSE);
	str_cat(tmp, uni("\\Fennec\\Thumbnails\\Albums"));

	CreateDirectory(tmp, 0);

	memset(tmp, 0, sizeof(tmp));
	SHGetSpecialFolderPath(0, tmp, CSIDL_PERSONAL, FALSE);
	str_cat(tmp, uni("\\Fennec\\Thumbnails\\Artists"));

	CreateDirectory(tmp, 0);
}

int skin_create_images(void)
{
	letter    skin_path[512], skin_ap[512];

	int       color_set1 = 0, color_set2 = 0;


	if(settings_data.theme.use_theme == 0)
	{
		text_color[0] = RGB(230, 217, 207);
		text_color[1] = RGB(222, 204, 191);
	}else{
		skin_color_hue   = settings_data.theme.text.h;
		skin_color_sat   = settings_data.theme.text.s;
		skin_color_light = settings_data.theme.text.l;

		text_color[0] = RGB(240, 179, 117);
		text_color[1] = RGB(237, 159, 82);

		set_color_32((unsigned char *) &text_color[0]);
		set_color_32((unsigned char *) &text_color[1]);
	}

	if(settings_data.theme.use_theme)
	{
		color_set1 = 1, color_set2 = 2;
	}

	skin.shared->general.getskinspath(skin_path, sizeof(skin_path));
	str_cat(skin_path, uni("/neo/"));

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("main.png"));
	background_dc = png_get_hdc(skin_ap, color_set1);
	main_w = png_w;
	main_h = png_h;

	mdc = CreateCompatibleDC(hdc);
	mbmp = CreateCompatibleBitmap(hdc, main_w, main_h);
	SelectObject(mdc, mbmp);


	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("play.png"));
	ndc = png_get_hdc(skin_ap, color_set2);
	png_play_w = png_w;
	png_play_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("play_h.png"));
	play_hover_dc = png_get_hdc(skin_ap, color_set2);
	png_playh_w = png_w;
	png_playh_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("main_vis.png"));
	main_vis_dc = png_get_hdc(skin_ap, color_set1);
	png_vis_w = png_w;
	png_vis_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("position_wheel.png"));
	wheel_pos_dc = png_get_hdc(skin_ap, color_set2);
	png_wheel_pos_w = png_w;
	png_wheel_pos_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("volume_wheel.png"));
	wheel_vol_dc = png_get_hdc(skin_ap, color_set2);
	png_wheel_vol_w = png_w;
	png_wheel_vol_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("basicicons.png"));
	basic_icons_dc = png_get_hdc(skin_ap, color_set2);
	basic_icons_w = png_w;
	basic_icons_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("mainicons.png"));
	main_icons_dc = png_get_hdc(skin_ap, color_set1);
	main_icons_w = png_w;
	main_icons_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("mainpanel.png"));
	main_panel_dc = png_get_hdc(skin_ap, color_set1);
	main_panel_w = png_w;
	main_panel_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("sparkles.png"));
	sparkles_dc = png_get_hdc(skin_ap, color_set1);
	sparkles_w = png_w;
	sparkles_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("menuicons.png"));
	menuicons_dc = png_get_hdc(skin_ap, color_set1);
	menuicons_w = png_w;
	menuicons_h = png_h;

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("windowsheets.png"));
	windowsheet_dc = png_get_hdc(skin_ap, color_set1);

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("mask.png"));
	mask_dc = png_get_hdc(skin_ap, 0);

	return 1;
}

int skin_delete_images(void)
{
	DeleteDC(background_dc);
	DeleteDC(mdc);
	DeleteObject(mbmp);

	DeleteDC(ndc);
	DeleteDC(play_hover_dc);
	DeleteDC(main_vis_dc);
	DeleteDC(wheel_pos_dc);
	DeleteDC(wheel_vol_dc);
	DeleteDC(basic_icons_dc);
	DeleteDC(main_icons_dc);
	DeleteDC(main_panel_dc);
	DeleteDC(sparkles_dc);
	DeleteDC(mask_dc);

	return 1;
}

void skin_recreate(void)
{
	skin_delete_images();
	skin_create_images();
}


/*
 * uninitialize callback.
 */
int skin_uninitialize(int inum, void *idata)
{
	skin_closing = 1;

	if(window_vid)vid_close();
	if(window_media) media_close();
	if(window_search) search_close();

	settings_data.main.enable_transparency = enable_transparency;
	settings_data.main.transparency_amount = window_transparency_amount;

	if(enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
	
	SetClassLong(wnd, GCL_STYLE, GetClassLong(wnd, GCL_STYLE) & ~CS_DROPSHADOW);

	sset_save();

	ReleaseDC(wnd, mdc);

	skin_delete_images();

	KillTimer(0, timer_id_seek);

	skin_closing = 0;
	return 1;
}


/*
 * refresh callback.
 */
int skin_refresh(int inum, void *idata)
{
	struct fennec_audiotag  ctag;
	letter        fname[v_sys_maxpath];
	string        fpath;
	unsigned long id;

	artist[0] = 0;
	title[0]  = 0;
	album[0]  = 0;

	dpoffset_update = 1;
	dpoffset_artist = dpoffset_max_artist = dpoffset_rollback_artist = 0;
	dpoffset_title = dpoffset_max_title = dpoffset_rollback_title = 0;
	dpoffset_album_year = dpoffset_max_album_year = dpoffset_rollback_album_year = 0;



	if(!skin.shared->audio.output.playlist.getcount())return 0;

	fpath = skin.shared->audio.output.playlist.getsource(skin.shared->audio.output.playlist.getcurrentindex());
	
	if(!fpath)return 0;

	id = skin.shared->audio.input.tagread(fpath, &ctag);

	if(ctag.tag_artist.tsize)
		str_ncpy(artist, ctag.tag_artist.tdata, sizeof(artist) / sizeof(letter));
	else
		str_cpy(artist, uni("Unknown"));

	if(ctag.tag_album.tsize)
		str_ncpy(album, ctag.tag_album.tdata, sizeof(album) / sizeof(letter));

	if(ctag.tag_title.tsize)
		str_ncpy(title, ctag.tag_title.tdata, sizeof(title) / sizeof(letter));

	if(ctag.tag_year.tsize)
		str_ncpy(year, ctag.tag_year.tdata, sizeof(year) / sizeof(letter));

	if(!ctag.tag_title.tsize)
	{
		_wsplitpath(fpath, 0, 0, fname, 0);
		str_ncpy(title, fname, sizeof(title) / sizeof(letter));
	}

	tag_available_albumandyear = 1;

	if(album[0])
	{
		str_cpy(album_year, album);
		str_cat(album_year, uni(" - "));
		str_cat(album_year, year);
	}else{
		if(year[0])
		{
			str_cpy(album_year, year);
		}else{
			str_cpy(album_year, uni("Unknown"));
			tag_available_albumandyear = 0;
		}
	}
	

	skin.shared->audio.input.tagread_known(id, 0, &ctag);

	delay_counter = 100;


	if(media_init)
	{
		fullview_refresh(inum);
	}
	return 1;
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
 * sub skins.
 */

int skin_subskins_get(subskins_callback callfunc)
{
	return 0;
}

int skin_subskins_select(const string fname)
{
	return 0;
}

/*
 * apply theme colors.
 */
void set_theme_colors(void)
{

}


/*
 * set colors (hue, combsel - saturation | lightness)
 */
int skin_set_color(int hue, unsigned long combsl)
{
	skin_color_hue   = hue;
	skin_color_sat   = (int)LOWORD(combsl);
	skin_color_light = (int)HIWORD(combsl);

	if(hue == -1) /* get */
	{
		switch(skin_color_part)
		{
		case 1:
			switch(combsl)
			{
			case 0: return settings_data.theme.base.h;
			case 1: return settings_data.theme.base.s;
			case 2: return settings_data.theme.base.l;
			}
			break;

		case 2:
			switch(combsl)
			{
			case 0: return settings_data.theme.control.h;
			case 1: return settings_data.theme.control.s;
			case 2: return settings_data.theme.control.l;
			}
			break;

		case 3:
			switch(combsl)
			{
			case 0: return settings_data.theme.text.h;
			case 1: return settings_data.theme.text.s;
			case 2: return settings_data.theme.text.l;
			}
			break;
		}
	}


	switch(skin_color_part)
	{
	case 1:
		settings_data.theme.base.h = skin_color_hue  ;
		settings_data.theme.base.s = skin_color_sat  ;
		settings_data.theme.base.l = skin_color_light;
		break;
	case 2:
		settings_data.theme.control.h = skin_color_hue  ;
		settings_data.theme.control.s = skin_color_sat  ;
		settings_data.theme.control.l = skin_color_light;
		break;
	case 3:
		settings_data.theme.text.h = skin_color_hue  ;
		settings_data.theme.text.s = skin_color_sat  ;
		settings_data.theme.text.l = skin_color_light;
		break;


	default:
		settings_data.theme.base.h = skin_color_hue  ;
		settings_data.theme.base.s = skin_color_sat  ;
		settings_data.theme.base.l = skin_color_light;
		break;
	}

	if(skin_color_part == 3) /* text color */
	{
		text_color[0] = RGB(240, 179, 117);
		text_color[1] = RGB(237, 159, 82);

		set_color_32((unsigned char *) &text_color[0]);
		set_color_32((unsigned char *) &text_color[1]);

		settings_data.theme.text.h = skin_color_hue  ;
		settings_data.theme.text.s = skin_color_sat  ;
		settings_data.theme.text.l = skin_color_light;
		
	}else{

		skin_recreate();
	}
	return 1;
}


int skin_set_full_color(int bh, int bs, int bl, int ch, int cs, int cl, int th, int ts, int tl)
{
	settings_data.theme.base.h = bh;
	settings_data.theme.base.s = bs;
	settings_data.theme.base.l = bl;

	settings_data.theme.control.h = ch;
	settings_data.theme.control.s = cs;
	settings_data.theme.control.l = cl;

	settings_data.theme.text.h = th;
	settings_data.theme.text.s = ts;
	settings_data.theme.text.l = tl;
	skin_recreate();
	return 1;
}

int skin_copy_full_color()
{
	int bh, bs, bl, ch, cs, cl, th, ts, tl;
	HANDLE  mem;
	char *txt;

	OpenClipboard(wnd);
	EmptyClipboard();

	mem = GlobalAlloc(GMEM_DDESHARE, 128);

	bh = settings_data.theme.base.h   ;
	bs = settings_data.theme.base.s   ;
	bl = settings_data.theme.base.l   ;
	  	
	ch = settings_data.theme.control.h;
	cs = settings_data.theme.control.s;
	cl = settings_data.theme.control.l;
	  	
	th = settings_data.theme.text.h   ;
	ts = settings_data.theme.text.s   ;
	tl = settings_data.theme.text.l   ;

	txt = GlobalLock(mem);
	sprintf(txt, "%d, %d, %d,  %d, %d, %d,  %d, %d, %d", bh, bs, bl, ch, cs, cl, th, ts, tl);

	GlobalUnlock(mem);

	SetClipboardData(CF_TEXT, mem);

	CloseClipboard();

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
	case 1:  str_cpy(name, uni("")); break;
	case 2:  str_cpy(name, uni("Adjust Colors For - Base")); break;
	case 3:  str_cpy(name, uni("Adjust Colors For - Control Dock")); break;
	case 4:  str_cpy(name, uni("Adjust Colors For - Display")); break;
	case 5:  str_cpy(name, uni("")); break;
	case 6:  str_cpy(name, uni("--- Copy Values ---")); break;
	case 7:  str_cpy(name, uni("")); break;
	case 8:  str_cpy(name, uni("Green Fairy")); break;
	case 9:  str_cpy(name, uni("Emerald")); break;
	case 10: str_cpy(name, uni("Happy Hipster")); break;
	case 11: str_cpy(name, uni("Hospital")); break;
	case 12: str_cpy(name, uni("Silver")); break;
	case 13: str_cpy(name, uni("Oak")); break;
	case 14: str_cpy(name, uni("Custom Preset 9")); break;
	case 15: str_cpy(name, uni("Custom Preset 10")); break;
	case 16: str_cpy(name, uni("Custom Preset 11")); break;
	case 17: str_cpy(name, uni("Custom Preset 12")); break;
	case 18: str_cpy(name, uni("Custom Preset 13")); break;
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

	if(id == 0) settings_data.theme.use_theme = 0;
	else        settings_data.theme.use_theme = 1;

	switch(id)
	{
	case 0: /* default */
		skin_recreate();
		break;

	/* sep */

	case 2: /* base color */
		skin_color_part = 1;
		break;

	case 3: /* control color */
		skin_color_part = 2;
		break;

	case 4: /* text color */
		skin_color_part = 3;
		break;

	/* sep */

	case 6: /* text color */
		skin_copy_full_color();
		break;

	case 8:  skin_set_full_color(63, 5, 15,  32, 65, 57,  31, 19, 50); break;
	case 9:  skin_set_full_color(70, 11, 31,  32, 65, 57,  0, 0, 72); break;
	case 10: skin_set_full_color(72, 31, 79,  55, 65, 63,  13, 13, 72); break;
	case 11: skin_set_full_color(1, 53, 88,  21, 0, 88,  66, 25, 100); break;
	case 12: skin_set_full_color(31, 0, 100,  66, 74, 64,  0, 0, 0); break;
	case 13: skin_set_full_color(33, 45, 77,  46, 31, 80,  32, 22, 83); break;
	
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
	switch(id)
	{
	case get_visual:
		if(window_vid && vis_used)
		{
			*((HWND*)rdata) = window_vid;
			return 1;
		}else{
			*((HWND*)rdata) = 0;
			return 0;
		}


	case get_visual_dc:
		if(hdc_vid && vis_used)
		{
			*((HDC*)rdata) = hdc_vid;
			return 1;
		}else{
			*((HDC*)rdata) = 0;
			return 0;
		}


	case get_visual_x:
		if(window_vid && vis_used)
		{
			RECT rd;
			vid_get_position(&rd);
			return rd.left;
		}
		return 0;

	case get_visual_y:
		if(window_vid && vis_used)
		{
			RECT rd;
			vid_get_position(&rd);
			return rd.top;
		}
		return 0;

	case get_visual_w:
		if(window_vid && vis_used)
		{
			RECT rd;
			vid_get_position(&rd);
			return rd.right - rd.left;
		}
		return 0;

	case get_visual_h:
		if(window_vid && vis_used)
		{
			RECT rd;
			vid_get_position(&rd);
			return rd.bottom - rd.top;
		}
		return 0;

	case get_visual_winproc:
		if(window_vid && vis_used)
		{
			*((WNDPROC*)rdata) = callback_vid_window;
		}
		return 0;

	case set_msg_proc:
		if(window_vid && vis_used)
		{
			vis_message = (fn_vis_message)rdata;
			return 1;
		}else{
			return 0;
		}

	case get_color:
		return 0;

	case get_window_playlist:
		return 0;

	case get_window_vis:
		if(window_vid && vis_used)
		{
			*((HWND*)rdata) = window_vid;
			return 1;
		}else{
			*((HWND*)rdata) = 0;
			return 0;
		}

	case get_window_ml:
		return 0;

	case get_window_misc:
		return 0;

	case get_window_options:
		return 0;
	
	case get_window_video:
		if(window_vid && !vis_used)
		{
			*((HWND*)rdata) = window_vid;
			return 1;
		}else{
			*((HWND*)rdata) = 0;
			return 0;
		}


	case get_window_video_dc:

		if(hdc_vid && !vis_used)
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

		last_video_dsize = dsize;
		
		if(!settings_data.vis.show_vis)
		{
			int prevst = 0;

			if(settings_data.video.force_hide)
			{
				if(dsize)
					is_video_playing = 1;
				else
					is_video_playing = 0;

				return prevst;
			}

			vis_used = 0;

			if(dsize)
			{
				prevst = vid_init;
				vid_create(skin.wnd, (double)dsize / 1000000);
				is_video_playing = 1;
				skin.shared->call_function(call_videoout_initialize, 0, 0, 0);

			}else{
				prevst = vid_init;
				is_video_playing = 0;
				//skin.shared->call_function(call_videoout_uninitialize, 0, 0, 0);
				vid_close();
			}
			return prevst;
		}else{
			if(dsize)
			{
				vis_used = 0;
				vid_create(skin.wnd, (double)dsize / 1000000);
				skin.shared->call_function(call_videoout_initialize, 0, 0, 0);
				is_video_playing = 1;
				skin.shared->call_function(call_visualizations_select_none, 0, 0, 0);
			}else{
				vis_used = 1;
				//skin.shared->call_function(call_videoout_uninitialize, 0, 0, 0);
				is_video_playing = 0;
				str_cpy(skin.shared->settings.general->visualizations.selected, settings_data.vis.current_vis);
				skin.shared->call_function(call_visualizations_select_next, 0, 0, 0);
			}
			return 0;
		}


	}

	return 1;
}



/* skin interface  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void display_control_button(BLENDFUNCTION *bf, int mode)
{
	if(mode == 0)
	{
		switch(skin.shared->audio.output.getplayerstate())
		{
		case v_audio_playerstate_playingandbuffering:
		case v_audio_playerstate_playing:
			AlphaBlend(mdc, 40, 36, png_play_w, png_play_h, ndc, 0, 0, png_play_w, png_play_h, *bf);
			break;

		case v_audio_playerstate_paused:
			AlphaBlend(mdc, 37, 36, 40, 46, basic_icons_dc, 0, 92, 40, 46, *bf);
			break;

		case v_audio_playerstate_stopped:
		case v_audio_playerstate_init:
		case v_audio_playerstate_loaded:

			AlphaBlend(mdc, 37, 36, 40, 46, basic_icons_dc, 0, 46, 40, 46, *bf);
			break;

		case v_audio_playerstate_buffering:
			AlphaBlend(mdc, 40, 36, png_play_w, png_play_h, ndc, 0, 0, png_play_w, png_play_h, *bf);
			break;

		case v_audio_playerstate_notinit:
			AlphaBlend(mdc, 37, 36, 40, 46, basic_icons_dc, 0, 46, 40, 46, *bf);
			break;
		}
	}else{
		switch(skin.shared->audio.output.getplayerstate())
		{
		case v_audio_playerstate_playingandbuffering:
		case v_audio_playerstate_playing:
			AlphaBlend(mdc, 40-2, 36-2, png_playh_w, png_playh_h, play_hover_dc, 0, 0, png_playh_w, png_playh_h, *bf);
			break;

		case v_audio_playerstate_paused:
			AlphaBlend(mdc, 37, 36, 40, 46, basic_icons_dc, 40, 92, 40, 46, *bf);
			break;

		case v_audio_playerstate_stopped:
		case v_audio_playerstate_init:
		case v_audio_playerstate_loaded:

			AlphaBlend(mdc, 37, 36, 40, 46, basic_icons_dc, 40, 46, 40, 46, *bf);
			break;

		case v_audio_playerstate_buffering:
			AlphaBlend(mdc, 40-2, 36-2, png_playh_w, png_playh_h, play_hover_dc, 0, 0, png_playh_w, png_playh_h, *bf);
			break;

		case v_audio_playerstate_notinit:
			AlphaBlend(mdc, 37, 36, 40, 46, basic_icons_dc, 40, 46, 40, 46, *bf);
			break;
		}
	}
}


int  media_get_item(int relation)
{
	struct fennec_audiotag  ctag;
	letter        fname[v_sys_maxpath];
	string        fpath;
	unsigned long id, cid;

	next_artist[0] = 0;
	next_title[0]  = 0;
	next_album[0]  = 0;

	cid = skin.shared->audio.output.playlist.getcurrentindex();

	if(cid + relation > skin.shared->audio.output.playlist.getcount()) return 0;
	if(cid + relation < 0) return 0;



	fpath = skin.shared->audio.output.playlist.getsource(skin.shared->audio.output.playlist.getrealindex(skin.shared->audio.output.playlist.getlistindex(cid) + relation));
	
	if(!fpath)return 0;

	id = skin.shared->audio.input.tagread(fpath, &ctag);

	if(ctag.tag_artist.tsize)
		str_ncpy(next_artist, ctag.tag_artist.tdata, sizeof(next_artist) / sizeof(letter));

	if(ctag.tag_album.tsize)
		str_ncpy(next_album, ctag.tag_album.tdata, sizeof(next_album) / sizeof(letter));

	if(ctag.tag_title.tsize)
		str_ncpy(next_title, ctag.tag_title.tdata, sizeof(next_title) / sizeof(letter));

	if(ctag.tag_year.tsize)
		str_ncpy(next_year, ctag.tag_year.tdata, sizeof(next_year) / sizeof(letter));

	if(!ctag.tag_title.tsize)
	{
		_wsplitpath(fpath, 0, 0, fname, 0);
		str_ncpy(next_title, fname, sizeof(next_title) / sizeof(letter));
	}

	str_cpy(next_album_year, next_album);
	str_cat(next_album_year, uni(" - "));
	str_cat(next_album_year, next_year);
	

	skin.shared->audio.input.tagread_known(id, 0, &ctag);
	return 1;
}


void neo_display_update()
{
	HRGN trgn;
	SIZE  txtsz;
	letter dur_str[32], pos_str[64];


	
	misc_time_to_string((int)(skin.shared->audio.output.getduration_ms() / 1000), dur_str);
	misc_time_to_string((int)(skin.shared->audio.output.getposition_ms() / 1000), pos_str);

	str_cat(pos_str, uni(" / "));
	str_cat(pos_str, dur_str);


	if(display_content_x < -10)
	{
		trgn = CreateRectRgn(105, 7, 105 + 270 + display_content_x, 7 + 106);
		SelectClipRgn(mdc, trgn);
	}else{
		SelectClipRgn(mdc, display_clip);
	}

	if(mouse_down_controller == 1)
	{
		dpoffset_title = dpoffset_artist = dpoffset_album_year = 0;
	}

	typo_print_shadow(mdc, title,  116 + display_content_x - dpoffset_title, 10 + display_content_y, text_color[0], typo_song_title);
	
	if(dpoffset_update){
		GetTextExtentPoint32(mdc, title, (int)str_len(title), &txtsz);
		dpoffset_max_title = txtsz.cx - 270;
	}

	typo_print_shadow(mdc, artist, 116 + display_content_x - dpoffset_artist, 40 + display_content_y, text_color[0], typo_song_artist);
	
	if(dpoffset_update){
		GetTextExtentPoint32(mdc, artist, (int)str_len(artist), &txtsz);
		dpoffset_max_artist = txtsz.cx - 270;
	}

	typo_print_shadow(mdc, album_year,  116 + display_content_x - dpoffset_album_year, 62 + display_content_y, text_color[1], typo_song_album);
	
	if(dpoffset_update){
		GetTextExtentPoint32(mdc, album_year, (int)str_len(album_year), &txtsz);
		dpoffset_max_album_year = txtsz.cx - 270;
	}

	typo_print_shadow(mdc, pos_str, 116 + display_content_x, 83 + display_content_y, text_color[1], typo_song_position);

	dpoffset_update = 0;


	if(dpoffset_max_title > 0 + 50)
	{
		if(!dpoffset_rollback_title)
		{
			dpoffset_title+=2;
			if(dpoffset_title > dpoffset_max_title)
				dpoffset_rollback_title = 1;
		}else{
			dpoffset_title *= 6;
			dpoffset_title /= 7;
			if(dpoffset_title <= 0)
			{
				dpoffset_rollback_title = 0;
				dpoffset_title = 0;
			}
		}
	}

	if(dpoffset_max_artist > 0 + 50)
	{
		if(!dpoffset_rollback_artist)
		{
			dpoffset_artist+=2;
			if(dpoffset_artist > dpoffset_max_artist)
				dpoffset_rollback_artist = 1;
		}else{
			dpoffset_artist *= 6;
			dpoffset_artist /= 7;
			if(dpoffset_artist <= 0)
			{
				dpoffset_rollback_artist = 0;
				dpoffset_artist = 0;
			}
		}

	}

	if(dpoffset_max_album_year > + 50)
	{
		if(!dpoffset_rollback_album_year)
		{
			dpoffset_album_year+=2;
			if(dpoffset_album_year > dpoffset_max_album_year)
				dpoffset_rollback_album_year = 1;
		}else{
			dpoffset_album_year *= 6;
			dpoffset_album_year /= 7;
			if(dpoffset_album_year <= 0)
			{
				dpoffset_rollback_album_year = 0;
				dpoffset_album_year = 0;
			}
		}

	}





	SelectClipRgn(mdc, 0);

	{
		

		if(display_content_x > 10)
		{
			trgn = CreateRectRgn(105, 7, 105 + display_content_x, 7 + 106);
			SelectClipRgn(mdc, trgn);
		}else{
			SelectClipRgn(mdc, display_clip);
		}

		


		if(display_content_x > 10)
		{
			if(media_get_item(+1))
			{
				typo_print_shadow(mdc, next_title,  116 + display_content_x - 270, 10 + display_content_y, text_color[0], typo_song_title);
				typo_print_shadow(mdc, next_artist, 116 + display_content_x - 270, 40 + display_content_y, text_color[0], typo_song_artist);
				typo_print_shadow(mdc, next_album_year,  116 + display_content_x - 270, 62 + display_content_y, text_color[1], typo_song_album);
				typo_print_shadow(mdc, uni("00:00 / 00:00"), 116 + display_content_x - 270, 83 + display_content_y, text_color[1], typo_song_position);
			}
		}

		if(display_content_x < -10)
		{
			if(media_get_item(-1))
			{
				typo_print_shadow(mdc, next_title,  116 + display_content_x + 270, 10 + display_content_y, text_color[0], typo_song_title);
				typo_print_shadow(mdc, next_artist, 116 + display_content_x + 270, 40 + display_content_y, text_color[0], typo_song_artist);
				typo_print_shadow(mdc, next_album_year,  116 + display_content_x + 270, 62 + display_content_y, text_color[1], typo_song_album);
				typo_print_shadow(mdc, uni("00:00 / 00:00"), 116 + display_content_x + 270, 83 + display_content_y, text_color[1], typo_song_position);
			}
		}

		SelectClipRgn(mdc, 0);
	}
	
	
	


}

void neo_view_small_icons()
{
	if(is_video_playing && settings_data.video.force_hide)
	{
		BLENDFUNCTION bf;

		if(button_video_transparency > 100)button_video_transparency = 100;
		else if(button_video_transparency < 0)button_video_transparency = 0;

		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = (BYTE)(((button_video_transparency * 150) / 100) + 20);

		AlphaBlend(mdc, 249, 70, 30, 30, main_icons_dc, 0, 190, 30, 30, bf);
	}
}


void vis_update()
{
	int i, x = 0, n;
	static int y[32]; /* 97 max */
	static float ss[512];

	skin.shared->audio.output.getfloatbuffer((float*)&ss, 512, (dword)-1);

	beat_level = 0.0f;

	for(i=0; i<22; i++)
	{
		n = (int)(ss[511 / 22 * i] * 97.0f);
		beat_level += (float)fabs(((float)n) / 97.0f);
		//n *= 97;

		y[i] -= 4;
		if(y[i] < 0)y[i] = 0;
		if(n > y[i]) y[i] = n;

		BitBlt(mdc, 109 + x, 108 - y[i], 12, main_h, main_vis_dc, 109 + x, 108 - y[i], SRCCOPY);
		x += 14;
	}

	if(beat_level > 20.0f) beat_level = 20.0f;
	else if(beat_level < 0.0f) beat_level = 0.0f;

}

void menu_display()
{
	BLENDFUNCTION bf;

	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.AlphaFormat = AC_SRC_ALPHA;
	bf.SourceConstantAlpha = 0xff;

	if(mouse_down_controller == 0)
	{
		if(menu_position > 0)
		{
			menu_position /= 2;
			menu_button_shift = 0;

		}else if(-menu_position % 66 > 0){

			menu_position += (-menu_position % 66) / 2;

			menu_button_shift = (-menu_position) / 66;
		}
	}

	SelectClipRgn(mdc, display_clip);

	if(menu_position > -50)
		AlphaBlend(mdc, 120 + menu_position, 33, 66, 51, menuicons_dc, 0, (menu_selected_item == 0 ? 53 : 0), 66, 51, bf);
	
	AlphaBlend(mdc, 120 + menu_position + 66 * 1 + ((66 - 48) / 2), 33, 48, 51, menuicons_dc, 66, (menu_selected_item == 1 ? 53 : 0), 48, 51, bf);
	AlphaBlend(mdc, 120 + menu_position + 66 * 2 + ((66 - 50) / 2), 33, 50, 51, menuicons_dc, 114, (menu_selected_item == 2 ? 53 : 0), 50, 51, bf);
	AlphaBlend(mdc, 120 + menu_position + 66 * 3 + ((66 - 50) / 2), 33, 50, 51, menuicons_dc, 164, (menu_selected_item == 3 ? 53 : 0), 50, 51, bf);
	
	if(menu_position < -50)
		AlphaBlend(mdc, 120 + menu_position + 66 * 4 + ((66 - 52) / 2), 33, 52, 51, menuicons_dc, 214, (menu_selected_item == 4 ? 53 : 0), 52, 51, bf);

	SelectClipRgn(mdc, 0);
}


void eq_update()
{
	int i;
	float preampv; /* preamp value */
	float eb[10];  /* bands */
	letter barvalue[16];
	static string bandhertz[] = {uni("31Hz"), uni("63Hz"), uni("125Hz"), uni("250Hz"), uni("500Hz"), uni("1kHz"), uni("2kHz"), uni("4kHz"), uni("8kHz"), uni("16kHz")};

	preampv = skin.shared->audio.equalizer.get_preamp(eq_channel == -1 ? 0 : eq_channel);
	skin.shared->audio.equalizer.get_bands(eq_channel == -1 ? 0 : eq_channel, 10, eb);

	memset(barvalue, 0, sizeof(barvalue));

	if(eq_current_bar >= 0)
		swprintf(barvalue, 16, uni("%.2fdB"), eb[eq_current_bar]);
	else
		swprintf(barvalue, 16, uni("%.2fdB"), preampv);
	
	preampv /= 12.0f;
	preampv *= 97.0f / 2.0f;
	preampv += 97.0f / 2.0f;
	BitBlt(mdc, 109, (int)(108.0f - preampv), 12, main_h, main_vis_dc, 109, (int)(108.0f - preampv), SRCCOPY);
	BitBlt(mdc, 109, (int)(108.0f - preampv - 2), 12, 4, main_vis_dc, 109, 10, SRCCOPY);

	for(i=0; i<10; i++)
	{
		eb[i] /= 12.0f;
		eb[i] *= (97.0f / 2.0f);
		eb[i] += (97.0f / 2.0f);

		BitBlt(mdc, 109 + ((i + 1) * 14) + 14, (int)(108.0f - eb[i]), 12, main_h, main_vis_dc, 109 + ((i + 1) * 14), (int)(108.0f - eb[i]), SRCCOPY);
		BitBlt(mdc, 109 + ((i + 1) * 14) + 14, (int)(108.0f - eb[i] - 2), 12, 4, main_vis_dc, 109 + ((i + 1) * 14), 10, SRCCOPY);
	}

	if(eq_channel == -1)
		typo_print_shadow(mdc, uni("Universal"),  116 + 170, 17 + 20, text_color[0], typo_song_album);
	else{
		letter chanstr[32];
		str_cpy(chanstr, uni("Channel "));
		_itow(eq_channel + 1, chanstr + 8, 10);
		typo_print_shadow(mdc, chanstr,  116 + 170, 17 + 20, text_color[0], typo_song_album);

	}

	typo_print_shadow(mdc, uni("Equalizer"),  116 + 170, 17, text_color[0], typo_song_artist);
	
	typo_print_shadow(mdc, barvalue,  116 + 170, 17 + 40, text_color[0], typo_song_album);

	if(eq_current_bar >= 0)
		typo_print_shadow(mdc, bandhertz[eq_current_bar],  116 + 170, 17 + 60, text_color[0], typo_song_album);
	else
		typo_print_shadow(mdc, uni("Preamp"),  116 + 170, 17 + 60, text_color[0], typo_song_album);

}


void eq_menu()
{
	HMENU   hmen;
	int     i, r;
	letter  cname[255];
	letter  ibuf[10];
	POINT   pt;

	GetCursorPos(&pt);
	hmen = CreatePopupMenu();

	AppendMenu(hmen, MF_STRING, 0, uni("Universal"));
	if(eq_channel == -1)
			CheckMenuItem(hmen, 0, MF_BYPOSITION | MF_CHECKED);

	for(i=0; i<max_channels; i++)
	{
		memset(ibuf, 0, sizeof(ibuf));

		str_cpy(cname, uni("Channel "));
		str_itos(i + 1, ibuf, 10);
		str_cat(cname, ibuf);
		AppendMenu(hmen, MF_STRING, i + 1, cname);

		if(eq_channel == i)
			CheckMenuItem(hmen, i + 1, MF_BYPOSITION | MF_CHECKED);

	}

	r = (int)TrackPopupMenu(hmen, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, 0);
	
	switch(r)
	{
	case 0:
		eq_channel = -1;
		break;

	default:
		eq_channel = r - 1;
		break;
	}

	DestroyMenu(hmen);
}


void redraw_main_window()
{
	BLENDFUNCTION blend;
	SIZE          size;
	POINT         ptsrc;

	if(enable_transparency)
	{
		ptsrc.x = 0;
		ptsrc.y = 0;

		size.cx = main_w;
		size.cy = main_h;

		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.AlphaFormat = 0;
		blend.SourceConstantAlpha = (BYTE)((255 * (100 - sleep_alpha)) / 100);
	 
		UpdateLayeredWindow(wnd, NULL, NULL, &size, mdc, &ptsrc, 0, &blend, ULW_ALPHA);
	}else{
		BitBlt(hdc, 0, 0, main_w, main_h, mdc, 0, 0, SRCCOPY);
	}
}



/* Windows specific callbacks -----------------------------------------------*/

void CALLBACK display_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	int delayset = 0;


	if(IsIconic(wnd) || !IsWindowVisible(wnd)) return;
	
	BitBlt(mdc, 105, 0, main_w, main_h, background_dc, 105, 0, SRCCOPY);

	
	switch(skin_mode)
	{
	case mode_eq:
		eq_update();
		break;

	case mode_display:
		
		vis_update();
		neo_display_update();

		neo_view_small_icons();
		break;


	case mode_menu:

		menu_display();
		break;
	}


	if(mouse_down_controller != 1)
	{
		if(display_content_x > 0 || display_content_x < 0)
			display_content_x /= 2;
	}


	if(delay_counter > 10 || play_button_position_set)
	{
		if(mouse_over_controller != 1 && mouse_hover_val <= 0)
		{
			
			BLENDFUNCTION bf;
			double p, vl, vr;

			BitBlt(mdc, 0, 0, 105, main_h, background_dc, 0, 0, SRCCOPY);

			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.AlphaFormat = AC_SRC_ALPHA;
			bf.SourceConstantAlpha = 0xff;

			skin.shared->audio.output.getposition(&p);
			AlphaBlend(mdc, 4, 9, 100, 100, wheel_pos_dc, 0, (int)(p * 34.0) * 100, 100, 100, bf);

			bf.SourceConstantAlpha = 0xaf;
			skin.shared->audio.output.getvolume(&vl, &vr);
			AlphaBlend(mdc, 12, 17, 85, 85, wheel_vol_dc, 0, (int)(vl * 34.0) * 85, 85, 85, bf);
			bf.SourceConstantAlpha = 0xff;

			display_control_button(&bf, 0);
			//AlphaBlend(mdc, 40, 36, png_play_w, png_play_h, ndc, 0, 0, png_play_w, png_play_h, bf);
		
			delayset = 1;
		}

		delay_counter = 0;
	}else{
		delay_counter++;
	}

	
	if(mouse_over_controller == 1)
	{
		BLENDFUNCTION bf;
		double p, vl, vr;

		if(!delayset)
			BitBlt(mdc, 0, 0, 105, main_h, background_dc, 0, 0, SRCCOPY);
		
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = 0xff;

		
		skin.shared->audio.output.getposition(&p);
		AlphaBlend(mdc, 4, 9, 100, 100, wheel_pos_dc, 0, (int)(p * 34.0) * 100, 100, 100, bf);

		bf.SourceConstantAlpha = 0xaf;
		skin.shared->audio.output.getvolume(&vl, &vr);
		AlphaBlend(mdc, 12, 17, 85, 85, wheel_vol_dc, 0, (int)(vl * 34.0) * 85, 85, 85, bf);
		bf.SourceConstantAlpha = 0xff;


		display_control_button(&bf, 0);
		//AlphaBlend(mdc, 40, 36, png_play_w, png_play_h, ndc, 0, 0, png_play_w, png_play_h, bf);


		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = (BYTE)mouse_hover_val;

		mouse_hover_val+=20;
		if(mouse_hover_val > 255) mouse_hover_val = 255;

		display_control_button(&bf, 1);
		//AlphaBlend(mdc, 40-2, 36-2, png_playh_w, png_playh_h, play_hover_dc, 0, 0, png_playh_w, png_playh_h, bf);
	}else if(mouse_hover_val > 0){
		BLENDFUNCTION bf;
		double p, vl, vr;

		if(!delayset)
			BitBlt(mdc, 0, 0, 105, main_h, background_dc, 0, 0, SRCCOPY);

		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = 0xff;

		skin.shared->audio.output.getposition(&p);
		AlphaBlend(mdc, 4, 9, 100, 100, wheel_pos_dc, 0, (int)(p * 34.0) * 100, 100, 100, bf);

		bf.SourceConstantAlpha = 0xaf;
		skin.shared->audio.output.getvolume(&vl, &vr);
		AlphaBlend(mdc, 12, 17, 85, 85, wheel_vol_dc, 0, (int)(vl * 34.0) * 85, 85, 85, bf);
		bf.SourceConstantAlpha = 0xff;

		display_control_button(&bf, 0);
		//AlphaBlend(mdc, 40, 36, png_play_w, png_play_h, ndc, 0, 0, png_play_w, png_play_h, bf);

		
		if(mouse_hover_val > 0)
			mouse_hover_val -= 20;
		if(mouse_hover_val <= 0) mouse_hover_val = 0;

	
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = (BYTE)mouse_hover_val;

		display_control_button(&bf, 1);
		//AlphaBlend(mdc, 40-2, 36-2, png_playh_w, png_playh_h, play_hover_dc, 0, 0, png_playh_w, png_playh_h, bf);

	}
	


	if(main_panel_active == 1)
	{
		int offset = 0;
		BLENDFUNCTION bf; 

		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = 0xff;

		AlphaBlend(mdc, offset + 335, 0, main_panel_w, main_panel_h, main_panel_dc, 0, 0, main_panel_w, main_panel_h, bf);

		//AlphaBlend(mdc, offset + 335+43, 12, main_icons_w/2, main_icons_h, main_icons_dc, 0, 0, main_icons_w/2, main_icons_h, bf);
		
		switch(skin_mode)
		{
		case mode_display:
		case mode_menu:

			if(mouse_over_controller == 2)
				AlphaBlend(mdc, offset + 335+43, 12, 29, 30, main_icons_dc, 29, 0, 29, 30, bf);
			else
				AlphaBlend(mdc, offset + 335+43, 12, 29, 30, main_icons_dc, 0, 0, 29, 30, bf);
		
			if(mouse_over_controller == 3)
				AlphaBlend(mdc, offset + 335+43, 41, 29, 32, main_icons_dc, 29, 29, 29, 32, bf);
			else
				AlphaBlend(mdc, offset + 335+43, 41, 29, 32, main_icons_dc, 0, 29, 29, 32, bf);

			if(mouse_over_controller == 4)
				AlphaBlend(mdc, offset + 335+43, 75, 29, 30, main_icons_dc, 29, 63, 29, 30, bf);
			else
				AlphaBlend(mdc, offset + 335+43, 75, 29, 30, main_icons_dc, 0, 63, 29, 30, bf);

			break;

		case mode_eq:

			if(mouse_over_controller == 2)
				AlphaBlend(mdc, offset + 335+43, 12, 29, 30, main_icons_dc, 29, 0 + 95, 29, 30, bf);
			else
				AlphaBlend(mdc, offset + 335+43+1, 12, 29, 30, main_icons_dc, 0, 0 + 95, 29, 30, bf);
		
			if(mouse_over_controller == 3)
				AlphaBlend(mdc, offset + 335+43, 41, 29, 32, main_icons_dc, 29, 29 + 95, 29, 32, bf);
			else
				AlphaBlend(mdc, offset + 335+43+1, 41, 29, 32, main_icons_dc, 0, 29 + 95, 29, 32, bf);

			if(skin.shared->settings.general->player.equalizer_enable)
			{
				if(mouse_over_controller == 4)
					AlphaBlend(mdc, offset + 335+43, 75, 29, 30, main_icons_dc, 29, 63 + 95, 29, 30, bf);
				else
					AlphaBlend(mdc, offset + 335+43+1, 75, 29, 30, main_icons_dc, 0, 63 + 95, 29, 30, bf);

			}else{
				if(mouse_over_controller == 4)
					AlphaBlend(mdc, offset + 335+43, 75, 29, 32, main_icons_dc, 29, 29 + 95, 29, 32, bf);
				else
					AlphaBlend(mdc, offset + 335+43+1, 75, 29, 32, main_icons_dc, 0, 29 + 95, 29, 32, bf);
			}

			break;
		}



		main_panel_display_time--;
		if(main_panel_display_time <= 0) main_panel_active = 0;
	}

	
	
	if(beat_level > 9.0f)
	{
		BLENDFUNCTION  bf; 
		static int     rnd = 0;
		static int     side = 30;
		int            sw, sh, x, y;
		static float   pi = 3.1415926535897932384626433832795f;

		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bf.SourceConstantAlpha = (BYTE)((beat_level / 20.0f) * 255.0f);

		if(beat_level > 11.0f)
		{
			rnd = rand() % 360;
			

			if(side > 0) side = -30;
			else side = 30;
		}else
			rnd += side;

		sw = rand() % 200;
		if(sw < 40) sw = 40;

		sh = (int)(((float)sparkles_h / sparkles_w) * sw);

		x = (int)(54.0 + sin((float)rnd / 180.0f * pi) * 46.0);
		y = (int)(57.0 + cos((float)rnd / 180.0f * pi) * 46.0);

		AlphaBlend(mdc, x-(sw / 2), y-(sh / 2), sw, sh, sparkles_dc, 0, 0, sparkles_w, sparkles_h, bf);

 		delay_counter = 11;
	}



	//BitBlt(hdc, 0, 0, main_w, main_h, mdc, 0, 0, SRCCOPY);


	button_video_transparency += button_video_transparency_add;

	if(button_video_transparency >= 100)
		button_video_transparency_add = -button_video_transparency_add;
	else if(button_video_transparency <= 0)
		button_video_transparency_add = -button_video_transparency_add;
	

	if(!vid_init && sleep_timeout > 10 && sleep_alpha < window_transparency_amount)
	{
		sleep_alpha++;
		//SetLayeredWindowAttributes(wnd, 0, (255 * (100 - sleep_alpha)) / 100, LWA_ALPHA);
	}else{
		sleep_timeout+=1;
	}

	if(vid_init)
	{
		sleep_timeout = sleep_alpha = 0;
	}

	if(!eq_wait)
		skin_panel_display_time++;

	if(skin_panel_display_time >= 100)
	{
		skin_mode = mode_display;
		skin_panel_display_time = 0;
	}


	redraw_main_window();
}

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

int get_mask_id(int x, int y)
{
	if(x > main_w) return 0;
	if(y > main_h) return 0;
	if(x < 0) return 0;
	if(y < 0) return 0;

	return GetGValue(GetPixel(mask_dc, x, y));
}

int get_mask_value(int x, int y)
{
	if(x > main_w) return 0;
	if(y > main_h) return 0;
	if(x < 0) return 0;
	if(y < 0) return 0;

	return GetRValue(GetPixel(mask_dc, x, y));
}

/*
 * fennec main window.
 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int downid = -1, dx = 0, dy = 0;
	static int downx, downy;
	static double start_p = 0.0;
	static double start_v = 0.0;
	static int  vid_diff_x = 0, vid_diff_y = 0;
	static unsigned long lbuttondown_time = 0;

	switch(msg)
	{
	case WM_MOUSEMOVE:
		dx = LOWORD(lParam), dy = HIWORD(lParam);

		sleep_timeout = sleep_alpha = 0;
		skin_panel_display_time = 0;


		if(skin_mode == mode_menu)
		{
			menu_selected_item = -1;
			if(dy > 33 && dy < 33 + 51)
			{
				int mx = (dx - 120 - menu_position) / 66;

				if(mx >= 0 && mx <= 4) menu_selected_item = mx;
			}
		}

	
		if(mouse_down_controller == 0)
		{
			switch(get_mask_id(dx, dy))
			{
			case mask_id_play:
				mouse_over_controller = 1;
				break;

			case mask_id_close:
				mouse_over_controller = 2;
				break;

			case mask_id_eject:
				mouse_over_controller = 3;
				break;

			case mask_id_maximize:
				mouse_over_controller = 4;
				break;

			default:
				mouse_over_controller = 0;
				break;
			}



			if(dx > 340)
			{
				main_panel_active = 1;
				main_panel_display_time = 100;
				main_panel_slide = 0;
			}else{
				main_panel_active = 0;
			}

		}else{
			mouse_over_controller = 0;
		}

		switch(mouse_down_controller)
		{
		case 1:
			switch(skin_mode)
			{
			case mode_display:
				display_content_x = dx - downx;
				if(display_content_x > 290) display_content_x = 290;
				if(display_content_x < -290) display_content_x = -290;
				break;

			case mode_menu:
				menu_position = dx - downx - (menu_button_shift * 66);
				if(menu_position  > 70) menu_position  = 70;
				if(menu_position  < -70) menu_position  = -70;
				break;

			case mode_eq:
				{
					int ex = dx - 109, ey = dy - (108 - 94 / 2);
					int barwidth = 14;

					if(ex > 0 && ex < 200)
					{
						if(ex < barwidth) /* first bar - preamp */
						{
							int j;
							float v;

							eq_current_bar = -1;

							if(down_button == 2)
							{
								v = 0.0f;
							}else{

								v = ey / (float)(94 / 2);

								if(v > 1.0f)       v = 1.0f;
								else if(v < -1.0f) v = -1.0f;

								v *= -12.0f;
							}


							if(eq_channel == -1)
							{
								for(j=0; j<max_channels; j++)
								{
									skin.shared->audio.equalizer.set_preamp(j, v);
								}
							}else{
								skin.shared->audio.equalizer.set_preamp(eq_channel, v);
							}

						}else if(ex > (barwidth * 2)){

							/* ----------- eq set*/
							
							int barid = ((ex - (barwidth * 2)) / barwidth);
							float v, eb[10];
							int   j;

							if(barid >= 10) break;

							eq_current_bar = barid;

							eq_wait = 1;

							skin.shared->audio.equalizer.get_bands(eq_channel == -1 ? 0 : eq_channel, 10, eb);
							
							if(down_button == 2)
							{
								eb[barid] = 0.0f;
							}else{
								v = ey / (float)(94 / 2);

								if(v > 1.0f)       v = 1.0f;
								else if(v < -1.0f) v = -1.0f;

								eb[barid] = v * -12.0f;
							}
							

							if(eq_channel == -1)
							{
								for(j=0; j<max_channels; j++)
								{
									skin.shared->audio.equalizer.set_bands(j, 10, eb);
								}
							}else{
								skin.shared->audio.equalizer.set_bands(eq_channel, 10, eb);
							}

							

							/*------------- */

						}
					}
				}
				break;
			}
			break;

		case 2:
			//if(!media_init)
			{
				POINT pt;

				GetCursorPos(&pt);
				
				settings_data.main.x = pt.x - downx;
				settings_data.main.y = pt.y - downy;

				SetWindowPos(wnd, 0, pt.x - downx, pt.y - downy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				if(window_vid && vid_init)
				{
					SetWindowPos(window_vid, 0, pt.x - downx - vid_diff_x, pt.y - downy - vid_diff_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			break;

		case 4:
			{
				double v, pi = 3.1415926535897932384626433832795;
				static double lastv = -1.0;

				if(get_mask_id(downx, downy) == mask_id_volume)
				{
					int vx, vy;
					POINT pt;

					GetCursorPos(&pt);
					ScreenToClient(wnd, &pt);

					vx = pt.x - 58;
					vy = pt.y - 58;

					if(vx)
					{
						if(vx < 0)
							v = 0.25 + ((atan((double)vy / (double)vx)) / (2 * pi));
						else
							v = 0.75 + ((atan((double)vy / (double)vx)) / (2 * pi));
					}else{

						if(vy < 0)
							v = 0.5;
						else
							v = 0.0;
					}

					if(v > 0.95)v = 1.0;
					else if(v < 0.05)v = 0.0;

					if(lastv > 0.9 && v < 0.7) break;
					if(lastv < 0.2 && v > 0.4)
						break;
					lastv = v;

					skin.shared->audio.output.setvolume(v, v);
					delay_counter = 100;
				}
			}
			break;

		case 5:
			{
				double v, pi = 3.1415926535897932384626433832795;
				static double lastv = -1.0;

				if(get_mask_id(downx, downy) == mask_id_position)
				{
					int vx, vy;
					POINT pt;

					GetCursorPos(&pt);
					ScreenToClient(wnd, &pt);

					vx = pt.x - 58;
					vy = pt.y - 58;

					if(vx)
					{
						if(vx < 0)
							v = 0.25 + ((atan((double)vy / (double)vx)) / (2 * pi));
						else
							v = 0.75 + ((atan((double)vy / (double)vx)) / (2 * pi));
					}else{

						if(vy < 0)
							v = 0.5;
						else
							v = 0.0;
					}

					if(v > 1.0)v = 1.0;
					else if(v < 0.05)v = 0.0;

					if(lastv > 0.9 && v < 0.7) break;
					if(lastv < 0.2 && v > 0.4)
						break;
					lastv = v;

					skin.shared->audio.output.setposition(v);
					delay_counter = 100;
				}
			}
			break;

		case 3:
			{
				POINT pt;
				int   mx, my;

				GetCursorPos(&pt);
				ScreenToClient(wnd, &pt);

				mx = pt.x; my = pt.y;

				if(downx + 20 < mx || downx - 20 > mx || play_button_position_set == 1)
				{
					if(play_button_position_set != 2)
					{
						double p = 0.0;

						skin.shared->audio.output.getposition(&p);

						if(play_button_position_set != 1) start_p = p;

						//if(downx - mx > 0) p -= 0.01;
						//else               p += 0.01;

						if(downx - mx > 0)
							p = start_p + -(downx - mx - 20) / 200.0;
						else
							p = start_p + -(downx - mx + 20) / 200.0;

						if(p > 1.0)p = 1.0;
						else if(p < 0.0)p = 0.0;

						skin.shared->audio.output.setposition(p);

						play_button_position_set = 1;
					}
				}

				if(downy + 20 < my || downy - 20 > my || play_button_position_set == 2)
				{
					if(play_button_position_set != 1)
					{
						double l = 0.0, r = 0.0;

						skin.shared->audio.output.getvolume(&l, &r);


						//if(downy - my > 0)l += 0.02;
						//else              l -= 0.02;

						if(play_button_position_set != 2)start_v = l;


						//if(downx - mx > 0) p -= 0.01;
						//else               p += 0.01;

						if(downy - my > 0)
							l = start_v + (downy - my) / 100.0;
						else
							l = start_v + (downy - my) / 100.0;

						if(l > 1.0)l = 1.0;
						else if(l < 0.0)l = 0.0;

						skin.shared->audio.output.setvolume(l, l);

						play_button_position_set = 2;
					}
				}
			}
			break;
		}
		break;


	case WM_RBUTTONUP:

		down_button = 0;
		ReleaseCapture();
		mouse_down_controller = 0;


		switch(get_mask_id(downx, downy))
		{
		case mask_id_display:
			if(skin_mode != mode_eq)
			{
				if(downy < 60)
					skin.shared->general.show_tageditor(0, 0, 0);
				else
					SendMessage(hwnd, (WM_USER + 123), 0, WM_RBUTTONUP);
			}
			break;
		
		case mask_id_eject:
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

		case mask_id_play:
			skin.shared->audio.output.stop();
			break;
		
		default:
			SendMessage(hwnd, (WM_USER + 123), 0, WM_RBUTTONUP);
			break;
		}
		break;

	case WM_RBUTTONDOWN:
		down_button = 2;

	case WM_LBUTTONDOWN:
		downx = LOWORD(lParam), downy = HIWORD(lParam);

		if(down_button == 0) down_button = 1;

		if(window_vid && vid_init)
		{
			RECT rct, rctm;

			GetWindowRect(hwnd, &rctm);
			GetWindowRect(window_vid, &rct);
			
			vid_diff_x = rctm.left - rct.left;
			vid_diff_y = rctm.top - rct.top;
		}


		switch(get_mask_id(downx, downy))
		{
		case mask_id_display:
			mouse_down_controller = 1;
			break;

		case mask_id_play:
			mouse_down_controller = 3;
			break;

		case mask_id_volume:
			mouse_down_controller = 4;
			break;

		case mask_id_position:
			mouse_down_controller = 5;
			break;

		case mask_id_close:
			mouse_down_controller = 6;
			break;

		case mask_id_eject:
			mouse_down_controller = 7;
			break;

		case mask_id_maximize:
			mouse_down_controller = 8;
			break;

		
		default:
			mouse_down_controller = 2;
		}

		play_button_position_set = 0;

		SetCapture(wnd);
		break;

	case WM_LBUTTONUP:
		
		down_button = 0;
		ReleaseCapture();

		if(GetTickCount() - lbuttondown_time < GetDoubleClickTime())
		{
			search_create(wnd);
		}

		lbuttondown_time = GetTickCount();


		if(skin_mode == mode_display && settings_data.video.force_hide && is_video_playing)
		{
			POINT pt;

			GetCursorPos(&pt);
			ScreenToClient(wnd, &pt);

			if(downx > 242 && downx < 242 + 30 && downy > 70 && downy < 70 + 30)
			{
				if(pt.x > 242 && pt.x < 242 + 30 && pt.y > 70 && pt.y < 70 + 30)
				{
					settings_data.video.force_hide = 0;
				
					vid_create(skin.wnd, (double)last_video_dsize / 1000000);

					if(settings_data.vis.show_vis && !settings_data.vis.video_when_available)
					{

					}else{
						vis_used = 0;
						skin.shared->call_function(call_videoout_initialize, 0, 0, 0);
					}
				}
			}
		}

		switch(mouse_down_controller)
		{
			case 1:
				{
					POINT pt;

					GetCursorPos(&pt);
					ScreenToClient(wnd, &pt);

					if(downy - pt.y > 50)
					{
						if(skin_mode < mode_count - 1) skin_mode++;
						skin_panel_display_time = 0;

					}else if(downy - pt.y < -50){

						if(skin_mode > 0) skin_mode--;
						skin_panel_display_time = 0;
					}

					eq_wait = 0;
				}


				if(skin_mode == mode_menu)
				{
					switch(menu_selected_item)
					{
					case 0: /* conversion */
						skin.shared->general.show_conversion(0, 0, 0);
						break;

					case 1: /* joining */
						skin.shared->general.show_joining(0, 0, 0);
						break;

					case 2: /* ripping */
						skin.shared->general.show_ripping(0, 0, 0);
						break;

					case 3: /* visualization */
						if(settings_data.vis.show_vis)
						{
							vid_close();
							vis_active = 0;
							vis_used = 0;
							settings_data.vis.show_vis = 0;

						}else{
							
							vis_used = 1;
							vis_active = 1;
							settings_data.vis.show_vis = 1;
							vid_create(wnd, 16.0 / 9.0);

							if(is_video_playing && !settings_data.vis.video_when_available)
							{
								if(!settings_data.video.force_hide)
								{
									vis_used = 0;
									skin.shared->call_function(call_videoout_initialize, 0, 0, 0);
								}else{
									str_cpy(skin.shared->settings.general->visualizations.selected, settings_data.vis.current_vis);
									skin.shared->call_function(call_visualizations_select_next, 0, 0, 0);
								}
							}else{
								str_cpy(skin.shared->settings.general->visualizations.selected, settings_data.vis.current_vis);
								skin.shared->call_function(call_visualizations_select_next, 0, 0, 0);
							}
						}
						break;

					case 4: /* settings */
						skin.shared->general.show_settings(0, 0, 0);
						break;
					}

				}
				break;

			case 6:
				switch(skin_mode)
				{
				case mode_menu:
				case mode_display:
					SendMessage(wnd, WM_DESTROY, 0, 0);
					break;

				case mode_eq:
					eq_wait = 1;
					eq_menu();
					eq_wait = 0;
					break;
				}
				break;

			case 7:
				switch(skin_mode)
				{
				case mode_menu:
				case mode_display:
					skin.shared->simple.show_openfile();
					break;

				case mode_eq:
					eq_wait = 1;
					skin.shared->simple.show_equalizer_presets((void*)wnd);
					eq_wait = 0;
					break;
				}
				break;

			case 8:
				switch(skin_mode)
				{
				case mode_menu:
				case mode_display:
					if(media_init)
						media_close();
					else
						media_create(hwnd);

					settings_data.playlist.visible = media_init;
					break;

				case mode_eq:
					skin.shared->settings.general->player.equalizer_enable ^= 1;
					break;
				}
				break;

			case 4:
			{
				double v, pi = 3.1415926535897932384626433832795;
				static double lastv = -1.0;

				if(get_mask_id(downx, downy) == mask_id_volume)
				{
					int vx, vy;
					POINT pt;

					GetCursorPos(&pt);
					ScreenToClient(wnd, &pt);

					vx = pt.x - 58;
					vy = pt.y - 58;

					if(vx)
					{
						if(vx < 0)
							v = 0.25 + ((atan((double)vy / (double)vx)) / (2 * pi));
						else
							v = 0.75 + ((atan((double)vy / (double)vx)) / (2 * pi));
					}else{

						if(vy < 0)
							v = 0.5;
						else
							v = 0.0;
					}

					if(v > 0.95)v = 1.0;
					else if(v < 0.05)v = 0.0;

					if(lastv > 0.9 && v < 0.7) break;
					if(lastv < 0.2 && v > 0.4)
						break;
					lastv = v;

					skin.shared->audio.output.setvolume(v, v);
					delay_counter = 100;
				}
			}
			break;

			case 5:
			{
				double v, pi = 3.1415926535897932384626433832795;
				static double lastv = -1.0;

				if(get_mask_id(downx, downy) == mask_id_position)
				{
					int vx, vy;
					POINT pt;

					GetCursorPos(&pt);
					ScreenToClient(wnd, &pt);

					vx = pt.x - 58;
					vy = pt.y - 58;

					if(vx)
					{
						if(vx < 0)
							v = 0.25 + ((atan((double)vy / (double)vx)) / (2 * pi));
						else
							v = 0.75 + ((atan((double)vy / (double)vx)) / (2 * pi));
					}else{

						if(vy < 0)
							v = 0.5;
						else
							v = 0.0;
					}

					if(v > 1.0)v = 1.0;
					else if(v < 0.05)v = 0.0;

					if(lastv > 0.9 && v < 0.7) break;
					if(lastv < 0.2 && v > 0.4)
						break;
					lastv = v;

					skin.shared->audio.output.setposition(v);
					delay_counter = 100;
				}
			}
			break;
		}

		if(skin_mode == mode_display)
		{
			if(display_content_x > 100)
			{
				skin.shared->audio.output.playlist.next();
				display_content_x = display_content_x - 270;
			}

			if(display_content_x <= -100)
			{
				skin.shared->audio.output.playlist.previous();
				display_content_x = display_content_x + 270;
			}
		}


		if(mouse_down_controller == 3) /* play button */
		{
			if(!play_button_position_set)
				skin.shared->audio.output.play();

		}

		
		mouse_down_controller = 0;
		break;

	case WM_LBUTTONDBLCLK:
		//downx = LOWORD(lParam), downy = HIWORD(lParam);

		//if(downx > 105)
		//{
			skin.shared->simple.show_openfile();
		//}else{
		//	skin.shared->audio.output.stop();
		//}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT  ps;
			BLENDFUNCTION bf;
			double        p;
			double        vl, vr;

			BeginPaint(hwnd, &ps);

			BitBlt(mdc, 0, 0, main_w, main_h, background_dc, 0, 0, SRCCOPY);

			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.AlphaFormat = AC_SRC_ALPHA;
			bf.SourceConstantAlpha = 0xff;

			
			display_control_button(&bf, 0);

			
			skin.shared->audio.output.getposition(&p);
			AlphaBlend(mdc, 4, 9, 100, 100, wheel_pos_dc, 0, (int)(p * 34.0) * 100, 100, 100, bf);

			bf.SourceConstantAlpha = 0xaf;
			skin.shared->audio.output.getvolume(&vl, &vr);
			AlphaBlend(mdc, 12, 17, 85, 85, wheel_vol_dc, 0, (int)(vl * 34.0) * 85, 85, 85, bf);


			neo_display_update();

			//BitBlt(hdc, 0, 0, main_w, main_h, mdc, 0, 0, SRCCOPY);

			EndPaint(hwnd, &ps);
		}
		break;

	}
	return 1;
}

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
