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

#include <math.h>
#include "skin.h"




/* defines ------------------------------------------------------------------*/

#define equalizer_sbutton_sx      41
#define equalizer_sbutton_sy      165
#define equalizer_sbutton_w       4
#define equalizer_sbutton_h       3

#define equalizer_button_mode     0x1
#define equalizer_button_presets  0x2
#define equalizer_button_reset    0x3
#define equalizer_button_power    0x4
#define equalizer_button_close    0x5
#define equalizer_button_curve    0x6
#define equalizer_button_linear   0x7
#define equalizer_button_single   0x8

#define scroll_preamp             0
#define scroll_band_start         1

#define eq_window_class uni("fennec.skin.3.eq")




/* prototypes ---------------------------------------------------------------*/

LRESULT CALLBACK callback_eq_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);




/* data ---------------------------------------------------------------------*/

int   eq_mode    = 0;  /* equalization mode (single/curve/linear) */
int   eq_channel = -1; /* -1 - universal, 0 - left... */
int   eq_init    = 0;  /* initialized? */
HWND  window_eq  = 0;  /* equalizer window */
HDC   hdc_eq;         /* equalizer window dc */
HRGN  rgn_eq;         /* equalizer window rgn */

const RECT coord_scrolls[] = {
	/* x, y, w, h */
	{8,   8, 6, 39}, /* preamp */
	{28,  8, 6, 39}, /* band 1 */
	{44,  8, 6, 39}, /* band 2 */
	{59,  8, 6, 39}, /* band 3 */
	{74,  8, 6, 39}, /* band 4 */
	{89,  8, 6, 39}, /* band 5 */
	{104, 8, 6, 39}, /* band 6 */
	{119, 8, 6, 39}, /* band 7 */
	{134, 8, 6, 39}, /* band 8 */
	{149, 8, 6, 39}, /* band 9 */
	{164, 8, 6, 39}, /* band 10 */
};




/* code ---------------------------------------------------------------------*/


/*
 * create and display equalizer window.
 */
void eq_create(HWND hwndp)
{
	WNDCLASS wndc;

	if(!skin_settings.eq_show) return;
	if(eq_init) return;

	wndc.style         = CS_HREDRAW | CS_VREDRAW;
	wndc.lpfnWndProc   = (WNDPROC)callback_eq_window;
	wndc.cbClsExtra    = 0;
	wndc.cbWndExtra    = 0;
	wndc.hInstance     = instance_skin;;
	wndc.hIcon         = LoadIcon(skin.finstance, (LPCTSTR)0);
	wndc.hCursor       = LoadCursor(0, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndc.lpszMenuName  = 0;
	wndc.lpszClassName = eq_window_class;

	RegisterClass(&wndc);
	
	window_eq = CreateWindow(eq_window_class, uni("Equalizer"), WS_POPUP, skin_settings.eq_x, skin_settings.eq_y, cr(coords.window_eq.width), cr(coords.window_eq.height), hwndp, 0, instance_skin, 0);
	
	setwinpos_clip(window_eq, 0, skin_settings.eq_x, skin_settings.eq_y, cr(coords.window_eq.width), cr(coords.window_eq.height), SWP_NOSIZE | SWP_NOZORDER);


	rgn_eq = CreateRoundRectRgn(0, 0, cr(coords.window_eq.width), cr(coords.window_eq.height), cr(5), cr(5));
	SetWindowRgn(window_eq, rgn_eq, 1);

	ShowWindow(window_eq, SW_SHOW);										  
	UpdateWindow(window_eq);											  

	eq_init = 1;
}


/*
 * close equalizer window.
 */
void eq_close(void)
{
	if(!eq_init) return;
	
	DeleteDC(hdc_eq);
	DestroyWindow(window_eq);
	eq_init = 0;
}


/*
 * refresh equalizer window.
 */
void eq_refresh(int lv)
{
	if(!eq_init)return;

	SendMessage(window_eq, WM_PAINT, 0, 0);
}


/*
 * (re)draw equalizer window.
 */
void eq_draw(void)
{
	float preampv; /* preamp value */
	float eb[10];  /* bands */

	/* draw background */


	StretchBlt(hdc_eq, 0, 0, cr(coords.window_eq.width), cr(coords.window_eq.height), mdc_sheet, coords.window_eq.background_sx, coords.window_eq.background_sy, coords.window_eq.width, coords.window_eq.height, SRCCOPY);
	//BitBlt(hdc_eq, 0, 0, equalizer_window_w, equalizer_window_h, mdc_sheet, equalizer_window_sx, equalizer_window_sy, SRCCOPY);
	
	/* draw preamp */

	preampv = skin.shared->audio.equalizer.get_preamp(eq_channel == -1 ? 0 : eq_channel);

	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[0], &coords.window_eq.bandbutton, -preampv / 24.0f);
	//BitBlt(hdc_eq, 8   + 1, (8 + (39 / 2)) + (int)(preampv * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	
	skin.shared->audio.equalizer.get_bands(eq_channel == -1 ? 0 : eq_channel, 10, eb);

	/* draw band buttons */

	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[1], &coords.window_eq.bandbutton,  -eb[0] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[2], &coords.window_eq.bandbutton,  -eb[1] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[3], &coords.window_eq.bandbutton,  -eb[2] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[4], &coords.window_eq.bandbutton,  -eb[3] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[5], &coords.window_eq.bandbutton,  -eb[4] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[6], &coords.window_eq.bandbutton,  -eb[5] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[7], &coords.window_eq.bandbutton,  -eb[6] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[8], &coords.window_eq.bandbutton,  -eb[7] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[9], &coords.window_eq.bandbutton,  -eb[8] / 24.0f);
	blt_button_on_coord_vb(hdc_eq, mdc_sheet, &coords.window_eq.bands[10], &coords.window_eq.bandbutton, -eb[9] / 24.0f);

	
	/*
	BitBlt(hdc_eq, 28  + 1, (8 + (39 / 2)) + (int)(eb[0] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 44  + 1, (8 + (39 / 2)) + (int)(eb[1] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 59  + 1, (8 + (39 / 2)) + (int)(eb[2] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 74  + 1, (8 + (39 / 2)) + (int)(eb[3] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 89  + 1, (8 + (39 / 2)) + (int)(eb[4] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 104 + 1, (8 + (39 / 2)) + (int)(eb[5] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 119 + 1, (8 + (39 / 2)) + (int)(eb[6] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 134 + 1, (8 + (39 / 2)) + (int)(eb[7] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 149 + 1, (8 + (39 / 2)) + (int)(eb[8] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	BitBlt(hdc_eq, 164 + 1, (8 + (39 / 2)) + (int)(eb[9] * -1.625) - 2, equalizer_sbutton_w, equalizer_sbutton_h, mdc_sheet, equalizer_sbutton_sx, equalizer_sbutton_sy, SRCCOPY);
	*/

	/* others */

	if(skin.shared->settings.general->player.equalizer_enable)
		blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_power_on);
	else
		blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_power_off);

	/* mode */
	if(eq_channel == -1)
		blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_channel_u);
	else
		blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_channel_s);


	blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_presets);
	blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_reset);
}


/*
 * get button index by position.
 */
int eq_get_button_index(int x, int y)
{
	if(incoordx(x, y, &coords.window_eq.button_channel_u))return equalizer_button_mode;
	if(incoordx(x, y, &coords.window_eq.button_presets))return equalizer_button_presets;
	if(incoordx(x, y, &coords.window_eq.button_reset))return equalizer_button_reset;
	if(incoordx(x, y, &coords.window_eq.button_power_off))return equalizer_button_power;
	if(incoordx(x, y, &coords.window_eq.button_exit))return equalizer_button_close;
	if(incoordx(x, y, &coords.window_eq.button_curve))return equalizer_button_curve;
	if(incoordx(x, y, &coords.window_eq.button_linear))return equalizer_button_linear;
	if(incoordx(x, y, &coords.window_eq.button_single))return equalizer_button_single;
	
	return 0;
}


/*
 * draw button (normal state).
 */
void eq_draw_normal(int idx)
{
	switch(idx)
	{
	case equalizer_button_close:  blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_exit);   break;
	case equalizer_button_curve:  blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_curve);  break;
	case equalizer_button_linear: blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_linear); break;
	case equalizer_button_single: blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_single); break;

	case equalizer_button_mode:
		if(eq_channel == -1)
			blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_channel_u);
		else
			blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_channel_s);
		break;

	case equalizer_button_presets: blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_presets); break;
	case equalizer_button_reset:   blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_reset); break;
	
	case equalizer_button_power:
		if(skin.shared->settings.general->player.equalizer_enable)
			blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_power_on);
		else
			blt_coord(hdc_eq, mdc_sheet, 0, &coords.window_eq.button_power_off);
		break;
	}
}


/*
 * draw button (hover).
 */
void eq_draw_hover(int idx)
{
	static int last_idx = 0;

	switch(idx)
	{
	case equalizer_button_close:   show_tip(oooo_skins_close, 0);                break;
	case equalizer_button_curve:   show_tip(oooo_skins_equalizer_curve, 0);      break;
	case equalizer_button_linear:  show_tip(oooo_skins_equalizer_linear, 0);     break;
	case equalizer_button_single:  show_tip(oooo_skins_equalizer_singleband, 0); break;
	case equalizer_button_mode:    show_tip(oooo_skins_equalizer_mode, 0);       break;
	case equalizer_button_presets: show_tip(oooo_skins_equalizer_presets, 0);    break;
	case equalizer_button_reset:   show_tip(oooo_skins_equalizer_reset, 0);      break;
	case equalizer_button_power:   show_tip(oooo_skins_equalizer_toggle, 0);     break;
	}



	if(idx == last_idx)return;

	switch(idx)
	{
	case equalizer_button_close:  blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_exit);   break;
	case equalizer_button_curve:  blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_curve);  break;
	case equalizer_button_linear: blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_linear); break;
	case equalizer_button_single: blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_single); break;

	case equalizer_button_mode:
		if(eq_channel == -1)
			blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_channel_u);
		else
			blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_channel_s);
		break;

	case equalizer_button_presets: blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_presets); break;
	case equalizer_button_reset:   blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_reset); break;
	
	case equalizer_button_power:
		if(skin.shared->settings.general->player.equalizer_enable)
			blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_power_on);
		else
			blt_coord(hdc_eq, mdc_sheet, 1, &coords.window_eq.button_power_off);
		break;
	}

	/* clear out last hover */

	eq_draw_normal(last_idx);

	last_idx = idx;

	return;
}


/*
 * mouse messages.
 */
int eq_mousemsg(int x, int y, int m)
{
	static int rdown = 0, ldown = 0, mdown = 0;
	static int selband = -1;

	if (m == mm_down_r)rdown = 1;
	else if(m == mm_up_r)rdown = 0;

	if (m == mm_down_l)ldown = 1;
	else if(m == mm_up_l)ldown = 0;

	if(m == mm_move)
	{
		int idx = eq_get_button_index(x, y);

		eq_draw_hover(idx);
	}

	if(m == mm_up_l)
	{
		int idx = eq_get_button_index(x, y);

		switch(idx)
		{
		case equalizer_button_close:
			if(skin_settings.skin_lock)
			{
				SendMessage(skin.wnd, WM_DESTROY, 0, 0);
			}else{
				skin_settings.eq_show = 0;
				eq_close();
			}
			break;

		case equalizer_button_mode:
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

				r = (int)TrackPopupMenu(hmen, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_eq, 0);
				
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
				
				eq_draw();
			}
			break;

		case equalizer_button_presets:
			skin.shared->simple.show_equalizer_presets((void*)window_eq);
			break;

		case equalizer_button_reset:
			{
				float fbs[10];
				int   i, j;

				for(i=0; i<10; i++)
					fbs[i] = 0.0f;

				if(eq_channel == -1)
				{
					for(j=0; j<max_channels; j++)
					{
						skin.shared->audio.equalizer.set_bands(j, 10, fbs);
						skin.shared->audio.equalizer.set_preamp(j, 0.0f);
					}
				}else{
					skin.shared->audio.equalizer.set_bands(eq_channel, 10, fbs);
					skin.shared->audio.equalizer.set_preamp(eq_channel, 0.0f);
				}

				eq_draw();
			}
			break;

		case equalizer_button_single:
			eq_mode = 0;
			break;

		case equalizer_button_linear:
			eq_mode = 1;
			break;

		case equalizer_button_curve:
			eq_mode = 2;
			break;

		case equalizer_button_power:
			skin.shared->settings.general->player.equalizer_enable ^= 1;
			eq_draw();
			break;
		}

		return 1;
	}

	if(m == mm_down_l)
	{
		int i;

		for(i=0; i<sizeof(coord_scrolls)/sizeof(coord_scrolls[0]); i++)
		{
			if(incoordx(x, y, &coords.window_eq.bands[i]))
			{
				selband = i;
				return 1;
			}
		}
		return 0;
	}

	if(m == mm_up_l)selband = -1;

	if((m == mm_move) && ldown)
	{
		int i;

		for(i=0; i<sizeof(coord_scrolls)/sizeof(coord_scrolls[0]); i++)
		{
			if(incoordx(x, y, &coords.window_eq.bands[i]))
			{
				selband = i;
				break;
			}
		}

		if(selband >= scroll_band_start)
		{
			float v, eb[10];
			int   i, cb, j, sb_y, sb_h;

			sb_y = cr(coords.window_eq.bands[selband].y);
			sb_h = cr(coords.window_eq.bands[selband].h);

			v = (float)y;
			if(v < sb_y)v = (float)sb_y;
			if(v > sb_y + sb_h)v = (float)(sb_y + sb_h);
			v -= sb_y;
			v -= (sb_h / 2);

			v = (-v * 24.0f) / sb_h;


			skin.shared->audio.equalizer.get_bands(eq_channel == -1 ? 0 : eq_channel, 10, eb);
			
			cb = selband - scroll_band_start;

			eb[selband - scroll_band_start] = v;

			if(eq_mode == 1)
			{

				for(i=0; i<cb; i++)
					eb[i] += (eb[cb] - eb[i]) * 0.15f * ((float)(i + 1)/(float)(cb + 1));

				for(i=cb; i<10; i++)
					eb[i] += (eb[cb] - eb[i]) * 0.15f * ((float)(10 - i)/(float)(10 - cb));


			}else if(eq_mode == 2){


				for(i=0; i<cb; i++)
					eb[i] += (eb[cb] - eb[i]) * 0.8f * (float)pow(((double)(i + 1)/(double)(cb + 1)), 8.0);

				for(i=cb; i<10; i++)
					eb[i] += (eb[cb] - eb[i]) * 0.8f * (float)pow(((double)(10 - i)/(double)(10 - cb)), 8.0);


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

			eq_draw();

		}else if(selband == scroll_preamp){
			
			float v;
			int   j, sb_y, sb_h;

			sb_y = cr(coords.window_eq.bands[selband].y);
			sb_h = cr(coords.window_eq.bands[selband].h);

			v = (float)y;
			if(v < sb_y)v = (float)sb_y;
			if(v > sb_y + sb_h)v = (float)(sb_y + sb_h);
			v -= sb_y;
			v -= (sb_h / 2);

			v = (-v * 24.0f) / sb_h;

			if(eq_channel == -1)
			{
				for(j=0; j<max_channels; j++)
				{
					skin.shared->audio.equalizer.set_preamp(j, v);
				}
			}else{
				skin.shared->audio.equalizer.set_preamp(eq_channel, v);
			}

			eq_draw();
		}
		return 1;
	}

	return 0;
}

void eq_skinchange(void)
{
	if(skin_settings.eq_d)
	{
		if(skin_settings.eq_x == skin_settings.main_x) /* bottom/top */
		{
			if(skin_settings.eq_y > skin_settings.main_y) /* bottom */
				skin_settings.eq_y = skin_settings.main_y + cr(coords.window_main.height);
			else /* top */
				skin_settings.eq_y = skin_settings.main_y - cr(coords.window_eq.height);
		}

		if(skin_settings.eq_y == skin_settings.main_y) /* right/left */
		{
			if(skin_settings.eq_x > skin_settings.main_x) /* right */
				skin_settings.eq_x = skin_settings.main_x + cr(coords.window_main.width);
			else /* left */
				skin_settings.eq_x = skin_settings.main_x - cr(coords.window_eq.width);
		}
	}

	SetWindowRgn(window_eq, 0, 0);

	if(rgn_eq) DeleteObject(rgn_eq);
	rgn_eq = CreateRoundRectRgn(0, 0, cr(coords.window_eq.width), cr(coords.window_eq.height), cr(5), cr(5));
	SetWindowRgn(window_eq, rgn_eq, 1);

	
	setwinpos_clip(window_eq, 0, skin_settings.eq_x, skin_settings.eq_y, cr(coords.window_eq.width), cr(coords.window_eq.height), SWP_NOZORDER);
	
	eq_draw();
}



/* callback functions -------------------------------------------------------*/


/*
 * equalizer window callback.
 */
LRESULT CALLBACK callback_eq_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, dx, dy;


	switch(msg)
	{
	case WM_CREATE:
		hdc_eq = GetDC(hwnd);
		break;

	case WM_MOUSEMOVE:
		{
			RECT rct;

			GetClientRect(hwnd, &rct);

			if(mdown == 1)
			{
				POINT pt;
				
				GetCursorPos(&pt);
				
				move_docking_window(window_id_eq, pt.x - dx, pt.y - dy);

			}else{
				eq_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_move);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		if(!eq_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_down_l))
		{
			mdown = 1;
			dx = (int)(short)LOWORD(lParam);
			dy = (int)(short)HIWORD(lParam);
			
			SetCapture(hwnd);

			if(skin_settings.skin_lock)
			{
				RECT  rctm;
				POINT pt;
				
				GetWindowRect(skin.wnd, &rctm);
				GetCursorPos(&pt);
				last_dx = pt.x - rctm.left;
				last_dy = pt.y - rctm.top;
			}
		}
		break;

	case WM_LBUTTONUP:
		mdown = 0;
		eq_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_up_l);
		ReleaseCapture();
		break;

	case WM_PAINT:
		eq_draw();
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
