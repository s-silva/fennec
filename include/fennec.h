/*-----------------------------------------------------------------------------
  fennec header file for plug-ins.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#ifndef header_fennec
#define header_fennec


#ifndef header_fennec_settings
#	include "settings.h"
#endif

#ifndef header_system
#	include "system.h"
#endif

typedef unsigned long  dword;
typedef void*          t_sys_library_handle;
typedef int (*subskins_callback)(string, string); /* file name, title */



/* plugins ------------------------------------------------------------------*/



#define    callc __stdcall                       /* call convention */





/* general plugins */

#define    plugin_version                 1      /* plugin version (1.0.0) */
#define    plugin_player_base             5      /* startup player count */
#define    plugin_player_add              5      /* player resize */
#define    plugin_player_max              1000   /* maximum players count */

#define    fennec_plugintype_audiodecoder 0x1    /* audio decoder plugin */
#define    fennec_plugintype_audioencoder 0x2    /* audio encoder plugin */   
#define    fennec_plugintype_audiooutput  0x4    /* audio output plugin */
#define    fennec_plugintype_audiodsp     0x8    /* DSP plugin */
#define    fennec_plugintype_audiovisual  0x10   /* visualization plugin */
#define    fennec_plugintype_general      0x20   /* general plugin */





/* input (decoder) plugins */

#define    fennec_input_open_file             1  /* open local file */
#define    fennec_input_open_internet_stream  2  /* open internet stream */

#define    fennec_input_format_base           1  /* get format from 'BaseFormat' */
#define    fennec_input_info_base             1  /* get information from 'BaseInfo' */
#define    fennec_input_status_base           1  /* get status from 'BaseStatus' */

#define    fennec_input_status_loading        4  /* loading file */
#define    fennec_input_status_buffering      5  /* buffering */
#define    fennec_input_status_connecting     6  /* connecting to the server */
#define    fennec_input_status_decoding       7  /* decoding */
#define    fennec_input_status_busy           8  /* busy (state unspecified) */
#define    fennec_input_status_sleeping       9  /* doing nothing */

#define    fennec_input_position_32bytes      0  /* dword, byte count */
#define    fennec_input_position_64bytes      1  /* 64 bit int, byte count */
#define    fennec_input_position_32ms         2  /* dword, miliseconds */
#define    fennec_input_position_64ms         3  /* 64 bit int, miliseconds */
#define    fennec_input_position_seconds      4  /* dword, seconds */
#define    fennec_input_position_frames       5  /* dword, frames */
#define    fennec_input_position_percent      6  /* byte, percent */
#define    fennec_input_position_fraction     7  /* double */

#define    fennec_input_initialized           1  /* no errors */
#define    fennec_input_invalidversion        2  /* version not supported */
#define    fennec_input_unknownversion        3  /* unknown or new version */
#define    fennec_input_unknownerror          4  /* unknown error */
#define    fennec_input_invalidtype           5  /* I'm not the one you thought ! */
#define    fennec_input_nodata                10 /* end of the file/stream */

#define    fennec_input_openfault     0xFFFFFFFF /* fault in "open file", otherwise returns the file id */
#define    fennec_input_invalidfile   0xFFFFFFFE /* invalid file */

#define    fennec_information_basic           1  /* basic information - 'initialiaztion_info' */
#define    fennec_invalid_index              -1  /* invalid index */





/* encoder plugins */

#define    fennec_encoder_file               0x1 /* encode directly to a file */
#define    fennec_encoder_encode             0x2 /* encode to a buffer */

#define    fennec_encode_tellmode_pointer    0x0
#define    fennec_encode_tellmode_end        0x2

#define    fennec_encode_seekmode_begin      0x0
#define    fennec_encode_seekmode_pointer    0x1
#define    fennec_encode_seekmode_end        0x2

#define    fennec_encode_openmode_create     0x0 /* just create and write from the beginning */
#define    fennec_encode_openmode_createnew  0x1 /* if existing, return zero */
#define    fennec_encode_openmode_append     0x2




/* misc */

#define    score_min      0
#define    score_max      10
#define    score_def      5
#define    get_samples    0x1  /* cid - channel, mdata - samples */
#define    get_fft        0x2  /* cid - channel, mdata - real, sdata - imaginary */
#define    get_fft_mod    0x3  /* cid - channel, mdata - fft */
#define    get_fft_vis    0x4  /* cid - channel, mdata - real, sdata - imaginary */
#define    get_fft_mvis   0x5  /* cid - channel, mdata - fft */
#define    get_beats      0x6  /* cid - channel, return - peak power */
#define    get_window     0x7  /* cid - channel, return - peak power */
#define    get_dc         0x8  /* cid - channel, return - peak power */
#define    get_x          0x9  /* get x coordinates, return - x */
#define    get_y          0xa  /* get x coordinates, return - y */
#define    get_w          0xb  /* get width, return - width */
#define    get_h          0xc  /* get height, return - height */
#define    set_default    0xd  /* set default plugin/remove itself */

#define    get_playlist   0x1  /* get playlist window, 0 - not available; main window - same */
#define    get_visual     0x2  /* get visualizations window, 0 - not available; main window - same */
#define    get_visual_dc  0x3 
#define    get_visual_x   0x4 
#define    get_visual_y   0x5 
#define    get_visual_w   0x6  
#define    get_visual_h   0x7  
#define    set_msg_proc   0x8
#define    get_color      0x9  /* dsize - index, rdata - sub index */
#define    get_visual_winproc   0xa  
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

#define    color_normal   0x1
#define    color_light    0x2
#define    color_dark     0x3
#define    color_shifted  0x4

#define    max_channels   16

#define    msg_keys       1
#define    msg_mousemove  2
#define    msg_leftdown   3
#define    msg_leftup     4
#define    msg_rightdown  5
#define    msg_rightup    6
#define    msg_wheel      7

#define    panel_general          0
#define    panel_files            1
#define    panel_skins            2
#define    panel_themes           3
#define    panel_languages        4
#define    panel_formatting       5
#define    panel_shortcuts        6
#define    panel_internalout      7
#define    panel_decoders         8
#define    panel_outputplugins    9
#define    panel_dsp              10
#define    panel_encoders         11
#define    panel_visualizations   12


/* audio preview actions */

#define    audio_preview_play      0x1
#define    audio_preview_pause     0x2
#define    audio_preview_stop      0x3
#define    audio_preview_load      0x4
#define    audio_preview_seek      0x5
#define    audio_preview_volume    0x6
#define    audio_preview_getpos    0x7


/* imaginary/built-in directories */

#define media_library_dir_root     0x1 /* root (where you find other dirs like genres...) */
#define media_library_dir_all      0x2 /* all music */
#define media_library_dir_rating   0x3

/* real directories */

#define media_library_dir_artists  0xa
#define media_library_dir_albums   0xb
#define media_library_dir_genres   0xc
#define media_library_dir_years    0xd




/* calling addresses */

#define vis_function_initialize         "fennec_visualization_initialize"
#define out_function_initialize         "fennec_initialize_output"
#define skin_function_initialize        "fennec_skin_initialize"
#define encoder_function_initialize     "fennec_initialize_encoder"
#define input_function_initialize       "fennec_initialize_input"
#define dsp_function_initialize         "fennec_initialize_dsp"
#define videoout_function_initialize    "fennec_videoout_initialize"
#define videoeffect_function_initialize "fennec_videoeffect_initialize"





/* tags */


#define    tag_memmode_static  0 /* don't need to be deallocated */
#define    tag_memmode_dynamic 1 /* 'free' to deallocate */




/* player state */

#define v_audio_playerstate_null                0 /* error */
#define v_audio_playerstate_notinit             1 /* player ain't initialized */
#define v_audio_playerstate_init                2 /* initialized */
#define v_audio_playerstate_loaded              3 /* loaded */
#define v_audio_playerstate_buffering           4 /* writing buffer (not playing) */
#define v_audio_playerstate_stopped             5 /* stopped */
#define v_audio_playerstate_paused              6 /* paused */
#define v_audio_playerstate_opening             7 /* opening (or buffering for the first time) */
#define v_audio_playerstate_playing             8 /* playing */
#define v_audio_playerstate_playingandbuffering 9 /* buffering while playing */





/* equalizer values */

#define v_audio_equalizer_default               12345.0





/* function masks */

#define v_shared_function_mask_audio_input      0x2
#define v_shared_function_mask_audio_output     0x4
#define v_shared_function_mask_general          0x8
#define v_shared_function_mask_simple           0x10
#define v_shared_function_mask_settings         0x20
#define v_shared_function_mask_media_library    0x40
#define v_shared_function_mask_video            0x80




/* refresh values */

#define v_fennec_refresh_force_not  0 /* call inquiring functions only */
#define v_fennec_refresh_force_less 1 /* call additional clean ups */
#define v_fennec_refresh_force_high 2 /* call additional refreshes i.e. redraw */
#define v_fennec_refresh_force_full 3 /* call additional re-initializations */




/* get data modes */

#define audio_v_data_frequency                  1 /* [int] current frequency */
#define audio_v_data_bitspersample              2 /* [int] */
#define audio_v_data_channels                   3 /* [int] */
#define audio_v_data_bitrate                    4 /* [int] */
#define audio_v_data_ids_pid                    5 /* return */
#define audio_v_data_ids_fid                    6 /* return */
#define audio_v_data_position_for_video         7 /* [double*] in seconds */




/* ids to get information from the decoder */

#define video_set_buffersize                  0x1 /* audio buffer size for sync */
#define video_set_subfile                     0x2
#define video_get_subfile                     0x3
#define video_set_sub_language                0x4
#define video_get_sub_language_current        0x5
#define video_get_sub_language_count          0x6
#define video_set_video_stream                0x7
#define video_get_video_stream_current        0x8
#define video_get_video_stream_count          0x9
#define video_set_audio_stream                0xa
#define video_get_audio_stream_current        0xb
#define video_get_audio_stream_count          0xc
#define video_set_movie_chapter               0xd
#define video_get_movie_chapter_current       0xe
#define video_get_movie_chapter_count         0xf
#define video_set_movie_title                 0x10
#define video_get_movie_title_current         0x11
#define video_get_movie_title_count           0x12
#define video_set_movie_angle                 0x13
#define video_get_movie_angle_current         0x14
#define video_get_movie_angle_count           0x15
#define video_set_movie_navigation            0x16
#define video_get_movie_navigation_current    0x17
#define video_get_movie_navigation_count      0x18





/* types */

typedef double fennec_sample;
typedef int   (callc *encode_realloc) (void* data, int nsize);
typedef void* (callc *func_realloc)   (void *imem, unsigned int rsize);






/* structs ------------------------------------------------------------------*/

struct plugin_settings
{
	char* (*plugin_settings_ret     )(const char *splg, const char *sname, unsigned int *vzret /* value size */, unsigned int *azret /* absolute size */);
	int   (*plugin_settings_get     )(const char *splg, const char *sname, char *sval, unsigned int vlength);
	int   (*plugin_settings_getnum  )(const char *splg, const char *sname, int *idata, long *ldata, double *fdata);
	int   (*plugin_settings_set     )(const char *splg, const char *sname, const char *sval);
	int   (*plugin_settings_setfloat)(const char *splg, const char *sname, double sval);
	int   (*plugin_settings_setnum  )(const char *splg, const char *sname, int sval);
	int   (*plugin_settings_remove  )(const char *splg, const char *sname);	
};








/*
 * tag item.
 */
struct fennec_audiotag_item
{
	unsigned int   tsize;     /* tag size (zero: no tag) */
	unsigned short tmode;     /* memory allocation mode */
	string         tdata;     /* data */
	unsigned int   tdatai;    /* numeric data */
	unsigned int   treserved; /* reserved */
};




/*
 * tags collection.
 */
struct fennec_audiotag
{
	struct fennec_audiotag_item    tag_title;          /* title */
	struct fennec_audiotag_item    tag_album;          /* album */
	struct fennec_audiotag_item    tag_artist;         /* artist */
	struct fennec_audiotag_item    tag_origartist;     /* original artist */
	struct fennec_audiotag_item    tag_composer;       /* composer */
	struct fennec_audiotag_item    tag_lyricist;       /* lyricist */
	struct fennec_audiotag_item    tag_band;           /* band/orchestra */
	struct fennec_audiotag_item    tag_copyright;      /* copyright */
	struct fennec_audiotag_item    tag_publish;        /* publisher */
	struct fennec_audiotag_item    tag_encodedby;      /* encoded by */
	struct fennec_audiotag_item    tag_genre;          /* genre */
	struct fennec_audiotag_item    tag_year;           /* year/date */
	struct fennec_audiotag_item    tag_url;            /* artist/media URL */
	struct fennec_audiotag_item    tag_offiartisturl;  /* official artist URL */
	struct fennec_audiotag_item    tag_filepath;       /* file path (not required) */
	struct fennec_audiotag_item    tag_filename;       /* file name (not required) */
	struct fennec_audiotag_item    tag_comments;       /* comments */
	struct fennec_audiotag_item    tag_lyric;          /* lyrics */
	struct fennec_audiotag_item    tag_bpm;            /* beats per minute */
	struct fennec_audiotag_item    tag_tracknum;       /* track number/disc number (indexing) */
	struct fennec_audiotag_item    tag_image;          /* album art/artist(s)...  */


	unsigned int                   tid;                /* index of the tags */
	unsigned int                   treserved;
};





/*
 * shared functions.
 */
struct fennec
{
	struct /* audio */
	{
		struct /* output */
		{

			int     (*initialize)(void);
			int     (*uninitialize)(void);
			int     (*load)(const string spath);
			int     (*add)(dword idx, const string spath);
			int     (*play)(void);
			int     (*pause)(void);
			int     (*stop)(void);
			int     (*setposition)(double cpos);
			int     (*setposition_ms)(double mpos);
			int     (*getplayerstate)(void);
			double  (*getposition)(double* cpos);
			double  (*getposition_ms)(void);
			double  (*getduration_ms)(void);
			int     (*getpeakpower)(dword* pleft, dword* pright);
			int     (*setvolume)(double vl, double vr);
			int     (*getvolume)(double* vl, double* vr);
			int     (*getcurrentbuffer)(void* dout, dword* dsize);
			int     (*getfloatbuffer)(float* dout, dword scount, dword channel);
			int     (*getdata)(unsigned int di, void* ret);

			struct /* playlist */
			{
				int          (*switch_list)(dword idx);
				int          (*clear)(void);
				int          (*add)(string fname, int checkfile, int gettags);
				int          (*move)(dword idd, dword ids, int mmode);
				int          (*setshuffle)(int shuffle, int doaction);
				int          (*shuffle)(int entirelist);
				int          (*unshuffle)(void);
				int          (*remove)(dword idx);
				int          (*mark)(dword idx);
				int          (*unmark)(dword idx);
				int          (*ismarked)(dword idx);
				int          (*getinfo)(dword idx, string sname, string sartist, string salbum, string sgenre, dword* durationms, dword* fsize);
				int          (*next)(void);
				int          (*previous)(void);
				int          (*switchindex)(dword idx);
				dword        (*getcurrentindex)(void);
				dword        (*getrealindex)(dword idx);
				dword        (*getlistindex)(dword ridx);
				dword        (*getcount)(void);
				const string (*getsource)(dword idx);
			}playlist;

		}output;

		struct /* input */
		{

			struct /* plugins */
			{
				int    (*initialize)(dword pid);
				int    (*uninitialize)(dword pid, int force);
				int    (*mark_usage)(dword pid, int isused);
				int    (*loadfile)(dword pid, const string sfile, dword* fid);
				int    (*getformat)(dword pid, dword fid, dword* freq, dword* bps, dword* nchannels);
				int    (*readdata)(dword pid, dword fid, dword* dsize, void* odata);
				dword  (*getduration_ms)(dword pid, dword fid);
				int    (*unloadfile)(dword pid, dword fid);
				int    (*setposition)(dword pid, dword fid, double spos);

			}plugins;

			int                    (*initialize)(void);
			int                    (*uninitialize)(void);
			int                    (*selectinput)(const string fname);
			int                    (*getextensionsinfo)(dword id, string ext, string info);
			int                    (*tagread_known)(unsigned long pid, const string fname, struct fennec_audiotag* rtag);
			int                    (*tagwrite)(const string fname, struct fennec_audiotag* rtag);
			unsigned long          (*tagread)(const string fname, struct fennec_audiotag* rtag);
			t_sys_library_handle   (*gethandle)(const string fname);

		}input;

		struct
		{
			int   (*set_bands)(int bcount, int channelid, float *dvalues);
			int   (*get_bands)(int bcount, int channelid, float *dvalues);
			int   (*set_preamp)(int channelid, float dvalue);
			float (*get_preamp)(int channelid);
			int   (*reset)(void);
			int   (*switch_current)(int bcount);
			int   (*equalize)(void *outbuf, const void *inbuf, int nchan, int freq, int bpsample, unsigned int dlength);

		}equalizer;

		struct
		{
			int   (*initialize)(void);
			int   (*reinitialize)(void); /* initialize recently added plugins only */
			int   (*uninitialize)(void);
			int   (*initialize_index)(unsigned long pid, unsigned long id);
			int   (*uninitialize_index)(unsigned long pid, unsigned long id);
			void* (*process)(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize);
			int   (*showabout)(unsigned int pid, void* pdata);
			int   (*showconfig)(unsigned int pid, void* pdata);

		}dsp;

	}audio;

	struct /* general */
	{
		int (*show_settings)(void *indata, void *outdata, unsigned int osize);
		int (*show_tageditor)(void *indata, void *outdata, unsigned int osize);
		int (*show_about)(void *indata, void *outdata, unsigned int osize);
		int (*show_credits)(void *indata, void *outdata, unsigned int osize);
		int (*show_license)(void *indata, void *outdata, unsigned int osize);
		int (*show_conversion)(void *indata, void *outdata, unsigned int osize);
		int (*show_ripping)(void *indata, void *outdata, unsigned int osize);
		int (*show_joining)(void *indata, void *outdata, unsigned int osize);
		int (*show_error)(int etype, const char *text);

		int (*getfennecpath)(string fpath, unsigned int psize);
		int (*getpluginspath)(string fpath, unsigned int psize);
		int (*getskinspath)(string fpath, unsigned int psize);

		int (*sendcommand)(const string tcmd);
	}general;

	struct
	{
		int (*show_openfile)(void);
		int (*show_addfile)(void);
		int (*show_save_playlist)(void);
		int (*show_addfolder)(void);
		int (*show_equalizer_presets)(void* wmodal);
		int (*list_fastsort)(void);
		int (*list_unsort)(void);

	}simple;

	struct
	{
		int (*load)(void);
		int (*save)(void);
		int (*reset)(void);
		
		struct
		{
			int (*add)(dword sid, void *val, dword vsz);
			int (*set)(dword sid, dword iid, void *val);
			int (*get)(dword sid, dword iid, void *val, dword *vsz);
			int (*remove)(dword sid, dword iid);
			int (*clean)(dword sid);

		}additional;
		
		settings_struct *general;
		
	}settings;

	string  *language_text;

	struct
	{
		int   (*media_library_initialize)(void);
		int   (*media_library_uninitialize)(void);
		int   (*media_library_add_file)(const string spath);
		int   (*media_library_save)(const string spath);
		int   (*media_library_load)(const string spath);
		void* (*media_library_get)(unsigned long id);
		int   (*media_library_translate)(unsigned long id, struct fennec_audiotag *at);
		int   (*media_library_get_dir_names)(unsigned long id, unsigned long pdir, string *dname);
		int   (*media_library_get_dir_files)(unsigned long id, unsigned long pdir, string cdir, uint32_t *tid);
		int   (*media_library_add_dir)(const string fpath);
		int   (*media_library_advanced_function)(int fid, int fdata, void *rf);

	}mlib;

	int   (*call_function)(int id, int data, void *a, void *b);

	struct
	{
		int   (*easy_draw)(HWND hwnd, HDC dc, RECT *crect);
	}video;

	int    (*setposition_ex)(unsigned long pid, unsigned long fid, double *spos);

};





/*
 * data for encoders (streaming encoders - not implemented).
 */
struct general_encoder_data
{
	unsigned long fiversion; /* fennec input version (input) */
	unsigned long pversion;  /* plugin version */
	unsigned long ptype;     /* plugin type (encoder/decoder etc.) (input) */

	/* functions */

	int           (callc *initialize)              (void);
	int           (callc *uninitialize)            (void);
	
	/* file encoder */

	unsigned long (callc *file_create)             (string fname, unsigned long fcreatemode);
	int           (callc *file_write)              (unsigned long id, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize);
	int           (callc *file_close)              (unsigned long id); 
	int           (callc *file_flush)              (unsigned long id);
	int           (callc *file_seek)               (unsigned long id, double spos);
	double        (callc *file_tell)               (unsigned long id, unsigned long tellmode);
	
	/* stream encoder */

	unsigned long (callc *encode_initialize)       (void);
	int           (callc *encode_encode)           (unsigned long id, void* pcmin, void* eout, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize, unsigned long* osize, encode_realloc reallocfunc); /* if osize is not enough, call 'encode_realloc' */ 
	int           (callc *encode_uninitialize)     (unsigned long id);

	int           (callc *about)                   (unsigned long id, void* odata); /* os dependent */
	int           (callc *settings_fileencoder)    (unsigned long id, void* odata); /* os dependent */
	int           (callc *settings_encoder)        (unsigned long id, void* odata); /* os dependent */

	int           (callc *get_initialization_info) (unsigned long inforreq, void* outinfo, unsigned long* outsize); /* int to bool */

	string        *language_text;

	struct plugin_settings fsettings;
};




/*
 * video decoding functions.
 */
struct video_dec
{
	int    (callc *video_decoder_getsize)      (unsigned long id, int    *width, int *height);
	int    (callc *video_decoder_getframe)     (unsigned long id, void  **buffer);
	int    (callc *video_decoder_seek)         (unsigned long id, double *pos,   int iframe, int rel);
	int    (callc *video_decoder_getframe_sync)(unsigned long id, void  **buffer, double audiosec);
	string (callc *video_decoder_getsubtitle)  (unsigned long id, float ctime);
	int    (callc *video_decoder_trans_info)   (int id, long dt_l, double dt_d, void *data);

};



/*
 * data for decoders.
 */
struct general_input_data
{
	unsigned long fiversion; /* fennec input version (input) */
	unsigned long pversion;  /* plugin version */
	unsigned long ptype;     /* plugin type (encoder/decoder etc.) (input) */

	/* functions */

	int           (callc *initialize)              (void);
	unsigned long (callc *open)                    (unsigned long otype, unsigned long osize, const string odata);
	int           (callc *getformat)               (unsigned long id, unsigned long* ftype, void* fdata);
	int           (callc *getinfo)                 (unsigned long id, unsigned long* itype, void* idata); 
	int           (callc *getduration)             (unsigned long id, unsigned long* ptype, void* pos);
	int           (callc *getposition)             (unsigned long id, unsigned long* ptype, void* pos);
	int           (callc *setposition)             (unsigned long id, unsigned long  ptype, void* pos);
	int           (callc *getstatus)               (unsigned long id, unsigned long* stype, void* stt);
	int           (callc *read)                    (unsigned long id, unsigned long  dsize, unsigned long* dread, void* rdata);
	int           (callc *close)                   (unsigned long id);
	int           (callc *uninitialize)            (void);
	
	int           (callc *about)                   (void *pdata);
	int           (callc *settings)                (void *pdata);

	int           (callc *get_initialization_info) (unsigned long inforreq, void* outinfo, unsigned long* outsize); /* int to bool */
	int           (callc *get_extension)           (int id, string out_ext, string out_desc, int* newdsc);    /* int to bool */

	int           (callc *tagread)                 (const string fname, struct fennec_audiotag* rtag);
	int           (callc *tagwrite)                (const string fname, struct fennec_audiotag* rtag);

	int           (callc *score)                   (const string fname, int options);
		/* min score: 0, max score: 10, default score (mp3 etc.): 5 */

	string        *language_text;

	struct plugin_settings   fsettings;

	struct video_dec        *vid_dec;
};




/*
 * data for visualizations.
 */
struct general_visualization_data
{
	unsigned long fiversion; /* fennec input version (input) */
	unsigned long pversion;  /* plugin version */
	unsigned long ptype;     /* plugin type (encoder/decoder etc.) (input) */

	/* functions */

	int           (callc *initialize)              (void);
	int           (callc *refresh)                 (int, void*);
	int           (callc *uninitialize)            (int, void*);
	
	int           (callc *about)                   (void *pdata);
	int           (callc *settings)                (void *pdata);

	unsigned long (callc *getdata)                 (int id, int cid, void *mdata, void *sdata);
		/* see misc */

	struct plugin_settings fsettings;
	struct fennec *shared;
};




/*
 * data for video output plugins.
 */
struct general_videoout_data
{
	unsigned long fiversion; /* fennec input version (input) */
	unsigned long pversion;  /* plugin version */
	unsigned long ptype;     /* plugin type (encoder/decoder etc.) (input) */

	/* functions */

	int           (callc *initialize)              (void);
	int           (callc *refresh)                 (int rlevel);
	int           (callc *pushtext)                (int textmode, int addmode, void *data, int x, int y, int w, int h);
	int           (callc *setinfo_ex)              (int id, void *data, int (*afunc)());
	int           (callc *setinfo)                 (int frame_w, int frame_h, int frame_rate, int colors);
	int           (callc *setstate)                (int state, int *extradata);
	int           (callc *display)                 (void *data, int mode, void *modedata);
	int           (callc *uninitialize)            (int, void*);
	
	int           (callc *about)                   (void *pdata);
	int           (callc *settings)                (void *pdata);

	unsigned long (callc *getdata)                 (int id, int cid, void *mdata, void *sdata);
		/* see misc */


	struct plugin_settings fsettings;
	struct fennec *shared;
};



/*
 * data digital signal processing plug-ins.
 */
struct general_dsp_data
{
	unsigned long fiversion; /* fennec input version (input) */
	unsigned long pversion;  /* plugin version */
	unsigned long ptype;     /* plugin type (encoder/decoder etc.) (input) */

	/* functions */

	int           (callc *initialize)              (void);
	unsigned long (callc *open)                    (unsigned long id);
	void*         (callc *process)                 (unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr);
	int           (callc *close)                   (unsigned long id);
	int           (callc *uninitialize)            (void);
	
	int           (callc *about)                   (void *pdata);
	int           (callc *settings)                (void *pdata);
	
	int           (callc *callengine)              (int id, int a, void* b, double d);
	int           (callc *messageproc)             (int id, int a, void* b, double d);

	string        *language_text;

	struct plugin_settings fsettings;
};




/*
 * data for output plugins.
 */
struct general_output_data
{
    int                  (*settings                           )(void*);
	int                  (*about                              )(void*);

	int                  (*audio_initialize                   )(void);
	int                  (*audio_uninitialize                 )(void);
	int                  (*audio_load                         )(const string spath);
	int                  (*audio_add                          )(unsigned long idx, const string spath);
	int                  (*audio_play                         )(void);
	int                  (*audio_playpause                    )(void);
	int                  (*audio_pause                        )(void);
	int                  (*audio_stop                         )(void);
	int                  (*audio_setposition                  )(double cpos);
	int                  (*audio_setposition_ms               )(double mpos);
	int                  (*audio_getplayerstate               )(void);
	double               (*audio_getposition                  )(void);
	double               (*audio_getposition_ms               )(void);
	double               (*audio_getduration_ms               )(void);
	int                  (*audio_getpeakpower                 )(unsigned long* pleft, unsigned long* pright);
	int                  (*audio_setvolume                    )(double vl, double vr);
	int                  (*audio_getvolume                    )(double* vl, double* vr);
	int                  (*audio_getcurrentbuffer             )(void* dout, unsigned long* dsize);
	int                  (*audio_getfloatbuffer               )(float* dout, unsigned long scount, unsigned long channel);
	int                  (*audio_getdata                      )(unsigned int di, void* ret);
	int                  (*audio_preview_action               )(int actionid, void *adata);

	struct fennec *shared;
};




/*
 * audio format.
 */
struct base_format
{
	unsigned long bfrequency;
	unsigned long bbits;
	unsigned long bchannels;
};




/*
 * status.
 */
struct base_status
{
	unsigned short bstate;
};





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
 * main functions
 */

typedef unsigned long (__stdcall * fn_vis_message)  (int id, int mdata, int sdata);

typedef unsigned long (__stdcall * fennec_decoder_initialize) (struct general_input_data          *gin);
typedef unsigned long (__stdcall * fennec_encoder_initialize) (struct general_encoder_data        *gin);
typedef unsigned long (__stdcall * fennec_effects_initialize) (struct general_dsp_data            *gin, string pname);
typedef unsigned long (__stdcall * fennec_visuals_initialize) (struct general_visualization_data  *gin, string pname);
typedef unsigned long (__stdcall * fennec_output_initialize)  (struct general_output_data         *gin, string pname);




#endif /* </ !header_fennec > */

/*-----------------------------------------------------------------------------
  fennec header file for plug-ins.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/
