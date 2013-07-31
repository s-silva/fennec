/*-----------------------------------------------------------------------------
  example skin file (fennec, minimalization).
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <zmouse.h>
#include "../../include/fennec.h"
#include "../../include/text ids.h"
#include "../../include/ids.h"

extern struct skin_data  skin;
extern HINSTANCE         instance_skin;
extern HDC               mdc_sheet;
extern HWND              window_ml;
extern HWND              window_eq;
extern int               vis_init;
extern HWND              window_vis;
extern HWND              window_vid;
extern HDC               hdc_vis;
extern int               last_dx, last_dy;

int visualization_messages(int id, int mdata, int sdata);

void ml_create(HWND hwndp);
void ml_close(void);
void ml_refresh(int lv);
void eq_create(HWND hwndp);
void eq_close(void);
void eq_refresh(int lv);
void eq_skinchange(void);

HBITMAP load_skin_sheet_bitmap(void);
int  skin_file_load(const string fname);

void vis_close(void);
void vis_create(HWND hwndp);
int  vis_get_position(RECT *retp);
void vis_lyrics_refresh(int inum);

void vid_create(HWND hwndp);
void vid_draw_background(int maxd);
int vid_refresh(void);
int vid_get_position(RECT *retp);
void vid_close(void);

void drawrect(HDC dc, int x, int y, int w, int h, COLORREF c);
void color_hsv_2_rgb_fullint(int h, int s, int v, unsigned char* r, unsigned char* g, unsigned char* b);


extern fn_vis_message   vis_message;

#define incoord(x, y, sx, sy, sw, sh) (((x) > (sx)) && ((y) > (sy)) && ((x) < ((sx) + (sw))) && ((y) < ((sy) + (sh))))



typedef struct _graphic_context
{
	HDC      dc;
	HPEN     lpen, firstpen;
	HBRUSH   lbrush, firstbrush;
	HFONT    lfont, firstfont;
	int      ltext_mode;
	uint32_t lbrush_color;
	uint32_t lpen_color;
	uint32_t ltext_color;
}graphic_context;


typedef struct _local_skin_settings
{
	int main_x;
	int main_y;
	int main_w;
	int main_h;

	int eq_x;
	int eq_y;
	int eq_d;
	int eq_show;

	int ml_x;
	int ml_y;
	int ml_w;
	int ml_h;
	int ml_d;
	int ml_pl_xoff[32];
	int ml_show;

	int hue;
	int sat;
	int light;
	int use_color;
	int sel_theme;
	int theme_mode;

	int vis_x;
	int vis_y;
	int vis_w;
	int vis_h;
	int vis_d;
	int vis_show;

	int ml_font_size;

	int             ml_sorted_column;
	int             ml_sorted_mode;
	unsigned long   ml_current_dir;
	int             ml_in_dir;

	int             ml_dir_sort_mode;
	int             mode_ml;

	float           zoom;

	letter          font_display[256];
	letter          skin_file_name[256];
	letter          current_vis[256];

	
	int             vid_x, vid_y, vid_w, vid_h, vid_d, vid_show;

	int             skin_lock;

}local_skin_settings;


#define coord_align_top_left     1
#define coord_align_top_right    2
#define coord_align_bottom_left  3
#define coord_align_bottom_right 4

struct coord
{
	int       x, y, w, h;
	int       sx_n, sy_n, sx_h, sy_h, sx_d, sy_d;
	int       align;
	uint32_t  ncolor, hcolor;
	int       mode;
	letter    icon_text; 
	int       font_size;
	int       font_id;
	int       bk;
	uint32_t  bk_ncolor, bk_hcolor;
};

typedef struct _skin_coords
{
	float        zoom;

	struct
	{
		int            width, height, background_sx, background_sy;
		int            window_mode /* 0 - round rect, 1 - custom */, window_edge;
		int            font_size;
		string         font_name;
		struct coord   button_play;
		struct coord   button_stop;
		struct coord   button_previous;
		struct coord   button_next;
		struct coord   button_open;
		struct coord   button_playlist;
		struct coord   button_eq;

		struct coord   button_rewind;
		struct coord   button_forward;

		struct coord   button_settings;
		struct coord   button_convert;
		struct coord   button_rip;
		struct coord   button_join;
		struct coord   button_vis;
		struct coord   button_video;
		struct coord   button_minimize;
		struct coord   button_exit;
		struct coord   button_dsp;
		struct coord   button_lock;
		struct coord   button_unlock;

		struct coord   bar_seek;
		struct coord   bar_volume;

		struct coord   display_area;
		struct coord   display_text_area;
		struct coord   display_region;

		int            infotext_x, infotext_y;
		int            pos_text_x, pos_text_y;
		int            dur_text_x, dur_text_y;

	}window_main;

	struct
	{
		int            width, height, background_sx, background_sy;

		struct coord   bands[11]; /* band 0 = preamp */

		struct coord   button_channel_u; /* universal */
		struct coord   button_channel_s; /* single */
		struct coord   button_presets;
		struct coord   button_reset;
		struct coord   button_power_on;
		struct coord   button_power_off;
		struct coord   button_curve;
		struct coord   button_linear;
		struct coord   button_single;
		struct coord   button_exit;

		struct coord   bandbutton;

	}window_eq;

	struct
	{
		int            min_width, min_height;
		int            window_mode /* 0 - round rect, 1 - custom */, window_edge;
		int            region_sx, region_sy, mask_buttons_sx, mask_buttons_sy;
		int            vis_l, vis_r, vis_t, vis_b;
		int            font_size;
		letter         font_name[128];
		letter         region_file_path[260], mask_buttons_file_path[260];
		int            button_close_align; /* 0 - top left, 1 - top right, 2 - bottom left, 3 - bottom right */
		int            button_max_align;

		struct coord   crop_tl;
		struct coord   crop_tm;
		struct coord   crop_tr;
		struct coord   crop_ml;
		struct coord   crop_mr;
		struct coord   crop_bl;
		struct coord   crop_bm;
		struct coord   crop_br;
		struct coord   button_close;
		struct coord   button_max;

	}window_vis;

	struct
	{
		int            min_width, min_height;
		int            window_mode /* 0 - round rect, 1 - custom */, window_edge;
		int            region_sx, region_sy, mask_buttons_sx, mask_buttons_sy;
		int            vid_l, vid_r, vid_t, vid_b;
		int            font_size;
		letter         font_name[128];
		letter         region_file_path[260], mask_buttons_file_path[260];
		int            button_close_align; /* 0 - top left, 1 - top right, 2 - bottom left, 3 - bottom right */
		int            button_max_align;

		struct coord   crop_tl;
		struct coord   crop_tm;
		struct coord   crop_tr;
		struct coord   crop_ml;
		struct coord   crop_mr;
		struct coord   crop_bl;
		struct coord   crop_bm;
		struct coord   crop_br;
		struct coord   button_close;
		struct coord   button_max;

	}window_vid;

	struct
	{
		int            min_width, min_height;
		int            window_mode /* 0 - round rect, 1 - custom */, window_edge;
		int            region_sx, region_sy, mask_buttons_sx, mask_buttons_sy;
		int            list_l, list_r, list_t, list_b;
		int            font_size;
		letter         font_name[128];
		letter         region_file_path[260], mask_buttons_file_path[260];
		int            button_close_align; /* 0 - top left, 1 - top right, 2 - bottom left, 3 - bottom right */
		int            button_medialib_align;
		int			   button_playlist_align;
		int			   button_add_align;
		int			   button_remove_align;
		int			   button_save_align;
		int			   button_options_align;
		int			   button_sort_align;



		struct coord   crop_tl;
		struct coord   crop_tm;
		struct coord   crop_tr;
		struct coord   crop_ml;
		struct coord   crop_mr;
		struct coord   crop_bl;
		struct coord   crop_bm;
		struct coord   crop_br;
		struct coord   button_close;
		struct coord   button_medialib_back;
		struct coord   button_medialib_switch;
		struct coord   button_playlist;
		struct coord   button_add;
		struct coord   button_remove;
		struct coord   button_save;
		struct coord   button_options;
		struct coord   button_sort;
		
	}window_ml;

}skin_coords;


struct pl_cache
{
	struct fennec_audiotag ft;
	unsigned long          pid;
	char                   sel;
	string                 dname;
	int                    album_points[64];
	unsigned long          album_points_album_of[64]; /* the n'th item is of the same alubm name */
};

extern struct pl_cache  *cached_tags;

extern local_skin_settings skin_settings;

#define  header_tag_title         0
#define  header_tag_album         1
#define  header_tag_artist        2
#define  header_tag_origartist    3
#define  header_tag_composer      4
#define  header_tag_lyricist      5
#define  header_tag_band          6
#define  header_tag_copyright     7
#define  header_tag_publish       8
#define  header_tag_encodedby     9
#define  header_tag_genre         10
#define  header_tag_year          11
#define  header_tag_url           12
#define  header_tag_offiartisturl 13
#define  header_tag_filepath      14
#define  header_tag_filename      15
#define  header_tag_comments      16
#define  header_tag_lyric         17
#define  header_tag_bpm           18
#define  header_tag_tracknum      19
#define  header_tag_index         20

#define  mm_down_l  0x1
#define  mm_down_r  0x2
#define  mm_down_m  0x3
#define  mm_up_l    0x4
#define  mm_up_r    0x5
#define  mm_up_m    0x6
#define  mm_dbl_l   0x7
#define  mm_dbl_r   0x8
#define  mm_wheel   0x9
#define  mm_move    0xa

#define equalizer_window_sx 0
#define equalizer_window_sy 62
#define equalizer_window_w  273
#define equalizer_window_h  54

int move_docking_window(int window_id, int x, int y);

#define window_id_main  0
#define window_id_eq    1
#define window_id_lib   2
#define window_id_vis   3
#define window_id_vid   4
#define window_id_count 5

#define wraprgb(v)    (v > 255 ? 255 : (v < 0 ? 0 : v))
#define RGBx(r, g, b) (RGB(wraprgb((r)), wraprgb((g)), wraprgb((b))))


#define skin_main_width  273
#define skin_main_height 62
#define skin_main_x      0
#define skin_main_y      0

int vis_refresh(void);
#define coords_map(x, y, w)( ( (y) * (w) ) + (x) )
#define wrap_color(x)      ( (x) < 0 ? 0 : ( (x) > 255 ? 255 : (x) ) )


void show_tip(int dtid, string tiptext);
void show_tipex(int dtid, int dtaid, int tid, string conj);


void          ml_cache_init(void);
void          ml_cache_clearsel(void);
int           ml_cache_issel(unsigned long id);
int           ml_cache_shiftsel(long s);
unsigned long ml_cache_getcurrent_item(void);
const string  ml_cache_getpath(unsigned long itemid);
unsigned long ml_cache_getlistindex(unsigned long itemid);
unsigned long ml_cache_getrealindex(unsigned long itemid);
int           ml_cache_switch(unsigned long itemid);
int           ml_cache_switchlist(unsigned long itemid, int mplay);
int           ml_cache_setsel(unsigned long id, char v);
void         *ml_cache_get(unsigned long id);
void          ml_cache_freetags(void);
void          ml_cache_uninit(void);
void          ml_cache_removesel(void);
unsigned long ml_cache_getcount(void);
void          ml_cache_exchange(unsigned long di, unsigned long si);

int           ml_pl_preview_display_timer(void);

int setwinpos_clip(HWND hwnd, HWND hwa, int x, int y, int w, int h, UINT flags);

void list_sort_column(int cid, int smode);

HMENU user_create_menu(int mid, int flags);

#define menu_ml_add      0x1
#define menu_ml_options  0x2
#define menu_ml_popup    0x3
#define menu_ml_remove   0x4
#define menu_ml_save     0x5
#define menu_ml_sort     0x6
#define menu_open        0x7

extern skin_coords  coords;

#define  cr(v)((int)((float)(v) * coords.zoom))

int incoord_vpos(int x, int y, struct coord *sc, int valign, int w, int h);
int incoord_vpos_nozoom(int x, int y, struct coord *sc, int valign, int w, int h);
void blt_coord_vpos_nozoom(HDC dc, HDC sdc, int state, struct coord *sc, int valign, int w, int h);
void scoord(struct coord *c, int x, int y, int w, int h, int sxn, int syn, int sxh, int syh, int sxd, int syd);
int incoordx(int x, int y, struct coord *sc);
void blt_coord(HDC dc, HDC sdc, int state, struct coord *sc);
void blt_coord_ew(HDC dc, HDC sdc, int state, struct coord *sc, int w);
void blt_coord_nozoom(HDC dc, HDC sdc, int state, struct coord *sc);
void fill_skin_coords(void);
void blt_button_on_coord_vb(HDC dc, HDC sdc, struct coord *bar, struct coord *button, float pos);
LRESULT CALLBACK callback_vis_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int skin_subskins_get(subskins_callback callfunc);
int skin_subskins_select(const string fname);
void skin_recreate(void);
int skin_move_window(HWND hwnd, int dx, int dy);
int skin_move_main_window(int dx, int dy);

void gr_init(graphic_context *gr);
void gr_delete(graphic_context *gr);
void gr_setcolor(graphic_context *gr, uint32_t pencolor, uint32_t brushcolor);
void gr_rect(graphic_context *gr, uint32_t color, int x, int y, int w, int h);
void gr_roundrect(graphic_context *gr, int cornor, uint32_t color, int x, int y, int w, int h);
void gr_circle(graphic_context *gr, uint32_t color, int x, int y, int w, int h);
void gr_line(graphic_context *gr, int size, uint32_t color, int x1, int y1, int x2, int y2);
void gr_text(graphic_context *gr, int mode, const string text, int x, int y, int w, int h);
void gr_setfont(graphic_context *gr, const string fontface, int size, int bold, int italic, int underlined, int extramode);
void gr_blit(graphic_context *grdst, int sx, int sy, int dx, int dy, int w, int h);
void gr_blitto(graphic_context *grsrc, int sx, int sy, int dx, int dy, int w, int h);
void gr_settextcolor(graphic_context *gr, uint32_t fcolor, uint32_t bkcolor, int bkmode);

void DrawCurve(HDC context, POINT *points, int npoints, float tension, int color);

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
