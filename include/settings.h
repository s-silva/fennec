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


#ifndef header_fennec_settings
#define header_fennec_settings

#ifndef header_system
#	include "system.h"
#endif


/* defines ------------------------------------------------------------------*/

#define setting_id_input_plugin       0x1
#define setting_id_equalizer_preset   0x2

#define setting_window_minimized      0x1
#define setting_window_normal         0x2
#define setting_window_docked         0x4
#define setting_window_hidden         0x8
#define setting_window_hidden_docked  0x9

#define setting_priority_idle         0x1
#define setting_priority_lowest       0x2
#define setting_priority_low          0x3
#define setting_priority_below_normal 0x4
#define setting_priority_normal       0x5
#define setting_priority_above_normal 0x6
#define setting_priority_high         0x7
#define setting_priority_highest      0x8
#define setting_priority_realtime     0x9




/* structs ------------------------------------------------------------------*/

struct dsp_plugin
{
	letter path[v_sys_maxpath];                  /* file path */
	letter name[128];                            /* plug-in title */
	char   index;                                /* plug-in index (order) */
};


struct internal_input_plugin
{
	unsigned short   extensions_count;         /* extensions in 'extensions' array */
	letter           extensions[200][16];       /* extensions, null terminated data without leading period */
	letter           typedescription[200][128]; /* file type description for extensions */
	letter           pluginname[128];          /* plugin name */
	letter           fname[1024];              /* plugin file name */
	int              plugin_type;              /* 0 - default, 1 - check-out */
};


struct setting_key
{
	unsigned short  kaction;                   /* action */
	unsigned short  kcomb;                     /* key combination */
};

typedef struct _equalizer_preset
{
	letter    name[30];
	float     preamp[16];
	float     boost[16][32]; /* 32 bands x 16 channels */
	int32_t   center[32];
	int32_t   Q[32]; 
	int16_t   parametric;    /* enable 'Q' and 'center' */
}equalizer_preset;

typedef struct _transcoder_settings
{
	struct
	{
		unsigned long      buffersize;
	}general;
	
	struct
	{
		unsigned char      enable_eq;
		unsigned long      current_eq_preset;
		equalizer_preset   eq;
	}eq;
	
	struct
	{
		unsigned char      enable_vol;
		double             vol;
		double             gain;
	}volume;
	
	struct
	{
		unsigned char      enable_tagging;
		unsigned char      e_title;
		unsigned char      e_artist;
		unsigned char      e_album;
		unsigned char      e_year;
		unsigned char      e_genre;
		unsigned char      e_comments;
		letter             d_title[1024];
		letter             d_artist[1024];
		letter             d_album[1024];
		letter             d_year[32];
		letter             d_genre[1024];
		letter             d_comments[2048];
	}tagging;

	struct
	{
		char               date_and_time_mode; /* 0 - original, 1 - current, 2 - custom */
		short              custom_year;
		unsigned char      custom_month;
		unsigned char      custom_date;
		unsigned char      custom_hour;
		unsigned char      custom_minute;
	}misc;

}transcoder_settings;



typedef struct settings_struct
{
	unsigned char  power_mode;

	struct
	{
		unsigned char    save_apply;

	}settings;


	struct
	{
		letter           last_file[v_sys_maxpath];

		unsigned char    equalizer_enable;
		unsigned short   equalizer_presets;
		equalizer_preset equalizer_last;
		long             equalizer_last_preset_id;

		unsigned char    playlist_shuffle;
		unsigned char    playlist_shuffle_rate;
		unsigned char    playlist_repeat_list;
		unsigned char    playlist_repeat_single;
		unsigned char    auto_switching;

		unsigned short   themecolor_h;
		unsigned short   themecolor_s;
		unsigned short   themecolor_l;

		double           volume[16];
		
		unsigned long    selected_drive;

		int              last_raw_samplerate;
		int              last_raw_channels;
		int              last_raw_bps;

		int              video_tested;
		int              video_window_manual_show;
	}player;


	struct
	{
		unsigned long   buffer_memory;
		unsigned long   output_device_id;
		unsigned long   bit_depth;
		unsigned long   bit_depth_float;
		unsigned long   bit_depth_signed;
		unsigned long   noise_reduction;
		unsigned long   perform_effects;

	}audio_output;


	struct
	{
		unsigned long  main_window_x;
		unsigned long  main_window_y;
		unsigned char  main_window_state; /* minimized etc. */
		unsigned long  advanced_open_x;
		unsigned long  advanced_open_y;
		unsigned long  advanced_open_w;
		unsigned long  advanced_open_h;

	}environment;


	struct
	{
		unsigned char  allow_multi_instances;
		unsigned char  auto_play;
		unsigned char  scroll_title_taskbar;
		unsigned char  show_splash;
		unsigned char  always_on_top;
		unsigned long  base_priority;
		unsigned long  threads_priority;
		unsigned char  use_advanced_open;
	
	}general;


	struct
	{
		letter         selected[v_sys_maxpath]; /* "" - base skin */
	
	}skins;


	struct
	{
		letter         main_title      [260];
		letter         scrolling_title [260];
		letter         playlist_item   [260];

	}formatting;

	struct
	{
		unsigned char       enable_local;
		unsigned char       enable_global;
		struct setting_key  localkeys [256];
		struct setting_key  globalkeys[256];

	}shortcuts;


	struct
	{
		unsigned char  refresh_at_startup;
		unsigned char  search_sub_folders;

	}internal_input;


	struct
	{
		letter         selected[v_sys_maxpath]; /* "" - Internal output */
	}output_plugins;

	struct
	{
		letter         selected[v_sys_maxpath]; /* "" - for nothing */
	}visualizations;

	
	struct
	{
		letter         selected[v_sys_maxpath]; /* "" - for nothing */
	}videooutput;

	struct
	{
		letter         rename_formatting[260]; /* current tag rename formatting */
		unsigned char  checked_tags[32];
	}tag_editing;

	struct
	{
		char                  use_equalizer;                            /* [to be removed] */
		letter                last_path[v_sys_maxpath];
		letter                last_formatting[260];
		unsigned long         last_buffer_size;                         /* [to be removed] */
		unsigned long         last_encoder;
		double                volume;                                   /* [to be removed] */
		double                volume_gain; /* only if volume == 1.0 */  /* [to be removed] */
		equalizer_preset      equalizer_bands;                          /* [to be removed] */
		unsigned short        last_equalizer_preset_id;                 /* [to be removed] */
		unsigned char         volume_normalization;                     /* [to be removed] */
		unsigned char         stop_playback;
		transcoder_settings   trans;
		WINDOWPLACEMENT       winplace;
	}conversion;

	struct
	{
		char                  use_equalizer;                            /* [to be removed] */
		letter                last_path[v_sys_maxpath];
		letter                last_formatting[260];
		unsigned long         last_buffer_size;                         /* [to be removed] */
		unsigned long         last_encoder;
		double                volume;                                   /* [to be removed] */
		double                volume_gain; /* only if volume == 1.0 */  /* [to be removed] */
		equalizer_preset      equalizer_bands;                          /* [to be removed] */
		unsigned short        last_equalizer_preset_id;                 /* [to be removed] */
		unsigned char         volume_normalization;                     /* [to be removed] */
		unsigned char         stop_playback;
		transcoder_settings   trans;
		WINDOWPLACEMENT       winplace;
	}ripping;

	struct
	{
		char              use_equalizer;
		letter            last_path[v_sys_maxpath];
		letter            last_load[v_sys_maxpath];
		unsigned long     last_buffer_size;
		unsigned long     last_encoder;
		double            volume;
		double            volume_gain; /* only if volume == 1.0 */
		equalizer_preset  equalizer_bands;
		unsigned short    last_equalizer_preset_id;
		unsigned char     volume_normalization;
		unsigned char     stop_playback;
	}joining;

	struct
	{
		struct dsp_plugin plugins[16];
		unsigned long     plugins_count;
		unsigned char     enable;
		unsigned char     enablenextstartup;
	}dsp_plugins;

	struct
	{
		unsigned long  input_plugins_count;
		letter         input_path[v_sys_maxpath];
	}plugins;

	/* data values for additional (variable size) data */

	struct
	{
		unsigned long appending_start;
		unsigned long appending_end;
		unsigned long appending_size;

		unsigned long input_plugins_start;
		unsigned long input_plugins_size;

		/* equalizer presets */
		unsigned long equalizer_presets_start;
		unsigned long equalizer_presets_size;
	}appending_values;

	struct
	{
		letter        pack_path[v_sys_maxpath];
	}language_packs;

}settings_struct;



/* prototypes ---------------------------------------------------------------*/

int settings_load(void);
int settings_save(void);
int settings_default(void);
int settings_uninitialize(void);
int settings_data_get(unsigned long sid,unsigned long iid,void* val,unsigned long* vsz);
int settings_data_add(unsigned long sid,void* val,unsigned long vsz);
int settings_data_set(unsigned long sid,unsigned long iid,void * val);
int settings_data_remove(unsigned long sid,unsigned long iid);
int settings_data_clean(unsigned long sid);
int settings_verify(void);
int settings_set_as_bad(void);
int settings_set_as_fine(void);

/* external -----------------------------------------------------------------*/

extern settings_struct settings;




#endif /* header_fennec_settings */

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
