#include "skin.h"
#include "skin_settings.h"
#include <shlobj.h>
#include <userenv.h>

#define search_window_class uni("fennec.skin.3.search")

#define str_search_mode_track  uni("Tracks")
#define str_search_mode_album  uni("Artists and Albums")

#define search_mode_track  1
#define search_mode_album  2
#define search_mode_artist 2

#define result_item_height 26

LRESULT CALLBACK callback_search_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK callback_searchtext(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void search_drawitems(void);
void search_all(string  searchstr);

int     search_init = 0;
HWND    window_search = 0;
HDC     hdc_search = 0;
HDC     hdc_rsearch = 0;
HBITMAP hbm_search = 0;
HRGN    rgn_search = 0;
double  aspect = 16.0 / 9.0;

int     search_thread_enable = 1;
int     search_thread_terminate = 0;

HBRUSH  hbrush_searchtext = 0;
WNDPROC searchwindowtext_wndproc = 0;
HWND    hwnd_searchbox;
HFONT   search_font_main;
HFONT   search_font_title;
HFONT   search_font_item;

int     result_list_startid = 0;
int     result_list_max;


string  search_terms[20]; /* all these terms shall be found in a track */

struct  search_result_item
{
	unsigned long index;
	int           listindex;
};

struct  search_album_set
{
	string  artist;
	string  album;
};

struct  search_album_set    search_albums[32];
int		search_albums_count   = 0;
int		search_albums_startid = 0;
int		search_albums_max     = 0;
int		search_albums_offset  = 0;

struct  search_result_item  *search_results = 0;
int     search_result_count = 0;
int     search_result_max = 32;
int     search_result_curitem = 0;


static int     search_tab = search_mode_album;
static letter  search_find_text[1024];

static letter     thumbs_albums_dir[1024];
static letter     thumbs_artists_dir[1024];
static int        thumbs_albums_dir_slen = 0;
static int        thumbs_artists_dir_slen = 0;




void search_reinit(void)
{
	RECT     wrect;
	int      vheight;

	GetWindowRect(wnd, &wrect);

	vheight = (int)((double)(wrect.right - wrect.left) / aspect);
	
	SetWindowPos(window_search, 0, wrect.left, wrect.top - vheight - 5, wrect.right - wrect.left, vheight, SWP_NOZORDER);
	ShowWindow(window_search, SW_SHOW);
	search_thread_enable = 1;
	return;
}


void search_create(HWND hwndp)
{
	WNDCLASS wndc;
	RECT     wrect;
	int      vheight;

	if(search_init)
	{
		search_reinit();
		return;
	}

	skin.shared->mlib.media_library_advanced_function(1, 0, 0); /* safe initialize */


	wndc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_DROPSHADOW;
	wndc.lpfnWndProc   = (WNDPROC)callback_search_window;
	wndc.cbClsExtra    = 0;
	wndc.cbWndExtra    = 0;
	wndc.hInstance     = instance_skin;;
	wndc.hIcon         = LoadIcon(skin.finstance, (LPCTSTR)0);
	wndc.hCursor       = LoadCursor(0, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndc.lpszMenuName  = 0;
	wndc.lpszClassName = search_window_class;

	RegisterClass(&wndc);

	/* create window */

	hbrush_searchtext = CreateSolidBrush(0x000000);

	GetWindowRect(hwndp, &wrect);

	vheight = (int)((double)(wrect.right - wrect.left) / aspect);

	result_list_max = (vheight - (38 + 48)) / result_item_height;
	
	window_search = CreateWindow(search_window_class, uni("fennec.quicksearch"), WS_POPUP, wrect.left, wrect.top - vheight - 5, wrect.right - wrect.left, vheight, hwndp, 0, instance_skin, 0);


	/* memory allocation */

	search_results = (struct  search_result_item*) sys_mem_alloc(search_result_max * sizeof(struct  search_result_item));
	search_result_count = 0;
	search_result_max = 32;

	/* <textbox> */

	hwnd_searchbox = CreateWindowW(uni("EDIT"), uni(""), WS_CHILD | WS_VISIBLE, 7, vheight - 40, (wrect.right - wrect.left) - 14, 30, window_search, 0, skin.finstance, 0);
	searchwindowtext_wndproc = (WNDPROC)GetWindowLongPtr(hwnd_searchbox, GWLP_WNDPROC);
	SetWindowLongPtr(hwnd_searchbox, GWLP_WNDPROC, (LONG_PTR)callback_searchtext);

	search_font_main = CreateFont(-24,
                       0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                       DEFAULT_PITCH, uni("Arial"));

	search_font_title = CreateFont(-18,
                       0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                       DEFAULT_PITCH, uni("Arial"));

	search_font_item = CreateFont(14,
                       0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                       DEFAULT_PITCH, uni("Arial"));

	SendMessage(hwnd_searchbox, WM_SETFONT, (WPARAM)search_font_main, (LPARAM)0);


	/* </textbox> */

	memset(thumbs_albums_dir, 0, sizeof(thumbs_albums_dir));
	SHGetSpecialFolderPath(0, thumbs_albums_dir, CSIDL_PERSONAL, FALSE);
	str_cat(thumbs_albums_dir, uni("\\Fennec\\Thumbnails\\Albums\\"));
	thumbs_albums_dir_slen = str_len(thumbs_albums_dir);

	memset(thumbs_artists_dir, 0, sizeof(thumbs_artists_dir));
	SHGetSpecialFolderPath(0, thumbs_artists_dir, CSIDL_PERSONAL, FALSE);
	str_cat(thumbs_artists_dir, uni("\\Fennec\\Thumbnails\\Artists\\"));
	thumbs_artists_dir_slen = str_len(thumbs_artists_dir);



	
	ShowWindow(window_search, SW_SHOW);
	UpdateWindow(window_search);
	
	SetWindowText(hwnd_searchbox, uni(""));

	search_thread_enable = 1;
	search_init = 1;
}

void search_close(void)
{
	if(!search_init)return;

	search_init = 0;

	DeleteObject(hbrush_searchtext);
	DeleteObject(search_font_main);

	if(hdc_search)
	{
		DeleteObject(hbm_search);
		DeleteDC(hdc_search);
	}

	if(hdc_rsearch)DeleteDC(hdc_rsearch);

	DestroyWindow(window_search);
	if(rgn_search)DeleteObject(rgn_search);

	if(search_results)
		sys_mem_free(search_results);

	
}


void rdb_addresult(unsigned long mid, int listindex)
{
	if(search_result_count + 1 >= search_result_max)
	{
		search_result_max += 128;
		search_results = (struct  search_result_item*) sys_mem_realloc(search_results, search_result_max * sizeof(struct  search_result_item));
	}

	search_results[search_result_count].index     = mid;
	search_results[search_result_count].listindex = listindex;

	search_result_count++;
}

void rdb_clear(void)
{
	search_result_count = 0;
	search_result_curitem = 0;

	if(search_result_max > 10000)
	{
		search_result_max = 32;
		search_results = (struct  search_result_item*) sys_mem_realloc(search_results, search_result_max * sizeof(struct  search_result_item));
	}
}

unsigned long rdb_getid(int index)
{
	if(index < 0 || index >= search_result_count) return (unsigned long)-1;

	return search_results[index].index;
}


void search_draw_background(void)
{
	RECT  rct;
	HFONT oldfont;


	GetClientRect(window_search, &rct);	

	if(rgn_search)DeleteObject(rgn_search);
	rgn_search = CreateRoundRectRgn(0, 0, rct.right, rct.bottom, 4, 4);
	SetWindowRgn(window_search, rgn_search, 1);

	{
		struct coord tl = {0, 0, 6, 208, 0, 0, 0, 0, 0, 0};
		struct coord tr = {0, 0, 7, 208, 7, 0, 0, 0, 0, 0};
		struct coord bl = {0, 0, 7, 9, 0, 235, 0, 0, 0, 0};
		struct coord br = {0, 0, 7, 7, 7, 236, 0, 0, 0, 0};
		struct coord t  = {0, 0, 15, 6, 0, 244, 0, 0, 0, 0};
		struct coord b  = {0, 0, 15, 6, 0, 252, 0, 0, 0, 0};
		struct coord l  = {0, 0, 5, 28, 0, 208, 0, 0, 0, 0};
		struct coord r  = {0, 0, 5, 28, 9, 208, 0, 0, 0, 0};

		draw_imagebox(hdc_search, windowsheet_dc, rct.right, rct.bottom, &tl, &tr, &bl, &br, &t, &b, &l, &r);
	}

	drawrect(hdc_search, 5, 5, rct.right - 10, rct.bottom - 10, 0x0);


	oldfont = (HFONT)SelectObject(hdc_search, search_font_title);

	SetTextColor(hdc_search, 0xe6e6e6);

	drawrect(hdc_search, 9, 10, rct.right - 18, 27, 0x4f4f4f);

	if(search_tab == search_mode_album)
		TextOut(hdc_search, 16, 12, str_search_mode_album, str_len(str_search_mode_album));
	else
		TextOut(hdc_search, 16, 12, str_search_mode_track, str_len(str_search_mode_track));


	SelectObject(hdc_search, oldfont);


	search_drawitems();

	
	BitBlt(hdc_rsearch, 0, 0, rct.right, rct.bottom, hdc_search, 0, 0, SRCCOPY);

	InvalidateRect(hwnd_searchbox, 0, 0);
	UpdateWindow(hwnd_searchbox);

}


void search_result_draw(int id, int x, int y, int w, unsigned long mid)
{
	static letter outtext[1024];
	struct fennec_audiotag at;

	if(mid == (unsigned long)-1) return;

	skin.shared->mlib.media_library_translate(mid, &at);

	if(id == search_result_curitem)
	{
		drawrect(hdc_search, x, y, w, 26, 0x888888);

	}else{
		if(!(id % 2))
			drawrect(hdc_search, x, y, w, 26, 0x626262);
		else
			drawrect(hdc_search, x, y, w, 26, 0x434343);
	}

	outtext[0] = 0;

	if(at.tag_artist.tdata && at.tag_artist.tsize)
	{
		str_cpy(outtext, at.tag_artist.tdata);
		str_cat(outtext, uni(" - "));
	}

	if(at.tag_title.tdata && at.tag_title.tsize)
	{
		str_cat(outtext, at.tag_title.tdata);
	}

	TextOut(hdc_search, x + 6, y + 9, outtext, str_len(outtext));
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

void search_drawitems(void)
{
	RECT  rct;
	HFONT oldfont;
	int   i, y = 38, k = 0, ofst;

	GetClientRect(window_search, &rct);	


	if(search_tab == search_mode_album)
	{
		oldfont = (HFONT)SelectObject(hdc_search, search_font_item);

		

		search_albums_max = ((rct.right - 20) / 113) + 1;

		k = search_albums_startid;
		ofst = search_albums_offset;

		for(i=0; i<search_albums_max; i++)
		{
			HRGN rgn = CreateRectRgn(ofst + (i * 113) + 10, 0, min(ofst + (i * 113) + 109 + 10, rct.right - 10), rct.bottom);
			HDC  dct;
			int  w = 225, h = 225;
			int  tlen;
			
			if(k >= search_albums_count) break;
			
			tlen = str_len( search_albums[k].artist);

			SelectClipRgn(hdc_search, rgn);

			SetTextColor(hdc_search, 0xdddddd);
			TextOut(hdc_search, ofst + (i * 113) + 10, rct.bottom - 87, search_albums[k].artist, tlen);
			
			if(search_albums[k].album)
			{
				SetTextColor(hdc_search, 0xaaaaaa);
				TextOut(hdc_search, ofst + (i * 113) + 10, rct.bottom - 87 + 15, search_albums[k].album, str_len(search_albums[k].album));
			}

			if(!search_albums[k].album)
			{
				str_ncpy(thumbs_artists_dir + thumbs_artists_dir_slen, search_albums[k].artist, tlen);
				thumbs_artists_dir[thumbs_artists_dir_slen + tlen] = 0;

				dct = (HDC) cover_image_get(thumbs_artists_dir, thumbs_artists_dir_slen, &w, &h);
			}else{

				str_cpy(thumbs_albums_dir + thumbs_albums_dir_slen, search_albums[k].artist);
				str_cat(thumbs_albums_dir + thumbs_albums_dir_slen, uni(" - "));
				str_ncat(thumbs_albums_dir + thumbs_albums_dir_slen, search_albums[k].album, str_len(search_albums[k].album));

				dct = cover_image_get(thumbs_albums_dir, thumbs_albums_dir_slen, &w, &h);
			}

			if(dct)
			{
				if(w > h) w = h;
				if(h > w) h = w;
				StretchBlt(hdc_search, ofst + (i * 113) + 10, 42, 109, 109, dct, 0, 0, w, h, SRCCOPY);
				DeleteDC(dct);
			}else{

				drawrect(hdc_search, ofst + (i * 113) + 10, 42, 109, 109, 0x555555);
			}

			thumbs_artists_dir[thumbs_artists_dir_slen] = 0;


			SelectClipRgn(hdc_search, 0);
			DeleteObject(rgn);

			k++;
		}
		

		SelectObject(hdc_search, oldfont);

	}else{

		oldfont = (HFONT)SelectObject(hdc_search, search_font_item);

		SetTextColor(hdc_search, 0xdddddd);


		for(i=result_list_startid; i<result_list_startid+result_list_max; i++)
		{
			if(i >= search_result_count) break;

			search_result_draw(i, 9, y, rct.right - 18, rdb_getid(i));
			y += 27;
		}
		

		SelectObject(hdc_search, oldfont);
	}
}

string  str_instr(string ss, string tt)
{
	static letter   sb[1024];
	static letter   tb[1024];
	string   t = tb;
	string   s = sb;
	string   mterm = t;
	int      matchstarted = 0;

	str_ncpy(s, ss, 1023);
	str_ncpy(t, tt, 1023);

	str_lower(s); str_lower(t);

	for(;;s++)
	{
		if(*s == 0) return 0;

		if(*s == *mterm)
		{
			matchstarted = 1;
			mterm++;

			if(*mterm == 0) return s + 1;

		}else{
			if(matchstarted)
			{
				mterm = t;
				matchstarted = 0;
			}
		}
	} 
}

void search_all(string  searchstr)
{
	unsigned long scount = 0;
	string s = searchstr, o = searchstr;
	unsigned long i, j, artists_end_pos = 0;
	unsigned long c;
	struct fennec_audiotag at;

	rdb_clear();

	for(i=0; i<20; i++)
		search_terms[i] = 0;

	/* [todo] */
	
	search_terms[0] = searchstr;
	scount = 1;


	c = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0); /* get count */

	for(i=0; i<c; i++)
	{
		int allfound = 1;

		skin.shared->mlib.media_library_translate(i, &at);

		for(j=0; j<scount; j++) /* for each term */
		{

			if(at.tag_artist.tdata && at.tag_artist.tsize)
				if(str_instr(at.tag_artist.tdata, search_terms[j])) continue;

			if(at.tag_album.tdata && at.tag_album.tsize)
				if(str_instr(at.tag_album.tdata, search_terms[j])) continue;

			if(search_tab != search_mode_album)
			{
				if(at.tag_title.tdata && at.tag_title.tsize)
					if(str_instr(at.tag_title.tdata, search_terms[j])) continue;

				if(at.tag_filepath.tdata && at.tag_filepath.tsize)
					if(str_instr(at.tag_filepath.tdata, search_terms[j])) continue;
			}

			allfound = 0;
			break;
		}

		if(allfound)rdb_addresult(i, 0);
	}

	if(search_result_count < 10 && search_tab == search_mode_album)
		search_tab = search_mode_track;
	

	if(search_tab == search_mode_album) /* generate artist, album list */
	{
		search_albums_count = 0;

		/* artists set */
		for(i=0; i<(unsigned long)search_result_count; i++)
		{
			int           found = 0;
			unsigned long mid = rdb_getid(i);

			skin.shared->mlib.media_library_translate(mid, &at);

			for(j=0; j<(unsigned long)search_albums_count; j++)
			{
				if(!at.tag_artist.tdata || !at.tag_artist.tsize) {found = 1; break;}
				if(!at.tag_album.tdata || !at.tag_album.tsize) {found = 1; break;}
				
				if(str_icmp(search_albums[j].artist, at.tag_artist.tdata) == 0)
				{
					found = 1;
					break;
				}
			}

			if(!found && at.tag_artist.tdata && at.tag_artist.tsize)
			{
				search_albums[search_albums_count].artist = at.tag_artist.tdata;
				search_albums[search_albums_count].album  = 0;
				search_albums_count++;

				if(search_albums_count >= 32) return;
			}
		}

		artists_end_pos = search_albums_count;

		/* albums set */
		for(i=0; i<(unsigned long)search_result_count; i++)
		{
			int           found = 0;
			unsigned long mid = rdb_getid(i);

			skin.shared->mlib.media_library_translate(mid, &at);

			for(j=artists_end_pos; j<(unsigned long)search_albums_count; j++)
			{
				if(!at.tag_artist.tdata || !at.tag_artist.tsize) {found = 1; break;}
				if(!at.tag_album.tdata || !at.tag_album.tsize) {found = 1; break;}

				if(!search_albums[j].album) continue;

				if(str_icmp(search_albums[j].artist, at.tag_artist.tdata) == 0 && str_icmp(search_albums[j].album, at.tag_album.tdata) == 0)
				{
					found = 1;
					break;
				}
			}

			if(!found && at.tag_artist.tdata && at.tag_artist.tsize && at.tag_album.tdata && at.tag_album.tsize)
			{
				search_albums[search_albums_count].artist = at.tag_artist.tdata;
				search_albums[search_albums_count].album  = at.tag_album.tdata;
				search_albums_count++;

				if(search_albums_count >= 32) return;
			}
		}

	}
}

int search_refresh(void)
{
	search_draw_background();
	return 1;
}

void pick_album(const string artist, const string album)
{
	int i;
	struct fennec_audiotag at;

	for(i=0; i<search_result_count; i++)
	{
		unsigned long mid = rdb_getid(i);

		skin.shared->mlib.media_library_translate(mid, &at);
		
		if(!at.tag_artist.tdata || !at.tag_artist.tsize) continue;
		
		if(album)
			if(!at.tag_album.tdata || !at.tag_album.tsize) continue;


		if(str_icmp(at.tag_artist.tdata, artist)) continue;

		if(album)
			if(str_icmp(at.tag_album.tdata, album)) continue;
			

		skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
	}

}


LRESULT CALLBACK callback_searchtext(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
	case WM_KEYDOWN:
		if(wParam == VK_RETURN)
		{
			letter newtext[1024];
			GetWindowText(hwnd, newtext, 1024);

			if(!newtext[0]) break;

			if(str_cmp(newtext, search_find_text) == 0) /* same */
			{
				if(search_result_count)
				{
					if(search_result_curitem >= 0 && search_result_curitem < search_result_count)
					{
						struct fennec_audiotag at;
						unsigned long mid;

						mid = rdb_getid(search_result_curitem);

						skin.shared->mlib.media_library_translate(mid, &at);

						if(at.tag_filepath.tdata && at.tag_filepath.tsize)
						{
							skin.shared->audio.output.playlist.clear();
							skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
							skin.shared->audio.output.playlist.switch_list(0);
							skin.shared->audio.output.play();
							break;
						}
					}
				}
			}else{
				str_cpy(search_find_text, newtext);
			}

			search_all(search_find_text);
			result_list_startid = 0;
			search_draw_background();

		}else{
			callback_search_window(window_search, msg, wParam, lParam);
		}
		break;

	case WM_KILLFOCUS:
		if(GetFocus() != hwnd_searchbox && GetFocus() != window_search)
		{
			ShowWindow(window_search, SW_HIDE);
			search_thread_enable = 0;
		}
		break;

	default:

		if(searchwindowtext_wndproc)
			return CallWindowProc(searchwindowtext_wndproc, hwnd, msg, wParam, lParam);
		else
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK callback_search_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, dx, dy, lx, ly, maximized = 0;
	static int lastsid = 0;


	switch(msg)
	{
	case WM_TIMER:
		if(search_albums_offset)
		{
			search_albums_offset /= 4;
			search_draw_background();
		}
		break;


	case WM_MOUSEMOVE:
		if(mdown)
		{
			POINT pt;
			int x;
			int sv;
			static int last_sv = 0;

			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);

			x = pt.x;
			sv = (dx - x) / 113;

			search_albums_startid = lastsid + sv;

			if(search_albums_startid < 0)search_albums_startid = 0;
			if(search_albums_startid >= search_albums_count)search_albums_startid = search_albums_count - 1;

			search_albums_offset = (x - dx) % 113;
			search_draw_background();


			if(sv != last_sv)
			{
				
				last_sv = sv;
			}
		}
		break;

	case WM_SETFOCUS:
		SetFocus(hwnd_searchbox);
		break;

	case WM_LBUTTONDOWN:
		if((int)HIWORD(lParam) < 37)
		{
			if(search_tab == search_mode_track)
				search_tab = search_mode_album;
			else
				search_tab = search_mode_track;

			search_draw_background();
			break;
		}

		if(search_tab == search_mode_track)
		{
			RECT  rct;
			int x = (int)LOWORD(lParam);
			int y = (int)HIWORD(lParam);

			GetClientRect(window_search, &rct);	

			if(x > 9 && x < rct.right - 18)
			{
				if(y > 38 && y < rct.bottom - 48)
				{
					int id = (y - 38) / (result_item_height + 1);

					id += result_list_startid;

					if(id >= 0 && id < search_result_count)
					{
						search_result_curitem = id;
						search_draw_background();
					}
				}
			}
		}else{
			mdown = 1;
			dx = (int)LOWORD(lParam);
			dy = (int)HIWORD(lParam);

			lastsid = search_albums_startid;
			SetCapture(hwnd);
		}
		break;

	case WM_LBUTTONUP:
		mdown = 0;
		ReleaseCapture();
		break;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
		if(search_tab == search_mode_track)
		{
			RECT  rct;
			int x = (int)LOWORD(lParam);
			int y = (int)HIWORD(lParam);

			GetClientRect(window_search, &rct);	

			if(x > 9 && x < rct.right - 18)
			{
				if(y > 38 && y < rct.bottom - 48)
				{
					int id = (y - 38) / (result_item_height + 1);

					id += result_list_startid;

					if(id >= 0 && id < search_result_count)
					{
						struct fennec_audiotag at;
						unsigned long mid;

						mid = rdb_getid(id);

						skin.shared->mlib.media_library_translate(mid, &at);

						if(at.tag_filepath.tdata && at.tag_filepath.tsize)
						{
							if(msg == WM_LBUTTONDBLCLK)
								skin.shared->audio.output.playlist.clear();

							skin.shared->audio.output.playlist.add(at.tag_filepath.tdata, 0, 0);
							
							if(msg == WM_LBUTTONDBLCLK)
							{
								skin.shared->audio.output.playlist.switch_list(0);
								skin.shared->audio.output.play();
							}
						}
					}

				}
			}
		}else{

			int x = (int)LOWORD(lParam);
			int id = 0;

			id = (x - 10) / 113;
			id += search_albums_startid;

			if(id < 0) break;
			if(id >= search_albums_count) break;

			if(msg == WM_LBUTTONDBLCLK)
				skin.shared->audio.output.playlist.clear();

			pick_album(search_albums[id].artist, search_albums[id].album);

			if(msg == WM_LBUTTONDBLCLK)
			{
				skin.shared->audio.output.playlist.switch_list(0);
				skin.shared->audio.output.play();
			}
		}
		break;
		
	case WM_MOUSEWHEEL:
		if((int)((short)HIWORD(wParam)) < 0)
		{
			result_list_startid += 3;
			if(result_list_startid > search_result_count - result_list_max + 2)
				result_list_startid = search_result_count - result_list_max + 2;

			search_albums_startid--;
			if(search_albums_startid < 0)search_albums_startid = 0;

			search_draw_background();

		}else{
			result_list_startid -= 3;
			if(result_list_startid < 0)
				result_list_startid = 0;

			search_albums_startid++;
			if(search_albums_startid >= search_albums_count)search_albums_startid = search_albums_count - 1;

			search_draw_background();
		}
		break;

	case WM_CTLCOLOREDIT:
		SetTextColor((HDC)wParam, 0xe6e6e6);
		SetBkColor((HDC)wParam, 0x000000);
		return (LRESULT)hbrush_searchtext;

	case WM_KILLFOCUS:
		if(GetFocus() != hwnd_searchbox && GetFocus() != window_search)
		{
			ShowWindow(hwnd, SW_HIDE);
			search_thread_enable = 0;
		}
		break;
	
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_SPACE:
			break;

		case VK_LEFT:
			//PostMessage(wnd, msg, wParam, lParam);
			break;

		case VK_RIGHT:
			//PostMessage(wnd, msg, wParam, lParam);
			break;

		case VK_UP:
			search_result_curitem--;
			if(search_result_curitem < 0) search_result_curitem = 0;

			if(search_result_curitem < result_list_startid) result_list_startid = search_result_curitem;

			search_albums_startid--;
			if(search_albums_startid < 0)search_albums_startid = 0;

			search_draw_background();
			break;

		case VK_DOWN:
			search_result_curitem++;
			if(search_result_curitem >= search_result_count)search_result_curitem = search_result_count - 1;

			if(search_result_curitem > result_list_startid + result_list_max - 1) result_list_startid = search_result_curitem - result_list_max + 1;
			
			search_albums_startid++;
			if(search_albums_startid >= search_albums_count)search_albums_startid = search_albums_count - 1;

			search_draw_background();
			break;
		}
		break;


	case WM_CREATE:
		{
			RECT rct;

			GetClientRect(hwnd, &rct);

			hdc_rsearch = GetDC(hwnd);
			hdc_search = CreateCompatibleDC(hdc_rsearch);
			hbm_search = CreateCompatibleBitmap(hdc_rsearch, rct.right, rct.bottom);

			SelectObject(hdc_search, hbm_search);

			SetStretchBltMode(hdc_search, HALFTONE);
			SetBrushOrgEx(hdc_search, 0, 0, 0);

			SetBkMode(hdc_search, TRANSPARENT);
			SetTimer(hwnd, 124, 80, 0);
		}
		break;

	case WM_PAINT:
		search_draw_background();
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
