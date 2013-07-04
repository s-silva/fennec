#include "skin.h"
#include "media.h"
#include "skin settings.h"

extern HDC   hdc_fview;
extern int   media_init;
extern HDC   fv_sheet;
extern HWND  window_media;

extern int	 scroll_value;
extern int   scroll_max;


extern letter    full_view_text_location[260];

HDC     hdc_backup;
HFONT   dv_font_item = 0, old_font;

HDC     hdc_mediasheet;
int     mediasheet_w, mediasheet_h;

unsigned long     media_lib_count = 0;
static letter     dv_curdir[1024];
int		          dv_curdir_len = 0;
int               dv_textheight = 11;
int               dv_showtags = 1;

unsigned int timer_dv = 0;

int               dv_view_w = 0, dv_view_h = 0;

float             dvzoom = 0.5f;
float             dvzoom_r = 0.5f; /* real value, not aligned */

#define           dz(x) ((int)((float)(x) * dvzoom))

int halt_media_lib = 0;

struct dv_item
{
	int     rindex;
	string  title;
	int     title_len;
	int     mode;
	int     param;
};

void CALLBACK dv_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


enum
{
	imode_folder,
	imode_file,
	imode_album
};


struct dv_item *dvitem_list;
int             dvitem_list_len = 0;
int             dvitem_list_cur;

int             dvitems_start_row = 0;
int             dvitems_items_a_row = 0;
int             dvitems_max_rows = 0;

void media_show(int param);
void media_addtolist(void);

/* item list */

void dvit_init(void)
{
	if(dvitem_list_len) return;

	dvitem_list_len = 64;
	dvitem_list_cur = 0;
	dvitem_list = (struct dv_item *) sys_mem_alloc(dvitem_list_len * sizeof(struct dv_item));
}

void dvit_uninit(void)
{
	if(dvitem_list_len)
	{
		dvitem_list_len = 0;
		dvitem_list_cur = 0;
		sys_mem_free(dvitem_list);
	}
}

void dvit_clear(void)
{
	dvitem_list_cur = 0;
}

void dvit_add(const string title, int titlelen, int mode, int param)
{

	int i;

	if(!dvitem_list) return;

	for(i=0; i<dvitem_list_cur; i++)
	{
		if(dvitem_list[i].mode == mode && dvitem_list[i].title_len == titlelen)
		{
			if(_wcsnicmp(title, dvitem_list[i].title, titlelen) == 0) return;
		}

	}

	dvitem_list[dvitem_list_cur].rindex    = dvitem_list_cur;
	dvitem_list[dvitem_list_cur].title     = title;
	dvitem_list[dvitem_list_cur].title_len = titlelen;
	dvitem_list[dvitem_list_cur].mode      = mode;
	dvitem_list[dvitem_list_cur].param     = param;

	dvitem_list_cur++;

	if(dvitem_list_cur >= dvitem_list_len)
	{
		dvitem_list_len += 100;

		dvitem_list = (struct dv_item *) sys_mem_realloc(dvitem_list, dvitem_list_len * sizeof(struct dv_item));
	}
}



/* real stuff */


void dv_init(HDC dc)
{
	TEXTMETRIC  tm;
	letter      skin_path[512], skin_ap[512];

	hdc_backup = dc;

	dvzoom = dvzoom_r = settings_data.dv.zoomv;

	if(settings_data.playlist.display_mode == playlist_display_small)
	{
		dv_font_item = CreateFont(-11,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}else{

		dv_font_item = CreateFont(-14,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}

	scroll_value = 0;

	old_font = SelectObject(hdc_backup, dv_font_item);

	memset(&tm, 0, sizeof(tm));
	GetTextMetrics(hdc_backup, &tm);

	dv_textheight = tm.tmHeight;

	skin.shared->general.getskinspath(skin_path, sizeof(skin_path));
	str_cat(skin_path, uni("/neo/"));

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("mediasheet.png"));
	hdc_mediasheet = png_get_hdc(skin_ap, 0);
	mediasheet_w = png_w;
	mediasheet_h = png_h;

	SetBkMode(dc, TRANSPARENT);

	dvit_init();

	timer_dv = (unsigned int)SetTimer(0, 0, 35, (TIMERPROC) dv_timer);


	/* media lib */
	skin.shared->mlib.media_library_advanced_function(1, 0, 0); /* safe initialize */
	media_lib_count = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0);

	
	if(!dv_curdir_len)
		memset(dv_curdir, 0, sizeof(dv_curdir));

	media_show(0);
}

void dv_uninit(void)
{
	DeleteDC(hdc_mediasheet);

	KillTimer(0, timer_dv);

	dvit_uninit();

	DeleteObject(dv_font_item);
	SelectObject(hdc_backup, old_font);
}

void dv_set_sizes(void)
{
	TEXTMETRIC  tm;

	SelectObject(hdc_backup, old_font);
	DeleteObject(dv_font_item);

	if(settings_data.playlist.display_mode == playlist_display_small)
	{
		dv_font_item = CreateFont(-11,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}else{

		dv_font_item = CreateFont(-14,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}

	SelectObject(hdc_backup, dv_font_item);

	memset(&tm, 0, sizeof(tm));
	GetTextMetrics(hdc_backup, &tm);

	dv_textheight = tm.tmHeight;
}

void draw_dvitem(HDC dc, int x, int y, int mode, const string ititle, int tlen, int clear, int light)
{
	RECT rct;
	int vx, vy;

	vx = 10 + dz(x * 185);
	vy = 10 + 74 + ((y - dvitems_start_row) * (dz(208 + 10) + 20));

	if(vy < 0) return;
	//if(y - dvitems_start_row >= dvitems_max_rows) return 0;

	if(clear)
		fullview_clear(vx, vy, dz(185), dz(208));

	alpha_blit_resize(dc, hdc_mediasheet, vx, vy, dz(185), dz(208), 0, 0, 185, 208, light ? 60 : 48);

	switch(mode)
	{
	case imode_folder:
		alpha_blit_resize(dc, hdc_mediasheet, vx + dz(21), vy + dz(30), dz(149), dz(115), 208, 127, 149, 115, 100);
		break;

	case imode_file:
		alpha_blit_resize(dc, hdc_mediasheet, vx + dz(18), vy + dz(25), dz(148), dz(123), 207, 3, 148, 123, 100);
		break;

	case imode_album:
		alpha_blit_resize(dc, hdc_mediasheet, vx + dz(19), vy + dz(12), dz(148), dz(148), 206, 245, 148, 148, 100);
		break;
	}

	rct.top = vy + dz(182);
	rct.left = vx + dz(10);
	rct.bottom = rct.top + (36 / dv_textheight) * dv_textheight;
	rct.right = rct.left + dz(166);

	DrawText(dc, ititle, tlen, &rct, DT_CENTER | DT_END_ELLIPSIS | DT_WORDBREAK);

	alpha_blit_resize(dc, hdc_mediasheet, vx, vy, dz(185), dz(87), 0, 220, 185, 87, light ? 80 : 48);

	if(clear)
		fullview_render(vx, vy,  dz(185),  dz(208));

}


void draw_itemfull(HDC dc, int itemid, int xv, int yv, int clear, int light)
{
	if(dv_showtags && dvitem_list[itemid].mode == imode_file)
	{
		struct fennec_audiotag at;
		letter buf[1024];

		skin.shared->mlib.media_library_translate(dvitem_list[itemid].param, &at);
		if(at.tag_artist.tdata && at.tag_artist.tsize)
		{
			str_cpy(buf, at.tag_artist.tdata);
			if(at.tag_title.tdata && at.tag_title.tsize)
			{
				str_cat(buf, uni(" - "));
				str_cat(buf, at.tag_title.tdata);
			}
		
			draw_dvitem(dc, xv, yv, dvitem_list[itemid].mode, buf, str_len(buf), clear, light);

		}else if(at.tag_title.tdata && at.tag_title.tsize){
			
			str_cpy(buf, at.tag_title.tdata);
			draw_dvitem(dc, xv, yv, dvitem_list[itemid].mode, buf, str_len(buf), clear, light);

		}else{
			draw_dvitem(dc, xv, yv, dvitem_list[itemid].mode, dvitem_list[itemid].title, dvitem_list[itemid].title_len, clear, light);
		}
		
	}else{
		draw_dvitem(dc, xv, yv, dvitem_list[itemid].mode, dvitem_list[itemid].title, dvitem_list[itemid].title_len, clear, light);
	}
}

void dv_drawitems(HDC dc)
{
	int   i = 0, ri, xv, yv, ni, si, j = 0, ci;
	HRGN  rgn;

	if(!dvitem_list_cur && !dv_curdir[0]) draw_medialibrary_message(dc);

	rgn = CreateRectRgn(0, 74, dv_view_w, dv_view_h + 74 - 35);
	SelectClipRgn(dc, rgn);

	si = (dvitems_start_row * dvitems_items_a_row);
	ci = (dvitems_items_a_row * dvitems_max_rows);
	ni = si + ci;
	
	//for(i=(dvitems_start_row * dvitems_items_a_row); i<ni; i++)

	for(i=0; i<dvitem_list_cur; i++)
	{
		ri = dvitem_list[i].rindex;

		//if(i >= dvitem_list_cur) break;

		if(ri < si) continue;
		if(ri >= ni) continue;
		if(j > ci) break;

		xv = ri % dvitems_items_a_row;
		yv = ri / dvitems_items_a_row;

		draw_itemfull(dc, i, xv, yv, 0, 0);

		j++;
	}

	SelectClipRgn(dc, 0);
}


void dv_drawoneitem(HDC dc, int itemid, int light)
{
	int xv, yv;
	

	if(!dvitem_list_cur) return;
	if(itemid < 0) return;
	if(itemid >= dvitem_list_cur) return;
	
	xv = dvitem_list[itemid].rindex % dvitems_items_a_row;
	yv = dvitem_list[itemid].rindex / dvitems_items_a_row;

	if(yv < dvitems_start_row) return;
	if(yv >= dvitems_start_row + dvitems_max_rows) return;

	//yv -= dvitems_start_row;//* dvitems_items_a_row);

	draw_itemfull(dc, itemid, xv, yv, 1, light);
}

void dv_draw(HDC dc, int w, int h)
{
	dv_view_w = w;
	dv_view_h = h;

	dvitems_items_a_row = (w - 20) /  dz(185);
	dvitems_max_rows = (h - 10 - 74) /  (dz((208 + 10)) + 20);


	scroll_max = dvitem_list_cur / dvitems_items_a_row;
	dvitems_start_row = scroll_value;

	if(dvitems_start_row > dvitem_list_cur / dvitems_items_a_row) dvitems_start_row =  dvitem_list_cur / dvitems_items_a_row;
	else if(dvitems_start_row < 0) dvitems_start_row = 0;

	dv_drawitems(dc);
}

void dv_halt(void)
{
	dvit_clear();
	halt_media_lib = 1;
}

void dv_halt_resume(void)
{
	halt_media_lib = 0;
	media_show(0);
}



void dv_mousemsg(int x, int y, int action)
{
	static int last_itemid = 0;
	static int ldown = 0;
	int  cx, cy, itemid, i;

	
	if(action == mm_wheel)
	{
		if(x != 0)
		{
			if((GetKeyState(VK_CONTROL) & 0x8000)){

				dvzoom_r += (float)(x / abs(x)) * 0.05f;

				if(dvzoom_r < 0.25f) dvzoom_r = 0.25f;

				dvzoom = dvzoom_r;

				if(dvzoom_r < 1.0f && dvzoom_r > 0.9f) dvzoom = 1.0f;
				if(dvzoom_r < 1.1f && dvzoom_r > 1.0f) dvzoom = 1.0f;

				if(dvzoom_r < 0.7f && dvzoom_r > 0.6f) dvzoom = 0.7f;
				if(dvzoom_r < 0.8f && dvzoom_r > 0.7f) dvzoom = 0.7f;

				if(dvzoom_r < 0.5f && dvzoom_r > 0.4f) dvzoom = 0.5f;
				if(dvzoom_r < 0.6f && dvzoom_r > 0.5f) dvzoom = 0.5f;

				if(dvzoom_r < 2.0f && dvzoom_r > 1.8f) dvzoom = 2.0f;
				if(dvzoom_r < 2.2f && dvzoom_r > 2.0f) dvzoom = 2.0f;

				settings_data.dv.zoomv = dvzoom;

				
				fullview_drawgeneral();

			}else{
				dvitems_start_row -= (x / abs(x));

				if(dvitems_start_row > dvitem_list_cur / dvitems_items_a_row) dvitems_start_row =  dvitem_list_cur / dvitems_items_a_row;
				else if(dvitems_start_row < 0) dvitems_start_row = 0;

				fullview_drawgeneral();
			}
		}
	}

	if(x < 20) return;
	if(y < 74) return;
	if(y > dv_view_h - 35) return;

	/* ----------------------------------- */

	/* calculate item id */

	cx = (x - 10) / dz(185);
	cy = (y - 10 - 74) / (dz(208 + 10) + 20);

	if((y - 10 - 74) % (dz(208 + 10) + 20) > dz(185)) return;

	if(cx < 0) return;
	if(cy < 0) return;
	if(cx >= dvitems_items_a_row) return;

	cy += dvitems_start_row;

	itemid = cx + (cy * dvitems_items_a_row);

	if(itemid >= dvitem_list_cur)return;// itemid = dvitem_list_cur - 1;

	
	for(i=0; i<dvitem_list_cur; i++)
	{
		if(itemid == dvitem_list[i].rindex)
		{
			itemid = i;
			break;
		}
	}

	if(action == mm_down_l) ldown = 1;
	if(action == mm_up_l) ldown = 0;


	

	if(action == mm_down_r)
	{
		POINT pt;
		int   rv;
		HMENU mc = user_create_menu(menu_ml_dv_popup, mode_ml);
		
		GetCursorPos(&pt);

		rv = (int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0);
		
		if(rv == mid_dv_add || rv == mid_dv_play)
		{
			if(rv == mid_dv_play)
				skin.shared->audio.output.playlist.clear();

			if(dvitem_list[itemid].mode == imode_folder)
			{
				letter dv_cur_bkp[1024];
				int    dv_curdir_len_bkp = dv_curdir_len;

				str_cpy(dv_cur_bkp, dv_curdir);

				str_ncat(dv_curdir, dvitem_list[itemid].title, dvitem_list[itemid].title_len);
				dv_curdir[dv_curdir_len + dvitem_list[itemid].title_len] = uni('\\');
				dv_curdir[dv_curdir_len + dvitem_list[itemid].title_len + 1] = uni('\0');

				dv_curdir_len = str_len(dv_curdir);

				media_addtolist();
				
				dv_curdir_len = dv_curdir_len_bkp;
				str_cpy(dv_curdir, dv_cur_bkp);


			}else{
				struct fennec_audiotag at;

				skin.shared->mlib.media_library_translate(dvitem_list[itemid].param, &at);
				skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
			}

			if(rv == mid_dv_play)
			{
				skin.shared->audio.output.playlist.switch_list(0);
				skin.shared->audio.output.play();
				SetFocus(window_media);
			}else{
				
				fullview_refresh(0);
			}
		}

		DestroyMenu(mc);
		return;
	}


	if(action == mm_down_l)
	{
		if(dvitem_list[itemid].mode == imode_folder)
		{
			str_ncat(dv_curdir, dvitem_list[itemid].title, dvitem_list[itemid].title_len);
			dv_curdir[dv_curdir_len + dvitem_list[itemid].title_len] = uni('\\');
			dv_curdir[dv_curdir_len + dvitem_list[itemid].title_len + 1] = uni('\0');

			dv_curdir_len = str_len(dv_curdir);

			dvit_clear();
			media_show(0);

		}else{
			struct fennec_audiotag at;

			skin.shared->audio.output.playlist.clear();

			skin.shared->mlib.media_library_translate(dvitem_list[itemid].param, &at);

			skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
			skin.shared->audio.output.playlist.switch_list(0);
			skin.shared->audio.output.play();

			SetFocus(window_media);
		}
	}
	if(action == mm_dbl_l)
	{
		if(dvitem_list[itemid].mode == imode_file)
		{
			int i, k = 0;
			struct fennec_audiotag at;

			skin.shared->audio.output.playlist.clear();

			
			for(i=0; i<dvitem_list_cur; i++)
			{
				if(dvitem_list[i].mode == imode_file)
				{
					if(itemid == i) k = i;
					skin.shared->mlib.media_library_translate(dvitem_list[i].param, &at);
					skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
				}
			}

			skin.shared->audio.output.playlist.switch_list(k);
			skin.shared->audio.output.play();

			SetFocus(window_media);
		}
	}


	if(action == mm_move)
	{
		if(last_itemid != itemid)
		{
			dv_drawoneitem(hdc_backup, last_itemid, 0);
			dv_drawoneitem(hdc_backup, itemid, 1);

			last_itemid = itemid;
		}

	}

	

}

void CALLBACK dv_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	
}



int dv_keymsg(int key)
{
	switch(key)
	{
	case VK_BACK:
		{
			int i, nf = 1;

			for(i=dv_curdir_len-2; i>=0; i--)
			{
				if(dv_curdir[i] == uni('\\'))
				{
					nf = 0;
					dv_curdir[i+1] = 0;
					break;
				}
			}

			if(nf)
			{
				dv_curdir[0] = 0;
				dv_curdir_len = 0;
			}else{
		
				dv_curdir_len = str_len(dv_curdir);
			}

			dvit_clear();
			media_show(0);
		}
		break;
	}

	return 1;
}



void dv_addtopl(const string fname, int fnlen)
{
	letter buf[1024];

	if(fnlen <= 0)return;

	str_ncpy(buf, fname, fnlen);
	buf[fnlen] = 0;

	skin.shared->audio.output.playlist.add(buf, 0, 0);
}





void media_show(int param)
{
	struct fennec_audiotag at;
	static letter lastpath[1024];
	unsigned long i, j;
	int     slashi = 0;
	string   str;
	int     lastpath_len = 0;
	string  fpath;

	if(halt_media_lib) return;

	media_lib_count = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0);


	str_cpy(lastpath, uni(""));

	for(i=0; i<media_lib_count; i++)
	{
		skin.shared->mlib.media_library_translate(i, &at);
		fpath = at.tag_filepath.tdata + 3;

		if(str_incmp(fpath, dv_curdir, dv_curdir_len) == 0)
		{
			str = fpath + dv_curdir_len;
			slashi = -1;
			j = 0;

			while(*str)
			{
				if((*str) == uni('\\'))
				{
					slashi = j;
					break;
				}
				str++;
				j++;
			}
	
			
			if(slashi < 0)
			{
				dvit_add(fpath + dv_curdir_len, str_len(fpath + dv_curdir_len), imode_file, i);
			}else{

				if((lastpath_len != dv_curdir_len + slashi) || _wcsnicmp(lastpath, fpath, dv_curdir_len + slashi) != 0)
				{
					dvit_add(fpath + dv_curdir_len, slashi, imode_folder, i);

					str_ncpy(lastpath, fpath, dv_curdir_len + slashi);
					lastpath[dv_curdir_len + slashi] = 0;
					lastpath_len = dv_curdir_len + slashi;
				}
			}

		}
	}

	/* arrange it */

	j = 0;

	for(i=0; i<(unsigned long)dvitem_list_cur; i++)
	{
		if(dvitem_list[i].mode == imode_folder)
		{
			dvitem_list[i].rindex = j;
			j++;
		}
	}

	for(i=0; i<(unsigned long)dvitem_list_cur; i++)
	{
		if(dvitem_list[i].mode != imode_folder)
		{
			dvitem_list[i].rindex = j;
			j++;
		}
	}


	memset(full_view_text_location, 0, sizeof(full_view_text_location));
	str_ncpy(full_view_text_location, dv_curdir, dv_curdir_len);
	fullview_drawgeneral();
}


void media_addtolist(void)
{
	struct fennec_audiotag at;
	unsigned long i;

	if(halt_media_lib) return;

	media_lib_count = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0);

	for(i=0; i<media_lib_count; i++)
	{
		skin.shared->mlib.media_library_translate(i, &at);
		
		if(str_incmp(at.tag_filepath.tdata + 3, dv_curdir, dv_curdir_len) == 0)
		{
			dv_addtopl(at.tag_filepath.tdata, at.tag_filepath.tsize);
		}
	}

}