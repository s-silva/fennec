/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
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

#ifndef header_main
#define header_main

#include <stdio.h>
#include <stdarg.h>
#include "systems/system.h"
#include "shared functions.h"
#include "../../data/system/windows/resource.h"
#include "../include/text ids.h"
#include "plugins.h"



typedef int (*subskins_callback)(string, string); /* file name, title */




/* structs ------------------------------------------------------------------*/

/*
 * skin data (out from fennec).
 */
struct skin_data
{
	/* <dep> win */

	WNDPROC        wproc;
	HWND           wnd;
	HINSTANCE      finstance;

	/* </dep> */


	int            (*showtip)   (int x, int y, int dtext, string ttext);
	int            (*showerror) (int eid, string etitle, string etext);
	struct fennec  *shared;
};



/*
 * skin data return (out from the plug-in).
 */
struct skin_data_return
{
	int        (*uninitialize) (int, void*);
	int        (*refresh)      (int, void*);
	int        (*setcolor)     (int, unsigned long);
	int        (*settheme)     (int);
	int        (*getthemes)    (int, string tname, int osize);
	int        (*getinfo)      (int, string tname, int osize);
	int        (*getdata)      (int dtid, void* odata, int osize);
	int        (*subs_get)     (subskins_callback  callfunc);
	int        (*subs_select)  (string fname);

	/* <dep> win */

	WNDPROC    callback;
	LONG       woptions;

	/* </dep> */
};


/*
 * equalizer bands.
 */
typedef struct
{
	struct
	{
		double x[5];
		double y[5];
	}bdata[32][5];

}eq_bands_data;




/* types --------------------------------------------------------------------*/


typedef unsigned long (__stdcall * fennec_skins_initialize)   (struct skin_data  *gin, struct skin_data_return  *gout);
typedef void          (*file_list_callback) (const string fname);


#include "trash.h"






/* globals variables --------------------------------------------------------*/

extern int              fennec_power_state;
extern struct           fennec fennec;
extern int              keys_globalids[256];
extern struct           skin_data fennec_skin_data;

#ifdef system_microsoft_windows

	extern WNDPROC      skinproc;
	extern HINSTANCE    instance_fennec;
	extern HWND         window_main;
	extern HWND         window_active_dialog;
	extern HDC          window_main_dc;
	extern HMENU        menu_main;

#endif





/* defines ------------------------------------------------------------------*/

#define fennec_v_refresh_force_not   0 /* call inquiring functions only */
#define fennec_v_refresh_force_less  1 /* call additional clean ups */
#define fennec_v_refresh_force_high  2 /* call additional refreshes i.e. redraw */
#define fennec_v_refresh_force_full  3 /* call additional re-initializations */

#define fennec_v_settings_ui_panel_general             0x0
#define fennec_v_settings_ui_panel_file_association    0x1
#define fennec_v_settings_ui_panel_skins               0x2
#define fennec_v_settings_ui_panel_themes              0x3
#define fennec_v_settings_ui_panel_languages           0x4
#define fennec_v_settings_ui_panel_formatting          0x5
#define fennec_v_settings_ui_panel_shortcutkeys        0x6
#define fennec_v_settings_ui_panel_internaloutput      0x7
#define fennec_v_settings_ui_panel_outputplugins       0x8
#define fennec_v_settings_ui_panel_inputplugins        0x9
#define fennec_v_settings_ui_panel_encoders            0xa
#define fennec_v_settings_ui_panel_dsp                 0xb
#define fennec_v_settings_ui_panel_visualizations      0xc

#define fennec_power_state_active    0x0
#define fennec_power_state_sleeping  0x1
#define fennec_power_state_minimized 0x2
#define fennec_power_state_busy      0x4
#define fennec_power_state_heavyuse  0x8 /* heavy usage */
#define fennec_power_state_idle      0x10
#define fennec_power_state_hidden    0x20

#define fennec_restore_default       0x1
#define fennec_restore_r_main        0x2
#define fennec_restore_r_skin        0x4
#define fennec_restore_r_plugin      0x8
#define fennec_restore_r_media       0x10

#define fennec_power_mode_exit       0x0  /* exit without saving */
#define fennec_power_mode_save       0x1  /* save settings and exit */
#define fennec_power_mode_sleep      0x2  /* sleep */
#define fennec_power_mode_default    0x3  /* user selected mode */



#define icon_library                 uni("icons.dll")

#define fennec_version_int           701003 /* 7.1.1 - (7 x 100,000) (1 x 1,000) + (0.1 x 10) */
#define fennec_version_string        "7.1.3"
#define fennec_version_text          "fennec player 7.1.3"
#define fennec_version_text_full     "Fennec Player 1.3 (Fennec 7.1.3) \"Rainwater Lake\" (Beta 1, 0210040200)"
#define fennec_version_information   "Beta Release 1"
#define fennec_player_version_text   "Fennec Player 1.3"

#define fennec_u_version_string        uni("7.1.3")
#define fennec_u_version_text          uni("fennec player 7.1.3")
#define fennec_u_version_text_full     uni("Fennec Player 1.3 (Fennec 7.1.3) \"Rainwater Lake\" (Beta 2, 0210040200)")
#define fennec_u_version_information   uni("Beta Release 1")
#define fennec_u_player_version_text   uni("Fennec Player 1.3")

#define get_playlist   0x1  /* get playlist window, 0 - not available; main window - same */
#define get_visual     0x2  /* get visualizations window, 0 - not available; main window - same */
#define get_visual_dc  0x3 
#define get_visual_x   0x4 
#define get_visual_y   0x5 
#define get_visual_w   0x6  
#define get_visual_h   0x7  
#define set_msg_proc   0x8
#define get_color      0x9
#define get_visual_winproc      0xa  
#define    get_window_playlist  0xb
#define    get_window_vis       0xc
#define    get_window_video     0xd
#define    get_window_eq        0xe
#define    get_window_ml        0xf
#define    get_window_misc      0x10
#define    get_window_options   0x11
#define    get_window_video_dc     0x13
#define    get_window_video_rect   0x14
#define    get_window_video_state  0x15
#define    set_video_window        0x16

#define rt_error       0x1
#define rt_warning     0x2
#define rt_stepping    0x4
#define rt_info        0x8
#define rt_suberror    0x10

#define wait_window_setcancel   0x1
#define wait_window_set_detail  0x2
#define wait_window_set_text    0x3
#define wait_window_end         0x4

#define file_dialog_openfiles          0x1
#define file_dialog_addfiles           0x2
#define file_dialog_opendir            0x3
#define file_dialog_adddir             0x4

#define file_dialog_mode_normal        0x0
#define file_dialog_mode_experimental  0x1

#define video_show_auto                0x0
#define video_show_manual              0x1
#define video_show_auto_displayed      0x2


/* declarations -------------------------------------------------------------*/

void           fennec_power_sleep(void);
void           fennec_power_wake (void);
int            fennec_refresh(int flevel);
unsigned short fennec_convertkey(void* ikey);
long           fennec_convertkeyrev(unsigned short fkey, long *mkey);
int            fennec_show_file_dialog(int id, int mode, string title, void *data);
int            playback_loadtracks(void);
void           refresh_others(void);

int            file_list_open_ex(const string fpaths, file_list_callback  fc);
int            file_list_open_dir(const string fpath, file_list_callback  fc);
int            path_make_co(string pstr);
int            path_make(const string pstr);

int playlist_t_save_current(const string fname);
int playlist_t_load_current(const string fname);
string playlist_t_get_extensions(int i);
int playlist_t_isplaylistfile(const string fname);

int   plugin_settings_fillstruct(struct plugin_settings *o);
int   plugin_settings_load      (const string fname);
int   plugin_settings_save      (const string fname);
int   plugin_settings_unload    (void);
char* plugin_settings_ret       (const char *splg, const char *sname, unsigned int *vzret /* value size */, unsigned int *azret /* absolute size */);
int   plugin_settings_get       (const char *splg, const char *sname, char *sval, unsigned int vlength);
int   plugin_settings_getnum    (const char *splg, const char *sname, int *idata, long *ldata, double *fdata);
int   plugin_settings_set       (const char *splg, const char *sname, const char *sval);
int   plugin_settings_setfloat  (const char *splg, const char *sname, double sval);
int   plugin_settings_setnum    (const char *splg, const char *sname, int sval);
int   plugin_settings_remove    (const char *splg, const char *sname);

int visualizations_initialize  (const string fname);
int visualizations_uninitialize();
int visualizations_checkfile   (const string fname);
int visualizations_refresh     (int rlevel);
int visualizations_settings    (void* fdata);
int visualizations_about       (void* fdata);

int text_command_base(const string tcommand);

int                      skins_initialize  (const string fname, struct skin_data *sdata);
int                      skins_uninitialize();
int                      skins_checkfile   (const string fname);
int                      skins_refresh     (int rlevel);
struct skin_data_return *skins_getdata     (void);
int                      skins_getthemes   (int tid, string tname);
int                      skins_settheme    (int tid);
int                      skins_setcolor    (int hue, int sat, int light);
int    skins_function_getdata(int id, void *rdata, int dsize);
int    skins_function_subskins_get(subskins_callback   callfunc);
int    skins_function_subskins_select(string fname);

int            output_plugin_initialize  (const string fname);
int            output_plugin_uninitialize();
int            output_plugin_checkfile   (const string fname);

int            external_output_about(void* vdata);
int            external_output_settings(void* vdata);
int            external_output_initialize(void);
int            external_output_uninitialize(void);
int            external_output_load(const string spath);
int            external_output_add(unsigned long idx, const string spath);
int            external_output_play(void);
int            external_output_playpause(void);
int            external_output_pause(void);
int            external_output_stop(void);
int            external_output_setposition(double cpos);
int            external_output_setposition_ms(double mpos);
int            external_output_getplayerstate(void);
double         external_output_getposition(void);
double         external_output_getposition_ms(void);
double         external_output_getduration_ms(void);
int            external_output_getpeakpower(unsigned long* pleft, unsigned long* pright);
int            external_output_setvolume(double vl, double vr);
int            external_output_getvolume(double* vl, double* vr);
int            external_output_getcurrentbuffer(void* dout, unsigned long* dsize);
int            external_output_getfloatbuffer(float* dout, unsigned long scount, unsigned long channel);
int            external_output_getdata(unsigned int di, void* ret);
int            external_output_preview_action(int actionid, void *adata);

int shared_functions_fill(unsigned long fmask, struct fennec *o);

int          fileassociation_restore(const string ext);
int          fileassociation_set(const string ext, const string acname, const string lacname, const string icpath, const string accmd, const string dsc, int icidx);
int          fileassociation_selected(const string ext);
unsigned int fileassociation_geticonid(const string ext);

int settings_ui_show(void);
int settings_ui_show_ex(unsigned int pid);


string fennec_get_path(string out, size_t length);
string fennec_file_dialog(short issave, const string title, const string sfilter, const string initdir);
string fennec_file_dialog_ex(short issave, const string title, const string sfilter, const string initdir);
void   fennec_get_plugings_path(string mem);
int    fennec_get_rel_path(string rpath, const string apath);
int    fennec_get_abs_path(string apath, const string rpath);
int    fennec_register_contextmenu(int regc);
int    __stdcall fennec_control_function(unsigned long id, void *indata, void *outdata);
int    skins_apply(const string rskin_path);


int   tips_create(void);
int   tips_display(int x, int y, string msg);

int   fennec_power_set(unsigned long pmode);
void  fennec_power_exit(void);
void  fennec_power_wake(void);
void  fennec_power_sleep(void);

void  color_hsv_2_rgb_fullint(unsigned short h, unsigned char s, unsigned char v, unsigned char* r, unsigned char* g, unsigned char* b);
int   setting_priority_to_sys(int sp);


int    basewindows_show_tagging(int wmodal, const string tfile);
string basewindows_getuserinput(const string title, const string cap, const string dtxt);
string basewindows_show_open(const string cap, const string idir, int dcus);
int    basewindows_show_license(void);
int    basewindows_show_about(int wmodal);
int    basewindows_show_presets(void* wmodal);
int    basewindows_show_addfolder(void);
int    basewindows_wait_function(int id, int data, void *ptr);
int    basewindows_show_wait(void* wmodal);
int    basewindows_selectformat(int *samplerate, int *channels, int *bps);
int    basewindows_show_conversion(int wmodal);
int    basewindows_show_ripping(int wmodal);
int    basewindows_show_restore_settings(void* wmodal);

int video_refresh(HWND hwnd, HDC dc, RECT *crect);
int video_initialize(void);
int video_uninitialize(void);

int    transcode_settings_show(HWND hwndp, transcoder_settings *tsettings);

void main_refresh(void);
string fennec_loadtext(const string rtype, const string rname);

int lang_uninitialize(void);
int lang_load(string fname);
int lang_check(string fname, string tlang, string ttitle, string tauthor, string tcomments);

/* error/info reporting */

int  reporting_start(void);
int  reporting_end(void);
void report(char *msg, int etype, ...);
void reportu(string msg, int etype, ...);
void reportx(char *msg, int etype, ...);

int call_function(int id, int data, void *a, void *b);
int inform_other_programs(string str);

/* audio convert */

typedef int (*audioconvert_file_pfunc) (double);

int audioconvert_convertfile(const string infile, string outfile, unsigned long eid, unsigned int bsize, int* cancelop, int* pauseop, audioconvert_file_pfunc cfunc);
int audioconvert_convertfile_ex(const string infile, string outfile, unsigned long eid, unsigned int bsize, int* cancelop, int* pauseop, audioconvert_file_pfunc cfunc, transcoder_settings *trans);

int audiojoining_start(const string infile, string outfile, unsigned long eid, unsigned int bsize, int* cancelop, int* pauseop, double spos, double epos, audioconvert_file_pfunc cfunc);
int audiojoining_push(int hjoin /* not used */, const string fpath, double spos, double epos);
int audiojoining_end(int hjoin /* not used */);

#endif

/*-----------------------------------------------------------------------------
 fennec.
-----------------------------------------------------------------------------*/
