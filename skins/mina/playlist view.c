#include "skin.h"
#include "media.h"
#include "skin_settings.h"

extern HDC   hdc_fview;
extern int   media_init;
extern HDC   fv_sheet;
extern HWND  window_media;

extern int	 scroll_value;
extern int   scroll_max;

#define  sel_toggle 4

int     playlist_scroll_x = 0;
int     sel_column        = 0;
int     next_item_id = 0;
int     current_item_id = 0;

int     special_item = -1;
int		preview_item_index = -1;


/* display sizes */

int     view_bar_h = 39;
int     item_bar_h = 37;
int     playlist_font_size = 14;
int     text_voffset = 12;
int     playlist_font_weight = FW_BOLD;
int     marker_size = 26;
int     marker_spacing = 5;


#define text_offset 35

HFONT   item_font = 0, old_font;
HDC     hdc_backup;

int     playlist_w, playlist_h, playlist_x, playlist_y;
int     playlist_scroll_row = 0;
int     citem = 0;

unsigned long curitem = 0;


int     xoff_widths[32];
int     xoff_visible[32];
HRGN    xoff_rgns[32];
int     rgns_init = 0;

int     playlist_refresh_request = 0;


void playlist_set_sizes(void)
{

	if(settings_data.playlist.display_mode == playlist_display_small)
	{
		view_bar_h           = 22;
		item_bar_h           = 20;
		playlist_font_size   = 10;
		text_voffset         = 3;
		playlist_font_weight = FW_NORMAL;
		marker_size          = 15;
		marker_spacing       = 2;
	}else{

		view_bar_h = 39;
		item_bar_h = 37;
		playlist_font_size = 14;
		text_voffset = 12;
		playlist_font_weight = FW_BOLD;
		marker_size = 26;
		marker_spacing = 5;
	}

	
	if(item_font)
	{
		SelectObject(hdc_backup, old_font);
		DeleteObject(item_font);
	}

	item_font = CreateFont(-playlist_font_size,
                                0, 0, 0, playlist_font_weight, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, uni("Tahoma"));

	old_font = SelectObject(hdc_backup, item_font);
}



void playlist_init(HDC dc)
{
	hdc_backup = dc;

	playlist_set_sizes();

	
	SetBkMode(dc, TRANSPARENT);

	if(playlist_refresh_request)
	{
		ml_cache_uninit();
		ml_cache_init();
		playlist_refresh_request = 0;
	}
}

void playlist_uninit(void)
{
	DeleteObject(item_font);
	SelectObject(hdc_backup, old_font);
}

void playlist_refresh(int lv)
{
	if(lv > v_fennec_refresh_force_less)
	{
		ml_cache_uninit();
		ml_cache_init();
		playlist_redraw(-1);
	}else{
		playlist_redraw(-2);
	}
	
}



void generate_xoff_data(void)
{
	int i, j, closest_x, cx, colx, colw;


	/* delete any rgns */

	if(rgns_init)
	{
		for(i=0; i<32; i++)
		{
			if(xoff_rgns[i])
			{
				DeleteObject(xoff_rgns[i]);
				xoff_rgns[i] = 0;
			}
		}

		rgns_init = 0;
	}

	/* prepare widths */

	for(i=0; i<32; i++)
	{
		colx      = settings_data.playlist.xoff[i];
		closest_x = -1;

		if(colx < 0) continue;

		for(j=0; j<32; j++)
		{
			cx = settings_data.playlist.xoff[j];

			if(cx > colx && closest_x == -1)    
				closest_x = cx;
			else if(cx > colx && cx < closest_x)
				closest_x = cx;
		}

		if(closest_x == -1) /* not found */
			closest_x = colx + 400;

		xoff_widths[i] = closest_x - colx - 5;
		if(xoff_widths[i] < 0) xoff_widths[i] = 0;
	}

	/* check visibility */

	for(i=0; i<32; i++)
	{
		colx      = settings_data.playlist.xoff[i];
		colw      = xoff_widths[i];

		if(colx < 0) continue;

		xoff_visible[i] = 1;

		if(colx + colw < playlist_scroll_x) xoff_visible[i] = 0;
		if(colx - playlist_scroll_x > playlist_w) xoff_visible[i] = 0;
	}

	/* make regions */

	for(i=0; i<32; i++)
	{
		int rx, rx2;

		colx      = settings_data.playlist.xoff[i];
		colw      = xoff_widths[i];

		if(!xoff_visible[i] || colx < 0)
		{
			xoff_rgns[i] = 0;
			continue;
		}

		rx = colx - playlist_scroll_x;
		rx2 = playlist_x + text_offset + colx + colw - playlist_scroll_x;

		if(rx < 0) rx = 0;
		if(rx2 > playlist_x + playlist_w) rx2 = playlist_x + playlist_w;

		xoff_rgns[i] = CreateRectRgn(playlist_x + text_offset + rx, playlist_y, rx2, playlist_y + playlist_h);
	}

	rgns_init = 1;
}

int get_xoff_max_extent(void)
{
	int i, c = 0;

	for(i=0; i<32; i++)
	{
		if(settings_data.playlist.xoff[i] > c)
			c = settings_data.playlist.xoff[i];
	}
	return c;
}


void playlist_textout(HDC dc, const string text, int x, int y, int column)
{
	if(settings_data.playlist.xoff[column] < 0) return;
	if(xoff_visible[column] == 0) return;
	
	SelectClipRgn(dc, xoff_rgns[column]);

	
	TextOut(dc, x - playlist_scroll_x, y, text, str_len(text));

	SelectClipRgn(dc, 0);
}

void draw_header(HDC dc, int x, int y)
{
	int    text_x = x + text_offset;
	int    text_y = y + text_voffset;
	int    i, tid = -1;

	drawbar(hdc_fview, fv_sheet, x, y, playlist_w, item_bar_h, 140, 179, 3, 143, 179, 195, 338, 179, 3, 50);

	

	for(i=0; i<32; i++)
	{
		if(settings_data.playlist.xoff[i] == -1) continue;

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

		if(tid != -1)
		{
			playlist_textout(dc, skin.shared->language_text[tid], text_x + settings_data.playlist.xoff[i], text_y, i);
		}
	}
}

void tags_out(HDC dc, struct fennec_audiotag *ctag, int x, int y)
{
	letter                        cbuffer[512];
	string                        cstr;
	int                           i, cbuf;
	struct fennec_audiotag_item  *at;
	
	for(i=0; i<32; i++)
	{
		if(settings_data.playlist.xoff[i] >= 0)
		{
			cbuf = 0;
			cstr = 0;
			at   = 0;

			switch(i)
			{
			case header_tag_title: 
				if(ctag->tag_title.tdata && ctag->tag_title.tsize)
				{
					at = &ctag->tag_title;
				}else{
					const string fpath = ml_cache_getpath(citem);
					if(fpath){ _wsplitpath(fpath, 0, 0, cbuffer, 0); cstr = cbuffer; }
				}
				break;


			case header_tag_album:          at = &ctag->tag_album; break;
			case header_tag_artist:         at = &ctag->tag_artist; break;
			case header_tag_origartist:     at = &ctag->tag_origartist; break;
			case header_tag_composer:       at = &ctag->tag_composer; break;
			case header_tag_lyricist:       at = &ctag->tag_lyricist; break;
			case header_tag_band:			at = &ctag->tag_band; break;
			case header_tag_copyright:		at = &ctag->tag_copyright; break;
			case header_tag_publish:		at = &ctag->tag_publish; break;
			case header_tag_encodedby:		at = &ctag->tag_encodedby; break;
			case header_tag_genre:          at = &ctag->tag_genre; break;
			case header_tag_year:			at = &ctag->tag_year; break;
			case header_tag_url:			at = &ctag->tag_url; break;
			case header_tag_offiartisturl:	at = &ctag->tag_offiartisturl; break;
			case header_tag_filepath:
				cstr = ml_cache_getpath(citem);
				break;

			case header_tag_filename:
				{
					int          ctag_pt;

					cstr = ml_cache_getpath(citem);

					if(cstr)
					{
						ctag_pt = (int)str_len(cstr);

						for(; ctag_pt>=0; ctag_pt--)
							if(cstr[ctag_pt] == '/' || cstr[ctag_pt] == '\\')break;

						ctag_pt++;

						cstr += ctag_pt;
					}
				}
				break;

			case header_tag_comments:		at = &ctag->tag_comments; break;
			case header_tag_lyric:			at = &ctag->tag_lyric; break;
			case header_tag_bpm:			at = &ctag->tag_bpm; break;
			case header_tag_tracknum:		at = &ctag->tag_tracknum; break;
			case header_tag_index:
				at = 0;
				cbuf = 1;
				str_itos(citem + 1, cbuffer, 10);
				break;
			}


			if(at) cstr = at->tdata;
			else if(cbuf) cstr = cbuffer;
			
			if(at && at->tsize <= 0) continue;


			if(cstr)
				playlist_textout(dc, cstr, x + settings_data.playlist.xoff[i], y, i);


		} /* end printing one detail */
	}

}



void draw_item(HDC dc, struct fennec_audiotag *ctag, int x, int y, int instance)
{
	int    text_x = x + text_offset;
	int    text_y = y + text_voffset;
	
	if(special_item == -2) /* only update items with markers */
	{
		if(citem != next_item_id && citem != current_item_id) return;
	}



	/* draw bar */

	switch(instance)
	{
	case 1: drawbar(hdc_fview, fv_sheet, x, y, playlist_w, item_bar_h, 140, 217, 3, 143, 217, 195, 338, 217, 3, 50); break;
	case 2: drawbar(hdc_fview, fv_sheet, x, y, playlist_w, item_bar_h, 140, 217, 3, 143, 217, 195, 338, 217, 3, 80); break;
	case 3: drawbar(hdc_fview, fv_sheet, x, y, playlist_w, item_bar_h, 140, 255, 3, 143, 255, 195, 338, 255, 3, 30); break;
	}

	/* markers */

	if(citem == next_item_id)
		alpha_blit_resize(hdc_fview, fv_sheet, x + marker_spacing, y + marker_spacing, marker_size, marker_size, 264, 0, 26, 26, 80);

	if(citem == current_item_id)
		alpha_blit_resize(hdc_fview, fv_sheet, x + marker_spacing, y + marker_spacing, marker_size, marker_size, 237, 0, 26, 26, 80);




	if(special_item == -2) return; /* done, leave */

	/* output text */

	tags_out(dc, ctag, text_x, text_y);
}


void playlist_view(HDC dc, int x, int y, int w, int h, int item)
{
	int  i, instance;
	int  item_y = y + view_bar_h; /* for header */

	struct fennec_audiotag *ctag = 0;

	int  start = ml_pl_startid;
	int  count = ml_cache_getcount() - start;
	int  display_count = (h - view_bar_h) / view_bar_h;
	int  min_count = min(count, display_count);

	playlist_w = w;
	playlist_h = h;
	playlist_x = x;
	playlist_y = y;

	scroll_max   = ml_cache_getcount() - display_count;

	if(scroll_max < 0)scroll_max = 0;

	next_item_id = ml_cache_getnext_item();
	current_item_id = ml_cache_getcurrent_item();

	generate_xoff_data();

	/* header */

	SetTextColor(dc, 0xffffff); /* light font */
	draw_header(dc, x, y);
	SetTextColor(dc, 0); /* dark font */


	/* display */
	
	for(i=0; i<min_count; i++)
	{
		citem = start + i;

		ctag = ml_cache_get(citem);
		if(ctag == (void*) -1)continue;

		instance = i % 2 ? 1 : 2;

		if(ml_cache_issel(citem)) instance = 3; /* selected */
	
		draw_item(dc, ctag, x, item_y, instance);


		item_y += view_bar_h;
	}


}






void popup_columns_menu(void)
{
	int   htv;
	POINT pt;
	HMENU mc = user_create_menu(menu_ml_columns, 0);
	
	GetCursorPos(&pt);

	if(settings_data.playlist.xoff[header_tag_title]         != -1) CheckMenuItem(mc, midc_title, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_album]         != -1) CheckMenuItem(mc, midc_album, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_artist]        != -1) CheckMenuItem(mc, midc_artist, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_origartist]    != -1) CheckMenuItem(mc, midc_oartist, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_composer]      != -1) CheckMenuItem(mc, midc_composer, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_lyricist]      != -1) CheckMenuItem(mc, midc_lyricist, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_band]          != -1) CheckMenuItem(mc, midc_band, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_copyright]     != -1) CheckMenuItem(mc, midc_copyright, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_publish]       != -1) CheckMenuItem(mc, midc_publisher, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_encodedby]     != -1) CheckMenuItem(mc, midc_encodedby, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_genre]         != -1) CheckMenuItem(mc, midc_genre, MF_CHECKED); 
	if(settings_data.playlist.xoff[header_tag_year]          != -1) CheckMenuItem(mc, midc_year, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_url]           != -1) CheckMenuItem(mc, midc_url, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_offiartisturl] != -1) CheckMenuItem(mc, midc_ourl, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_filepath]      != -1) CheckMenuItem(mc, midc_filepath, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_filename]      != -1) CheckMenuItem(mc, midc_filename, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_bpm]           != -1) CheckMenuItem(mc, midc_bpm, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_tracknum]      != -1) CheckMenuItem(mc, midc_track, MF_CHECKED);
	if(settings_data.playlist.xoff[header_tag_index]         != -1) CheckMenuItem(mc, midc_index, MF_CHECKED);

	switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, GetFocus(), 0))
	{
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
		
		if(settings_data.playlist.xoff[htv] == -1)
			settings_data.playlist.xoff[htv] = get_xoff_max_extent() + 100;
		else
			settings_data.playlist.xoff[htv] = -1;

		playlist_redraw(-1);
		break;
	}
}


/*
 * mouse messages
 */

int playlist_keymsg(int key)
{
	switch(key)
	{
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
				curitem = 0;
			}

			if(!ml_cache_shiftsel(1))
			{
				int plmc = playlist_h / view_bar_h;
				curitem++;

				if(curitem >= (unsigned long)(ml_pl_startid + plmc))
				{
					ml_pl_startid = curitem - plmc + 1;
				}
				
				playlist_redraw(-1);
			}
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
				curitem = 0;
			}

			if(!ml_cache_shiftsel(-1))
			{
				if(curitem > 0)curitem--;
				if(curitem < (unsigned long)(ml_pl_startid))
					ml_pl_startid = curitem;

				playlist_redraw(-1);
			}
		}
		break;

	case VK_DELETE:
		ml_cache_removesel();
		if(cached_tags_count)
			ml_cache_setsel(0, 0);
		if(curitem < cached_tags_count)
		{
			ml_cache_setsel(curitem, 1);
		}
		playlist_redraw(-1);
		break;

	case VK_RETURN:
	case VK_SPACE:
		if(curitem < cached_tags_count)
		{
			ml_cache_switchlist(curitem, 1);
			SetFocus(window_media);
		}
		break;
	}

	return 1;
}

int playlist_mousemsg(int x, int y, int m)
{
	static int rdown = 0, ldown = 0, mdown = 0;
	static int sel_column_dx = 0;
	static int down_s_v = 0, down_s_h = 0, fdown = 1;
	static int dx, dy, dys, dxs;
	static int col_moved;
	static int hdown = 0;
	
	
	static unsigned long curitemd = 0;
	static unsigned long last_curitem = 0;
	static unsigned long citem = 0;

	if (m == mm_down_r){rdown = 1; dx = x; dy = y;}
	else if(m == mm_up_r)rdown = 0;

	if (m == mm_down_l){ldown = 1; dx = x; dy = y;}
	else if(m == mm_up_l)ldown = 0;

#ifdef laterlater
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
#endif

	/* </preview> */
	/* <header> */

	if(m == mm_up_r && incoord(x, y, 0, 0, playlist_w, view_bar_h))
	{
		popup_columns_menu();

	}


	if(hdown || incoord(x, y, 0, 0, playlist_w, view_bar_h))
	{
		int mx, my;

		mx = x;
		my = y;

		if(m == mm_up_l)
		{
			hdown = 0;
		}

		if(m == mm_down_l || m == mm_dbl_l)
		{
			/* find column id */

			int ux = mx + playlist_scroll_x;

			int i, sel = -1;

			col_moved = 0;

			for(i=0; i<sizeof(settings_data.playlist.xoff)/sizeof(int); i++)
			{
				if(settings_data.playlist.xoff[i] == -1)continue;

				if(settings_data.playlist.xoff[i] < ux - text_offset)
				{
					if(sel != -1)
					{
						if(settings_data.playlist.xoff[i] > settings_data.playlist.xoff[sel])
							sel = i;
					}else{
						sel = i;
					}
				}
			}

			sel_column_dx = ux - settings_data.playlist.xoff[sel];
			sel_column = sel;

			hdown = 1;

			if(m == mm_dbl_l)
			{
			
				if(settings_data.playlist.sorted_column == sel_column)
					list_sort_column(sel_column, !settings_data.playlist.sort_mode);
				else
					list_sort_column(sel_column, 0 /* ascending */);
			
				/* ml_draw_window(redraw_playlist), if single click shouldn't do the work */
			}
			
			playlist_redraw(-1);
			return 1;
		}



		if((m == mm_move) && ldown && hdown)
		{
			if(sel_column != -1)
			{
				int lastxo = settings_data.playlist.xoff[sel_column];

				if(mx >= sel_column_dx)
					settings_data.playlist.xoff[sel_column] = mx + playlist_scroll_x - sel_column_dx;
				else
					settings_data.playlist.xoff[sel_column] = 0;
				
				playlist_redraw(-1);

				if(lastxo != settings_data.playlist.xoff[sel_column])
					col_moved = 1;
			}

			return 1;
		}
	}

	/* </header> */


	if(m == mm_wheel)
	{
		ml_pl_startid_last = ml_pl_startid;

		if(x > 0)
			ml_pl_startid -= 3;
		else
			ml_pl_startid += 3;
		
		if(ml_pl_startid < 0)ml_pl_startid = 0;
		
		if((unsigned long)ml_pl_startid > 2+ /* <- just to keep free space */ cached_tags_count - (playlist_h / view_bar_h) )
			ml_pl_startid = 2+ /* <- just to keep free space */ cached_tags_count - (playlist_h / view_bar_h);

		playlist_redraw(-1);

		return 1;
	}

	if(m == mm_dbl_l)
	{
		unsigned long i = ((y - view_bar_h)  / view_bar_h) + ml_pl_startid;

		if(mode_ml && (GetKeyState(VK_CONTROL) & 0x8000))
		{
			ml_cache_add_to_playlist(i);

		}else{

			if(i < cached_tags_count)
			{
				ml_cache_switchlist(i, 1);
			}
		}

		SetFocus(window_media);
		playlist_redraw(-2); /* only markers */
		return 1;
	}

	if(m == mm_up_l || m == mm_up_r)
	{
		last_curitem = curitem;
		curitem = ml_pl_startid + ((y - view_bar_h) / view_bar_h);
		if((int)curitem < 0) curitem = 0;
	}

	if(m == mm_down_l)
	{
		curitemd = ml_pl_startid + ((y - view_bar_h) / view_bar_h);
		if((int)curitemd < 0) curitemd = 0;
	}

	if(m == mm_up_r && !incoord(x, y, 0, 0, playlist_w, view_bar_h))
	{
		POINT pt;
		HMENU mc = user_create_menu(menu_ml_popup, mode_ml);
		
		GetCursorPos(&pt);

		

		switch((int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0))
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
				playlist_redraw(-1);
			}
			break;

		case mid_preview:
			if(curitem < cached_tags_count)
			{
				skin.shared->call_function(call_audio_preview_action, audio_preview_load, ml_cache_getpath(curitem), 0);
				skin.shared->call_function(call_audio_preview_action, audio_preview_play, 0, 0);
				preview_item_index = curitem;
				playlist_redraw(-1);
			}
			break;

		case mid_preview_stop:
			if(curitem < cached_tags_count)
			{
				skin.shared->call_function(call_audio_preview_action, audio_preview_stop, 0, 0);
				preview_item_index = -1;
				playlist_redraw(-1);
			}
			break;


		case mid_removeall:
			skin.shared->audio.output.playlist.clear();
			ml_cache_uninit();
			ml_cache_init();
			playlist_redraw(-1);
			break;

		case mid_addtoplaylist:
			/*if(curitem < ml_cache_getcount() && ml_current_dir != media_library_dir_root && mode_ml && ml_in_dir)
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
			}*/

			break;
		}

		DestroyMenu(mc);
	}


	if(m == mm_up_l)
	{
		if(curitem < ml_cache_getcount())
		{
			if((GetKeyState(VK_SHIFT) & 0x8000))
			{
				if(last_curitem < ml_cache_getcount())
				{
					unsigned long i;
					char          v = 1; 

					if((GetKeyState(VK_CONTROL) & 0x8000)) v = 0; /* deselect mode */

					if(last_curitem < curitem)
					{
						for(i=last_curitem; i<=curitem; i++)
							ml_cache_setsel(i, v);

					}else if(last_curitem > curitem){
						
						for(i=curitem; i<=last_curitem; i++)
							ml_cache_setsel(i, v);

					}else if(last_curitem == curitem){
						ml_cache_setsel(curitem, v);
					}
				}

			}else if((GetKeyState(VK_CONTROL) & 0x8000)){

				ml_cache_setsel(curitem, sel_toggle);

			}else{
				ml_cache_clearsel();
				ml_cache_setsel(curitem, sel_toggle);
			}

			playlist_redraw(-1);

			/* playlist_redraw(curitem); should implement this to save cpu by not redrawing it all but the current item(s) */

		}
	}



	if((m == mm_move) && ldown)
	{
		citem = ml_pl_startid + ((y - view_bar_h) / view_bar_h);
		if((int)citem < 0) curitemd = 0;
		if(citem >= cached_tags_count) curitemd = cached_tags_count - 1;

		if((curitemd != citem))
		{
			unsigned long i, ok = 1;

			/* get lowest */

			if(citem < curitemd)
			{
				for(i=0; i<cached_tags_count; i++)
				{
					if(cached_tags[i].sel)
					{
						if(i < (curitemd - citem))ok = 0;
						break;
					}
				}
			}else if(curitemd < citem){

				for(i=(cached_tags_count-1); i>0; i--)
				{
					if(cached_tags[i].sel)
					{
						if(i + (citem - curitemd) >= cached_tags_count)ok = 0;
						break;
					}
				}
			}



			if(ok)
			{
				if(citem < curitemd)
				{
					for(i=0; i<cached_tags_count; i++)
					{
						if(cached_tags[i].sel)
							skin.shared->audio.output.playlist.move(i + ((long)citem - (long)curitemd), i, 0);
					}
				}else if(citem > curitemd){

					for(i=(cached_tags_count-1); i>0; i--)
					{
						if(cached_tags[i].sel)
							skin.shared->audio.output.playlist.move(i + ((long)citem - (long)curitemd), i, 0);
					}
				}

				ml_cache_shiftsel((long)citem - (long)curitemd);
				curitemd = citem;
				ml_cache_freetags();
				playlist_redraw(-1);
			}
		}
		return 1;
	}

#ifdef laterlater

	


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


#endif

	if(m == mm_up_l)
	{
		down_s_v = 0;
		down_s_h = 0;
	}

	if(m == mm_up_l)
		down_s_v = 0;


	return 0;
}


void playlist_scroll(int sv)
{
	ml_pl_startid = sv;
}