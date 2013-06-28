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
#include "data/system/windows/resource.h"
#include "ids.h"
#include "zmouse.h"
#include <shlobj.h>



/* structs ------------------------------------------------------------------*/



/* prototypes ---------------------------------------------------------------*/

LRESULT CALLBACK callback_ml_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ml_draw_window(int rd);
void ml_cache_uninit(void);
void ml_cache_init(void);




/* defines ------------------------------------------------------------------*/

#define  media_status_icon_width 14
#define  window_ml_crop_tl_x 1
#define  window_ml_crop_tl_y 187
#define  window_ml_crop_tl_w 91
#define  window_ml_crop_tl_h 33

#define  window_ml_crop_tr_x 133
#define  window_ml_crop_tr_y 187
#define  window_ml_crop_tr_w 37
#define  window_ml_crop_tr_h 33

#define  window_ml_crop_bl_x 1
#define  window_ml_crop_bl_y 221
#define  window_ml_crop_bl_w 43
#define  window_ml_crop_bl_h 31

#define  window_ml_crop_br_x 92
#define  window_ml_crop_br_y 221
#define  window_ml_crop_br_w 34
#define  window_ml_crop_br_h 31

#define  window_ml_crop_tm_x 93
#define  window_ml_crop_tm_y 187
#define  window_ml_crop_tm_w 39
#define  window_ml_crop_tm_h 33

#define  window_ml_crop_bm_x 45
#define  window_ml_crop_bm_y 221
#define  window_ml_crop_bm_w 46
#define  window_ml_crop_bm_h 31

#define  window_ml_crop_ml_x 171
#define  window_ml_crop_ml_y 187
#define  window_ml_crop_ml_w 39
#define  window_ml_crop_ml_h 15

#define  window_ml_crop_mr_x 171
#define  window_ml_crop_mr_y 203
#define  window_ml_crop_mr_w 39
#define  window_ml_crop_mr_h 15

#define  ml_button_playlist   1
#define  ml_button_library    2
#define  ml_button_in         3
#define  ml_button_out        4
#define  ml_button_remove     5
#define  ml_button_options    6
#define  ml_button_sort       7
#define  ml_button_close      8


#define  sel_toggle 4

#define  redraw_all       1
#define  redraw_default   0
#define  redraw_playlist  4
#define  redraw_playlist_scroll 5

#define  ml_window_class uni("fennec.skin.ml")

#define  preview_panel_h      12
#define  preview_panel_w      125
#define  preview_panel_x      100
#define  preview_panel_y      2
#define  preview_panel_play_x 1
#define  preview_panel_play_y 1
#define  preview_panel_play_w 10
#define  preview_panel_play_h 10
#define  preview_panel_stop_x 11
#define  preview_panel_stop_y 1
#define  preview_panel_stop_w 10
#define  preview_panel_stop_h 10
#define  preview_panel_seek_x 24
#define  preview_panel_seek_y 1
#define  preview_panel_seek_w 100
#define  preview_panel_seek_h 10
#define  artist_field_item_height 50

#define  timer_id_i_search    1231

#define  txt_ml_add_files   uni("Please add files to the media library first.")


/* data ---------------------------------------------------------------------*/


int               ml_init = 0;

int               ml_pl_x, ml_pl_y, ml_pl_w, ml_pl_h = 0;
int               cpl_x, cpl_y, cpl_w, cpl_h;
int               hscroll_bw, hscroll_v = 0, hscroll_x = 0;
int               vscroll_bh, vscroll_v = 0, vscroll_y = 0;
int               ml_col_xpos = 0;      /* left shift value for columns */
int               lindex_mode = 0;      /* real index, shuffle index */
int               fast_pl = 1;

long              ml_pl_startid = 0;
long              ml_pl_startid_last;

unsigned long     cached_tags_count;
unsigned long     pl_last_cur = 0;
unsigned long     pl_cur_item = 0;

int               last_v_scroll_h = 0;

struct pl_cache  *cached_tags = 0;

HWND              window_ml;
HDC               hdc_ml;
HRGN              rgn_ml = 0;

HFONT             font_ml_pl, font_ml_pl_small_link;
TEXTMETRIC        tm_ml_pl;
COLORREF          color_ml_pl_normal   = RGB(204, 204, 255);
COLORREF          color_ml_pl_faint    = RGB(53,  77,  173);
COLORREF          color_ml_pl_item_cur = RGB(124, 124, 192);
COLORREF          color_ml_pl_item_sel = RGB(170, 154, 90);
COLORREF          color_ml_pl_item_col = RGB(118, 132, 205);
COLORREF          color_ml_pl_header   = RGB(0, 30, 50);

//int               mode_ml = 0;
unsigned long     ml_current_item = 0;
unsigned long     ml_last_dir    = media_library_dir_root;
unsigned long     ml_current_dir = media_library_dir_root;
int               ml_in_dir = 1;
int               ml_dir_changed = 1;
int               ml_last_dir_index = 0;


string            ml_current_cdir = 0;

#define mode_ml            skin_settings.mode_ml


letter            ml_cdir_text[512];

int               preview_item_index = -1;
int               preview_x, preview_y, preview_last_move_id = 0;

int               sel_column = -1;
int               ml_list_item_height = artist_field_item_height;
#define           ml_list_header_height tm_ml_pl.tmHeight
#define			  ml_pl_scroll_size    ((tm_ml_pl.tmHeight / 5) * 3)

int               ml_artists_album_pos_x;
int               ml_artists_album_pos_y;
int               ml_artists_album_pos_h;

int               isearch_pos = 0;

HDC               tiny_mdc_stars;
HBITMAP           tiny_bmp_stars, tiny_bmp_stars_old;

/* code ---------------------------------------------------------------------*/


/* cache - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * (re)initialize cache.
 */
void ml_cache_init(void)
{
	unsigned long i, j, k;
	unsigned long c, z;

	if(mode_ml && ml_current_dir == media_library_dir_artists && !ml_current_cdir)
	{
		ml_list_item_height = artist_field_item_height;
	}else{
		ml_list_item_height = tm_ml_pl.tmHeight + 2;
	}

	if(!mode_ml)
	{
		c = skin.shared->audio.output.playlist.getcount();
	}else{
		skin.shared->mlib.media_library_advanced_function(1, 0, 0); /* safe initialize */


		if(ml_in_dir)
		{
			z = 2;
			cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);

			for(i=0, k=0;;)
			{
				c = skin.shared->mlib.media_library_get_dir_names(k, ml_current_dir, &cached_tags[i].dname);
				if(!c) break;
				
				
				for(j=0; j<i; j++)
				{
					if(!str_icmp(cached_tags[j].dname, cached_tags[i].dname))
					{
						skin.shared->mlib.media_library_get_dir_names((unsigned long)-1, 0, (string*)cached_tags[i].dname); /* free */
						i--;
						break;
					}
				}

				

				i++;
				k++;
				if(i >= z)
				{
					z = i + 2;
					cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);
				}
			}

			cached_tags_count = i;

			for(i=0; i<cached_tags_count; i++)
			{
				cached_tags[i].pid   = (unsigned long)-1;
				cached_tags[i].sel   = 0;
				cached_tags[i].album_points[0]  = 0; /* like text strings, ends with (0) */
			}

			if(skin_settings.ml_dir_sort_mode >= 2)
				list_sort_column(ml_current_dir, skin_settings.ml_dir_sort_mode);

			return;


		}else{

			if(ml_current_dir == media_library_dir_all)
			{
				c = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0); /* get count */

			}else{

				uint32_t  litem = 0;

				z = 2;
				cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);

				for(i=0;;)
				{
					c = skin.shared->mlib.media_library_get_dir_files(i, ml_current_dir, ml_current_cdir, &litem);
					if(!c) break;

					skin.shared->mlib.media_library_translate(litem, &cached_tags[i].ft);
					litem++;
					
					i++;
		
					if(i >= z)
					{
						z = i + 2;
						cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);
					}
				}

				cached_tags_count = i;

				for(i=0; i<cached_tags_count; i++)
				{
					cached_tags[i].pid   = 1;
					cached_tags[i].sel   = 0;
					cached_tags[i].dname = 0;
					cached_tags[i].album_points[0]  = 0;
				}
				return;
			}
		}
	}
	
	if(cached_tags)ml_cache_uninit();
	
	if(!c)
	{
		cached_tags = 0;
		return;
	}

	cached_tags = (struct pl_cache*) malloc(sizeof(struct pl_cache) * c);

	cached_tags_count = c;

	for(i=0; i<c; i++)
	{
		cached_tags[i].pid   = (unsigned long)-1;
		cached_tags[i].sel   = 0;
		cached_tags[i].dname = 0;
		cached_tags[i].album_points[0]  = 0;
	}

	if(pl_cur_item < cached_tags_count)
		cached_tags[pl_cur_item].sel = 1;
}

void ml_cache_exchange(unsigned long di, unsigned long si)
{
	struct pl_cache bpc;

	if(ml_in_dir && mode_ml && (ml_current_dir == media_library_dir_root)) return;

	if(!ml_in_dir && mode_ml)
	{
		skin.shared->mlib.media_library_advanced_function(3 /* set sorting mode */, 1, 0);
		skin.shared->mlib.media_library_advanced_function(2 /* exchange */, di, ULongToPtr(si));
	}

	memcpy(&bpc, &cached_tags[di], sizeof(struct pl_cache));
	memcpy(&cached_tags[di], &cached_tags[si], sizeof(struct pl_cache));
	memcpy(&cached_tags[si], &bpc, sizeof(struct pl_cache));
}


/*
 * clear selection.
 */
void ml_cache_clearsel(void)
{
	unsigned long i;

	if(!cached_tags)return;

	for(i=0; i<cached_tags_count; i++)
	{
		cached_tags[i].sel = 0;
	}
}


/*
 * is selected?
 */
int ml_cache_issel(unsigned long id)
{
	if(!cached_tags || id >= cached_tags_count)return (int)-1;
	return (int)cached_tags[id].sel;
}


/*
 * shift selection (up or down).
 */
int ml_cache_shiftsel(long s)
{
	long i;

	if(!cached_tags)return -1;

	if(s > 0)
	{
		if(cached_tags[cached_tags_count - 1].sel) return -2;

		for(i=(cached_tags_count-1); i>=0; i--)
		{
			if((unsigned long)(i + s) < cached_tags_count)
			{
				cached_tags[i + s].sel = cached_tags[i].sel;
				cached_tags[i].sel = 0;
			}

		}
	}else if(s < 0){

		if(cached_tags[0].sel) return -2;

		for(i=0; i<(long)cached_tags_count; i++)
		{
			if((i + s) >= 0)
			{
				cached_tags[i + s].sel = cached_tags[i].sel;
				cached_tags[i].sel = 0;
			}

		}
	}
	return 0;
}

unsigned long ml_cache_getcurrent_item(void)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getcurrentindex();
	else
		return skin.shared->audio.output.playlist.getcurrentindex();
}


unsigned long ml_cache_getnext_item(void)
{
	unsigned long cpid = ml_cache_getlistindex(ml_cache_getcurrent_item()) + 1;
	if(cpid >= cached_tags_count) return -1;

	return ml_cache_getrealindex(cpid);
}

const string ml_cache_getpath(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getsource(itemid);
	else
		return cached_tags[itemid].ft.tag_filepath.tdata;
}

unsigned long ml_cache_getlistindex(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getlistindex(itemid);
	else
		return itemid;
}

unsigned long ml_cache_getrealindex(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getrealindex(itemid);
	else
		return itemid;
}

int ml_cache_switch(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.switchindex(itemid);
	else
		return 0;
}

int ml_cache_switchlist(unsigned long itemid, int mplay)
{
	if(!mode_ml)
	{
		int rv;
		rv = skin.shared->audio.output.playlist.switch_list(itemid);
		if(mplay)skin.shared->audio.output.play();
		return rv;

	}else{

		if(!ml_in_dir)
		{
			unsigned long i;
			struct fennec_audiotag *at;

			ml_current_item = itemid;
			

			if(ml_dir_changed || (cached_tags_count != skin.shared->audio.output.playlist.getcount()) )
			{
				skin.shared->audio.output.playlist.clear();

				for(i=0; i<cached_tags_count; i++)
				{
					at = (struct fennec_audiotag*) ml_cache_get(i);
					//skin.shared->audio.output.playlist.add((string)at->tag_filepath.tdata, 0, 0);
					skin.shared->audio.output.playlist.add(0, 0, i);
				}
				ml_dir_changed = 0;
			}

			at = ml_cache_get(itemid);
			skin.shared->mlib.media_library_advanced_function(8 /* update info */, 0, (void*)(string)at->tag_filepath.tdata);

			skin.shared->audio.output.playlist.switch_list(itemid);
			if(mplay)skin.shared->audio.output.play();
		}else{
			
			ml_last_dir = ml_current_dir;

			ml_dir_changed = 1;

			if(ml_current_dir == media_library_dir_root)
			{	
				ml_current_dir = skin.shared->mlib.media_library_advanced_function(4 /* get child dir index */, itemid, 0);
			}else{
				if(itemid < cached_tags_count)
					str_cpy(ml_cdir_text, cached_tags[itemid].dname);
				ml_current_cdir = ml_cdir_text;

				ml_last_dir_index = ml_pl_startid;
				ml_pl_startid = 0;
			}

			ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
			
			ml_cache_uninit();
			ml_cache_init();
		}
		return 0;
	}
}

int ml_cache_add_to_playlist(unsigned long itemid)
{
	if(!mode_ml)
	{
		return skin.shared->audio.output.playlist.switch_list(itemid);
	}else{
		if(!ml_in_dir)
		{
			struct fennec_audiotag *at;
			at = (struct fennec_audiotag*) ml_cache_get(itemid);
			skin.shared->audio.output.playlist.add((string)at->tag_filepath.tdata, 0, 0);
		}
		return 0;
	}
}



/*
 * set selection.
 */
int ml_cache_setsel(unsigned long id, char v)
{
	if(!cached_tags || id >= cached_tags_count)return -1;
	if(v != sel_toggle)
		cached_tags[id].sel = v ? 1 : 0;
	else
		cached_tags[id].sel ^= 1;
	return 0;
}


/*
 * get tags from the cache.
 */
void *ml_cache_get(unsigned long id)
{
	if(!cached_tags || id >= cached_tags_count)return (void*)-1;

	if(cached_tags[id].pid != -1)
	{
		return (void*) &cached_tags[id].ft;
	}else{

		if(!mode_ml)
		{
			unsigned long mlid = (unsigned long) skin.shared->audio.output.playlist.getinfo(id, 0, 0, 0, 0, 0, 0);
			if(mlid == (unsigned long)-1)
			{
				cached_tags[id].pid = skin.shared->audio.input.tagread(skin.shared->audio.output.playlist.getsource(id), &cached_tags[id].ft);
			}else{
				skin.shared->mlib.media_library_translate(mlid, &cached_tags[id].ft);
				cached_tags[id].pid = 1;
			}

		}else{
			if(!ml_in_dir)
			{
				skin.shared->mlib.media_library_translate(id, &cached_tags[id].ft);
			}else{
				skin.shared->mlib.media_library_get_dir_names(id, ml_current_dir, &cached_tags[id].dname);
			}

			cached_tags[id].pid = 1;
		}

		return (void*) &cached_tags[id].ft;
	}
}


/*
 * free all the tags allocated.
 */
void ml_cache_freetags(void)
{
	unsigned long i;

	if(!cached_tags)return;

	for(i=0; i<cached_tags_count; i++)
	{
		if(!mode_ml)
		{
			if(cached_tags[i].pid != -1)
				skin.shared->audio.input.tagread_known(cached_tags[i].pid, 0, &cached_tags[i].ft);
		
			if(cached_tags[i].dname)
				skin.shared->mlib.media_library_get_dir_names((unsigned long)-1, 0, (string*)cached_tags[i].dname); /* free */
		}else{

		}

		cached_tags[i].pid = (unsigned long)-1;
	}
}


/*
 * uninitialization.
 */
void ml_cache_uninit(void)
{
	unsigned long i;

	if(!cached_tags)return;

	for(i=0; i<cached_tags_count; i++)
	{
		if(!mode_ml)
		{
			if(cached_tags[i].pid != -1)
				skin.shared->audio.input.tagread_known(cached_tags[i].pid, 0, &cached_tags[i].ft);
		}else{
			if(cached_tags[i].dname)
				skin.shared->mlib.media_library_get_dir_names((unsigned long)-1, 0, (string*)cached_tags[i].dname); /* free */
		}

		cached_tags[i].pid = (unsigned long)-1;
	}

	free(cached_tags);
	cached_tags = 0;
}


/*
 * remove selected media from the playlist.
 */
void ml_cache_removesel(void)
{
	unsigned long i;
	unsigned long j;

	if(!cached_tags)return;

	if(!mode_ml)
	{
		for(i=0, j=0; i<cached_tags_count; i++, j++)
		{
			if(cached_tags[i].sel)
			{
				skin.shared->audio.output.playlist.remove(j);
				j--;
			}
		}

		ml_cache_uninit();
		ml_cache_init();
	}
}

unsigned long ml_cache_getcount(void)
{
	return cached_tags_count;
}


/* display - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


COLORREF color_setbrightness(COLORREF c, double ca)
{
	double cr = GetRValue(c), cg = GetGValue(c), cb = GetBValue(c);
	cr *= ca; cg *= ca; cb *= ca;
	return RGB((unsigned char)min(cr, 255), (unsigned char)min(cg, 255), (unsigned char)min(cb, 255));
}


/*
 * get index from point.
 */
int ml_get_button_index(int x, int y)
{
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_playlist,         coords.window_ml.button_playlist_align, skin_settings.ml_w, skin_settings.ml_h)) return ml_button_playlist;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_medialib_switch,  coords.window_ml.button_medialib_align, skin_settings.ml_w, skin_settings.ml_h)) return ml_button_library;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_add,              coords.window_ml.button_add_align,      skin_settings.ml_w, skin_settings.ml_h)) return ml_button_in;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_save,             coords.window_ml.button_save_align,     skin_settings.ml_w, skin_settings.ml_h)) return ml_button_out;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_remove,           coords.window_ml.button_remove_align,   skin_settings.ml_w, skin_settings.ml_h)) return ml_button_remove;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_options,          coords.window_ml.button_options_align,  skin_settings.ml_w, skin_settings.ml_h)) return ml_button_options;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_sort,             coords.window_ml.button_sort_align,     skin_settings.ml_w, skin_settings.ml_h)) return ml_button_sort;
	if(incoord_vpos_nozoom(x, y, &coords.window_ml.button_close,            coords.window_ml.button_close_align,    skin_settings.ml_w, skin_settings.ml_h)) return ml_button_close;
	return 0;
}


/*
 * draw normal state button (clear).
 */
int ml_draw_normalex(int idx, HDC dc)
{
	switch(idx)
	{
	case ml_button_playlist: blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_playlist, coords.window_ml.button_playlist_align, skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_library:
		if(mode_ml)
			blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_medialib_back,   coords.window_ml.button_medialib_align, skin_settings.ml_w, skin_settings.ml_h);
		else
			blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_medialib_switch, coords.window_ml.button_medialib_align, skin_settings.ml_w, skin_settings.ml_h);
		break;

	case ml_button_in:       blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_add,     coords.window_ml.button_add_align,     skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_out:      blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_save,    coords.window_ml.button_save_align,    skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_remove:   blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_remove,  coords.window_ml.button_remove_align,  skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_options:  blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_options, coords.window_ml.button_options_align, skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_sort:     blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_sort,    coords.window_ml.button_sort_align,    skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_close:    blt_coord_vpos_nozoom(dc, mdc_sheet, 0, &coords.window_ml.button_close,   coords.window_ml.button_close_align,   skin_settings.ml_w, skin_settings.ml_h); break;
	default: return 0;
	}

	return 1;
}

void ml_draw_normal(int idx)
{
	ml_draw_normalex(idx, hdc_ml);
}


/*
 * draw lit button.
 */
void ml_draw_hover(int idx)
{
	static int last_idx = 0;

	switch(idx)
	{
	case ml_button_playlist: show_tip(oooo_skins_tip_showplaylist, 0); break;
	case ml_button_library: 
		if(mode_ml) show_tip(oooo_skins_tip_back, 0);
		else        show_tip(oooo_skins_tip_showmedialib, 0);
		break;

	case ml_button_in:       show_tip(oooo_skins_tip_pl_addfiles,  0); break;
	case ml_button_out:      show_tip(oooo_skins_tip_pl_save,      0); break;
	case ml_button_remove:   show_tip(oooo_skins_tip_pl_remove,    0); break;
	case ml_button_options:  show_tip(oooo_skins_tip_pl_options,   0); break;
	case ml_button_sort:     show_tip(oooo_skins_tip_pl_sorting,   0); break;
	case ml_button_close:    show_tip(oooo_skins_close,            0); break;
	}


	if(idx == last_idx)return;

	switch(idx)
	{
	case ml_button_playlist: blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_playlist, coords.window_ml.button_playlist_align, skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_library:
		if(mode_ml)
			blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_medialib_back,   coords.window_ml.button_medialib_align, skin_settings.ml_w, skin_settings.ml_h);
		else
			blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_medialib_switch, coords.window_ml.button_medialib_align, skin_settings.ml_w, skin_settings.ml_h);
		break;

	case ml_button_in:       blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_add,     coords.window_ml.button_add_align,     skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_out:      blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_save,    coords.window_ml.button_save_align,    skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_remove:   blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_remove,  coords.window_ml.button_remove_align,  skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_options:  blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_options, coords.window_ml.button_options_align, skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_sort:     blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_sort,    coords.window_ml.button_sort_align,    skin_settings.ml_w, skin_settings.ml_h); break;
	case ml_button_close:    blt_coord_vpos_nozoom(hdc_ml, mdc_sheet, 1, &coords.window_ml.button_close,   coords.window_ml.button_close_align,   skin_settings.ml_w, skin_settings.ml_h); break;
	default: return;
	}


	ml_draw_normal(last_idx);

	last_idx = idx;

	return;
}


/* main code - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



/*
 * create media library window and initialize other stuff.
 */
void ml_create(HWND hwndp)
{
	WNDCLASS wndc;

	if(ml_init)return;
	if(!skin_settings.ml_show)return;

	ml_cache_init();

	wndc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndc.lpfnWndProc   = (WNDPROC)callback_ml_window;
	wndc.cbClsExtra    = 0;
	wndc.cbWndExtra    = 0;
	wndc.hInstance     = instance_skin;;
	wndc.hIcon         = LoadIcon(skin.finstance, (LPCTSTR)0);
	wndc.hCursor       = LoadCursor(0, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndc.lpszMenuName  = 0;
	wndc.lpszClassName = ml_window_class;

	RegisterClass(&wndc);

	
	
	window_ml = CreateWindow(ml_window_class, uni("Media Library"), WS_POPUP, skin_settings.ml_x, skin_settings.ml_y, skin_settings.ml_w, skin_settings.ml_h, hwndp, 0, instance_skin, 0);
	
	setwinpos_clip(window_ml, 0, skin_settings.ml_x, skin_settings.ml_y, skin_settings.ml_w, skin_settings.ml_h, SWP_NOSIZE | SWP_NOZORDER);


	skin_settings.ml_d = skin_settings.ml_d;

	ShowWindow(window_ml, SW_SHOW);										  
	UpdateWindow(window_ml);

	font_ml_pl = CreateFont(-MulDiv(skin_settings.ml_font_size, GetDeviceCaps(hdc_ml, LOGPIXELSY), 72),
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, skin_settings.font_display);

	font_ml_pl_small_link = CreateFont(-MulDiv(8, GetDeviceCaps(hdc_ml, LOGPIXELSY), 72),
                                0, 0, 0, FW_NORMAL, 0, 1, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, skin_settings.font_display);


	//tiny_bmp_stars = LoadBitmap(instance_skin, (LPCTSTR)bmp_stars);
	//tiny_mdc_stars = CreateCompatibleDC(hdc_ml);
	//tiny_bmp_stars_old = (HBITMAP) SelectObject(tiny_mdc_stars, tiny_bmp_stars);

	ml_init = 1;
	ml_refresh(v_fennec_refresh_force_full);
}


/*
 * close the window and uninitialize associated stuff.
 */
void ml_close(void)
{
	if(!ml_init)
	{
		skin_settings.ml_show = 0;
		return;
	}else{
		skin_settings.ml_show = 1;
	}

		
	//SelectObject(tiny_mdc_stars, tiny_bmp_stars_old);
	//DeleteObject(tiny_bmp_stars);
	//DeleteDC(tiny_mdc_stars);
	
	DeleteObject(font_ml_pl);
	DeleteObject(font_ml_pl_small_link);

	ml_cache_uninit();
	DeleteDC(hdc_ml);
	DestroyWindow(window_ml);
	ml_init = 0;
}


/*
 * refresh media window/list.
 */
void ml_refresh(int lv)
{
	if(!ml_init)return;

	if(!skin_settings.use_color)
	{
		color_ml_pl_normal   = RGB(204, 204, 255);
		color_ml_pl_faint    = RGB(53,  77,  173);
		color_ml_pl_item_cur = RGB(124, 124, 192);
		color_ml_pl_item_sel = RGB(170, 154, 90);
	    color_ml_pl_item_col = RGB(118, 132, 205);

	}else{
		unsigned char r, g, b;
		int           rv, gv, bv;
		int           lv;

		color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(skin_settings.light * 2.5), &r, &g, &b);
		rv = r; gv = g; bv = b;
		lv = skin_settings.theme_mode == 1 ? 140 : 40;
		color_ml_pl_normal   = RGBx(rv + lv, gv + lv, bv + lv);


		color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(skin_settings.light * 2.5), &r, &g, &b);
		rv = r; gv = g; bv = b;
		lv = skin_settings.theme_mode == 1 ? 80 : -80;
		color_ml_pl_item_col   = RGBx(rv + lv, gv + lv, bv + lv);


		color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(skin_settings.light * 2.5), &r, &g, &b);
		rv = r; gv = g; bv = b;
		lv = skin_settings.theme_mode == 1 ? 0 : -120;
		color_ml_pl_faint    = RGBx(rv + lv, gv + lv, bv + lv);


		color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5), (int)(skin_settings.sat * 2.5), (int)(skin_settings.light * 2.5), &r, &g, &b);
		rv = r; gv = g; bv = b;
		lv = skin_settings.theme_mode == 1 ? 50 : -25;
		color_ml_pl_item_cur = RGBx(rv + lv, gv + lv, bv + lv);


		color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5) + 50, (int)(skin_settings.sat * 2.5), (int)(skin_settings.light * 2.5), &r, &g, &b);
		rv = r; gv = g; bv = b;
		lv = skin_settings.theme_mode == 1 ? 70 : -35;
		color_ml_pl_item_sel = RGBx(rv + lv, gv + lv, bv + lv);


		color_hsv_2_rgb_fullint((int)(skin_settings.hue * 3.5) + 20, (int)(skin_settings.sat * 2.5), (int)(skin_settings.light * 2.5), &r, &g, &b);
		rv = r; gv = g; bv = b;
		lv = skin_settings.theme_mode == 1 ? -20 : -80;
		color_ml_pl_header = RGBx(rv + lv, gv + lv, bv + lv);
	}

	SendMessage(window_ml, WM_PAINT, 0, 0);

	if(lv >= v_fennec_refresh_force_less)
	{
		static unsigned long lcount = 0;
		long plmc;

		if(cached_tags_count != lcount)
		{
			ml_pl_startid = 0;
			lcount = cached_tags_count;
		}

		if(ml_pl_h && ml_list_item_height)
		{
			if(!(mode_ml && ml_dir_changed))
			{

				plmc = ml_pl_h / ml_list_item_height;
				if(ml_cache_getcurrent_item() >= (unsigned long)(ml_pl_startid + plmc))
				{
					ml_pl_startid = ml_cache_getcurrent_item() - plmc + 1;
				}

				if(ml_cache_getcurrent_item() < (unsigned long)(ml_pl_startid))
				{
					ml_pl_startid = ml_cache_getcurrent_item();
				}
			}
		}

		if(lv > v_fennec_refresh_force_less)
		{
			if(!mode_ml)
			{
				ml_cache_uninit();
				ml_cache_init();
			}
		}
	}
}


/*
 * get column width.
 */
int ml_col_getwidth(int idx)
{
	int i;
	int v = ml_pl_w;

	if(idx < 0 || idx > sizeof(skin_settings.ml_pl_xoff) / sizeof(int))return -1;
	if(skin_settings.ml_pl_xoff[idx] == -1)return -1;

	for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(int); i++)
	{
		if(skin_settings.ml_pl_xoff[i] == -1)continue;

		if((skin_settings.ml_pl_xoff[i] - skin_settings.ml_pl_xoff[idx] > 0) && (skin_settings.ml_pl_xoff[i] - skin_settings.ml_pl_xoff[idx] < v))
			v = skin_settings.ml_pl_xoff[i] - skin_settings.ml_pl_xoff[idx];
	}
	return v;
}


/*
 * clipping.
 */
void ml_col_tout_preaction(HDC dc, int x, int id)
{
	static HRGN rcr = 0;

	if(x != -1)
	{
		int cx, cy, cw, ch, r;

		if(id == -1)
		{
			cx = cpl_x;
			cw = cpl_w;

		}else{
			cx = cpl_x + skin_settings.ml_pl_xoff[id] - ml_col_xpos;
			cw = ml_col_getwidth(id) - 5;
		}

		cy = cpl_y;
		ch = cpl_h;

		r = cx + cw;

		cx = max(cpl_x, cx);
		r  = max(cpl_x, r);
		r  = min(r, cpl_w + cpl_x);

		rcr = CreateRectRgn(cx, cy, r, cy + ch);

		SelectClipRgn(dc, rcr);

	}else{
		if(rcr)
		{
			SelectClipRgn(dc, 0);
			DeleteObject(rcr);
			rcr = 0;
		}
	}
}


/*
 * draw vertical scroll bar.
 */
void ml_scroll_draw_v(HDC dc, int x, int y, int w, int h, int am, int dm, int v)
{
	int sch;

	last_v_scroll_h = h;
	
	/* (v - starting value, v + dm - ending value) */

	if(dm >= am || !am)return;

	sch = min(h * dm / am, h);
	if(v + dm > am)v = am - dm;

	vscroll_bh = sch;

	drawrect(dc, x, y, w, h, color_setbrightness(color_ml_pl_item_cur, 0.5));

	if(vscroll_v == -1)
		vscroll_y = (((h - sch) * v) / (am - dm));
	else
		vscroll_y = vscroll_v;

	//if(sch + vscroll_y > h)vscroll_y = h - sch;
	drawrect(dc, x, y + vscroll_y, w, max(sch, 1), color_ml_pl_item_cur);
	vscroll_v = -1;
}


/*
 * draw horizontal scroll bar.
 */
void ml_scroll_draw_h(HDC dc, int x, int y, int w, int h, int am, int dm, int v)
{
	int sch;
	
	/* (v - starting value, v + dm - ending value) */

	if(dm >= am || !am)return;

	sch = min(w * dm / am, w);
	if(v + dm > am)v = am - dm;

	hscroll_bw = sch;

	drawrect(dc, x, y, w, h, color_setbrightness(color_ml_pl_item_cur, 0.5));

	

	if(hscroll_v == -1)
		hscroll_x = ((w - sch) * v) / (am - dm);
	else
		hscroll_x = hscroll_v;

	drawrect(dc, x + hscroll_x, y, max(sch, 1), h, color_ml_pl_item_cur);
}

/*
 *
 */
void ml_back(void)
{
	ml_cache_uninit();
	if(ml_current_cdir)
	{
		ml_current_cdir = 0;
		ml_current_dir = ml_last_dir;
		ml_pl_startid = ml_last_dir_index;
	}else{
		ml_current_dir = media_library_dir_root;
		ml_pl_startid = 0;
	}
	ml_in_dir = (ml_current_dir != media_library_dir_all);
	ml_dir_changed = 1;

	ml_cache_init();
	ml_draw_window(0);
	ml_draw_normal(ml_button_library);
}


/*
 * get maximum extent where the last column starts.
 */
int ml_col_getmax_extent(void)
{
	int i;
	int cp = 0;

	for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(int); i++)
	{
		if(skin_settings.ml_pl_xoff[i] == -1)continue;

		if(skin_settings.ml_pl_xoff[i] > cp)cp = skin_settings.ml_pl_xoff[i];
	}
	return cp;
}

void ml_col_fixbeginning(void)
{
	int cxv = skin_settings.ml_pl_xoff[0], cxi = 0, i;


	for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(int); i++)
	{
		if(skin_settings.ml_pl_xoff[i] < cxv && skin_settings.ml_pl_xoff[i] > -1)
		{
			cxv = skin_settings.ml_pl_xoff[i];
			cxi = i;
		}
	}

	if(skin_settings.ml_pl_xoff[cxi] <= media_status_icon_width && skin_settings.ml_pl_xoff[cxi] > -1)
		skin_settings.ml_pl_xoff[cxi] = media_status_icon_width;
	return;
}


/*
 * draw playlist.
 */
void ml_playlist_draw(HDC dc, int x, int y, int w, int h)
{
	int            c = h / ml_list_item_height;
	int            i;
	int            stx = x, sty = y;
	int			   draw_playing_item_icon;
	unsigned long  nextitemid;

	letter         unkstr[] = uni("Unknown");
	int            unklen = (int)str_len(unkstr);

	struct fennec_audiotag *ctag = 0;
	struct fennec_audiotag_item *ct;

	cpl_x = x;
	cpl_y = y;
	cpl_w = w;
	cpl_h = h;

	
	//x += media_status_icon_width;
	x -= ml_col_xpos;
	ml_col_fixbeginning();

	/* draw items */

	nextitemid = ml_cache_getnext_item();

	for(i=0; i<c; i++)
	{
		draw_playing_item_icon = 0;

		

		if(ml_pl_startid + i >= (int)cached_tags_count)
		{
			if(ml_in_dir && mode_ml && !cached_tags_count)
			{
				ml_col_tout_preaction(dc, x + 2, -1);

				TextOut(dc, x + 2, y + (i * ml_list_item_height), txt_ml_add_files, (int)str_len(txt_ml_add_files));
				
				ml_col_tout_preaction(dc, -1, 0);
			}
			break;
		}

		if(!(ml_in_dir && mode_ml))
		{
			ctag = ml_cache_get(ml_pl_startid + i);
			if(ctag == (void*) -1)continue;
		}

		if(((i + ml_pl_startid) & 1))
		{
			drawrect(dc, stx, y + (i * ml_list_item_height), w, ml_list_item_height, color_ml_pl_item_col);
		}else{

		}

		if(!(ml_in_dir && mode_ml))
		{
			if((unsigned long)(ml_pl_startid + i) == ml_cache_getcurrent_item())
			{
				drawrect(dc, stx, y + (i * ml_list_item_height), w, ml_list_item_height, color_ml_pl_item_cur);
				draw_playing_item_icon = 1;
			}
		}


		if(ml_cache_issel(ml_pl_startid + i))
			drawrect(dc, stx, y + (i * ml_list_item_height), w, ml_list_item_height, color_ml_pl_item_sel);
		
		if(!mode_ml)
		{
			ml_col_tout_preaction(dc, x + 2, -1);
			if(draw_playing_item_icon)
				BitBlt(dc, x + 2, y + ((i * ml_list_item_height)) + ((ml_list_item_height - 10) / 2), 10, 10, mdc_sheet, 153, 324, SRCCOPY);

			if(nextitemid == ml_pl_startid + i)
				BitBlt(dc, x + 2, y + ((i * ml_list_item_height)) + ((ml_list_item_height - 10) / 2), 10, 10, mdc_sheet, 164, 324, SRCCOPY);

			//if(skin.shared->audio.output.playlist.ismarked(ml_pl_startid + i))
			//	BitBlt(dc, x + 2, y + ((i * ml_list_item_height)) + ((ml_list_item_height - 10) / 2), 10, 10, mdc_sheet, 164, 324, SRCCOPY);

			ml_col_tout_preaction(dc, -1, -1);
		}

		if(ml_pl_startid + i == preview_item_index)
		{
			drawrect(dc, stx + 2, y + (i * ml_list_item_height) + 2, w - 4, ml_list_item_height - 4, RGB(0, 0, 0));
			drawrect(dc, stx + 4, y + (i * ml_list_item_height) + 4, w - 8, ml_list_item_height - 8, RGB(125, 98, 0));
		}
		
		/*if(!ml_in_dir)
		{
			BitBlt(dc, stx + 1, y + (i * ml_list_item_height)      + tm_ml_pl.tmHeight + 5, 10, 10, tiny_mdc_stars, 0, 0, SRCPAINT );
			BitBlt(dc, stx + 1 + 10, y + (i * ml_list_item_height) + tm_ml_pl.tmHeight + 5, 10, 10, tiny_mdc_stars, 0, 0, SRCPAINT );
			BitBlt(dc, stx + 1 + 20, y + (i * ml_list_item_height) + tm_ml_pl.tmHeight + 5, 10, 10, tiny_mdc_stars, 0, 0, SRCPAINT );
			BitBlt(dc, stx + 1 + 30, y + (i * ml_list_item_height) + tm_ml_pl.tmHeight + 5, 10, 10, tiny_mdc_stars, 10, 0, SRCPAINT );
			BitBlt(dc, stx + 1 + 40, y + (i * ml_list_item_height) + tm_ml_pl.tmHeight + 5, 10, 10, tiny_mdc_stars, 10, 0, SRCPAINT );
			BitBlt(dc, stx + 1 + 50, y + (i * ml_list_item_height) + tm_ml_pl.tmHeight + 5, 10, 10, tiny_mdc_stars, 10, 0, SRCPAINT );
		}*/

		if(!(ml_in_dir && mode_ml))
		{
			if(skin_settings.ml_pl_xoff[header_tag_title] != -1)
			{
				ct = &ctag->tag_title;
			
				ml_col_tout_preaction(dc, x, header_tag_title);


				if(ct->tsize && ct->tdata)
				{
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_title], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				
				}else{
					const string fpath = ml_cache_getpath(ml_pl_startid + i);
					letter       fname[v_sys_maxpath];
					COLORREF     clbkp;

					if(fpath)
					{
						_wsplitpath(fpath, 0, 0, fname, 0);
						clbkp = GetTextColor(dc);
						SetTextColor(dc, RGB(200, 150, 0));

						TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_title], y + (i * ml_list_item_height), fname, (int)str_len(fname));
				
						SetTextColor(dc, clbkp);
					}
				}

				ml_col_tout_preaction(dc, -1, 0);
			}


			if(skin_settings.ml_pl_xoff[header_tag_album] != -1)
			{
				ct = &ctag->tag_album;
				ml_col_tout_preaction(dc, x, header_tag_album);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_album], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_album], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_artist] != -1)
			{
				ct = &ctag->tag_artist;
				ml_col_tout_preaction(dc, x, header_tag_artist);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_artist], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_artist], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_origartist] != -1)
			{
				ct = &ctag->tag_origartist;
				ml_col_tout_preaction(dc, x, header_tag_origartist);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_origartist], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_origartist], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_composer] != -1)
			{
				ct = &ctag->tag_composer;
				ml_col_tout_preaction(dc, x, header_tag_composer);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_composer], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_composer], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_lyricist] != -1)
			{
				ct = &ctag->tag_lyricist;
				ml_col_tout_preaction(dc, x, header_tag_lyricist);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_lyricist], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_lyricist], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_band] != -1)
			{
				ct = &ctag->tag_band;
				ml_col_tout_preaction(dc, x, header_tag_band);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_band], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_band], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_copyright] != -1)
			{
				ct = &ctag->tag_copyright;
				ml_col_tout_preaction(dc, x, header_tag_copyright);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_copyright], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_copyright], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_publish] != -1)
			{
				ct = &ctag->tag_publish;
				ml_col_tout_preaction(dc, x, header_tag_publish);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_publish], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_publish], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_encodedby] != -1)
			{
				ct = &ctag->tag_encodedby;
				ml_col_tout_preaction(dc, x, header_tag_encodedby);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_encodedby], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_encodedby], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_genre] != -1)
			{
				ct = &ctag->tag_genre;
				ml_col_tout_preaction(dc, x, header_tag_genre);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_genre], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_genre], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_year] != -1)
			{
				ct = &ctag->tag_year;
				ml_col_tout_preaction(dc, x, header_tag_year);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_year], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_year], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_url] != -1)
			{
				ct = &ctag->tag_url;
				ml_col_tout_preaction(dc, x, header_tag_url);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_url], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_url], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_offiartisturl] != -1)
			{
				ct = &ctag->tag_offiartisturl;
				ml_col_tout_preaction(dc, x, header_tag_offiartisturl);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_offiartisturl], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_offiartisturl], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_filepath] != -1)
			{
				const string ctag_file_path = ml_cache_getpath(ml_pl_startid + i);;

				if(ctag_file_path)
				{
					ml_col_tout_preaction(dc, x, header_tag_filepath);


					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_filepath], y + (i * ml_list_item_height), ctag_file_path, (int)str_len(ctag_file_path));
					
					ml_col_tout_preaction(dc, -1, 0);
				}
			}

			if(skin_settings.ml_pl_xoff[header_tag_filename] != -1)
			{
				int          ctag_pt;
				const string ctag_file_path = ml_cache_getpath(ml_pl_startid + i);

				if(ctag_file_path)
				{
					ml_col_tout_preaction(dc, x, header_tag_filename);

					ctag_pt = (int)str_len(ctag_file_path);

					for(; ctag_pt>=0; ctag_pt--)
						if(ctag_file_path[ctag_pt] == '/' || ctag_file_path[ctag_pt] == '\\')break;

					ctag_pt++;

					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_filename], y + (i * ml_list_item_height), ctag_file_path + ctag_pt, (int)str_len(ctag_file_path + ctag_pt));

					ml_col_tout_preaction(dc, -1, 0);
				}
			}

			if(skin_settings.ml_pl_xoff[header_tag_bpm] != -1)
			{
				ct = &ctag->tag_bpm;
				ml_col_tout_preaction(dc, x, header_tag_bpm);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_bpm], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_bpm], y + (i * ml_list_item_height), unkstr, unklen);

				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_tracknum] != -1)
			{
				ct = &ctag->tag_tracknum;
				ml_col_tout_preaction(dc, x, header_tag_tracknum);
				if(ct->tsize && ct->tdata)
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_tracknum], y + (i * ml_list_item_height), ct->tdata, (int)str_len(ct->tdata));
				else
					TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_tracknum], y + (i * ml_list_item_height), unkstr, unklen);

				
				ml_col_tout_preaction(dc, -1, 0);
			}

			if(skin_settings.ml_pl_xoff[header_tag_index] != -1)
			{
				letter tbuf[16];

				ml_col_tout_preaction(dc, x, header_tag_index);

				memset(tbuf, 0, sizeof(tbuf));

				if(lindex_mode == 0)
					_itow(ml_pl_startid + i + 1, tbuf, 10);
				else
					_itow(ml_cache_getlistindex(ml_pl_startid + i) + 1, tbuf, 10);

				TextOut(dc, x + skin_settings.ml_pl_xoff[header_tag_index], y + (i * ml_list_item_height), tbuf, (int)str_len(tbuf));
			
				ml_col_tout_preaction(dc, -1, 0);
			}
		}else{ /* directory name only */

			if(cached_tags)
			{
				if((unsigned long)(ml_pl_startid + i) < cached_tags_count)
				{
					size_t   stxlen;
					size_t   descstrlen;
					string   stxstr;

					ml_col_tout_preaction(dc, x + 2, -1);

					stxstr = cached_tags[ml_pl_startid + i].dname;
					stxlen = str_len(stxstr);

					if(!stxlen)
					{
						TextOut(dc, x + 2, y + (i * ml_list_item_height), uni("[Empty]"), (int)str_len(uni("[Empty]")));
					}else{
						TextOut(dc, x + 2, y + (i * ml_list_item_height), stxstr, (int)stxlen);
					}

					/* print a few album names for quick selection */

					if(ml_current_dir == media_library_dir_artists) 
					{
						struct fennec_audiotag  at;
						int            c, xoffset = 0;
						uint32_t       k = 0;
						unsigned long  j = 0, m = 0, n, albfound, albid = 0;
						SIZE           expoint;
						HFONT          font_old;
						string         albm_names[10];


						for(n=0; n<10; n++)
							albm_names[n] = 0;


						font_old = (HFONT) SelectObject(dc, font_ml_pl_small_link);

						for(;;)
						{
							c = skin.shared->mlib.media_library_get_dir_files(j, ml_current_dir, cached_tags[ml_pl_startid + i].dname, &k);
							if(!c) break;

							skin.shared->mlib.media_library_translate(k, &at);
							if(at.tag_album.tsize)
							{
								albfound = 0;
								for(n=0; n<m; n++)
								{
									if(albm_names[n])
									{
										if(!str_icmp(albm_names[n], at.tag_album.tdata))
										{
											albfound = 1;
											break;
										}
									}
								}

								if(!albfound)
								{
									descstrlen = str_len(at.tag_album.tdata);
									TextOut(dc, x + 20 + xoffset, y + (i * ml_list_item_height) + tm_ml_pl.tmHeight, at.tag_album.tdata, (int)descstrlen);
									GetTextExtentPoint32(dc, at.tag_album.tdata, (int)descstrlen, &expoint);
									xoffset += expoint.cx + 5;

									cached_tags[ml_pl_startid + i].album_points[albid] = xoffset;
									cached_tags[ml_pl_startid + i].album_points_album_of[albid] = k;

									albm_names[m] = at.tag_album.tdata;
									
									ml_artists_album_pos_x = 20;
									ml_artists_album_pos_y = tm_ml_pl.tmHeight;
									ml_artists_album_pos_h = expoint.cy;

									m++;
									albid++;
									if(m >= 10) break;
								}
							}
							
							k++;
						}

						SelectObject(dc, font_old);
					}

					ml_col_tout_preaction(dc, -1, 0);
				}
			}
		}

	}

	ml_scroll_draw_v(dc, stx + w, sty, ml_pl_scroll_size, h - ml_pl_scroll_size, (int)cached_tags_count, c, ml_pl_startid);
	ml_scroll_draw_h(dc, stx, sty + h, w - ml_pl_scroll_size, ml_pl_scroll_size, (int)ml_col_getmax_extent() + 200, w, ml_col_xpos);


	if(preview_item_index != -1)
	{
		BitBlt(hdc_ml, preview_panel_x, preview_panel_y, preview_panel_w, preview_panel_h, mdc_sheet, 1, 334, SRCCOPY);
	}

}

int ml_pl_preview_display_timer(void)
{
	double p = 10;
	int    ppos;

	if(preview_item_index != -1)
	{
		skin.shared->call_function(call_audio_preview_action, audio_preview_getpos, &p, 0);
		
		ppos = (int)(preview_panel_seek_w * p);
		BitBlt(hdc_ml, preview_panel_x + preview_panel_seek_x + ppos, preview_panel_y + preview_panel_seek_y, preview_panel_seek_w - ppos, preview_panel_seek_h, mdc_sheet, 25, 335, SRCCOPY);
		BitBlt(hdc_ml, preview_panel_x + preview_panel_seek_x, preview_panel_y + preview_panel_seek_y, ppos, preview_panel_seek_h, mdc_sheet, 127, 335, SRCCOPY);
	}

	return 1;
}


/*
 * mouse message callback.
 */
int ml_pl_mousemsg(int x, int y, int m)
{
	static int rdown = 0, ldown = 0, mdown = 0;
	static sel_column_dx = 0;
	static int down_s_v = 0, down_s_h = 0, fdown = 1;
	static int dx, dy, dys, dxs;
	static int col_moved;
	static int hdown = 0;
	
	unsigned long curitem;

	if (m == mm_down_r){rdown = 1; dx = x; dy = y; dys = dy - vscroll_y;  dxs = dx - hscroll_x;}
	else if(m == mm_up_r)rdown = 0;

	if (m == mm_down_l){ldown = 1; dx = x; dy = y; dys = dy - vscroll_y;  dxs = dx - hscroll_x;}
	else if(m == mm_up_l)ldown = 0;

	/* <preview> */

	if(m == mm_move && preview_item_index != -1 /* if preview is working */)
	{
		if(incoord(x, y, preview_panel_x, preview_panel_y, preview_panel_w, preview_panel_h))
		{
			int ppx, ppy, ppw, pph;

			
			ppx = preview_panel_x;
			ppy = preview_panel_y;
			ppw = preview_panel_w;
			pph = preview_panel_h;


			if(incoord(x, y, ppx + preview_panel_play_x, ppy + preview_panel_play_y, preview_panel_play_w, preview_panel_play_h))
			{
				BitBlt(hdc_ml, preview_panel_x + 1,  preview_panel_y + 1, 10, 10, mdc_sheet, 131, 324, SRCCOPY);
				BitBlt(hdc_ml, preview_panel_x + 12, preview_panel_y + 1, 10, 10, mdc_sheet, 13, 335, SRCCOPY); /* an easy way without a static value */

			}else if(incoord(x, y, ppx + preview_panel_stop_x, ppy + preview_panel_stop_y, preview_panel_stop_w, preview_panel_stop_h)){
				
				BitBlt(hdc_ml, preview_panel_x + 12, preview_panel_y + 1, 10, 10, mdc_sheet, 142, 324, SRCCOPY);
				BitBlt(hdc_ml, preview_panel_x + 1,  preview_panel_y + 1, 10, 10, mdc_sheet, 2, 335, SRCCOPY); /* an easy way without a static value */
			}
			
		}
	}

	if(m == mm_down_l && preview_item_index != -1 /* if preview is working */)
	{
		if(incoord(x, y, preview_panel_x + preview_panel_play_x, preview_panel_y + preview_panel_play_y, preview_panel_play_w, preview_panel_play_h))
		{
			skin.shared->call_function(call_audio_preview_action, audio_preview_play, 0, 0);

		}else if(incoord(x, y, preview_panel_x + preview_panel_stop_x, preview_panel_y + preview_panel_stop_y, preview_panel_stop_w, preview_panel_stop_h)){
				
			skin.shared->call_function(call_audio_preview_action, audio_preview_stop, 0, 0);
			preview_item_index = -1;
			ml_draw_window(redraw_all);

		}else if(incoord(x, y, preview_panel_x + preview_panel_seek_x, preview_panel_y + preview_panel_seek_y, preview_panel_seek_w, preview_panel_seek_h)){
		
			double p;
			p = (double)(x - preview_panel_x - preview_panel_seek_x) / (double)preview_panel_seek_w;
			
			if(p > 1.0)p = 1.0;
			else if(p < 0.0)p = 0.0;

			skin.shared->call_function(call_audio_preview_action, audio_preview_seek, &p, 0);

		}
	}


	/* </preview> */
	/* <header> */

	if(hdown || incoord(x, y, ml_pl_x, ml_pl_y, ml_pl_w, ml_list_header_height))
	{
		int mx, my;

		mx = x - ml_pl_x;
		my = y - ml_pl_y;

		if(m == mm_up_l)
		{
			hdown = 0;
		}

		if(m == mm_down_l || m == mm_dbl_l)
		{
			/* find column id */

			int ux = mx + ml_col_xpos;

			int i, sel = -1;

			col_moved = 0;

			for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(int); i++)
			{
				if(skin_settings.ml_pl_xoff[i] == -1)continue;

				if(skin_settings.ml_pl_xoff[i] < ux)
				{
					if(sel != -1)
					{
						if(skin_settings.ml_pl_xoff[i] > skin_settings.ml_pl_xoff[sel])
							sel = i;
					}else{
						sel = i;
					}
				}
			}

			sel_column_dx = ux - skin_settings.ml_pl_xoff[sel];
			sel_column = sel;

			hdown = 1;

			if(m == mm_dbl_l)
			{
				if(mode_ml && ml_in_dir)
				{
					if(skin_settings.ml_dir_sort_mode != 2)
						skin_settings.ml_dir_sort_mode = 2;
					else
						skin_settings.ml_dir_sort_mode = 3;

					list_sort_column(sel_column, skin_settings.ml_dir_sort_mode);

				}else{
					if(skin_settings.ml_sorted_column == sel_column)
						list_sort_column(sel_column, !skin_settings.ml_sorted_mode);
					else
						list_sort_column(sel_column, 0 /* ascending */);
				}

				/* ml_draw_window(redraw_playlist), if single click shouldn't do the work */
			}
			
			ml_draw_window(redraw_playlist);

			return 1;
		}



		if((m == mm_move) && ldown && hdown)
		{
			if(sel_column != -1)
			{
				int lastxo = skin_settings.ml_pl_xoff[sel_column];

				if(mx >= sel_column_dx)
					skin_settings.ml_pl_xoff[sel_column] = mx + ml_col_xpos - sel_column_dx;
				else
					skin_settings.ml_pl_xoff[sel_column] = 0;
				ml_draw_window(redraw_playlist);

				if(lastxo != skin_settings.ml_pl_xoff[sel_column])
					col_moved = 1;
			}

			return 1;
		}
	}

	/* </header> */


	if(!down_s_v && !down_s_h)
	{
		if(!incoord(x, y, ml_pl_x, ml_pl_y, ml_pl_w + ml_pl_scroll_size, ml_pl_h + ml_pl_scroll_size) && (m != mm_wheel))return -1;
	}

	x -= ml_pl_x;
	y -= ml_pl_y;

	curitem = min((unsigned long)(ml_pl_startid + ((y - ml_list_header_height) / ml_list_item_height)), cached_tags_count);

	if((m == mm_move || m == mm_up_l) && mode_ml && (ml_current_dir == media_library_dir_artists))
	{
		if(cached_tags[curitem].album_points[0] != 0)
		{
			int cur_y = y - ((curitem - ml_pl_startid) * ml_list_item_height) - ml_list_header_height;
			if((x < ml_pl_w) && (x > ml_artists_album_pos_x) && cur_y > ml_artists_album_pos_y && cur_y < (ml_artists_album_pos_y + ml_artists_album_pos_h))
			{
				int  albx = x + ml_col_xpos - ml_artists_album_pos_x, i;

				for(i=0; i<64; i++)
				{
					if(albx < cached_tags[curitem].album_points[i])
					{
						SetCursor(LoadCursor(0, IDC_HAND));
						if(m == mm_up_l)
						{
							struct fennec_audiotag at;
							unsigned long albid = cached_tags[curitem].album_points_album_of[i];
							skin.shared->mlib.media_library_translate(albid, &at);
							
							ml_last_dir_index = ml_pl_startid;
							ml_pl_startid     = 0;
							hscroll_v         = 0;
							ml_col_xpos       = 0;

							ml_last_dir = ml_current_dir;
							
							str_cpy(ml_cdir_text, at.tag_album.tdata);
							ml_current_cdir = ml_cdir_text;

							
							ml_cache_uninit();
				
							ml_current_dir  = media_library_dir_albums;
							
							

							ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
							ml_dir_changed = 1;

							ml_cache_init();
							ml_draw_window(redraw_playlist_scroll);
							return 1; /* must return, else: all the following code would be applied to the new dir */
						}
						break;
					}
					if(!cached_tags[curitem].album_points[i])break;
				}
			}
		}
	}
	
	if((m == mm_move) && ldown && !down_s_v && !down_s_h)
	{
		if((pl_last_cur != curitem) && !fdown && !mode_ml)
		{
			unsigned long i, ok = 1;

			/* get lowest */

			if(curitem < pl_last_cur)
			{
				for(i=0; i<cached_tags_count; i++)
				{
					if(cached_tags[i].sel)
					{
						if(i < (pl_last_cur - curitem))ok = 0;
						break;
					}
				}
			}else if(pl_last_cur < curitem){

				for(i=(cached_tags_count-1); i>0; i--)
				{
					if(cached_tags[i].sel)
					{
						if(i + (curitem - pl_last_cur) >= cached_tags_count)ok = 0;
						break;
					}
				}
			}



			if(ok)
			{
				if(curitem < pl_last_cur)
				{
					for(i=0; i<cached_tags_count; i++)
					{
						if(cached_tags[i].sel)
							skin.shared->audio.output.playlist.move(i + ((long)curitem - (long)pl_last_cur), i, 0);
					}
				}else if(curitem > pl_last_cur){

					for(i=(cached_tags_count-1); i>0; i--)
					{
						if(cached_tags[i].sel)
							skin.shared->audio.output.playlist.move(i + ((long)curitem - (long)pl_last_cur), i, 0);
					}
				}

				ml_cache_shiftsel((long)curitem - (long)pl_last_cur);
				pl_last_cur = curitem;
				ml_cache_freetags();
				ml_draw_window(redraw_playlist);
			}
		}
		return 1;
	}


	if(m == mm_down_r)
	{
		/* find column id */

		int ux = x + ml_col_xpos;

		int i, sel = -1;

		col_moved = 0;

		for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(int); i++)
		{
			if(skin_settings.ml_pl_xoff[i] == -1)continue;

			if(skin_settings.ml_pl_xoff[i] < ux)
			{
				if(sel != -1)
				{
					if(skin_settings.ml_pl_xoff[i] > skin_settings.ml_pl_xoff[sel])
						sel = i;
				}else{
					sel = i;
				}
			}
		}

		sel_column_dx = ux - skin_settings.ml_pl_xoff[sel];
		sel_column = sel;

		return 1;
	}

	if(m == mm_up_r && !col_moved)
	{
		POINT pt;
		HMENU mc = user_create_menu(menu_ml_popup, mode_ml);
		
		GetCursorPos(&pt);

		switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_ml, 0))
		{
		case mid_edittags:
			if(curitem < cached_tags_count)
				skin.shared->general.show_tageditor(ml_cache_getpath(curitem), 0, 0);
			break;

		case mid_removesel:
			if(curitem < cached_tags_count)
			{
				skin.shared->audio.output.playlist.remove(curitem);
				ml_cache_uninit();
				ml_cache_init();
				ml_draw_window(redraw_playlist);
			}
			break;

		case mid_preview:
			if(curitem < cached_tags_count)
			{
				skin.shared->call_function(call_audio_preview_action, audio_preview_load, ml_cache_getpath(curitem), 0);
				skin.shared->call_function(call_audio_preview_action, audio_preview_play, 0, 0);
				preview_item_index = curitem;
				ml_draw_window(redraw_playlist);
			}
			break;

		case mid_preview_stop:
			if(curitem < cached_tags_count)
			{
				skin.shared->call_function(call_audio_preview_action, audio_preview_stop, 0, 0);
				preview_item_index = -1;
				ml_draw_window(redraw_playlist);
			}
			break;

		case mid_addtoplaylist:
			if(curitem < ml_cache_getcount() && ml_current_dir != media_library_dir_root && mode_ml && ml_in_dir)
			{
				struct fennec_audiotag   at;
				unsigned long            i, c;
				uint32_t                 k = 0;

				for(i=0;;)
				{
					c = skin.shared->mlib.media_library_get_dir_files(i, ml_current_dir, cached_tags[curitem].dname, &k);
					if(!c) break;

					skin.shared->mlib.media_library_translate(k, &at);

					if(at.tag_filepath.tsize)
						skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);

					k++;
				}

			}else if(curitem < ml_cache_getcount() && ml_current_dir != media_library_dir_root && mode_ml && !ml_in_dir){
				ml_cache_add_to_playlist((y / ml_list_item_height) + ml_pl_startid);
			}

			break;
		}

		DestroyMenu(mc);
		
		return 1;
	}

	if((m == mm_move) && rdown)
	{
		if(sel_column != -1)
		{
			int lastxo = skin_settings.ml_pl_xoff[sel_column];

			if(x >= sel_column_dx)
				skin_settings.ml_pl_xoff[sel_column] = x + ml_col_xpos - sel_column_dx;
			else
				skin_settings.ml_pl_xoff[sel_column] = 0;
			ml_draw_window(redraw_playlist);

			if(lastxo != skin_settings.ml_pl_xoff[sel_column])
				col_moved = 1;
		}

		return 1;
	}

	if(m == mm_dbl_l)
	{
		unsigned long i = ((y - ml_list_header_height)  / ml_list_item_height) + ml_pl_startid;

		if(mode_ml && (GetKeyState(VK_CONTROL) & 0x8000))
		{
			ml_cache_add_to_playlist(i);

		}else{

			if(i < cached_tags_count)
			{
				ml_cache_switchlist(i, 1);
			}

			pl_cur_item = curitem;
		}

		return 1;
	}

	if(m == mm_wheel)
	{
		if(GetKeyState(VK_CONTROL) & 0x8000)
		{
			if(x > 0)
				skin_settings.ml_font_size++;
			else
				skin_settings.ml_font_size--;

			if(skin_settings.ml_font_size < 6)skin_settings.ml_font_size = 6;

			DeleteObject(font_ml_pl);

			font_ml_pl = CreateFont(-MulDiv(skin_settings.ml_font_size, GetDeviceCaps(hdc_ml, LOGPIXELSY), 72),
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, /*ANTIALIASED_QUALITY*/5,
                                DEFAULT_PITCH, skin_settings.font_display);

			ml_draw_window(redraw_playlist_scroll);

		}else{
			ml_pl_startid_last = ml_pl_startid;

			if(x > 0)
				ml_pl_startid -= 3;
			else
				ml_pl_startid += 3;

			
			if(ml_pl_startid < 0)ml_pl_startid = 0;
			
			if((unsigned long)ml_pl_startid > 2+ /* <- just to keep free space */ cached_tags_count - (ml_pl_h / ml_list_item_height) )
				ml_pl_startid = 2+ /* <- just to keep free space */ cached_tags_count - (ml_pl_h / ml_list_item_height);

			ml_draw_window(redraw_playlist_scroll);

		}
		return 1;
	}

	if(x > ml_pl_w && x < ml_pl_w + ml_pl_scroll_size && m == mm_down_l)
		down_s_v = 1;

	if(y > ml_pl_h && y < ml_pl_h + ml_pl_scroll_size && m == mm_down_l)
		down_s_h = 1;

	if((m == mm_move) && down_s_v)
	{
		int c = cached_tags_count - (ml_pl_h / ml_list_item_height) +2 /* <- just to keep free space */;
		int vm;

		if(last_v_scroll_h)
			vm = last_v_scroll_h - vscroll_bh;
		else
			vm = ml_pl_h - vscroll_bh - ml_pl_scroll_size;

		/* fake position */

		vscroll_v = min(max(y - dys + ml_pl_y, 0), vm);
		ml_pl_startid = (long)((double)vscroll_v / (double)vm * (double)c);
		
		
		ml_draw_window(redraw_playlist);
		return 1;
	}

	if((m == mm_move) && down_s_h)
	{
		int c = max((ml_col_getmax_extent() + 200) - ml_pl_w, 0);

		int vm = ml_pl_w - hscroll_bw - ml_pl_scroll_size;

		/* fake position */

		hscroll_v = min(max(x - dxs + ml_pl_x, 0), vm);

		ml_col_xpos = (int)((double)hscroll_v / (double)vm * (double)c);
		//ml_col_xpos = (max(x - (hscroll_bw / 2), 0) * c) / (ml_pl_w - ml_pl_scroll_size - hscroll_bw);
		//if(ml_col_xpos > c)ml_col_xpos = c;
		ml_draw_window(redraw_playlist);

		return 1;
	}

	if((m == mm_down_l) && !down_s_v && !down_s_h)
	{
		if(!(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000))
		{
			pl_last_cur = curitem;
			ml_cache_clearsel();
			ml_cache_setsel(curitem, sel_toggle);
			ml_draw_window(redraw_playlist);
		}

		
	}

	if((m == mm_up_l) && !down_s_v && !down_s_h)
	{
		if(GetKeyState(VK_SHIFT) & 0x8000) /* shift down */
		{
			unsigned long i;

			if(pl_last_cur >= cached_tags_count)
				pl_last_cur = 0;
				
			if(pl_last_cur > curitem)
			{
				for(i=curitem; i<pl_last_cur; i++)
				{
					ml_cache_setsel(i, 1);
				}
			}else{
				for(i=pl_last_cur; i<=curitem; i++)
				{
					ml_cache_setsel(i, 1);
				}
			}

		}else{

			if(GetKeyState(VK_CONTROL) & 0x8000)
				ml_cache_setsel(curitem, sel_toggle);
		}

		pl_last_cur = curitem;
		pl_cur_item = curitem;
		fdown = 0;
		ml_draw_window(redraw_playlist);

		return 1;
	}

	if(m == mm_up_l)
	{
		down_s_v = 0;
		down_s_h = 0;
	}

	if(m == mm_up_l)
		down_s_v = 0;


	return 0;
}


void ml_draw_header(HDC dc, int x, int y, int w, int h)
{
	COLORREF lc;
	HRGN crgn;
	int  i, tid = 0;

	drawrect(dc, x, y, w, ml_list_header_height, color_setbrightness(color_ml_pl_header, 1.7));

	lc = GetTextColor(dc);
	SetTextColor(dc, color_setbrightness(color_ml_pl_item_cur, 1.7));
	
	if(ml_in_dir && mode_ml)
	{
		int    xs = 0;
		string dname = uni("");
		
		switch(ml_current_dir)
		{
		case media_library_dir_root:     dname = uni("Root");       break;
		case media_library_dir_all:      dname = uni("All Music"); break;
		case media_library_dir_rating:   dname = uni("Rating");    break;
		case media_library_dir_artists:  dname = uni("Artists");   break;
		case media_library_dir_albums:   dname = uni("Albums");    break;
		case media_library_dir_genres:   dname = uni("Genres");    break;
		case media_library_dir_years:    dname = uni("Years");     break;
		}

		if(skin_settings.ml_dir_sort_mode >= 2 && ml_current_dir != media_library_dir_root)
		{
			xs = 10;
			if(skin_settings.ml_dir_sort_mode == 2)
				BitBlt(dc, x + 1, y + (h / 2) - 3, 6, 6, mdc_sheet, 161, 165, SRCCOPY);
			else
				BitBlt(dc, x + 1, y + (h / 2) - 3, 6, 6, mdc_sheet, 154, 165, SRCCOPY);
		}

		TextOut(dc, x + xs + 2 - ml_col_xpos, y, dname, (int)str_len(dname));
			
	}else{

		for(i=0; i<sizeof(skin_settings.ml_pl_xoff)/sizeof(skin_settings.ml_pl_xoff[0]); i++)
		{
			if(skin_settings.ml_pl_xoff[i] == -1) continue;

			switch(i)
			{
			case header_tag_title:          tid = oooo_tag_title; break;
			case header_tag_album:          tid = oooo_tag_album; break;
			case header_tag_artist:         tid = oooo_tag_artist; break;
			case header_tag_origartist:     tid = oooo_tag_origartist; break;
			case header_tag_composer:       tid = oooo_tag_composer; break;
			case header_tag_lyricist:       tid = oooo_tag_lyricist; break;
			case header_tag_band:			tid = oooo_tag_band; break;
			case header_tag_copyright:		tid = oooo_tag_copyright; break;
			case header_tag_publish:		tid = oooo_tag_publish; break;
			case header_tag_encodedby:		tid = oooo_tag_encodedby; break;
			case header_tag_genre:          tid = oooo_tag_genre; break;
			case header_tag_year:			tid = oooo_tag_year; break;
			case header_tag_url:			tid = oooo_tag_url; break;
			case header_tag_offiartisturl:	tid = oooo_tag_offiartisturl; break;
			case header_tag_filepath:		tid = oooo_tag_filepath; break;
			case header_tag_filename:       tid = oooo_tag_filename; break;
			case header_tag_comments:		tid = oooo_tag_comments; break;
			case header_tag_lyric:			tid = oooo_tag_lyric; break;
			case header_tag_bpm:			tid = oooo_tag_bpm; break;
			case header_tag_tracknum:		tid = oooo_tag_tracknum; break;
			case header_tag_index:		    tid = oooo_tag_index; break;
			}

			{
				int cx, cy, cw, ch, r, xs = 0;

				cx = x + skin_settings.ml_pl_xoff[i] - ml_col_xpos;
				cw = ml_col_getwidth(i) - 5;
				cy = y;
				ch = h;

				//xs += media_status_icon_width;

				r = cx + cw;

				cx = max(x, cx);
				r  = max(x, r);
				r  = min(r, w + x);

				if(skin_settings.ml_sorted_column == i)
				{
					xs += 10;
					if(skin_settings.ml_sorted_mode == 0)
						BitBlt(dc, cx + 1, y + (h / 2) - 3, 6, 6, mdc_sheet, 161, 165, SRCCOPY);
					else
						BitBlt(dc, cx + 1, y + (h / 2) - 3, 6, 6, mdc_sheet, 154, 165, SRCCOPY);
				}

				if(i == sel_column)
				{
				
					BitBlt(dc, cx + 1 + xs, y + (h / 2) - 3, 6, 6, mdc_sheet, 168, 165, SRCCOPY);
					xs += 10;
				}

				crgn = CreateRectRgn(cx, cy, r, cy + ch);

				SelectClipRgn(dc, crgn);

				TextOut(dc, x + xs + skin_settings.ml_pl_xoff[i] - ml_col_xpos, y, skin.shared->language_text[tid], (int)str_len(skin.shared->language_text[tid]));
			
				SelectClipRgn(dc, 0);
				DeleteObject(crgn);
			}
		}
	
	}

	SetTextColor(dc, lc);
}


/*
 * (re)draw window/list.
 */
void ml_draw_window(int rd)
{
	int  c;
	int  i;
	RECT rct;
	HDC      mdc_pl;
	HBITMAP  mbm_pl;

	int pl_x = coords.window_ml.list_l, pl_y = coords.window_ml.list_t;
	int pl_w, pl_h;
	
	if(!ml_init)return;

	GetClientRect(window_ml, &rct);
		
	pl_w = rct.right  - coords.window_ml.list_l - coords.window_ml.list_r;
	pl_h = rct.bottom - coords.window_ml.list_t - coords.window_ml.list_b;	
	
	ml_pl_x = pl_x + 2;
	ml_pl_y = pl_y + 2;
	ml_pl_w = pl_w - 4 - ml_pl_scroll_size;
	ml_pl_h = pl_h - 4 - ml_pl_scroll_size;


	if(fast_pl && (rd == redraw_playlist || rd == redraw_playlist_scroll))
	{

		mdc_pl = CreateCompatibleDC(hdc_ml);
		mbm_pl = CreateCompatibleBitmap(hdc_ml, pl_w, pl_h);

		SelectObject(mdc_pl, mbm_pl);

		SelectObject(mdc_pl, font_ml_pl);

		GetTextMetrics(mdc_pl, &tm_ml_pl);

		if(mode_ml && ml_current_dir == media_library_dir_artists && !ml_current_cdir)
		{
			ml_list_item_height = artist_field_item_height;
		}else{
			ml_list_item_height = tm_ml_pl.tmHeight + 2;
		}
	

		//ml_pl_y = pl_y + 2 + ml_list_item_height;
		//ml_pl_h = pl_h - 4 - ml_pl_scroll_size - ml_list_item_height;


		SetTextColor(mdc_pl, color_ml_pl_normal);
		SetBkMode(mdc_pl, TRANSPARENT);

		{
			drawrect(mdc_pl, 0, 0, pl_w, pl_h, color_ml_pl_faint);

			

			if(ml_init)
			{
				ml_draw_header(mdc_pl, 2, 2, ml_pl_w, ml_list_header_height);
				ml_playlist_draw(mdc_pl, 2, 2 + ml_list_header_height, ml_pl_w, ml_pl_h - ml_list_header_height);
			}
		}
		
		BitBlt(hdc_ml, pl_x, pl_y, pl_w, pl_h, mdc_pl, 0, 0, SRCCOPY);

		DeleteDC(mdc_pl);
		DeleteObject(mbm_pl);
		return;
	}


	/* rgn */


	if(rd == redraw_all)
	{
		if(rgn_ml)DeleteObject(rgn_ml);
		rgn_ml = CreateRoundRectRgn(0, 0, rct.right, rct.bottom, 5, 5);
		SetWindowRgn(window_ml, rgn_ml, 1);
	}

	mdc_pl = CreateCompatibleDC(hdc_ml);
	mbm_pl = CreateCompatibleBitmap(hdc_ml, rct.right, rct.bottom);

	SelectObject(mdc_pl, mbm_pl);


	SelectObject(mdc_pl, font_ml_pl);

	GetTextMetrics(mdc_pl, &tm_ml_pl);

	if(mode_ml && ml_current_dir == media_library_dir_artists && !ml_current_cdir)
	{
		ml_list_item_height = artist_field_item_height;
	}else{
		ml_list_item_height = tm_ml_pl.tmHeight + 2;
	}
	//ml_pl_y = pl_y + 2 + ml_list_header_height;
	//ml_pl_h = pl_h - 4 - ml_pl_scroll_size - ml_list_header_height;

	SetTextColor(mdc_pl, color_ml_pl_normal);
	SetBkMode(mdc_pl, TRANSPARENT);

	/* top */

	c = (rct.right - coords.window_ml.crop_tl.w - coords.window_ml.crop_tr.w);

	for(i=0; i<c; i+=coords.window_ml.crop_tm.w)
	{
		BitBlt(mdc_pl, coords.window_ml.crop_tl.w + i, 0, coords.window_ml.crop_tm.w, coords.window_ml.crop_tm.h, mdc_sheet, coords.window_ml.crop_tm.sx_n, coords.window_ml.crop_tm.sy_n, SRCCOPY);
	}

	/* bottom */

	c = (rct.right - coords.window_ml.crop_bl.w - coords.window_ml.crop_br.w);

	for(i=0; i<c; i+=coords.window_ml.crop_bm.w)
	{
		BitBlt(mdc_pl, coords.window_ml.crop_bl.w + i, rct.bottom - coords.window_ml.crop_br.h, coords.window_ml.crop_bm.w, coords.window_ml.crop_bm.h, mdc_sheet, coords.window_ml.crop_bm.sx_n, coords.window_ml.crop_bm.sy_n, SRCCOPY);
	}

	/* left */

	c = (rct.bottom - coords.window_ml.crop_tl.h - coords.window_ml.crop_bl.h);

	for(i=0; i<c; i+=coords.window_ml.crop_ml.h)
	{
		BitBlt(mdc_pl, 0, coords.window_ml.crop_tl.h + i, coords.window_ml.crop_ml.w, coords.window_ml.crop_ml.h, mdc_sheet, coords.window_ml.crop_ml.sx_n, coords.window_ml.crop_ml.sy_n, SRCCOPY);
	}

	/* right */

	c = (rct.bottom - coords.window_ml.crop_tl.h - coords.window_ml.crop_bl.h);

	for(i=0; i<c; i+=coords.window_ml.crop_mr.h)
	{
		BitBlt(mdc_pl, rct.right - coords.window_ml.crop_mr.w, coords.window_ml.crop_tl.h + i, coords.window_ml.crop_mr.w, coords.window_ml.crop_mr.h, mdc_sheet, coords.window_ml.crop_mr.sx_n, coords.window_ml.crop_mr.sy_n, SRCCOPY);
	}


	/* finalize */

	BitBlt(mdc_pl, 0, 0, coords.window_ml.crop_tl.w, coords.window_ml.crop_tl.h, mdc_sheet, coords.window_ml.crop_tl.sx_n, coords.window_ml.crop_tl.sy_n, SRCCOPY);
	BitBlt(mdc_pl, rct.right - coords.window_ml.crop_tr.w, 0, coords.window_ml.crop_tr.w, coords.window_ml.crop_tr.h, mdc_sheet, coords.window_ml.crop_tr.sx_n, coords.window_ml.crop_tr.sy_n, SRCCOPY);
	BitBlt(mdc_pl, 0, rct.bottom - coords.window_ml.crop_bl.h, coords.window_ml.crop_bl.w, coords.window_ml.crop_bl.h, mdc_sheet, coords.window_ml.crop_bl.sx_n, coords.window_ml.crop_bl.sy_n, SRCCOPY);
	BitBlt(mdc_pl, rct.right - coords.window_ml.crop_br.w, rct.bottom - coords.window_ml.crop_br.h, coords.window_ml.crop_br.w, coords.window_ml.crop_br.h, mdc_sheet, coords.window_ml.crop_br.sx_n, coords.window_ml.crop_br.sy_n, SRCCOPY);


	i = 1;
	while(ml_draw_normalex(i, mdc_pl))i++;
		
	
	drawrect(mdc_pl, pl_x, pl_y, pl_w, pl_h, color_ml_pl_faint);

	if(ml_init)
	{
		ml_draw_header(mdc_pl, ml_pl_x, ml_pl_y, ml_pl_w, ml_list_header_height);
		ml_playlist_draw(mdc_pl, ml_pl_x, ml_pl_y + ml_list_header_height, ml_pl_w, ml_pl_h - ml_list_header_height);
	}

	if(preview_item_index != -1)
		BitBlt(mdc_pl, preview_panel_x, preview_panel_y, preview_panel_w, preview_panel_h, mdc_sheet, 1, 334, SRCCOPY);


	BitBlt(hdc_ml, 0, 0, rct.right, rct.bottom, mdc_pl, 0, 0, SRCCOPY);

	DeleteDC(mdc_pl);
	DeleteObject(mbm_pl);

}


/* callbacks ----------------------------------------------------------------*/


/*
 * main window callback.
 */
LRESULT CALLBACK callback_ml_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, dx = 0, dy = 0, hb = 0;

	switch(msg)
	{
	case WM_CREATE:
		hdc_ml = GetDC(hwnd);
		break;

	case WM_CHAR:
		{
			unsigned long i, first_sel = 0;
			letter        inchr = (letter)wParam;
			static letter isearch_text[128];
			string        txt;

			if(isearch_pos > 126)isearch_pos = 0;
			if(inchr == (letter)VK_BACK && isearch_pos > 0)
			{
				isearch_pos--;
			}else{
				isearch_text[isearch_pos]     = inchr;
				isearch_text[isearch_pos + 1] = 0;
				isearch_pos++;
			}

			for(i=0; i<cached_tags_count; i++)
			{
				cached_tags[i].sel = 0;
			}
			
			if(isearch_pos == 0)
			{
				ml_pl_startid = 0;
				pl_cur_item   = 0;
				ml_draw_window(0);
				break;
			}

			for(i=0; i<cached_tags_count; i++)
			{
				if(mode_ml && ml_in_dir && !ml_current_cdir)
				{
					txt = cached_tags[i].dname;
				}else{
					txt = cached_tags[i].ft.tag_title.tdata;
				}


				if(!txt) continue;

				if(str_incmp(txt, isearch_text, isearch_pos) == 0)
				{
					int plmc = ml_pl_h / ml_list_item_height;
					
					cached_tags[i].sel = 1;

					if(!first_sel)
					{
						pl_cur_item        = i;

						if((long)pl_cur_item > plmc + ml_pl_startid - 1)
						{
							if((long)pl_cur_item >= plmc)
								ml_pl_startid = pl_cur_item - plmc + 2;
							else
								ml_pl_startid = 0;
						}else if((long)pl_cur_item < ml_pl_startid){
							ml_pl_startid = pl_cur_item;
						}
					}

					first_sel          = 1;
					
				}
			}

			KillTimer(hwnd, timer_id_i_search);
			SetTimer(hwnd, timer_id_i_search, 2000, 0);
			ml_draw_window(0);
			break;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_DELETE:
			ml_cache_removesel();
			ml_draw_window(0);
			isearch_pos = 0;
			break;

		case VK_DOWN:
			{
				unsigned long i, foundsel = 0;

				for(i=0; i<cached_tags_count; i++)
				{
					if(cached_tags[i].sel)
					{
						foundsel = 1;
						break;
					}
				}

				if(!foundsel)
				{
					cached_tags[0].sel = 1;
					pl_cur_item = 0;
				}

				if(!ml_cache_shiftsel(1))
				{
					int plmc = ml_pl_h / ml_list_item_height;
					pl_cur_item++;

					if(pl_cur_item >= (unsigned long)(ml_pl_startid + plmc))
					{
						ml_pl_startid = pl_cur_item - plmc + 1;
					}
					
					ml_draw_window(0);
				}
				isearch_pos = 0;
			}
			break;

		case VK_UP:
			{
				unsigned long i, foundsel = 0;

				for(i=0; i<cached_tags_count; i++)
				{
					if(cached_tags[i].sel)
					{
						foundsel = 1;
						break;
					}
				}

				if(!foundsel)
				{
					cached_tags[0].sel = 1;
					pl_cur_item = 0;
				}

				if(!ml_cache_shiftsel(-1))
				{
					if(pl_cur_item > 0)pl_cur_item--;
					if(pl_cur_item < (unsigned long)(ml_pl_startid))
						ml_pl_startid = pl_cur_item;
					ml_draw_window(0);
				}
				isearch_pos = 0;
			}
			break;

		case VK_RETURN:
		case VK_SPACE:
			if(pl_cur_item < cached_tags_count)
			{
				ml_cache_switchlist(pl_cur_item, 1);
			}
			isearch_pos = 0;
			break;

		case VK_TAB:
			SetFocus(skin.wnd);
			isearch_pos = 0;
			break;

		case VK_ESCAPE:
			ml_back();
			break;
		}
		break;

	case WM_MOUSEMOVE:
		{
			RECT rct;
			int  x = (int)LOWORD(lParam), y = (int)HIWORD(lParam);

			GetClientRect(hwnd, &rct);

			if(x > rct.right - 20 && y > rct.bottom - 20 &&
			   x <= rct.right && y <= rct.bottom)
			{
				SetCursor(LoadCursor(0, IDC_SIZENWSE));
			}

			if(mdown == 1)
			{
				POINT pt;

				GetCursorPos(&pt);

				if(!incoord(x, y, ml_pl_x, ml_pl_y, ml_pl_w + ml_pl_scroll_size, ml_pl_h + ml_pl_scroll_size))
					move_docking_window(window_id_lib, pt.x - dx, pt.y - dy);

			}else if(mdown == 2){
				
				POINT pt;
				RECT  wrct;
				int   cx, cy;

				GetWindowRect(hwnd, &wrct);
				GetCursorPos(&pt);

				cx = max(pt.x - wrct.left - dx, 50);
				cy = max(pt.y - wrct.top - dy, 50);

				if(skin_settings.ml_d)
				{
					RECT mwrect;

					GetClientRect(skin.wnd, &mwrect);

					if(cx > mwrect.right - 20 && cx < mwrect.right + 20)cx = mwrect.right;
					if(cy > mwrect.bottom - 20 && cy < mwrect.bottom + 20)cy = mwrect.bottom;
				}

				SetWindowPos(hwnd, 0, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);

				if(skin_settings.ml_show && GetWindowRect(window_ml, &rct))
				{
					skin_settings.ml_x = rct.left;
					skin_settings.ml_y = rct.top;
					skin_settings.ml_w = rct.right - rct.left;
					skin_settings.ml_h = rct.bottom - rct.top;
					skin_settings.ml_d = skin_settings.ml_d;
				}

			}

			ml_draw_hover(ml_get_button_index(LOWORD(lParam), HIWORD(lParam)));
			ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_move);
		}
		break;

	case WM_RBUTTONDOWN:
		ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_down_r);
		break;

	case WM_RBUTTONUP:
		ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_up_r);
		break;


	case WM_LBUTTONDOWN:
		{
			RECT rct;

			GetClientRect(hwnd, &rct);
			

			dx = (int)(short)LOWORD(lParam); dy = (int)(short)HIWORD(lParam);
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
			
			if(ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_down_l))
			{
				if(dx > (rct.right - 20) && dy > (rct.bottom - 20))
				{
					dx -= rct.right;
					dy -= rct.bottom;
					mdown = 2;
				}else{
					if(!incoord((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), ml_pl_x, ml_pl_y, ml_pl_w + ml_pl_scroll_size, ml_pl_h + ml_pl_scroll_size))
						mdown = 1;
				}
			}

			}
		break;

	case WM_LBUTTONDBLCLK:
		ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_dbl_l);
		break;

	case WM_RBUTTONDBLCLK:
		ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_dbl_r);
		break;

	case WM_MOUSEWHEEL:
		ml_pl_mousemsg((int)((short)HIWORD(wParam)), 0, mm_wheel); 
		break;


	case WM_LBUTTONUP:
		mdown = 0;
		ReleaseCapture();
		ml_pl_mousemsg((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), mm_up_l);

		switch(ml_get_button_index((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)))
		{
		case ml_button_playlist:
			ml_cache_uninit();
			mode_ml = 0;
			ml_cache_init();
			ml_pl_startid = 0;
			ml_draw_window(0);
			break;

		case ml_button_library:
			if(mode_ml)
			{
				ml_back();
			}else{
				ml_cache_uninit();
				mode_ml = 1;
				ml_cache_init();
				ml_pl_startid = ml_current_item;
				ml_draw_window(0);
				ml_draw_normal(ml_button_library);
			}
			break;

		case ml_button_in:
			{
				POINT pt;
				HMENU mc = user_create_menu(menu_ml_add, 0);
				
				GetCursorPos(&pt);

				switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0))
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

						bi.hwndOwner      = hwnd;
						bi.lpszTitle      = uni("Add to Media Library");
						bi.pszDisplayName = fpath;
						bi.lpfn           = 0;
						bi.iImage         = 0;
						bi.lParam         = 0;
						bi.pidlRoot       = 0;
						bi.ulFlags        = BIF_RETURNONLYFSDIRS;
			
						lpi = SHBrowseForFolder(&bi);
						SHGetPathFromIDList(lpi, fpath);

						if(str_len(fpath))
						{
							skin.shared->mlib.media_library_add_dir(fpath);
						}

						ml_cache_uninit();
						ml_cache_init();

						ml_draw_window(0);
					}
					break;
				}

				DestroyMenu(mc);
			}
			break;

		case ml_button_out:
			{
				POINT pt;
				HMENU mc = user_create_menu(menu_ml_save, 0);
				
				GetCursorPos(&pt);

				switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0))
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

		case ml_button_remove:
			{
				POINT pt;
				HMENU mc = user_create_menu(menu_ml_remove, 0);
				
				GetCursorPos(&pt);

				switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0))
				{
				case mid_removesel:
					ml_cache_removesel();
					ml_draw_window(0);
					break;

				case mid_removeall:
					skin.shared->audio.output.playlist.clear();
					ml_cache_uninit();
					ml_cache_init();
					ml_draw_window(0);
					break;

				case mid_removeml:
					skin.shared->mlib.media_library_advanced_function(5 /* clear media library */, 0, 0);
					ml_cache_uninit();
					ml_cache_init();
					ml_draw_window(0);
					break;
				}

				DestroyMenu(mc);
			}
			break;

		case ml_button_options:
			{
				int   htv;
				POINT pt;
				HMENU mc = user_create_menu(menu_ml_options, 0);

				GetCursorPos(&pt);

				if(skin.shared->settings.general->player.playlist_repeat_list)
					CheckMenuItem(mc, mid_repeat_list, MF_CHECKED);

				if(skin.shared->settings.general->player.playlist_repeat_single)
					CheckMenuItem(mc, mid_repeat_track, MF_CHECKED);

				if(skin.shared->settings.general->player.playlist_shuffle)
					CheckMenuItem(mc, mid_shuffle, MF_CHECKED);

				if(skin.shared->settings.general->player.auto_switching)
					CheckMenuItem(mc, mid_autoswitching, MF_CHECKED);

				if(lindex_mode == 1)
					CheckMenuItem(mc, mid_switch_sindex, MF_CHECKED);

				if(skin_settings.ml_pl_xoff[header_tag_title]         != -1) CheckMenuItem(mc, midc_title, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_album]         != -1) CheckMenuItem(mc, midc_album, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_artist]        != -1) CheckMenuItem(mc, midc_artist, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_origartist]    != -1) CheckMenuItem(mc, midc_oartist, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_composer]      != -1) CheckMenuItem(mc, midc_composer, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_lyricist]      != -1) CheckMenuItem(mc, midc_lyricist, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_band]          != -1) CheckMenuItem(mc, midc_band, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_copyright]     != -1) CheckMenuItem(mc, midc_copyright, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_publish]       != -1) CheckMenuItem(mc, midc_publisher, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_encodedby]     != -1) CheckMenuItem(mc, midc_encodedby, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_genre]         != -1) CheckMenuItem(mc, midc_genre, MF_CHECKED); 
				if(skin_settings.ml_pl_xoff[header_tag_year]          != -1) CheckMenuItem(mc, midc_year, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_url]           != -1) CheckMenuItem(mc, midc_url, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_offiartisturl] != -1) CheckMenuItem(mc, midc_ourl, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_filepath]      != -1) CheckMenuItem(mc, midc_filepath, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_filename]      != -1) CheckMenuItem(mc, midc_filename, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_bpm]           != -1) CheckMenuItem(mc, midc_bpm, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_tracknum]      != -1) CheckMenuItem(mc, midc_track, MF_CHECKED);
				if(skin_settings.ml_pl_xoff[header_tag_index]         != -1) CheckMenuItem(mc, midc_index, MF_CHECKED);

				switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0))
				{
				case mid_repeat_list:
					skin.shared->settings.general->player.playlist_repeat_list ^= 1;
					break;

				case mid_repeat_track:
					skin.shared->settings.general->player.playlist_repeat_single ^= 1;
					break;

				case mid_shuffle:
					//skin.shared->settings.general->player.playlist_shuffle ^= 1;
					skin.shared->audio.output.playlist.setshuffle(skin.shared->settings.general->player.playlist_shuffle ^ 1, 1);
					ml_draw_window(0);
					break;

				case mid_switch_sindex:
					lindex_mode ^= 1;
					ml_draw_window(0);
					break;

				case mid_autoswitching:
					skin.shared->settings.general->player.auto_switching ^= 1;
					break;

				case mid_settings:
					skin.shared->general.show_settings(0, 0, 0);
					break;

				case midc_font:
					{
						CHOOSEFONT cfd;
						LOGFONT    lf;

						
						memset(&cfd, 0, sizeof(cfd));
						memset(&lf, 0, sizeof(lf));

						lf.lfHeight         = 10;
						lf.lfWidth          = 0;
						lf.lfEscapement     = 0;
						lf.lfOrientation    = 0;
						lf.lfWeight         = FW_BOLD;
						lf.lfItalic         = 0;
						lf.lfUnderline      = 0;
						lf.lfStrikeOut      = 0;
						lf.lfCharSet        = DEFAULT_CHARSET;
						lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
						lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
						lf.lfQuality        = 5;
						lf.lfPitchAndFamily = DEFAULT_PITCH;

						str_cpy(lf.lfFaceName, skin_settings.font_display);

						cfd.lStructSize = sizeof(cfd);
						cfd.hwndOwner   = hwnd;
						cfd.lpLogFont   = &lf;
						cfd.rgbColors   = 0;
						cfd.Flags       = CF_SCREENFONTS;

						ChooseFont(&cfd);

						str_cpy(skin_settings.font_display, lf.lfFaceName);

						DeleteObject(font_ml_pl);

						font_ml_pl = CreateFont(-MulDiv(skin_settings.ml_font_size, GetDeviceCaps(hdc_ml, LOGPIXELSY), 72),
											0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
											OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, /*ANTIALIASED_QUALITY*/5,
											DEFAULT_PITCH, skin_settings.font_display);
						
						
						ml_draw_window(0);
					}
					break;

				case midc_title:      htv = header_tag_title         ; goto point_htags_apply;
				case midc_album:      htv = header_tag_album         ; goto point_htags_apply;
				case midc_artist:     htv = header_tag_artist        ; goto point_htags_apply;
				case midc_oartist:    htv = header_tag_origartist    ; goto point_htags_apply;
				case midc_composer:   htv = header_tag_composer      ; goto point_htags_apply;
				case midc_lyricist:   htv = header_tag_lyricist      ; goto point_htags_apply;
				case midc_band:       htv = header_tag_band          ; goto point_htags_apply;
				case midc_copyright:  htv = header_tag_copyright     ; goto point_htags_apply;
				case midc_publisher:  htv = header_tag_publish       ; goto point_htags_apply;
				case midc_encodedby:  htv = header_tag_encodedby     ; goto point_htags_apply;
				case midc_genre:      htv = header_tag_genre         ; goto point_htags_apply;
				case midc_year:       htv = header_tag_year          ; goto point_htags_apply;
				case midc_url:        htv = header_tag_url           ; goto point_htags_apply;
				case midc_ourl:       htv = header_tag_offiartisturl ; goto point_htags_apply;
				case midc_filepath:   htv = header_tag_filepath      ; goto point_htags_apply;
				case midc_filename:   htv = header_tag_filename      ; goto point_htags_apply;
				case midc_bpm:        htv = header_tag_bpm           ; goto point_htags_apply;
				case midc_track:      htv = header_tag_tracknum      ; goto point_htags_apply;
				case midc_index:      htv = header_tag_index         ; goto point_htags_apply;

point_htags_apply:
					
				if(skin_settings.ml_pl_xoff[htv] == -1)
					skin_settings.ml_pl_xoff[htv] = ml_col_getmax_extent() + 100;
				else
					skin_settings.ml_pl_xoff[htv] = -1;
				ml_draw_window(0);
				break;



				}

				DestroyMenu(mc);
			}
			break;

		case ml_button_sort:
			{
				POINT pt;
				HMENU mc = user_create_menu(menu_ml_sort, 0);
				
				GetCursorPos(&pt);

				switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0))
				{
				case mid_sort_filename:
					skin.shared->simple.list_fastsort();
					ml_draw_window(redraw_playlist);
					break;

				case mid_unsort:
					skin.shared->simple.list_unsort();
					ml_draw_window(redraw_playlist);
					break;
				}

				DestroyMenu(mc);
			}
			break;

		case ml_button_close:
			if(skin_settings.skin_lock)
			{
				SendMessage(skin.wnd, WM_DESTROY, 0, 0);
			}else{
				ml_close();
			}
			break;

		}
		break;

	case WM_TIMER:
		isearch_pos = 0;
		KillTimer(hwnd, timer_id_i_search);
		break;

	case WM_PAINT:
		ml_draw_window(redraw_all);
		break;

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

