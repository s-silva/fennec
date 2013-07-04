/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.3
 Copyright (C) 2011 Chase <c-h@users.sf.net>

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
#include "jpeg/jpegdec.h"
#include "skin settings.h"
#include <shlobj.h>

void reselect_wallpaper(void);

extern HWND  window_media;
extern HDC   hdc_media;
extern int   media_init;
extern int   playlist_refresh_request;
extern int   enable_transparency;
extern int   window_transparency_amount;
extern int   sleep_alpha;

HDC      hdc_fview = 0;
HBITMAP  hbm_fview;

HDC   fv_sheet = 0;
int   fv_sheet_w, fv_sheet_h;

HDC   fv_background = 0;
int   fv_background_w, fv_background_h;

int   hover_id = 0;
int   last_hover_id = 0;
int   fullview_timer_enable = 1;

int   fullview_return_to_library = 0;

int  fullview_drawex(int v);

letter    full_view_text_location[260];
letter    full_view_text_search[512];

HFONT     full_view_font_default = 0;
HWND      hwnd_searchtext;
WNDPROC   searchtext_wndproc;

int       fullview_switching_manually = 0;

unsigned int timer_fullview = 0;
int       fullview_cancel_dragging = 0;

int    view_mode = -1;

int       view_shift = 0;

int		scroll_value = 0, scroll_max = 10;
int		fv_scrollbar_show = 0;

enum
{
	button_id_play = 1,
	button_id_volume,
	button_id_position,
	button_id_back,
	button_id_next,
	button_id_shuffle,
	button_id_repeat,
	button_id_switching,
	button_id_import,
	button_id_export,
	button_id_minimize,
	button_id_restore,
	button_id_close,
	button_id_up,
	button_id_search,
	button_id_settings,
	button_id_hidemain,
	button_id_preferences,
	button_id_ripping,
	button_id_joining,
	button_id_conversion

};

void CALLBACK fullview_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


int  fullview_init(void)
{
	letter    skin_path[512], skin_ap[512];
	RECT      rct;

	GetClientRect(window_media, &rct);
	hbm_fview = CreateCompatibleBitmap(hdc_media, rct.right, rct.bottom);
	hdc_fview = CreateCompatibleDC(hdc_media);

	SelectObject(hdc_fview, hbm_fview);

	skin.shared->general.getskinspath(skin_path, sizeof(skin_path));
	str_cat(skin_path, uni("/neo/"));

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("fullviewsheet.png"));
	fv_sheet   = png_get_hdc(skin_ap, 0);
	fv_sheet_w = png_w;
	fv_sheet_h = png_h;


	fv_background = 0;
	reselect_wallpaper();

	/* set text */

	hwnd_searchtext = CreateWindowW(uni("EDIT"), uni(""), WS_CHILD, 0, 0, 10, 10, window_media, 0, skin.finstance, 0);
	searchtext_wndproc = (WNDPROC)GetWindowLongPtr(hwnd_searchtext, GWLP_WNDPROC);
	SetWindowLongPtr(hwnd_searchtext, GWLP_WNDPROC, (LONG_PTR)callback_searchbox);

	full_view_font_default = CreateFont(-12,
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, uni("Verdana"));

	SetBkMode(hdc_fview, TRANSPARENT);

	SendMessage(hwnd_searchtext, WM_SETFONT, (WPARAM)full_view_font_default, (LPARAM)0);
	
	str_cpy(full_view_text_search, uni("Search"));

	fullview_switching_manually = 1;
	fullview_switch(settings_data.main.media_window_mode);
	fullview_switching_manually = 0;
	
	timer_fullview = (unsigned int)SetTimer(0, 0, 35, (TIMERPROC) fullview_timer);
	return 0;
}

int  fullview_uninit(void)
{
	fullview_switch(-1);

	KillTimer(0, timer_fullview);

	if(fv_sheet)
		DeleteDC(fv_sheet);

	if(fv_background)
		DeleteDC(fv_background);

	if(hdc_fview)
	{
		DeleteObject(hbm_fview);
		DeleteDC(hdc_fview);
		hdc_fview = 0;
	}

	fv_background = 0;
	fv_sheet      = 0;
	return 0;
}

void fullview_refresh(int inum)
{
	if(view_mode == vmode_playlist)
		playlist_refresh(inum);
	else
		playlist_refresh_request = 1;
}


void fullview_switch(int vmode)
{
	if(view_mode != -1)
	{
		if(vmode == -1)
			fullview_switching_manually = 1;

		switch(view_mode)
		{
		case vmode_playlist:
			playlist_uninit();
			break;

		case vmode_media_intro:
			mintro_uninit();
			break;

		case vmode_dv:
			dv_uninit();
			break;

		case vmode_library:
			lib_uninit();
			break;
		}

		if(vmode == -1)
			fullview_switching_manually = 0;
	}
	if(vmode != -1)
	{
		switch(vmode)
		{
		case vmode_playlist:
			str_cpy(full_view_text_location, uni("Playlist - Now Playing"));
			playlist_init(hdc_fview);
			break;

		case vmode_media_intro:
			str_cpy(full_view_text_location, uni("Introduction to Media Library"));
			mintro_init(hdc_fview);
			break;

		case vmode_dv:
			str_cpy(full_view_text_location, uni("Root"));
			dv_init(hdc_fview);
			break;

		case vmode_library:
			str_cpy(full_view_text_location, uni("Root"));
			lib_init(hdc_fview);
			break;
		}

	}

	view_mode = vmode;

	if(vmode != -1)
		settings_data.main.media_window_mode = vmode;
}

void reselect_wallpaper(void)
{
	letter    skin_path[512], skin_ap[512];
	RECT      rct;

	GetClientRect(window_media, &rct);

	skin.shared->general.getskinspath(skin_path, sizeof(skin_path));
	str_cat(skin_path, uni("/neo/"));

	if(fv_background)
		DeleteDC(fv_background);

	
	if(settings_data.playlist.wallpaper[0])
	{
		fv_background_w = rct.right;
		fv_background_h = rct.bottom;
		fv_background = (HDC) jpeg_get(settings_data.playlist.wallpaper, &fv_background_w, &fv_background_h);
	
	}else{ /* default wallpaper */

		fv_background_w = rct.right;
		fv_background_h = rct.bottom;
		str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("wallpaper.jpg"));
		fv_background = (HDC) jpeg_get(skin_ap, &fv_background_w, &fv_background_h);
	}

	if(!fv_background || fv_background_w <= 0 || fv_background_h <= 0)
	{
		fv_background_w = rct.right;
		fv_background_h = rct.bottom;
		str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("wallpaper.jpg"));
		fv_background = (HDC) jpeg_get(skin_ap, &fv_background_w, &fv_background_h);
	}

	if(!fv_background || fv_background_w <= 0 || fv_background_h <= 0)
	{
		HDC     dc;
		HBITMAP hbm;

		dc  = CreateCompatibleDC(hdc_fview);
		hbm = CreateCompatibleBitmap(hdc_fview, rct.right, rct.bottom);
		SelectObject(dc, hbm);

		drawrect(dc, 0, 0, rct.right, rct.bottom, 0xaaaaaa);

		DeleteObject(hbm);
		fv_background = dc;
		fv_background_w = rct.right;
		fv_background_h = rct.bottom;
		return;
	}

	if(fv_background_w < rct.right || fv_background_h < rct.bottom)
	{

		if(fv_background_w > rct.right / 2 && fv_background_h > rct.bottom / 2) /* stretch */
		{
			HDC     dc;
			HBITMAP hbm;
			float   aspect = 1.0f;

			dc  = CreateCompatibleDC(hdc_fview);
			hbm = CreateCompatibleBitmap(hdc_fview, rct.right, rct.bottom);
			SelectObject(dc, hbm);

			SetStretchBltMode(dc, HALFTONE);
			SetBrushOrgEx(dc, 0, 0, 0);

			

			if(fv_background_h > fv_background_w)
			{
				aspect = (float)rct.bottom / (float)rct.right;
				StretchBlt(dc, 0, 0, rct.right, rct.bottom, fv_background, 0, 0, fv_background_w, (int)((float)fv_background_w * aspect), SRCCOPY);
			}else{
				aspect = (float)rct.right / (float)rct.bottom;
				StretchBlt(dc, 0, 0, rct.right, rct.bottom, fv_background, 0, 0, (int)((float)fv_background_h * aspect), fv_background_h,  SRCCOPY);
			}

			DeleteObject(hbm);
			fv_background = dc;
			fv_background_w = rct.right;
			fv_background_h = rct.bottom;

		}else{
			HDC     dc;
			HBITMAP hbm;
			int     i, j;

			dc  = CreateCompatibleDC(hdc_fview);
			hbm = CreateCompatibleBitmap(hdc_fview, rct.right, rct.bottom);
			SelectObject(dc, hbm);

			for(j=0; j < (rct.bottom/fv_background_h) + 1; j++)
			{
				for(i=0; i < (rct.right/fv_background_w) + 1; i++)
				{
					BitBlt(dc, i*fv_background_w, j*fv_background_h, fv_background_w, fv_background_h, fv_background, 0, 0, SRCCOPY);
				}
			}

			DeleteObject(hbm);
			fv_background = dc;
			fv_background_w = rct.right;
			fv_background_h = rct.bottom;
		}
	}
}

int  fullview_keymsg(int key)
{
	if(key == VK_TAB)
	{
		if(view_mode == vmode_playlist)
			fullview_switch(vmode_media_intro);
		else
			fullview_switch(vmode_playlist);

		fullview_drawgeneral();
		return 1;
	}

	if(view_mode == vmode_playlist)
		playlist_keymsg(key);
	else if(view_mode == vmode_dv)
		dv_keymsg(key);
	else if(view_mode == vmode_library)
		lib_keymsg(key);

	return 1;
}


void fullview_draw_scrollbar(HDC dc, int x, int y, int h, int value, int max)
{
	int i = 0, barsz, scspace, v;

	for(i=0; i<h-177; i+=177)
		alpha_blit(dc, fv_sheet, x, y + i, 42, 177, 345, 1, 50);

	if(h % 177)
		alpha_blit(dc, fv_sheet, x, y + i, 42, h % 177, 345, 1, 50);

	y += 5;
	h -= 10;

	barsz = h / max;
	if(barsz < 30) barsz = 30;

	scspace = h - barsz;
	v = (scspace * value) / max;

	for(i=0; i<barsz-177; i+=177)
		alpha_blit(dc, fv_sheet, x + 5, y + i + v, 42 - 10, 177, 345, 1, 80);

	if(h % 177)
		alpha_blit(dc, fv_sheet, x + 5, y + i + v, 42 - 10, barsz % 177, 345, 1, 80);


}

int  fullview_mousemsg(int x, int y, int action)
{
	static int dx = 0;
	static int dy = 0;
	static int ldown = 0;
	static int ldownx = 0;
	static int rdown = 0;
	static int playlist_moved = 0;
	static int playlist_downv = 0;
	static int playlist_downvy = 0;
	static int scroll_down = 0;

	static int xp_last_vol = 10 + 44 + 10;
	static int w_last_vol  = 200;
	static int w_last_pos  = 0;
	static int xp_last_pos = 10 + 44 + 10 + 200;

	RECT window_size;

	GetClientRect(window_media, &window_size);


	w_last_pos = (window_size.right - 342 - 46) - xp_last_pos;

	if(action == mm_down_l)
	{
		fv_scrollbar_show = 1;
		if(incoord(x, y, window_size.right - 45, 85, 42, window_size.bottom - 95 - 35))
		{
			scroll_down = 1;
			return 0;
		}
	}

	if(action == mm_move && scroll_down)
	{
		int v = y - 85, nv;

		nv = (int)(((double)v / (double)(window_size.bottom - 95 - 35)) * (double)scroll_max);

		if(nv != scroll_value)
		{

			scroll_value = nv;

			if(scroll_value < 0)scroll_value = 0;
			else if(scroll_value >= scroll_max) scroll_value = scroll_max - 1;
			

			switch(view_mode)
			{
			case vmode_playlist: playlist_scroll(scroll_value); break;
		
			}

			
			fullview_drawgeneral();
		}
		return 0;

	}else if(action == mm_move){
		
		if(incoord(x, y, window_size.right - 45, 85, 42, window_size.bottom - 95 - 35))
		{
			fv_scrollbar_show = 1;
			fullview_drawgeneral();
		}else{
			if(fv_scrollbar_show)
			{
				fv_scrollbar_show = 0;
				fullview_drawgeneral();
			}else{
				fv_scrollbar_show = 0;
			}
		}

	}

	if(action == mm_up_l) {fv_scrollbar_show = 0; scroll_down = 0; }


	/* hide search text window if it's visible */

	if(action == mm_up_l)
	{
		int ovmode = view_mode;

		ldown = 0;
		fullview_timer_enable = 1;


		fullview_switching_manually = 1;

		if(view_shift > 200)
		{
			if(view_mode == vmode_playlist)
				fullview_switch(vmode_media_intro);
			else if(view_mode == vmode_dv)
			{
				if(mode_ml)
				{
					ml_in_dir = 0;
					ml_pl_startid = 0;
					mode_ml = 0;
					ml_cache_uninit();
					ml_cache_init();
				}
				fullview_switch(vmode_playlist);
			}else if(view_mode == vmode_library)
				fullview_switch(vmode_dv);
		}

		
		if(view_shift < -200)
		{
			if(view_mode == vmode_media_intro)
			{
				if(mode_ml)
				{
					ml_in_dir = 0;
					ml_pl_startid = 0;
					mode_ml = 0;
					ml_cache_uninit();
					ml_cache_init();
				}
				fullview_switch(vmode_playlist);

			}else if(view_mode == vmode_playlist)
				fullview_switch(vmode_dv);
			else if(view_mode == vmode_dv)
				fullview_switch(vmode_library);
		}

		fullview_switching_manually = 0;

		if(ovmode != view_mode)
		{
			fullview_timer_enable = 1;
			view_shift = 0;
			ldown = 0;
			fullview_drawgeneral();
		}


		ShowWindow(hwnd_searchtext, SW_HIDE);
	}

	if(action == mm_down_l)
	{
		ldownx = x;
		ldown = 1;
	}

	/* <playlist stuff> */

	if(view_mode == vmode_playlist)
	{
		if(action == mm_down_r) playlist_moved = 0;

		if(action == mm_move && rdown)
		{
			playlist_scroll_x = playlist_downv - (x - dx);

			ml_pl_startid = playlist_downvy - ((y - dy) / view_bar_h);
			if(ml_pl_startid < 0) ml_pl_startid = 0;

			if(playlist_scroll_x < 0) playlist_scroll_x = 0;
			if(x - dx > 10 || x - dx < -10 || y - dy < -view_bar_h || y - dy > view_bar_h) playlist_moved = 1;
			playlist_redraw(-1);
			return 0;
		}

		if(action == mm_up_r && playlist_moved)
		{
			rdown = 0;
			return 0; /* skip popup menus */
		}
	}

	/* </playlist stuff> */


	if(view_mode == vmode_media_intro)
	{
		mintro_mousemsg(x, y, action);
	}else if(view_mode == vmode_dv){

		dv_mousemsg(x, y, action);
	}else if(view_mode == vmode_library){

		lib_mousemsg(x, y, action);
	}

	if(action == mm_move)
	{
		if(ldown && (ldownx < 20 || ldownx > window_size.right - 20))
		{
			view_shift = x - ldownx;
			fullview_timer_enable = 0;

			if(view_shift)
				fullview_drawgeneral();
		}

		if(incoord(x, y, window_size.right - 40, 0, 30, 33))
		{
			hover_id = button_id_close;
			BitBlt(hdc_media, window_size.right - 40, 0, 30, 33, fv_sheet, 53, 69, SRCCOPY);
		}

		if(incoord(x, y, window_size.right - 68, 0, 26, 33))
		{
			hover_id = button_id_restore;
			BitBlt(hdc_media, window_size.right - 68, 0, 26, 33, fv_sheet, 26, 69, SRCCOPY);
		}

		if(incoord(x, y, window_size.right - 94, 0, 26, 33))
		{
			hover_id = button_id_minimize;
			BitBlt(hdc_media, window_size.right - 94, 0, 26, 33, fv_sheet,  0, 69, SRCCOPY);
		}

		if(incoord(x, y,window_size.right - 120, 5, 24, 24))
		{
			hover_id = button_id_hidemain;
			alpha_blit(hdc_media, fv_sheet, window_size.right - 120    , 5, 24, 24, 96 + 120, 294, 100);
		}

		if(incoord(x, y, window_size.right - 146 - 5, 5, 24, 24))
		{
			hover_id = button_id_preferences;
			alpha_blit(hdc_media, fv_sheet, window_size.right - 146 - 5, 5, 24, 24, 72 + 120, 294, 100);
		}

		if(incoord(x, y, window_size.right - 172 - 5, 5, 24, 24))
		{
			hover_id = button_id_ripping;
			alpha_blit(hdc_media, fv_sheet, window_size.right - 172 - 5, 5, 24, 24, 48 + 120, 294, 100);
		}

		if(incoord(x, y, window_size.right - 198 - 5, 5, 24, 24))
		{
			hover_id = button_id_joining;
			alpha_blit(hdc_media, fv_sheet, window_size.right - 198 - 5, 5, 24, 24, 24 + 120, 294, 100);
		}

		if(incoord(x, y, window_size.right - 224 - 5, 5, 24, 24))
		{
			hover_id = button_id_conversion;
			alpha_blit(hdc_media, fv_sheet, window_size.right - 224 - 5, 5, 24, 24,  0 + 120, 294, 100);
		}

		if(last_hover_id != hover_id)
		{
			switch(last_hover_id)
			{
			case button_id_close:       BitBlt(hdc_media, window_size.right - 40, 0, 30, 33, fv_sheet, 53, 35, SRCCOPY); break;
			case button_id_restore:     BitBlt(hdc_media, window_size.right - 68, 0, 26, 33, fv_sheet, 26, 35, SRCCOPY); break;
			case button_id_minimize:    BitBlt(hdc_media, window_size.right - 94, 0, 26, 33, fv_sheet,  0, 35, SRCCOPY); break;
			case button_id_hidemain:    alpha_blit(hdc_media, fv_sheet, window_size.right - 120    , 5, 24, 24, 96, 294, 100); break;
			case button_id_preferences: alpha_blit(hdc_media, fv_sheet, window_size.right - 146 - 5, 5, 24, 24, 72, 294, 100); break;
			case button_id_ripping:     alpha_blit(hdc_media, fv_sheet, window_size.right - 172 - 5, 5, 24, 24, 48, 294, 100); break;
			case button_id_joining:     alpha_blit(hdc_media, fv_sheet, window_size.right - 198 - 5, 5, 24, 24, 24, 294, 100); break;
			case button_id_conversion:  alpha_blit(hdc_media, fv_sheet, window_size.right - 224 - 5, 5, 24, 24,  0, 294, 100); break;
			}

			last_hover_id = hover_id;
		}
	}


	if(action == mm_up_l)
	{
		if(incoord(x, y, window_size.right - 40, 0, 30, 33)) /* close */
		{
			SendMessage(wnd, WM_DESTROY, 0, 0);

		}else if(incoord(x, y, window_size.right - 68, 0, 26, 33)){ /* restore */

			media_close();
			settings_data.playlist.visible = 0;
			if(!IsWindowVisible(wnd)) /* should not hide both */
				ShowWindow(wnd, SW_SHOW);
			
		}else if(incoord(x, y, window_size.right - 94, 0, 26, 33)){ /* minimize */

			ShowWindow(window_media, SW_MINIMIZE);

		}else if(incoord(x, y,window_size.right - 120, 5, 24, 24)){

			if(IsWindowVisible(wnd))
				ShowWindow(wnd, SW_HIDE);
			else
				ShowWindow(wnd, SW_SHOW);

		}else if(incoord(x, y, window_size.right - 146 - 5, 5, 24, 24)){

			skin.shared->general.show_settings(0, 0, 0);

		}else if(incoord(x, y, window_size.right - 172 - 5, 5, 24, 24)){

			skin.shared->general.show_ripping(0, 0, 0);
		
		}else if(incoord(x, y, window_size.right - 198 - 5, 5, 24, 24)){

			skin.shared->general.show_joining(0, 0, 0);
		
		}else if(incoord(x, y, window_size.right - 224 - 5, 5, 24, 24)){

			skin.shared->general.show_conversion(0, 0, 0);


		}else if(incoord(x, y,  window_size.right - 10 - 430, 42, 430, 23)){ /* search box */

			SetWindowPos(hwnd_searchtext, 0, window_size.right - 10 - 430 + 6, 42 + 4, 430 - 12 - 30, 23 - 8, SWP_NOZORDER);
			ShowWindow(hwnd_searchtext, SW_SHOW);
			SendMessage(hwnd_searchtext, EM_SETSEL, 0, -1);
			SetFocus(hwnd_searchtext);
		}else if(incoord(x, y, 10, 42, window_size.right - 10 - 430 - 20, 23)){ /* location bar */

			fullview_switching_manually = 1;

			if(view_mode == vmode_dv)
				fullview_keymsg(VK_BACK);
			else if(view_mode == vmode_library)
				fullview_keymsg(VK_BACK);
			else if(view_mode == vmode_playlist && fullview_return_to_library)
				fullview_switch(vmode_library);
				

			fullview_switching_manually = 0;

			fullview_drawgeneral();
		}
	}


	if(y > window_size.bottom - 35)
	{
		if(action == mm_move)
		{
			int xp = 10;

			/* play/pause/stop */
			if(x > xp && x < xp + 44) {hover_id = button_id_play; goto pos_found_match;}
			xp += 44;

			/* volume */
			xp += 10;
			if(x > xp && x < xp + 200) {hover_id = button_id_volume; goto pos_found_match;}
			xp += 200;

			/* position */

			if(x > xp && x < window_size.right - 342 - 46) {hover_id = button_id_position; goto pos_found_match;}
			xp = window_size.right - 342 - 46;

			/* control set */

			xp += 10;

			if(x > xp && x < xp + 46) {hover_id = button_id_back; goto pos_found_match;};
			xp += 46;
			if(x > xp && x < xp + 46) {hover_id = button_id_next; goto pos_found_match;}
			xp += 46;

			/* shuffle, repeat, auto-switching */

			if(x > xp && x < xp + 46) {hover_id = button_id_shuffle; goto pos_found_match;}
			xp += 46;

			if(x > xp && x < xp + 46) {hover_id = button_id_repeat; goto pos_found_match;}
			xp += 46;

			if(x > xp && x < xp + 46) {hover_id = button_id_switching; goto pos_found_match;}
			xp += 46;


			/* media library set */

			if(x > xp && x < xp + 46) {hover_id = button_id_import; goto pos_found_match;}
			xp += 46;
			if(x > xp && x < xp + 46) {hover_id = button_id_export; goto pos_found_match;}
			xp += 46;

			if(x > xp && x < xp + 46) {hover_id = button_id_settings; goto pos_found_match;}
			xp += 46;

			return 0;
			pos_found_match:

			if(hover_id != last_hover_id)
			{
				fullview_drawex(1);
				last_hover_id = hover_id;
			}
			return 1;

		}else if(action == mm_up_l){

			switch(hover_id)
			{
			case button_id_play:
				skin.shared->audio.output.play();
				fullview_drawex(1);
				break;

			case button_id_volume:
				{
					double vl = 0.0, vr = 0.0;

					int v;

					v = (x - xp_last_vol);

					vl = (double)v / (double) w_last_vol;

					if(vl < 0.0) vl = 0.0;
					else if(vl > 1.0) vl = 1.0;

					vr = vl;

					skin.shared->audio.output.setvolume(vl, vr);
				}
				fullview_drawex(1);
				break;

			case button_id_position:
				{
					double p = 0.0;
					int v;

					v = (x - xp_last_pos);
					p = (double)v / (double) w_last_pos;

					if(p < 0.0) p = 0.0;
					else if(p > 1.0) p = 1.0;

					skin.shared->audio.output.setposition(p);
				}
				fullview_drawex(1);
				break;


			case button_id_shuffle:
				skin.shared->audio.output.playlist.setshuffle(skin.shared->settings.general->player.playlist_shuffle ^ 1, 1);
				playlist_redraw(-1);
				fullview_drawex(1);
				break;

			case button_id_next:
				skin.shared->audio.output.playlist.next();
				playlist_redraw(-2);
				fullview_drawex(1);
				break;

			case button_id_back:
				skin.shared->audio.output.playlist.previous();
				playlist_redraw(-2);
				fullview_drawex(1);
				break;

			case button_id_switching:
				skin.shared->settings.general->player.auto_switching ^= 1;
				fullview_drawex(1);
				break;

			case button_id_export:
				{
					POINT pt;
					HMENU mc = user_create_menu(menu_ml_save, mode_ml);
					
					GetCursorPos(&pt);

					switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0))
					{
					case mid_savepl:
						skin.shared->simple.show_save_playlist();
						break;

					case mid_import:
						skin.shared->mlib.media_library_advanced_function(6 /* import */, 0, 0);
						break;

					case mid_export:
						skin.shared->mlib.media_library_advanced_function(7 /* export */, 0, 0);
						break;
					}

					DestroyMenu(mc);
				}
				break;

			case button_id_import:
				{
					POINT pt;
					HMENU mc = user_create_menu(menu_ml_add, mode_ml);
					
					GetCursorPos(&pt);

					switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0))
					{
					case mid_addfiles:
						skin.shared->simple.show_addfile();
						break;
					case mid_add_dir:
						skin.shared->simple.show_addfolder();
						break;
					case mid_add_mlib:
						
						{
							letter        fpath[v_sys_maxpath];
							BROWSEINFO    bi;
							LPITEMIDLIST  lpi;

							fpath[0] = 0;

							bi.hwndOwner      = window_media;
							bi.lpszTitle      = uni("Add to Media Library");
							bi.pszDisplayName = fpath;
							bi.lpfn           = 0;
							bi.iImage         = 0;
							bi.lParam         = 0;
							bi.pidlRoot       = 0;
							bi.ulFlags        = BIF_RETURNONLYFSDIRS;
				
							lpi = SHBrowseForFolder(&bi);
							SHGetPathFromIDList(lpi, fpath);

							if(view_mode == vmode_dv || view_mode == vmode_library)
								dv_halt();

							if(str_len(fpath))
							{
								skin.shared->mlib.media_library_add_dir(fpath);
							}

							ml_cache_uninit();
							ml_cache_init();

							if(view_mode == vmode_dv || view_mode == vmode_library)
								dv_halt_resume();

							fullview_drawgeneral();
						}
						break;
					case mid_removeml:
						if(view_mode == vmode_dv || view_mode == vmode_library)
							dv_halt();

						skin.shared->mlib.media_library_advanced_function(5 /* clear media library */, 0, 0);
						ml_cache_uninit();
						ml_cache_init();

						if(view_mode == vmode_dv || view_mode == vmode_library)
							dv_halt_resume();
						fullview_drawgeneral();
						break;
					}

					DestroyMenu(mc);
				}
				break;

			case button_id_repeat:
				{
					POINT pt;
					HMENU mc = user_create_menu(menu_ml_repeat, mode_ml);
					
					GetCursorPos(&pt);

					if(skin.shared->settings.general->player.playlist_repeat_list)
						CheckMenuItem(mc, mid_repeat_list, MF_CHECKED);

					
					if(skin.shared->settings.general->player.playlist_repeat_single)
						CheckMenuItem(mc, mid_repeat_track, MF_CHECKED);

					switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0))
					{
					case mid_repeat_list:
						skin.shared->settings.general->player.playlist_repeat_list ^= 1;
						fullview_drawex(1);
						break;
					case mid_repeat_track:
						skin.shared->settings.general->player.playlist_repeat_single ^= 1;
						fullview_drawex(1);
						break;
					}

					DestroyMenu(mc);
				}
				break;

			case button_id_settings:
				{
					POINT pt;
					HMENU mc = user_create_menu(menu_ml_settings, mode_ml);
					
					GetCursorPos(&pt);

					switch((int)TrackPopupMenu(mc, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0))
					{
					case mid_settings_wallpaper_def:
					case mid_settings_wallpaper_ld:
						memset(settings_data.playlist.wallpaper, 0, sizeof(settings_data.playlist.wallpaper));
						reselect_wallpaper();
						fullview_drawex(0);
						break;

					case mid_settings_wallpaper_sel:
						{
							OPENFILENAME   ofn;
							letter         ifile[v_sys_maxpath];

							memset(ifile, 0, sizeof(ifile));
							memset(&ofn, 0, sizeof(ofn));

							ofn.lStructSize     = sizeof(ofn);
							ofn.lpstrTitle      = uni("Select Wallpaper");
							ofn.lpstrFile       = ifile;
							ofn.nMaxFile        = sizeof(ifile);
							ofn.lpstrFilter     = uni("JPEG files (*.jpg)\0*.jpg\0\0");
							ofn.hInstance       = skin.finstance;

							GetOpenFileName(&ofn);
							
							if(str_len(ifile) > 3)
							{
								str_cpy(settings_data.playlist.wallpaper, ifile);
								reselect_wallpaper();

								if(!fv_background)
								{
									memset(settings_data.playlist.wallpaper, 0, sizeof(settings_data.playlist.wallpaper));
									reselect_wallpaper();
								}

								fullview_drawex(0);
							}
						}
						break;

					case mid_settings_display_big:
						settings_data.playlist.display_mode = playlist_display_big;
						if(view_mode == vmode_playlist)
							playlist_set_sizes();
						else if(view_mode == vmode_dv)
							dv_set_sizes();
						else if(view_mode == vmode_library)
							lib_set_sizes();
							
						fullview_drawex(0);
						break;

					case mid_settings_display_small:
						settings_data.playlist.display_mode = playlist_display_small;
						if(view_mode == vmode_playlist)
							playlist_set_sizes();
						else if(view_mode == vmode_dv)
							dv_set_sizes();
						else if(view_mode == vmode_library)
							lib_set_sizes();
							
						fullview_drawex(0);
						break;

					case mid_settings_transparency_no:
						if(!enable_transparency) break;
						enable_transparency = 0;

						SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
						break;

					case mid_settings_transparency_10:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 10;
						break;

					case mid_settings_transparency_20:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 20;
						break;

					case mid_settings_transparency_30:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 30;
						break;

					case mid_settings_transparency_40:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 40;
						break;

					case mid_settings_transparency_50:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 50;
						break;

					case mid_settings_transparency_60:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 60;
						break;

					case mid_settings_transparency_70:
						if(!enable_transparency) SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						enable_transparency = 1;
						sleep_alpha = window_transparency_amount = 70;
						break;


					case mid_settings_vis_showvideo:
						settings_data.vis.video_when_available ^= 1;
						break;

					case mid_settings_covers_download:
						settings_data.advanced.auto_download_covers ^= 1;
						fullview_drawgeneral();
						break;

					case mid_settings_covers_albums:
						settings_data.advanced.enable_album_art ^= 1;
						fullview_drawgeneral();
						break;

					case mid_settings_covers_photos:
						settings_data.advanced.enable_artist_photo ^= 1;
						fullview_drawgeneral();
						break;
					}

					DestroyMenu(mc);
				}
				break;

			}
			return 0;
		}
	}

	/* <playlist stuff> */

	if(view_mode == vmode_playlist)
	{
		if(action == mm_down_r)
		{
			playlist_downv = playlist_scroll_x;
			playlist_downvy = ml_pl_startid;
			dx = x; dy = y; rdown = 1;
		}else if(action == mm_up_r){
			rdown = 0;
		}

		if(action != mm_wheel)
		{
			if(x > 22 && y > 84 && x < window_size.right - 44 && y < window_size.bottom - 84 - 45)
			{
				return playlist_mousemsg(x - 22, y - 84, action);
			}else{

			}
		}else{
			return playlist_mousemsg(x, y, action);
		}

	}
	
	/* </playlist stuff> */

	return 0;
}



void text_block_out(HDC dc, int x, int y, int bx, int by, int w, int h, const string text)
{
	HRGN rgn = CreateRectRgn(bx, by, bx + w, by + h);
	SelectClipRgn(dc, rgn);
	TextOut(dc, x, y, text, str_len(text));
	SelectClipRgn(dc, 0);
}


int  fullview_drawex(int v)
{
	RECT window_size;
	int  xp, bbalpha = 35;

	GetClientRect(window_media, &window_size);

	/* draw top bar */

	if(v == 0)
	{

		drawrect(hdc_fview, 0, 0, window_size.right, 74, 0);

		drawbar(hdc_fview, fv_sheet, 10, 0, window_size.right - 20, 34, 0, 0, 140, 141, 0, 95, 0, 35, 83, 100);

		drawbar(hdc_fview, fv_sheet, window_size.right - 10 - 430, 42, 430, 23, 84, 35, 4, 88, 35, 59, 173, 35, 24, 100);
		
		drawbar(hdc_fview, fv_sheet, 10, 42, window_size.right - 10 - 430 - 20, 23, 84, 35, 4, 88, 35, 59, 147, 35, 24, 100);



		/* draw wallpaper [fix: wallpaper must be in same size or bigger than the window] */

		BitBlt(hdc_fview, 0, 74, window_size.right, window_size.bottom - 74, fv_background, 0, 0, SRCCOPY);
		
	}else{

		BitBlt(hdc_fview, 0, window_size.bottom - 35, window_size.right, 35, fv_background, 0, window_size.bottom - 35 - 74, SRCCOPY);
	}

	/* bottom control bar */

	xp = 0;
	alpha_blit(hdc_fview, fv_sheet, 0, window_size.bottom - 35, 10, 35, 232, 71, bbalpha);
	xp += 10;

	/* play/pause/stop */
	{
		int pstate = skin.shared->audio.output.getplayerstate();
		switch(pstate)
		{
		case v_audio_playerstate_playing:
		case v_audio_playerstate_playingandbuffering:
			alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 44, 35, 0, 179 + (hover_id == button_id_play ? 36 : 0), bbalpha);
			break;

		case v_audio_playerstate_paused:
		case v_audio_playerstate_buffering:
			alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 44, 35, 46, 179 + (hover_id == button_id_play ? 36 : 0), bbalpha);
			break;

		default:
			alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 44, 35, 92, 179 + (hover_id == button_id_play ? 36 : 0), bbalpha);
			break;
		}

	}
	
	xp += 44;

	/* volume */
	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 10, 35, 232, 71, bbalpha);
	xp += 10;
	drawbar(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 200, 34, 262, 71, 10, 272, 71, 55, 327, 71, 12, bbalpha);
	{
		double vl = 0.0, vr = 0.0;

		skin.shared->audio.output.getvolume(&vl, &vr);
		drawbar(hdc_fview, fv_sheet, xp, window_size.bottom - 35, (int)(200.0 * vl), 34, 262, 71 - 35, 10, 272, 71 - 35, 55, 327, 71 - 35, 12, bbalpha);
	}

	xp += 200;

	/* position */

	drawbar(hdc_fview, fv_sheet, xp, window_size.bottom - 35, window_size.right - 342 - 46 - xp, 34, 262, 71, 10, 272, 71, 55, 327, 71, 12, bbalpha);
	{
		double p;

		skin.shared->audio.output.getposition(&p);
		drawbar(hdc_fview, fv_sheet, xp, window_size.bottom - 35, (int)((double)(window_size.right - 342 - 46 - xp) * p), 34, 262, 71 - 35, 10, 272, 71 - 35, 55, 327, 71 - 35, 12, bbalpha);
	}

	xp = window_size.right - 342 - 46;


	/* control set */

	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 10, 35, 232, 71, bbalpha);
	xp += 10;

	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 0, 107 + (hover_id == button_id_back ? 36 : 0), bbalpha);
	xp += 46;
	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 46, 107 + (hover_id == button_id_next ? 36 : 0), bbalpha);
	xp += 46;

	/* shuffle, repeat, auto-switching */

	if(skin.shared->settings.general->player.playlist_shuffle)
		alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 92,  71, bbalpha);
	else
		alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 92, 107 + (hover_id == button_id_shuffle ? 36 : 0), bbalpha);
	
	xp += 46;

	if(skin.shared->settings.general->player.playlist_repeat_single || skin.shared->settings.general->player.playlist_repeat_list)
		alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 138,  71, bbalpha);
	else
		alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 138, 107 + (hover_id == button_id_repeat ? 36 : 0), bbalpha);

	xp += 46;

	if(skin.shared->settings.general->player.auto_switching)
		alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 184,  71, bbalpha);
	else
		alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 184, 107 + (hover_id == button_id_switching ? 36 : 0), bbalpha);

	xp += 46;


	/* media library set */

	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 230, 107 + (hover_id == button_id_import ? 36 : 0), bbalpha);
	xp += 46;
	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 276, 107 + (hover_id == button_id_export ? 36 : 0), bbalpha);
	xp += 46;

	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, 46, 35, 0 + (hover_id == button_id_settings ? 45 : 0), 251, bbalpha);
	xp += 46;


	/* ending */
	alpha_blit(hdc_fview, fv_sheet, xp, window_size.bottom - 35, window_size.right - xp, 35, 232, 71, bbalpha);
	


	/* texts for boxes */

	if(v == 0)
	{
		COLORREF cr = GetTextColor(hdc_fview);
		HFONT    hfontold = (HFONT) SelectObject(hdc_fview, full_view_font_default);
		SetTextColor(hdc_fview, 0xdbdbdb);
		
		text_block_out(hdc_fview, 16, 46, 10, 42, window_size.right - 10 - 430 - 20, 23, full_view_text_location);
		text_block_out(hdc_fview, window_size.right - 10 - 430 + 6, 42 + 4, window_size.right - 10 - 430, 42, 430, 23, full_view_text_search);

		SetTextColor(hdc_fview, cr);
		SelectObject(hdc_fview, hfontold);
	


		/* main buttons */

		if(hover_id == button_id_close)
			BitBlt(hdc_fview, window_size.right - 40, 0, 30, 33, fv_sheet, 53, 69, SRCCOPY);
		if(hover_id == button_id_restore)
			BitBlt(hdc_fview, window_size.right - 68, 0, 26, 33, fv_sheet, 26, 69, SRCCOPY);
		if(hover_id == button_id_minimize)
			BitBlt(hdc_fview, window_size.right - 94, 0, 26, 33, fv_sheet,  0, 69, SRCCOPY);

		if(hover_id == button_id_hidemain)
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 120    , 5, 24, 24, 96 + 120, 294, 100);
		else
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 120    , 5, 24, 24, 96, 294, 100);
		
		if(hover_id == button_id_preferences)
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 146 - 5, 5, 24, 24, 72 + 120, 294, 100);
		else
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 146 - 5, 5, 24, 24, 72, 294, 100);
		
		if(hover_id == button_id_ripping)
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 172 - 5, 5, 24, 24, 48 + 120, 294, 100);
		else
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 172 - 5, 5, 24, 24, 48, 294, 100);
		
		if(hover_id == button_id_joining)
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 198 - 5, 5, 24, 24, 24 + 120, 294, 100);
		else
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 198 - 5, 5, 24, 24, 24, 294, 100);
		
		if(hover_id == button_id_conversion)
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 224 - 5, 5, 24, 24,  0 + 120, 294, 100);
		else
			alpha_blit(hdc_fview, fv_sheet, window_size.right - 224 - 5, 5, 24, 24,  0, 294, 100);

	}

	/* draw playlist */

	if(v == 0)
	{
		if(view_mode == vmode_playlist)
			playlist_view(hdc_fview, 22, 84, window_size.right - 44, window_size.bottom - 84 - 45, -1);
		else if(view_mode == vmode_media_intro)
			mintro_draw(hdc_fview, window_size.right, window_size.bottom);
		else if(view_mode == vmode_dv)
			dv_draw(hdc_fview, window_size.right, window_size.bottom);
		else if(view_mode == vmode_library)
			lib_draw(hdc_fview, window_size.right, window_size.bottom);


		if(view_shift > 0)
			drawrect(hdc_media, 0, 0, view_shift, window_size.bottom, 0);
		else if(view_shift < 0)
			drawrect(hdc_media, window_size.right - -view_shift, 0, -view_shift, window_size.bottom, 0);


		if(fv_scrollbar_show && scroll_max > 1)
			fullview_draw_scrollbar(hdc_fview, window_size.right - 45, 85, window_size.bottom - 95 - 35, scroll_value, scroll_max);

		BitBlt(hdc_media, view_shift, 0, window_size.right, window_size.bottom, hdc_fview, 0, 0, SRCCOPY);
	}else{

		BitBlt(hdc_media, view_shift, window_size.bottom - 35, window_size.right, 35, hdc_fview, 0, window_size.bottom - 35, SRCCOPY);
	}
	return 0;
}

int  fullview_drawgeneral(void)
{
	fullview_drawex(0);
	return 1;
}


void playlist_redraw(int item)
{
	RECT window_size;

	if(view_mode != vmode_playlist) return;

		
	GetClientRect(window_media, &window_size);

	if(item == -1)
	{
		BitBlt(hdc_fview, 22, 84, window_size.right - 44, window_size.bottom - 84 - 45, fv_background, 22, 84 - 74, SRCCOPY);
		playlist_view(hdc_fview, 22, 84, window_size.right - 44, window_size.bottom - 84 - 45, item);
		BitBlt(hdc_media, view_shift + 22, 84, window_size.right - 44, window_size.bottom - 84 - 45, hdc_fview, 22, 84, SRCCOPY);

	}else if(item == -2){ /* only draw markers */

		BitBlt(hdc_fview, 22, 84, 31, window_size.bottom - 84 - 45, fv_background, 22, 84 - 74, SRCCOPY);
		playlist_view(hdc_fview, 22, 84, window_size.right - 44, window_size.bottom - 84 - 45, item);
		BitBlt(hdc_media, view_shift + 22, 84, 31, window_size.bottom - 84 - 45, hdc_fview, 22, 84, SRCCOPY);
	}
}

void fullview_render(int x, int y, int w, int h)
{
	BitBlt(hdc_media, view_shift + x, y, w, h, hdc_fview, x, y, SRCCOPY);
}

void fullview_clear(int x, int y, int w, int h)
{
	BitBlt(hdc_fview, x, y, w, h, fv_background, x, y - 74, SRCCOPY);
}



void CALLBACK fullview_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	int k = view_shift;
	static int vu = 0;

	vu++;

	if(vu > 10)
	{
		fullview_drawex(1);
		vu = 0;
	}

	if(!fullview_timer_enable) return;

	view_shift /= 2;

	if(view_shift || k != view_shift)
		fullview_drawgeneral();
}







/**----------------------------------------------------------------------------
 eof.
----------------------------------------------------------------------------**/