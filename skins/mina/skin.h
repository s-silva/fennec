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
#include "media.h"

#define incoord(x, y, sx, sy, sw, sh) (((x) > (sx)) && ((y) > (sy)) && ((x) < ((sx) + (sw))) && ((y) < ((sy) + (sh))))

extern struct skin_data  skin;
extern HINSTANCE         instance_skin;
extern HDC               hdc;
extern HWND              wnd;

extern int skin_color_hue;
extern int skin_color_sat;
extern int skin_color_light;

extern COLORREF text_color[2];

extern int media_init;
extern int vid_init;

extern HDC   hdc_vid;
extern HDC windowsheet_dc;
extern HWND  window_vid;

extern int png_h, png_w;

extern fn_vis_message  vis_message;
extern int             vis_used, vis_active;

extern int error_in_last_image;

extern int fullview_switching_manually;

int skin_subskins_get(subskins_callback);
int skin_subskins_select(const string);



HDC png_get_hdc(string fname, int color_apply);
HDC png_get_hdc_24(string fname);


void misc_time_to_string(int seconds, string buf);

void neo_display_update();


/* typography */

enum
{
	typo_song_title = 0,
	typo_song_album,
	typo_song_artist,
	typo_song_position,

	typo_count
};

enum
{
	mode_menu = 0,
	mode_display,
	mode_eq,
	

	mode_count
};

void mintro_init(HDC dc);
void mintro_uninit(void);
void mintro_draw(HDC dc, int w, int h);
void mintro_mousemsg(int x, int y, int action);

void typo_create_fonts(void);
void typo_print_shadow(HDC hdc, const string text, int x, int y, COLORREF color, int ifont);
void color_hsv_2_rgb_fullint(int h, int s, int v, unsigned char* r, unsigned char* g, unsigned char* b);
int set_color_32(unsigned char *buf);


void media_create(HWND hwndp);
void media_close(void);
int  media_refresh(void);

void vid_create(HWND hwndp, double aspect);
void set_power_timeouts(void);
void restore_power_timeouts(void);
void vid_close(void);
int vid_get_position(RECT *retp);
int vid_refresh(void);

struct coord
{
	int   x, y, w, h;
	int   sx_n, sy_n, sx_h, sy_h, sx_d, sy_d;
};

/* -------------------------------- */

void CALLBACK display_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK callback_vid_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK callback_searchbox(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HMENU user_create_menu(int mid, int flags);

void drawrect(HDC dc, int x, int y, int w, int h, COLORREF c);
void drawbar(HDC ddc, HDC sdc, int x, int y, int w, int h, int sxl, int syl, int swl, int sxm, int sym, int swm, int sxr, int syr, int swr, int alphap);
HDC jpeg_get(const string fname, int *w, int *h);
void alpha_blit(HDC ddc, HDC sdc, int x, int y, int w, int h, int sx, int sy, int alphap);
void alpha_blit_resize(HDC ddc, HDC sdc, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int alphap);


int  fullview_init(void);
int  fullview_drawgeneral(void);
int  fullview_uninit(void);
int  fullview_mousemsg(int x, int y, int action);
void playlist_view(HDC dc, int x, int y, int w, int h, int item);
void playlist_init(HDC dc);
void playlist_uninit(void);
void playlist_refresh(int lv);
int  playlist_mousemsg(int x, int y, int m);
void playlist_redraw(int item);
void fullview_render(int x, int y, int w, int h);
void fullview_clear(int x, int y, int w, int h);
int  fullview_keymsg(int key);
void fullview_refresh(int inum);

void dv_init(HDC dc);
void dv_uninit(void);
void dv_draw(HDC dc, int w, int h);
void dv_mousemsg(int x, int y, int action);
int  dv_keymsg(int key);
void dv_halt(void);
void dv_halt_resume(void);
void dv_set_sizes(void);


void lib_init(HDC dc);
void lib_uninit(void);
void lib_draw(HDC dc, int w, int h);
void lib_mousemsg(int x, int y, int action);
int  lib_keymsg(int key);
void lib_halt(void);
void lib_halt_resume(void);
void lib_set_sizes(void);

void fullview_switch(int vmode);

int  covers_init(void);
int  covers_uninit(void);
int  covers_addtoqueue(const string artist, const string album);
int  cover_download_discog(const string artist, const string album);
void draw_imagebox(HDC ddc, HDC sdc, int w, int h, struct coord *lt, struct coord *rt, struct coord *bl, struct coord *br, struct coord *t, struct coord *b, struct coord *l, struct coord *r);

void search_create(HWND hwndp);
void search_close(void);
int  search_refresh(void);


#define menu_ml_add      0x1
#define menu_ml_options  0x2
#define menu_ml_popup    0x3
#define menu_ml_remove   0x4
#define menu_ml_save     0x5
#define menu_ml_sort     0x6
#define menu_open        0x7
#define menu_ml_columns  0x8
#define menu_ml_settings 0x9
#define menu_ml_repeat   0xa
#define menu_ml_dv_popup 0xb

#define mid_repeat_list                 1
#define mid_repeat_track                2
#define mid_shuffle                     3
#define mid_autoswitching               4
#define mid_settings                    5
#define midc_title                      6
#define midc_album                      7
#define midc_artist                     8
#define midc_oartist                    9
#define midc_composer                   10
#define midc_lyricist                   11
#define midc_band                       12
#define midc_copyright                  13
#define midc_publisher                  14
#define midc_encodedby                  15
#define midc_genre                      16
#define midc_year                       17
#define midc_url                        18
#define midc_ourl                       19
#define midc_filepath                   20
#define midc_filename                   21
#define midc_bpm                        22
#define midc_track                      23
#define midc_index                      24
#define mid_title                       25
#define mid_sort_filename               26
#define mid_unsort                      27
#define mid_addfiles                    28
#define mid_add_dir                     29
#define mid_add_mlib                    30
#define mid_switch_sindex               31
#define menu_edittags                   32
#define mid_removesel                   33
#define mid_removeall                   34
#define mid_removeml                    35
#define mid_edittags                    36
#define mid_addtoplaylist               37
#define mid_loadpl                      38
#define mid_savepl                      39
#define mid_import                      40
#define mid_export                      41
#define menu_save                       42
#define menu_remove                     43
#define menu_options                    44
#define menu_sort                       45
#define mid_adddirs                     46
#define mid_tray                        47
#define mid_experimental                48
#define midc_font                       49
#define mid_preview						50
#define mid_preview_stop				51
#define mid_loadtracks                  52
#define mid_seldrive                    53
#define mid_settings_wallpaper_def      54
#define mid_settings_wallpaper_ld       55
#define mid_settings_wallpaper_sel      56
#define mid_settings_display_big        57
#define mid_settings_display_small      58
#define mid_dv_play                     59
#define mid_dv_add                      60
#define mid_settings_transparency_no    61
#define mid_settings_transparency_10    62
#define mid_settings_transparency_20    63
#define mid_settings_transparency_30    64
#define mid_settings_transparency_40    65
#define mid_settings_transparency_50    66
#define mid_settings_transparency_60    67
#define mid_settings_transparency_70    68
#define mid_settings_vis_showvideo      69
#define mid_settings_covers_download    70
#define mid_settings_covers_albums      71
#define mid_settings_covers_photos      72






/* media library/playlist stuff ---------------------------------------------*/

enum
{
	vmode_media_intro = 1,
	vmode_playlist,
	vmode_library,
	vmode_dv /* directory view */
};


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

double list_sort_score(string sv);
void list_sort_column(int cid, int smode);
void playlist_set_sizes(void);
int playlist_keymsg(int key);
void playlist_scroll(int sv);
void draw_medialibrary_message(HDC dc);

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

#define  playlist_display_big     0x1
#define  playlist_display_small   0x2

void          ml_cache_init(void);
void          ml_cache_clearsel(void);
int           ml_cache_issel(unsigned long id);
int           ml_cache_shiftsel(long s);
unsigned long ml_cache_getnext_item(void);
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
int           ml_cache_add_to_playlist(unsigned long itemid);


/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/
