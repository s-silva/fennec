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


typedef struct _skin_data_item
{
	string         name;
	struct coord  *coord_data;
	int           *x, *y, *w, *h, *sx, *sy, mode /* 0 - coord, 1 - single (x) */;

}skin_data_item;

skin_coords             coords;

#define           file_coord_table_count      102

skin_data_item    file_coord_table[file_coord_table_count] = {
{uni("window.m"),                    0, 0, 0, &coords.window_main.width, &coords.window_main.height, &coords.window_main.background_sx, &coords.window_main.background_sy, 0},
{uni("window.m.play"),               &coords.window_main.button_play      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.stop"),               &coords.window_main.button_stop      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.previous"),           &coords.window_main.button_previous  , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.next"),               &coords.window_main.button_next      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.open"),               &coords.window_main.button_open      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.playlist"),           &coords.window_main.button_playlist  , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.eq"),                 &coords.window_main.button_eq        , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.settings"),           &coords.window_main.button_settings  , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.convert"),            &coords.window_main.button_convert   , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.rip"),                &coords.window_main.button_rip       , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.join"),               &coords.window_main.button_join      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.vis"),                &coords.window_main.button_vis       , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.video"),              &coords.window_main.button_video     , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.dsp"),                &coords.window_main.button_dsp       , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.lock"),               &coords.window_main.button_lock      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.unlock"),             &coords.window_main.button_unlock    , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.minimize"),           &coords.window_main.button_minimize  , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.exit"),               &coords.window_main.button_exit      , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.bar_seek"),           &coords.window_main.bar_seek         , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.bar_volume"),         &coords.window_main.bar_volume       , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.display_area"),       &coords.window_main.display_area     , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.display_text_area"),  &coords.window_main.display_text_area, 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.display_region"),     &coords.window_main.display_region   , 0, 0, 0, 0, 0, 0, 0},
{uni("window.m.infotext"),           0, &coords.window_main.infotext_x, &coords.window_main.infotext_y, 0, 0, 0, 0, 0},
{uni("window.m.pos_text"),           0, &coords.window_main.pos_text_x, &coords.window_main.pos_text_y, 0, 0, 0, 0, 0},
{uni("window.m.dur_text"),           0, &coords.window_main.dur_text_x, &coords.window_main.dur_text_y, 0, 0, 0, 0, 0},
{uni("window.m.window_mode"),        0, &coords.window_main.window_mode, 0, 0, 0, 0, 0, 1 /* single */},
{uni("window.m.window_edge"),        0, &coords.window_main.window_edge, 0, 0, 0, 0, 0, 1 /* single */},
{uni("window.m.font_size"),          0, &coords.window_main.font_size,   0, 0, 0, 0, 0, 1 /* single */},
/* equalizer window */
{uni("window.eq"),                   0, 0, 0, &coords.window_eq.width, &coords.window_eq.height, &coords.window_eq.background_sx, &coords.window_eq.background_sy, 0},
{uni("window.eq.band[0]"),           &coords.window_eq.bands[0],         0, 0, 0, 0, 0, 0, 0}, /* preamp */
{uni("window.eq.band[1]"),           &coords.window_eq.bands[1],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[2]"),           &coords.window_eq.bands[2],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[3]"),           &coords.window_eq.bands[3],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[4]"),           &coords.window_eq.bands[4],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[5]"),           &coords.window_eq.bands[5],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[6]"),           &coords.window_eq.bands[6],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[7]"),           &coords.window_eq.bands[7],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[8]"),           &coords.window_eq.bands[8],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[9]"),           &coords.window_eq.bands[9],         0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band[10]"),          &coords.window_eq.bands[10],        0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.band.button"),       &coords.window_eq.bandbutton,       0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_channel_u"),  &coords.window_eq.button_channel_u, 0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_channel_s"),  &coords.window_eq.button_channel_s, 0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_presets"),    &coords.window_eq.button_presets,   0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_reset"),      &coords.window_eq.button_reset,     0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_power_on"),   &coords.window_eq.button_power_on,  0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_power_off"),  &coords.window_eq.button_power_off, 0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_exit"),       &coords.window_eq.button_exit,      0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_curve"),      &coords.window_eq.button_curve,     0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_linear"),     &coords.window_eq.button_linear,    0, 0, 0, 0, 0, 0, 0},
{uni("window.eq.button_single"),     &coords.window_eq.button_single,    0, 0, 0, 0, 0, 0, 0},
/* visualization window */
{uni("window.vis.min_width"),        0, &coords.window_vis.min_width,    0, 0, 0, 0, 0, 1 /* single */},
{uni("window.vis.min_height"),       0, &coords.window_vis.min_height,   0, 0, 0, 0, 0, 1 /* single */},
{uni("window.vis.window_edge"),      0, &coords.window_vis.window_edge,  0, 0, 0, 0, 0, 1 /* single */},
{uni("window.vis.window_mode"),      0, &coords.window_vis.window_mode,  0, 0, 0, 0, 0, 1 /* single */},
{uni("window.vis.button_close"),     &coords.window_vis.button_close,    0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.button_max"),       &coords.window_vis.button_max,      0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.button_close_align"), 0, &coords.window_vis.button_close_align,    0, 0, 0, 0, 0, 1 /* single */},
{uni("window.vis.button_max_align")  , 0, &coords.window_vis.button_max_align,      0, 0, 0, 0, 0, 1 /* single */},

{uni("window.vis.crop_tl"),          &coords.window_vis.crop_tl,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_tm"),          &coords.window_vis.crop_tm,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_tr"),          &coords.window_vis.crop_tr,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_ml"),          &coords.window_vis.crop_ml,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_mr"),          &coords.window_vis.crop_mr,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_bl"),          &coords.window_vis.crop_bl,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_bm"),          &coords.window_vis.crop_bm,         0, 0, 0, 0, 0, 0, 0},
{uni("window.vis.crop_br"),          &coords.window_vis.crop_br,         0, 0, 0, 0, 0, 0, 0},

{uni("window.vis.vis_tl"),           0, &coords.window_vis.vis_l, &coords.window_vis.vis_t, 0, 0, 0, 0, 0},
{uni("window.vis.vis_br"),           0, &coords.window_vis.vis_r, &coords.window_vis.vis_b, 0, 0, 0, 0, 0},

/* media library window */
{uni("window.ml.min_width"),             0, &coords.window_ml.min_width,    0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.min_height"),            0, &coords.window_ml.min_height,   0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.window_edge"),           0, &coords.window_ml.window_edge,  0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.window_mode"),           0, &coords.window_ml.window_mode,  0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_close"),          &coords.window_ml.button_close,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_close_align"),    0, &coords.window_ml.button_close_align,    0, 0, 0, 0, 0, 1 /* single */},

{uni("window.ml.crop_tl"),               &coords.window_ml.crop_tl,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_tm"),               &coords.window_ml.crop_tm,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_tr"),               &coords.window_ml.crop_tr,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_ml"),               &coords.window_ml.crop_ml,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_mr"),               &coords.window_ml.crop_mr,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_bl"),               &coords.window_ml.crop_bl,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_bm"),               &coords.window_ml.crop_bm,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.crop_br"),               &coords.window_ml.crop_br,         0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_medialib_back"),  &coords.window_ml.button_medialib_back,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_medialib_switch"),&coords.window_ml.button_medialib_switch,  0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_playlist"),       &coords.window_ml.button_playlist     ,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_add"),            &coords.window_ml.button_add          ,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_remove"),         &coords.window_ml.button_remove       ,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_save"),           &coords.window_ml.button_save         ,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_options"),        &coords.window_ml.button_options      ,    0, 0, 0, 0, 0, 0, 0},
{uni("window.ml.button_sort"),           &coords.window_ml.button_sort         ,    0, 0, 0, 0, 0, 0, 0},

{uni("window.ml.list_tl"),               0, &coords.window_ml.list_l, &coords.window_ml.list_t, 0, 0, 0, 0, 0},
{uni("window.ml.list_br"),               0, &coords.window_ml.list_r, &coords.window_ml.list_b, 0, 0, 0, 0, 0},

{uni("window.ml.button_medialib_align"), 0, &coords.window_ml.button_medialib_align,    0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_playlist_align"), 0, &coords.window_ml.button_playlist_align,    0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_add_align"),      0, &coords.window_ml.button_add_align ,        0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_remove_align"),   0, &coords.window_ml.button_remove_align,      0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_save_align"),     0, &coords.window_ml.button_save_align,        0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_options_align"),  0, &coords.window_ml.button_options_align,     0, 0, 0, 0, 0, 1 /* single */},
{uni("window.ml.button_sort_align"),     0, &coords.window_ml.button_sort_align,        0, 0, 0, 0, 0, 1 /* single */}};
 
extern letter skin_sheet_bmp[v_sys_maxpath];
extern letter skin_sheet_table[v_sys_maxpath];

void scoord(struct coord *c, int x, int y, int w, int h, int sxn, int syn, int sxh, int syh, int sxd, int syd)
{
	c->x    = x;
	c->y    = y;
	c->w    = w;
	c->h    = h;
	c->sx_n = sxn;
	c->sy_n = syn;
	c->sx_h = sxh;
	c->sy_h = syh;
	c->sx_d = sxd;
	c->sy_d = syd;
	c->align = 0;
	c->icon_text = uni('a');
	c->bk   = 0;
	c->font_id = 0;
}

int incoordx(int x, int y, struct coord *sc)
{
	return incoord(x, y, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h));
}

int incoord_vpos(int x, int y, struct coord *sc, int valign, int w, int h)
{
	switch(valign)
	{
	case 0: /* top left */
		return incoord(x, y, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h));

	case 1: /* top right */
		return incoord(x, y, cr(w - sc->x), cr(sc->y), cr(sc->w), cr(sc->h));

	case 2: /* bottom left */
		return incoord(x, y, cr(sc->x), cr(h - sc->y), cr(sc->w), cr(sc->h));

	case 3: /* bottom right */
		return incoord(x, y, cr(w - sc->x), cr(h - sc->y), cr(sc->w), cr(sc->h));
	}
	return 0;
}


int incoord_vpos_nozoom(int x, int y, struct coord *sc, int valign, int w, int h)
{
	switch(valign)
	{
	case 0: /* top left */
		return incoord(x, y, (sc->x), (sc->y), (sc->w), (sc->h));

	case 1: /* top right */
		return incoord(x, y, (w - sc->x), (sc->y), (sc->w), (sc->h));

	case 2: /* bottom left */
		return incoord(x, y, (sc->x), (h - sc->y), (sc->w), (sc->h));

	case 3: /* bottom right */
		return incoord(x, y, (w - sc->x), (h - sc->y), (sc->w), (sc->h));
	}
	return 0;
}

void blt_coord(HDC dc, HDC sdc, int state, struct coord *sc)
{
	if     (state == 0) StretchBlt(dc, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h), sdc, sc->sx_n, sc->sy_n, sc->w, sc->h, SRCCOPY);
	else if(state == 1) StretchBlt(dc, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h), sdc, sc->sx_h, sc->sy_h, sc->w, sc->h, SRCCOPY);
	else                StretchBlt(dc, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h), sdc, sc->sx_d, sc->sy_d, sc->w, sc->h, SRCCOPY);
}

void blt_coord_ew(HDC dc, HDC sdc, int state, struct coord *sc, int w)
{
	if     (state == 0) StretchBlt(dc, cr(sc->x), cr(sc->y), cr(w), cr(sc->h), sdc, sc->sx_n, sc->sy_n, w, sc->h, SRCCOPY);
	else if(state == 1) StretchBlt(dc, cr(sc->x), cr(sc->y), cr(w), cr(sc->h), sdc, sc->sx_h, sc->sy_h, w, sc->h, SRCCOPY);
	else                StretchBlt(dc, cr(sc->x), cr(sc->y), cr(w), cr(sc->h), sdc, sc->sx_d, sc->sy_d, w, sc->h, SRCCOPY);
}

void blt_coord_nozoom(HDC dc, HDC sdc, int state, struct coord *sc)
{
	if     (state == 0) BitBlt(dc, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h), sdc, cr(sc->sx_n), cr(sc->sy_n), SRCCOPY);
	else if(state == 1) BitBlt(dc, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h), sdc, cr(sc->sx_h), cr(sc->sy_h), SRCCOPY);
	else                BitBlt(dc, cr(sc->x), cr(sc->y), cr(sc->w), cr(sc->h), sdc, cr(sc->sx_d), cr(sc->sy_d), SRCCOPY);
}

void blt_coord_vpos_nozoom(HDC dc, HDC sdc, int state, struct coord *sc, int valign, int w, int h)
{
	switch(valign)
	{
	case 0: /* top left */
		if     (state == 0) BitBlt(dc, sc->x, sc->y, sc->w, sc->h, sdc, sc->sx_n, sc->sy_n, SRCCOPY);
		else if(state == 1) BitBlt(dc, sc->x, sc->y, sc->w, sc->h, sdc, sc->sx_h, sc->sy_h, SRCCOPY);
		else                BitBlt(dc, sc->x, sc->y, sc->w, sc->h, sdc, sc->sx_d, sc->sy_d, SRCCOPY);
		break;

	case 1: /* top right */
		if     (state == 0) BitBlt(dc, w - sc->x, sc->y, sc->w, sc->h, sdc, sc->sx_n, sc->sy_n, SRCCOPY);
		else if(state == 1) BitBlt(dc, w - sc->x, sc->y, sc->w, sc->h, sdc, sc->sx_h, sc->sy_h, SRCCOPY);
		else                BitBlt(dc, w - sc->x, sc->y, sc->w, sc->h, sdc, sc->sx_d, sc->sy_d, SRCCOPY);
		break;

	case 2: /* bottom left */
		if     (state == 0) BitBlt(dc, sc->x, h - sc->y, sc->w, sc->h, sdc, sc->sx_n, sc->sy_n, SRCCOPY);
		else if(state == 1) BitBlt(dc, sc->x, h - sc->y, sc->w, sc->h, sdc, sc->sx_h, sc->sy_h, SRCCOPY);
		else                BitBlt(dc, sc->x, h - sc->y, sc->w, sc->h, sdc, sc->sx_d, sc->sy_d, SRCCOPY);
		break;

	case 3: /* bottom right */
		if     (state == 0) BitBlt(dc, w - sc->x, h - sc->y, sc->w, sc->h, sdc, sc->sx_n, sc->sy_n, SRCCOPY);
		else if(state == 1) BitBlt(dc, w - sc->x, h - sc->y, sc->w, sc->h, sdc, sc->sx_h, sc->sy_h, SRCCOPY);
		else                BitBlt(dc, w - sc->x, h - sc->y, sc->w, sc->h, sdc, sc->sx_d, sc->sy_d, SRCCOPY);
		break;
	}
}


void blt_button_on_coord_vb(HDC dc, HDC sdc, struct coord *bar, struct coord *button, float pos)
{
	StretchBlt(dc, cr(bar->x + (bar->w / 2) - (button->w / 2)), cr(bar->y - (button->h / 2) + (int)((float)bar->h * (pos + 0.5f))), cr(button->w), cr(button->h), sdc, button->sx_d, button->sy_d, button->w, button->h, SRCCOPY);
}

void scontrol(struct coord *c, int x, int y, int w, int h, int align, uint32_t ncolor, uint32_t hcolor, int mode, letter icon_text, int font_size)
{
	c->x = x;
	c->y = y;
	c->w = w;
	c->h = h;
	c->align     = align; 
	c->ncolor    = ncolor;
	c->hcolor    = hcolor;
	c->mode      = mode;
	c->icon_text = icon_text;
	c->font_size = font_size;
	c->bk        = 0;
	c->font_id   = 0;
}

void scontrol_ex(struct coord *c, int x, int y, int w, int h, int align, uint32_t ncolor, uint32_t hcolor, int mode, letter icon_text, int font_size, int usebk, uint32_t bk_ncolor, uint32_t bk_hcolor, int font_id)
{
	c->x = x;
	c->y = y;
	c->w = w;
	c->h = h;
	c->align     = align; 
	c->ncolor    = ncolor;
	c->hcolor    = hcolor;
	c->mode      = mode;
	c->icon_text = icon_text;
	c->font_size = font_size;
	c->bk        = usebk;
	c->bk_ncolor = bk_ncolor;
	c->bk_hcolor = bk_hcolor;
	c->font_id   = font_id;
}



void fill_skin_coords(void)
{
	//coords.zoom = 1.0f;

	/* main window */

	coords.window_main.width  = 272;
	coords.window_main.height = 62;
	coords.window_main.background_sx = 0;
	coords.window_main.background_sy = 0;

	coords.window_main.window_edge = 2;
	coords.window_main.window_mode = 0; /* round rect */

	coords.window_main.infotext_x = 7;
	coords.window_main.infotext_y = 15;
	coords.window_main.dur_text_x = 234;
	coords.window_main.dur_text_y = 15;
	coords.window_main.pos_text_x = 200;
	coords.window_main.pos_text_y = 15;

	scontrol(&coords.window_main.button_play,       10, 55, 55, 55, coord_align_bottom_left, 0x656565, 0xff2e3e, 0, uni('t'), 36);
	scontrol(&coords.window_main.button_stop,       65, 45, 33, 33, coord_align_bottom_left, 0x656565, 0xff2e3e, 0, uni('b'), 19);
	scontrol(&coords.window_main.button_previous,  101, 45, 33, 33, coord_align_bottom_left, 0x656565, 0xff2e3e, 0, uni('c'), 19);
	scontrol(&coords.window_main.button_next,      135, 45, 33, 33, coord_align_bottom_left, 0x656565, 0xff2e3e, 0, uni('d'), 19);
	scontrol(&coords.window_main.button_volume,    169, 45, 33, 33, coord_align_bottom_left, 0x656565, 0xff2e3e, 0, uni('e'), 19);
	scontrol(&coords.window_main.button_open,      204, 45, 33, 33, coord_align_bottom_left, 0x656565, 0xff2e3e, 0, uni('f'), 19);

	
	scontrol(&coords.window_main.button_playlist,  89,  44, 25, 25, coord_align_bottom_right, 0x656565, 0xff2e3e, 0, uni('g'), 17);
	scontrol(&coords.window_main.button_repeat,    62,  44, 25, 25, coord_align_bottom_right, 0x656565, 0xff2e3e, 0, uni('h'), 17);	
	scontrol(&coords.window_main.button_search,    34,  44, 25, 25, coord_align_bottom_right, 0x656565, 0xff2e3e, 0, uni('i'), 17);	

	/*scoord(&coords.window_main.button_eq,        232, 40, 36, 15, 229, 117, 229, 133, 229, 149); */
	scontrol(&coords.window_main.button_exit,      (27 * 1) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('r'), 13);				
	scontrol(&coords.window_main.button_minimize,  (27 * 2) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('q'), 13);
	scontrol(&coords.window_main.button_eq,        (27 * 3) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('p'), 13);
	scontrol(&coords.window_main.button_vis,       (27 * 4) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('o'), 13);
	scontrol(&coords.window_main.button_convert,   (27 * 5) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('n'), 13);
	scontrol(&coords.window_main.button_rip,       (27 * 6) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('m'), 13);
	scontrol(&coords.window_main.button_settings,  (27 * 7) - 6 + 12, 14, 25, 25, coord_align_top_right, 0xc8c8c8, 0xffffff, 0, uni('l'), 13);

	/*scoord(&coords.window_main.button_settings,  187, 3,  9,  9,  46,  165, 100, 165, 100, 165);
	scoord(&coords.window_main.button_convert,   198, 3,  9,  9,  57,  165, 111, 165, 111, 165);
	scoord(&coords.window_main.button_rip,       209, 3,  9,  9,  68,  165, 122, 165, 122, 165);
	scoord(&coords.window_main.button_join,      220, 3,  9,  9,  79,  165, 133, 165, 133, 165);
	scoord(&coords.window_main.button_vis,       231, 3,  9,  9,  90,  165, 144, 165, 144, 165);
	scoord(&coords.window_main.button_video,     176, 3,  9,  9, 175,  165, 196, 165, 196, 165);

	scoord(&coords.window_main.button_dsp,       153, 3,  9,  9, 237,  165, 247, 165, 247, 165);
	scoord(&coords.window_main.button_lock,      163, 3,  9,  9, 189,  221, 199, 221, 199, 221);
	scoord(&coords.window_main.button_unlock,    163, 3,  9,  9, 189,  231, 199, 231, 199, 231);

	scoord(&coords.window_main.display_area,     0, 14, 273, 24,  0,  14, 0,  14, 0,  14);
	
	scoord(&coords.window_main.display_text_area, 5, 15, 261, 14,  5, 15, 5, 15, 5, 15); 

	scoord(&coords.window_main.display_region,   5, 15, 180 + 5, 14 + 15, 0, 0, 0, 0, 0, 0); 
	scoord(&coords.window_main.bar_seek,         7, 30, 209, 5, 1, 175, 1, 181, 0, 0);
	scoord(&coords.window_main.bar_volume,       219, 30, 46, 5, 211, 175, 211, 181, 0, 0); */

	/* equalizer */

	coords.window_eq.width         = 272;
	coords.window_eq.height        = 54;
	coords.window_eq.background_sx = 0;
	coords.window_eq.background_sy = 62;

	scoord(&coords.window_eq.bands[0], 8,   8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[1], 28,  8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[2], 44,  8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[3], 59,  8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[4], 74,  8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[5], 89,  8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[6], 104, 8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[7], 119, 8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[8], 134, 8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[9], 149, 8, 6, 39, 0, 0, 0, 0, 0, 0);
	scoord(&coords.window_eq.bands[10],164, 8, 6, 39, 0, 0, 0, 0, 0, 0);
	
	

	scoord(&coords.window_eq.button_channel_u, 181, 2,  67, 11, 1,   253, 137, 253, 1,   253); 
	scoord(&coords.window_eq.button_channel_s, 181, 2,  67, 11, 69,  253, 69,  289, 69,  253);
	scoord(&coords.window_eq.button_presets  , 181, 14, 67, 11, 1,   265, 137, 265, 1,   265);
	scoord(&coords.window_eq.button_reset	 , 181, 26, 67, 11, 1,   277, 137, 277, 1,   277);
	scoord(&coords.window_eq.button_power_on , 181, 38, 67, 11, 69,  265, 69,  277, 69,  265);
	scoord(&coords.window_eq.button_power_off, 181, 38, 67, 11, 1,   289, 137, 289, 1,   289);

	scoord(&coords.window_eq.button_exit     , 259, 4,  9,  9,  259, 66,  11,  165, 259, 66);
	scoord(&coords.window_eq.button_curve    , 256, 22, 10, 7,  211, 187, 222, 187, 211, 187);
	scoord(&coords.window_eq.button_linear   , 256, 30, 10, 7,  211, 195, 222, 195, 211, 195);
	scoord(&coords.window_eq.button_single   , 256, 38, 10, 7,  211, 203, 222, 203, 211, 203);
	
	scoord(&coords.window_eq.bandbutton      , 0, 0, 4, 4,  41, 165, 41, 165, 41, 165);
	
	/* visualization window */

	coords.window_vis.min_width   = 100;
	coords.window_vis.min_height  = 50;
	coords.window_vis.window_mode = 0; /* round rect */
	coords.window_vis.window_edge = 5;

	coords.window_vis.vis_l = 10;
	coords.window_vis.vis_r = 12;
	coords.window_vis.vis_t = 18;
	coords.window_vis.vis_b = 13;

	coords.window_vis.button_close_align = 1;
	coords.window_vis.button_max_align   = 1;

	scoord(&coords.window_vis.crop_tl        , 0, 0, 61,  31,   1, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_tm        , 0, 0, 31,  31,  63, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_tr        , 0, 0, 35,  31,  95, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_ml        , 0, 0, 13,  22, 131, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_mr        , 0, 0, 14,  22, 146, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_bl        , 0, 0, 21,  21, 185, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_bm        , 0, 0, 22,  19, 207, 301,  0, 0, 0, 0);
	scoord(&coords.window_vis.crop_br        , 0, 0, 21,  21, 161, 301,  0, 0, 0, 0);
	
	scoord(&coords.window_vis.button_close   ,13, 4, 9, 9, 260, 3, 11, 165, 11, 165);
	scoord(&coords.window_vis.button_max     ,23, 4, 9, 9, 217, 165, 227, 165, 227, 165);

	/* video window */

	coords.window_vid.min_width   = 100;
	coords.window_vid.min_height  = 50;
	coords.window_vid.window_mode = 0; /* round rect */
	coords.window_vid.window_edge = 5;

	coords.window_vid.vid_l = 10;
	coords.window_vid.vid_r = 12;
	coords.window_vid.vid_t = 18;
	coords.window_vid.vid_b = 13;

	coords.window_vid.button_close_align = 1;
	coords.window_vid.button_max_align   = 1;

	scoord(&coords.window_vid.crop_tl        , 0, 0, 61,  31, 127, 221,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_tm        , 0, 0, 31,  31,  63, 301,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_tr        , 0, 0, 35,  31,  95, 301,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_ml        , 0, 0, 13,  22, 131, 301,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_mr        , 0, 0, 14,  22, 146, 301,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_bl        , 0, 0, 21,  21, 185, 301,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_bm        , 0, 0, 22,  19, 207, 301,  0, 0, 0, 0);
	scoord(&coords.window_vid.crop_br        , 0, 0, 21,  21, 161, 301,  0, 0, 0, 0);
	
	scoord(&coords.window_vid.button_close   ,13, 4, 9, 9, 260, 3, 11, 165, 11, 165);
	scoord(&coords.window_vid.button_max     ,23, 4, 9, 9, 217, 165, 227, 165, 227, 165);

	/* media library window */

	coords.window_ml.min_width   = 100;
	coords.window_ml.min_height  = 50;
	coords.window_ml.window_mode = 0; /* round rect */
	coords.window_ml.window_edge = 0;

	coords.window_ml.list_l = 26;
	coords.window_ml.list_r = 0;
	coords.window_ml.list_t = 0;
	coords.window_ml.list_b = 0;

	coords.window_ml.button_close_align     = 1;
	coords.window_ml.button_medialib_align  = 0;
	coords.window_ml.button_playlist_align  = 0;
	coords.window_ml.button_add_align       = 0;
	coords.window_ml.button_remove_align    = 0;
	coords.window_ml.button_save_align      = 0;
	coords.window_ml.button_options_align   = 0;
	coords.window_ml.button_sort_align      = 0;


	scoord(&coords.window_ml.crop_tl        , 0, 0, 91, 33,   1, 187,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_tm        , 0, 0, 39, 33,  93, 187,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_tr        , 0, 0, 37, 33, 133, 187,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_ml        , 0, 0, 39, 15, 171, 187,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_mr        , 0, 0, 39, 15, 171, 203,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_bl        , 0, 0, 43, 31,   1, 221,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_bm        , 0, 0, 46, 31,  45, 221,  0, 0, 0, 0);
	scoord(&coords.window_ml.crop_br        , 0, 0, 34, 31,  92, 221,  0, 0, 0, 0);
	
	scoord(&coords.window_ml.button_close,          13,   4,  9,  9, 260,   3,  11, 165,  11, 165);
	scoord(&coords.window_ml.button_medialib_back,   6,  35, 16, 14, 216, 226, 216, 226, 216, 226);
	scoord(&coords.window_ml.button_medialib_switch, 6,  35, 16, 14, 233, 203, 250, 203, 233, 203);
	scoord(&coords.window_ml.button_playlist,        6,  19, 16, 14, 233, 187, 250, 187, 233, 187);
	scoord(&coords.window_ml.button_add,             6,  54, 16, 14, 233, 222, 250, 222, 233, 222);
	scoord(&coords.window_ml.button_remove,          6,  86, 16, 14, 233, 254, 250, 254, 233, 254);
	scoord(&coords.window_ml.button_save,            6,  70, 16, 14, 233, 238, 250, 238, 233, 238);
	scoord(&coords.window_ml.button_options,         6, 102, 16, 14, 233, 270, 250, 270, 233, 270);
	scoord(&coords.window_ml.button_sort,            6, 118, 16, 14, 233, 286, 250, 286, 233, 286);

}



/* simple skin loading functions --------------------------------------------*/



int  skin_file_load(const string fname)
{
	letter         sline[256], sname[128], fpath[v_sys_maxpath];
	FILE          *sfile;
	int            i, slc = 256, format2_i1;
	/*                                  ( x,  y,  w,  h,sxn,syn,sxh,syh,sxd,syd) */
	string         sformat1 = uni("%s = (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)");
	string         sformat2 = uni("%s = %d");
	struct coord   cc;

	str_cpy(skin_settings.skin_file_name, fname);

	if(fname[0] == '<') /* <default> */
	{
		skin_sheet_bmp[0] = 0;
		skin_sheet_table[0] = 0;
		fill_skin_coords();
		skin_recreate();
		return 1;
	}

	skin.shared->general.getskinspath(fpath, sizeof(fpath));
	str_cat(fpath, uni("/skin player data/"));
	str_cat(fpath, fname);
	str_cat(fpath, uni(".inf"));

	sfile  = _wfopen(fpath, uni("r"));

	if(!sfile) return 0;


	str_cpy(skin_sheet_bmp,   fname);
	str_cat(skin_sheet_bmp,   uni(".bmp"));
	str_cpy(skin_sheet_table, fname);
	str_cat(skin_sheet_table, uni(".inf"));

	while(fgetws(sline, slc, sfile))
	{
		if(sline[0] == uni('/') && sline[1] == uni('/')) continue;

		swscanf(sline, sformat1, sname, &cc.x, &cc.y, &cc.w,  &cc.h, &cc.sx_n, &cc.sy_n, &cc.sx_h, &cc.sy_h, &cc.sx_d, &cc.sy_d);
		swscanf(sline, sformat2, sname, &format2_i1);

		for(i=0; i<file_coord_table_count; i++)
		{
			if(str_icmp(sname, file_coord_table[i].name)) continue;

			if(file_coord_table[i].mode == 0)
			{
				if(file_coord_table[i].coord_data)
					memcpy(file_coord_table[i].coord_data, &cc, sizeof(struct coord));


				if(file_coord_table[i].x)   *file_coord_table[i].x  = cc.x;
				if(file_coord_table[i].y)   *file_coord_table[i].y  = cc.y;
				if(file_coord_table[i].w)   *file_coord_table[i].w  = cc.w;
				if(file_coord_table[i].h)   *file_coord_table[i].h  = cc.h;
				if(file_coord_table[i].sx)  *file_coord_table[i].sx = cc.sx_n;
				if(file_coord_table[i].sy)  *file_coord_table[i].sy = cc.sy_n;
			}else if(file_coord_table[i].mode == 1 /* single (format2_i1 -> x) */){
				*file_coord_table[i].x = format2_i1;
			}
		}
	}


	fclose(sfile);

	skin_recreate();
	return 1;
}

/*
 * 
 */
int skin_subskins_get(subskins_callback callfunc)
{
	letter  fpath[v_sys_maxpath];
	int     i;
	WIN32_FIND_DATA  wfd;
	HANDLE           fhandle;

	callfunc(uni("<default>"), uni("Base skin"));
	
	skin.shared->general.getskinspath(fpath, sizeof(fpath));
	str_cat(fpath, uni("/skin player data/*.inf"));

	fhandle = FindFirstFile(fpath, &wfd);

	if(fhandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			for(i=(int)str_len(wfd.cFileName); i>0; i--)
				if(wfd.cFileName[i] == uni('.')) wfd.cFileName[i] = 0;
			callfunc(wfd.cFileName, 0);
		}while(FindNextFile(fhandle, &wfd));
		
		FindClose(fhandle);
	}

	return 1;
}

int skin_subskins_select(const string fname)
{
	skin_file_load(fname);
	return 1;
}

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
