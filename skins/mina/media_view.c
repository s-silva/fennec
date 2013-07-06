#include "skin.h"
#include "media.h"
#include "skin_settings.h"
#include <shlobj.h>
#include <userenv.h>

extern HDC   hdc_fview;
extern int   media_init;
extern HDC   fv_sheet;
extern HWND  window_media;

extern int	 scroll_value;
extern int   scroll_max;


extern letter    full_view_text_location[260];
extern int halt_media_lib;

extern int   fullview_return_to_library;

HDC     hdc_backup;
HFONT   lib_font_item = 0, old_font;

HDC     hdc_mediasheet;
int     mediasheet_w, mediasheet_h;

int               media_lib_empty = 0;

unsigned long     lib_media_lib_count = 0;
static letter     lib_curdir[1024];
int		          lib_curdir_len = 0;
int               lib_textheight = 11;
int               lib_showtags = 1;

int               lib_show_album = 0;

letter            current_artist[1024];
letter            current_album[1024];

unsigned int timer_lib = 0;

int               lib_view_w = 0, lib_view_h = 0;

float             libzoom = 0.5f;
float             libzoom_r = 0.5f; /* real value, not aligned */

#define           dz(x) ((int)((float)(x) * libzoom))


static letter     thumbs_albums_dir[1024];
static letter     thumbs_artists_dir[1024];
static int        thumbs_albums_dir_slen = 0;
static int        thumbs_artists_dir_slen = 0;

int               error_in_last_image = 0;

struct lib_item
{
	int     rindex;
	string  title;
	int     title_len;
	int     mode;
	int     param;
};

void CALLBACK lib_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


enum
{
	imode_folder,
	imode_file,
	imode_album
};


struct lib_item *libitem_list;
int             libitem_list_len = 0;
int             libitem_list_cur;

int             libitems_start_row = 0;
int             libitems_items_a_row = 0;
int             libitems_max_rows = 0;

void lib_media_show(int param);
void lib_media_addtolist(void);

/* item list */

void libit_init(void)
{
	if(libitem_list_len) return;

	libitem_list_len = 64;
	libitem_list_cur = 0;
	libitem_list = (struct lib_item *) sys_mem_alloc(libitem_list_len * sizeof(struct lib_item));
}

void libit_uninit(void)
{
	if(fullview_switching_manually)
	{
		str_cpy(settings_data.library.current_album, current_album);
		str_cpy(settings_data.library.current_artist, current_artist);
		settings_data.library.current_dir = ml_current_dir;
		settings_data.library.show_album  = lib_show_album;
	}



	if(libitem_list_len)
	{
		libitem_list_len = 0;
		libitem_list_cur = 0;
		sys_mem_free(libitem_list);
	}
}

void libit_clear(void)
{
	libitem_list_cur = 0;
}

void libit_add(const string title, int titlelen, int mode, int param)
{

	int i;

	for(i=0; i<libitem_list_cur; i++)
	{
		if(libitem_list[i].mode == mode && libitem_list[i].title_len == titlelen)
		{
			if(_wcsnicmp(title, libitem_list[i].title, titlelen) == 0) return;
		}

	}

	libitem_list[libitem_list_cur].rindex    = libitem_list_cur;
	libitem_list[libitem_list_cur].title     = title;
	libitem_list[libitem_list_cur].title_len = titlelen;
	libitem_list[libitem_list_cur].mode      = mode;
	libitem_list[libitem_list_cur].param     = param;

	libitem_list_cur++;

	if(libitem_list_cur >= libitem_list_len)
	{
		libitem_list_len += 100;

		libitem_list = (struct lib_item *) sys_mem_realloc(libitem_list, libitem_list_len * sizeof(struct lib_item));
	}
}



/* real stuff */


void lib_init(HDC dc)
{
	TEXTMETRIC  tm;
	letter      skin_path[512], skin_ap[512];

	hdc_backup = dc;

	libzoom = libzoom_r = settings_data.dv.zoomv;

	if(settings_data.playlist.display_mode == playlist_display_small)
	{
		lib_font_item = CreateFont(-11,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}else{

		lib_font_item = CreateFont(-14,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}


	scroll_value = 0;

	old_font = SelectObject(hdc_backup, lib_font_item);

	memset(&tm, 0, sizeof(tm));
	GetTextMetrics(hdc_backup, &tm);

	lib_textheight = tm.tmHeight;

	skin.shared->general.getskinspath(skin_path, sizeof(skin_path));
	str_cat(skin_path, uni("/neo/"));

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("mediasheet.png"));
	hdc_mediasheet = png_get_hdc(skin_ap, 0);
	mediasheet_w = png_w;
	mediasheet_h = png_h;

	SetBkMode(dc, TRANSPARENT);

	memset(thumbs_albums_dir, 0, sizeof(thumbs_albums_dir));
	SHGetSpecialFolderPath(0, thumbs_albums_dir, CSIDL_PERSONAL, FALSE);
	str_cat(thumbs_albums_dir, uni("\\Fennec\\Thumbnails\\Albums\\"));
	thumbs_albums_dir_slen = str_len(thumbs_albums_dir);

	memset(thumbs_artists_dir, 0, sizeof(thumbs_artists_dir));
	SHGetSpecialFolderPath(0, thumbs_artists_dir, CSIDL_PERSONAL, FALSE);
	str_cat(thumbs_artists_dir, uni("\\Fennec\\Thumbnails\\Artists\\"));
	thumbs_artists_dir_slen = str_len(thumbs_artists_dir);

	SetStretchBltMode(dc, HALFTONE);
	SetBrushOrgEx(dc, 0, 0, 0);

	if(!mode_ml)
	{
		ml_cache_uninit();
		mode_ml = 1;

		if(fullview_switching_manually)
		{
			str_cpy(current_album, settings_data.library.current_album);
			str_cpy(current_artist, settings_data.library.current_artist);
			ml_current_dir = settings_data.library.current_dir;
			lib_show_album = settings_data.library.show_album;
			ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
		}



		ml_cache_init();
		ml_pl_startid = 0;
	}else{

		if(fullview_switching_manually)
		{
			ml_current_cdir = current_artist;
			ml_current_dir = media_library_dir_artists;
			ml_dir_changed = 1;
			lib_show_album = 1;
			ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
			ml_pl_startid = 0;
			ml_cache_uninit();
			ml_cache_init();

			if(skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0) > 0) /* count */
				media_lib_empty = 0;
			else
				media_lib_empty = 1;
		}
		
	}

	libit_init();

	timer_lib = (unsigned int)SetTimer(0, 0, 35, (TIMERPROC) lib_timer);


	/* media lib */
	skin.shared->mlib.media_library_advanced_function(1, 0, 0); /* safe initialize */
	lib_media_lib_count = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0);

	
	if(!lib_curdir_len)
		memset(lib_curdir, 0, sizeof(lib_curdir));

	lib_media_show(0);
}

void lib_uninit(void)
{
	DeleteDC(hdc_mediasheet);

	KillTimer(0, timer_lib);

	libit_uninit();

	DeleteObject(lib_font_item);
	SelectObject(hdc_backup, old_font);
}

void lib_set_sizes(void)
{
	TEXTMETRIC  tm;

	SelectObject(hdc_backup, old_font);
	DeleteObject(lib_font_item);

	if(settings_data.playlist.display_mode == playlist_display_small)
	{
		lib_font_item = CreateFont(-11,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}else{

		lib_font_item = CreateFont(-14,
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
									DEFAULT_PITCH, uni("Tahoma"));
	}

	SelectObject(hdc_backup, lib_font_item);

	memset(&tm, 0, sizeof(tm));
	GetTextMetrics(hdc_backup, &tm);

	lib_textheight = tm.tmHeight;
}

void draw_medialibrary_message(HDC dc)
{
#define str_welcome uni("Welcome to Media Library!")
#define str_tip uni("Your library is empty, use this button to manage it...")
	RECT rct;
	COLORREF ocolor;
	HFONT    ofont, ntfont, ndfont;

	GetClientRect(window_media, &rct);

	ocolor = GetTextColor(dc);
	

	ntfont = CreateFont(-36, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
								DEFAULT_PITCH, uni("Tahoma"));

	ndfont = CreateFont(-24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
								DEFAULT_PITCH, uni("Tahoma"));

	ofont = (HFONT) SelectObject(dc, ntfont);
	SetTextColor(dc, 0x555555);
	TextOut(dc, (rct.right / 2) - 250 + 2, (rct.bottom / 2) - 40 + 2, str_welcome, str_len(str_welcome));
	SetTextColor(dc, 0xe6e6e6);
	TextOut(dc, (rct.right / 2) - 250, (rct.bottom / 2) - 40, str_welcome, str_len(str_welcome));
	
	SelectObject(dc, ndfont);
	SetTextColor(dc, 0x555555);
	TextOut(dc, (rct.right / 2) - 250 + 2, (rct.bottom / 2) - 40 + 37 + 2, str_tip, str_len(str_tip));
	SetTextColor(dc, 0xe6e6e6);
	TextOut(dc, (rct.right / 2) - 250, (rct.bottom / 2) - 40 + 37, str_tip, str_len(str_tip));
	
	drawrect(dc, rct.right - 135, (rct.bottom / 2) + 30, 14, rct.bottom - ((rct.bottom / 2) + 30 + 38), 0x00ccff);
	drawrect(dc, (rct.right / 2) - 250, (rct.bottom / 2) + 30, (rct.right - 135) - ((rct.right / 2) - 250), 14, 0x00ccff);

	SelectObject(dc, ofont);
	DeleteObject(ntfont);
	DeleteObject(ndfont);
	SetTextColor(dc, ocolor);
}


static HDC  cover_image_get(string fpath, int sendloc /* to speed up the process */, int *w, int *h)
{
	int slen;
	HDC dct = 0;

	slen = str_len(fpath + sendloc) + sendloc;

	str_cpy(fpath + slen, uni(".jpg"));

	dct = (HDC) jpeg_get(fpath, w, h);

	if(!dct) /* no jpeg, try png */
	{
		str_cpy(fpath + slen, uni(".png"));
		dct = png_get_hdc_24(fpath);

		if(dct)
		{
			*w = png_w;
			*h = png_h;
		}
	}

	return dct;
}

void draw_libitem(HDC dc, int x, int y, int mode, const string ititle, int tlen, int clear, int light, int ayear)
{
	RECT rct;
	int vx, vy;

	vx = 10 + dz(x * 185);
	vy = 10 + 74 + ((y - libitems_start_row) * (dz(208 + 10) + 20));

	if(vy < 0) return;
	//if(y - libitems_start_row >= libitems_max_rows) return 0;

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
		{
			HDC dct = 0;
			int thumbnotfound = 1;
			int w = 225, h = 225;

			if(ititle[0] == uni('[')) goto point_nothumb;
			if(!settings_data.advanced.enable_album_art) goto point_nothumb;

			if(!lib_show_album && !settings_data.advanced.enable_artist_photo)
			{
				dct = 0;
				goto point_get_albumart_for_artist;
			}

			if(lib_show_album)
			{
				str_cpy(thumbs_albums_dir + thumbs_albums_dir_slen, current_artist);
				str_cat(thumbs_albums_dir + thumbs_albums_dir_slen, uni(" - "));
				str_ncat(thumbs_albums_dir + thumbs_albums_dir_slen, ititle, tlen);
				//str_cat(thumbs_albums_dir + thumbs_albums_dir_slen, uni(".jpg"));

				//dct = (HDC) jpeg_get(thumbs_albums_dir, &w, &h);
				dct = cover_image_get(thumbs_albums_dir, thumbs_albums_dir_slen, &w, &h);
				
				if(dct) thumbnotfound = 0;

				if(!dct && !error_in_last_image)
				{
					if(settings_data.advanced.auto_download_covers)
					{
						letter atitle[1024];
						str_ncpy(atitle, ititle, tlen);
						atitle[tlen] = 0;
						covers_addtoqueue(current_artist, ititle);
					}
				}

				thumbs_albums_dir[thumbs_albums_dir_slen] = 0;
			}else{

				/* check direct artist picture */

				str_ncpy(thumbs_artists_dir + thumbs_artists_dir_slen, ititle, tlen);
				thumbs_artists_dir[thumbs_artists_dir_slen + tlen] = 0;
				//str_cat(thumbs_artists_dir + thumbs_artists_dir_slen, uni(".jpg"));

				dct = (HDC) cover_image_get(thumbs_artists_dir, thumbs_artists_dir_slen, &w, &h);
				
				if(dct) thumbnotfound = 0;

				if(!dct && !error_in_last_image)
				{
					if(settings_data.advanced.auto_download_covers)
					{
						letter atitle[1024];
						str_ncpy(atitle, ititle, tlen);
						atitle[tlen] = 0;
						covers_addtoqueue(atitle, 0);
					}
				}

				thumbs_artists_dir[thumbs_artists_dir_slen] = 0;

point_get_albumart_for_artist:

				if(!dct) /* try for an album art */
				{
					WIN32_FIND_DATA ffd;
					HANDLE ffile;

					thumbs_albums_dir[thumbs_albums_dir_slen] = 0;

					str_ncpy(thumbs_albums_dir + thumbs_albums_dir_slen, ititle, tlen);
					thumbs_albums_dir[thumbs_albums_dir_slen + tlen] = 0;
					str_cat(thumbs_albums_dir + thumbs_albums_dir_slen, uni(" - *.jpg"));

			
					ffile = FindFirstFile(thumbs_albums_dir, &ffd);

					if(ffile != INVALID_HANDLE_VALUE)
					{
						thumbs_albums_dir[thumbs_albums_dir_slen] = 0;
						str_cpy(thumbs_albums_dir + thumbs_albums_dir_slen, ffd.cFileName);
						FindClose(ffile);
						dct = (HDC) jpeg_get(thumbs_albums_dir, &w, &h);
						if(dct) thumbnotfound = 0;
					}

					thumbs_albums_dir[thumbs_albums_dir_slen] = 0;


					str_ncpy(thumbs_albums_dir + thumbs_albums_dir_slen, ititle, tlen);
					thumbs_albums_dir[thumbs_albums_dir_slen + tlen] = 0;
					str_cat(thumbs_albums_dir + thumbs_albums_dir_slen, uni(" - *.png"));

			
					ffile = FindFirstFile(thumbs_albums_dir, &ffd);

					if(ffile != INVALID_HANDLE_VALUE)
					{
						thumbs_albums_dir[thumbs_albums_dir_slen] = 0;
						str_cpy(thumbs_albums_dir + thumbs_albums_dir_slen, ffd.cFileName);
						FindClose(ffile);
						dct = (HDC) png_get_hdc_24(thumbs_albums_dir);
						w = png_w; h = png_h;
						if(dct) thumbnotfound = 0;
					}

					thumbs_albums_dir[thumbs_albums_dir_slen] = 0;

				}


			}

point_nothumb:

			if(!thumbnotfound)
			{
				if(w > h) w = h;
				if(h > w) h = w;

				StretchBlt(dc, vx + dz(14), vy + dz(7), dz(158), dz(158), dct, 0, 0, w, h, SRCCOPY);
				//alpha_blit_resize(dc, dct, vx + dz(19), vy + dz(12), dz(148), dz(148), 0, 0, w, h, 100);
				DeleteDC(dct);

			}else{
				alpha_blit_resize(dc, hdc_mediasheet, vx + dz(19), vy + dz(12), dz(148), dz(148), 206, 245, 148, 148, 100);
			}
		}
		break;
	}

	rct.top = vy + dz(182);
	rct.left = vx + dz(10);
	rct.bottom = rct.top + (36 / lib_textheight) * lib_textheight;
	rct.right = rct.left + dz(166);

	SetTextColor(dc, 0x0);
	DrawText(dc, ititle, tlen, &rct, DT_CENTER | DT_END_ELLIPSIS | DT_WORDBREAK);

	alpha_blit_resize(dc, hdc_mediasheet, vx, vy, dz(185), dz(87), 0, 220, 185, 87, light ? 80 : 48);

	if(mode == imode_album && ayear > 0)
	{
		letter buf[32];

		str_itos(ayear, buf, 10);
		alpha_blit_resize(dc, hdc_mediasheet, vx + dz(10), vy + dz(5), dz(51), dz(23), 7, 315, 51, 23, 50);
		SetTextColor(dc, 0xffffff);
		TextOut(dc, vx + dz(16), vy + dz(6), buf, str_len(buf));
	}

	if(clear)
		fullview_render(vx, vy,  dz(185),  dz(208));

}


void lib_draw_itemfull(HDC dc, int itemid, int xv, int yv, int clear, int light)
{
	if(lib_showtags && libitem_list[itemid].mode == imode_file)
	{
		struct fennec_audiotag at;
		letter buf[1024];

		skin.shared->mlib.media_library_translate(libitem_list[itemid].param, &at);
		if(at.tag_artist.tdata && at.tag_artist.tsize)
		{
			str_cpy(buf, at.tag_artist.tdata);
			if(at.tag_title.tdata && at.tag_title.tsize)
			{
				str_cat(buf, uni(" - "));
				str_cat(buf, at.tag_title.tdata);
			}
		
			draw_libitem(dc, xv, yv, libitem_list[itemid].mode, buf, str_len(buf), clear, light, libitem_list[itemid].param);

		}else if(at.tag_title.tdata && at.tag_title.tsize){
			
			str_cpy(buf, at.tag_title.tdata);
			draw_libitem(dc, xv, yv, libitem_list[itemid].mode, buf, str_len(buf), clear, light, libitem_list[itemid].param);

		}else{
			draw_libitem(dc, xv, yv, libitem_list[itemid].mode, libitem_list[itemid].title, libitem_list[itemid].title_len, clear, light, libitem_list[itemid].param);
		}
		
	}else{
		draw_libitem(dc, xv, yv, libitem_list[itemid].mode, libitem_list[itemid].title, libitem_list[itemid].title_len, clear, light, libitem_list[itemid].param);
	}
}

void lib_drawitems(HDC dc)
{
	int   i = 0, ri, xv, yv, ni, si, j = 0, ci;
	HRGN  rgn;

	if(media_lib_empty) draw_medialibrary_message(dc);

	rgn = CreateRectRgn(0, 74, lib_view_w, lib_view_h + 74 - 35);
	SelectClipRgn(dc, rgn);

	si = (libitems_start_row * libitems_items_a_row);
	ci = (libitems_items_a_row * libitems_max_rows);
	ni = si + ci;
	
	//for(i=(libitems_start_row * libitems_items_a_row); i<ni; i++)

	for(i=0; i<libitem_list_cur; i++)
	{
		ri = libitem_list[i].rindex;

		//if(i >= libitem_list_cur) break;

		if(ri < si) continue;
		if(ri >= ni) continue;
		if(j > ci) break;

		xv = ri % libitems_items_a_row;
		yv = ri / libitems_items_a_row;

		lib_draw_itemfull(dc, i, xv, yv, 0, 0);

		j++;
	}

	SelectClipRgn(dc, 0);
}


void lib_drawoneitem(HDC dc, int itemid, int light)
{
	int xv, yv;
	

	if(!libitem_list_cur) return;
	if(itemid < 0) return;
	if(itemid >= libitem_list_cur) return;
	
	xv = libitem_list[itemid].rindex % libitems_items_a_row;
	yv = libitem_list[itemid].rindex / libitems_items_a_row;

	if(yv < libitems_start_row) return;
	if(yv >= libitems_start_row + libitems_max_rows) return;

	//yv -= libitems_start_row;//* libitems_items_a_row);

	lib_draw_itemfull(dc, itemid, xv, yv, 1, light);
}

void lib_draw(HDC dc, int w, int h)
{
	lib_view_w = w;
	lib_view_h = h;

	libitems_items_a_row = (w - 20) /  dz(185);
	libitems_max_rows = (h - 10 - 74) /  (dz((208 + 10)) + 20);


	scroll_max = libitem_list_cur / libitems_items_a_row;
	libitems_start_row = scroll_value;

	if(libitems_start_row > libitem_list_cur / libitems_items_a_row) libitems_start_row =  libitem_list_cur / libitems_items_a_row;
	else if(libitems_start_row < 0) libitems_start_row = 0;

	lib_drawitems(dc);
}

void lib_halt(void)
{
	libit_clear();
	halt_media_lib = 1;
}

void lib_halt_resume(void)
{
	halt_media_lib = 0;
	lib_media_show(0);
}



void lib_mousemsg(int x, int y, int action)
{
	static int last_itemid = 0;
	static int ldown = 0;
	int  cx, cy, itemid, i;

	
	if(action == mm_wheel)
	{
		if(x != 0)
		{
			if((GetKeyState(VK_CONTROL) & 0x8000)){

				libzoom_r += (float)(x / abs(x)) * 0.05f;

				if(libzoom_r < 0.25f) libzoom_r = 0.25f;

				libzoom = libzoom_r;

				if(libzoom_r < 1.0f && libzoom_r > 0.9f) libzoom = 1.0f;
				if(libzoom_r < 1.1f && libzoom_r > 1.0f) libzoom = 1.0f;

				if(libzoom_r < 0.7f && libzoom_r > 0.6f) libzoom = 0.7f;
				if(libzoom_r < 0.8f && libzoom_r > 0.7f) libzoom = 0.7f;

				if(libzoom_r < 0.5f && libzoom_r > 0.4f) libzoom = 0.5f;
				if(libzoom_r < 0.6f && libzoom_r > 0.5f) libzoom = 0.5f;

				if(libzoom_r < 2.0f && libzoom_r > 1.8f) libzoom = 2.0f;
				if(libzoom_r < 2.2f && libzoom_r > 2.0f) libzoom = 2.0f;

				settings_data.dv.zoomv = libzoom;

				fullview_drawgeneral();

			}else{
				
				libitems_start_row -= (x / abs(x));

				if(libitems_start_row > libitem_list_cur / libitems_items_a_row) libitems_start_row =  libitem_list_cur / libitems_items_a_row;
				else if(libitems_start_row < 0) libitems_start_row = 0;

				fullview_drawgeneral();
			}
		}
	}

	if(x < 20) return;
	if(y < 74) return;
	if(y > lib_view_h - 35) return;

	/* ----------------------------------- */

	/* calculate item id */

	cx = (x - 10) / dz(185);
	cy = (y - 10 - 74) / (dz(208 + 10) + 20);

	if((y - 10 - 74) % (dz(208 + 10) + 20) > dz(185)) return;

	if(cx < 0) return;
	if(cy < 0) return;
	if(cx >= libitems_items_a_row) return;

	cy += libitems_start_row;

	itemid = cx + (cy * libitems_items_a_row);

	if(itemid >= libitem_list_cur)return;// itemid = libitem_list_cur - 1;

	
	for(i=0; i<libitem_list_cur; i++)
	{
		if(itemid == libitem_list[i].rindex)
		{
			itemid = i;
			break;
		}
	}

	if(action == mm_down_l) ldown = 1;
	if(action == mm_up_l) ldown = 0;


	

	

	
	if(action == mm_down_l)
	{
		if(!lib_show_album)
		{
			memset(current_artist, 0, sizeof(current_artist));
			str_ncpy(current_artist, libitem_list[itemid].title, libitem_list[itemid].title_len);
			lib_show_album = 1;
			libitems_start_row = 0;

			libit_clear();
			lib_media_show(0);
		
		}else{ /* in an album, display playlist */

			memset(current_album, 0, sizeof(current_album));
			str_ncpy(current_album, libitem_list[itemid].title, libitem_list[itemid].title_len);
	
			ml_current_cdir = current_album;
			ml_current_dir  = media_library_dir_albums;
			ml_dir_changed = 1;
			libitems_start_row = 0;

			ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
			
			ml_pl_startid = 0;
			ml_cache_uninit();
			ml_cache_init();

			fullview_return_to_library = 1;

			fullview_switch(vmode_playlist);
			fullview_drawgeneral();
		}
	}

/*
	if(action == mm_down_r)
	{
		POINT pt;
		int   rv;
		HMENU mc = user_create_menu(menu_ml_lib_popup, mode_ml);
		
		GetCursorPos(&pt);

		rv = (int)TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, window_media, 0);
		
		if(rv == mid_lib_add || rv == mid_lib_play)
		{
			if(rv == mid_lib_play)
				skin.shared->audio.output.playlist.clear();

			if(libitem_list[itemid].mode == imode_folder)
			{
				letter lib_cur_bkp[1024];
				int    lib_curdir_len_bkp = lib_curdir_len;

				str_cpy(lib_cur_bkp, lib_curdir);

				str_ncat(lib_curdir, libitem_list[itemid].title, libitem_list[itemid].title_len);
				lib_curdir[lib_curdir_len + libitem_list[itemid].title_len] = uni('\\');
				lib_curdir[lib_curdir_len + libitem_list[itemid].title_len + 1] = uni('\0');

				lib_curdir_len = str_len(lib_curdir);

				lib_media_addtolist();
				
				lib_curdir_len = lib_curdir_len_bkp;
				str_cpy(lib_curdir, lib_cur_bkp);


			}else{
				struct fennec_audiotag at;

				skin.shared->mlib.media_library_translate(libitem_list[itemid].param, &at);
				skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
			}

			if(rv == mid_lib_play)
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


	if(action == mm_dbl_l)
	{
		if(libitem_list[itemid].mode == imode_file)
		{
			int i, k = 0;
			struct fennec_audiotag at;

			skin.shared->audio.output.playlist.clear();

			
			for(i=0; i<libitem_list_cur; i++)
			{
				if(libitem_list[i].mode == imode_file)
				{
					if(itemid == i) k = i;
					skin.shared->mlib.media_library_translate(libitem_list[i].param, &at);
					skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
				}
			}

			skin.shared->audio.output.playlist.switch_list(k);
			skin.shared->audio.output.play();

			SetFocus(window_media);
		}
	}


	*/
	if(action == mm_move)
	{
		if(last_itemid != itemid)
		{
			lib_drawoneitem(hdc_backup, last_itemid, 0);
			lib_drawoneitem(hdc_backup, itemid, 1);

			last_itemid = itemid;
		}

	}

	

}

void CALLBACK lib_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	
}

int lib_keymsg(int key)
{
	switch(key)
	{
	case VK_BACK:
		if(lib_show_album)
		{
			libit_clear();
			lib_show_album = 0;
			scroll_value = 0;

			if(fullview_switching_manually)
			{
				ml_current_cdir = 0;
				ml_current_dir = media_library_dir_artists;
				ml_dir_changed = 1;
				//lib_show_album = 1;
				ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
				ml_pl_startid = 0;
				ml_cache_uninit();
				ml_cache_init();
			}

			lib_media_show(0);
		}else{
			
			fullview_switch(vmode_media_intro);
			fullview_drawgeneral();
		}
		break;
	}
	return 1;
}

void lib_media_show(int param)
{
	string  stxstr;
	int     i, stxlen;

	if(!lib_show_album) /* just display the artists list */
	{
		for(i=0; i<(int)cached_tags_count; i++)
		{
			stxstr = cached_tags[ml_pl_startid + i].dname;
			stxlen = str_len(stxstr);

			if(!stxlen)
			{
				libit_add(uni("[Empty]"), 7, imode_album, 0);
			}else{
				libit_add(stxstr, stxlen, imode_album, 0);
			}
		}

		memset(full_view_text_location, 0, sizeof(full_view_text_location));
		str_cpy(full_view_text_location, uni("Artists"));

	}else{
		
		struct fennec_audiotag  at;
		int            c;
		uint32_t       k = 0;
		unsigned long  j = 0, m = 0, n, albfound, albid = 0;
		string         albm_names[10];

		for(n=0; n<10; n++)
			albm_names[n] = 0;

		for(;;)
		{
			c = skin.shared->mlib.media_library_get_dir_files(j, ml_current_dir, current_artist, &k);
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
					int ayear = 0;

					albm_names[m] = at.tag_album.tdata;

					if(at.tag_year.tdata && at.tag_year.tsize)
						ayear = str_stoi(at.tag_year.tdata);

					libit_add(albm_names[m], str_len(at.tag_album.tdata), imode_album, ayear);

					m++;
					albid++;
					if(m >= 10) break;
				}
			}
			
			k++;
		}

		memset(full_view_text_location, 0, sizeof(full_view_text_location));
		str_cpy(full_view_text_location, current_artist);
	}


	fullview_drawgeneral();
}

void lib_media_addtolist(void)
{

}