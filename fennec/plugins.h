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


#ifndef header_plugins
#define header_plugins



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




/* calling addresses */

#define vis_function_initialize         "fennec_visualization_initialize"
#define out_function_initialize         "fennec_initialize_output"
#define skin_function_initialize        "fennec_skin_initialize"
#define encoder_function_initialize     "fennec_initialize_encoder"
#define input_function_initialize       "fennec_initialize_input"
#define dsp_function_initialize         "fennec_initialize_dsp"
#define videoout_function_initialize    "fennec_videoout_initialize"
#define videoeffect_function_initialize "fennec_videoeffect_initialize"






/* equalizer values */

#define v_audio_equalizer_default               12345.0




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
 * main functions
 */
typedef unsigned long (__stdcall * fennec_decoder_initialize) (struct general_input_data          *gin);
typedef unsigned long (__stdcall * fennec_encoder_initialize) (struct general_encoder_data        *gin);
typedef unsigned long (__stdcall * fennec_effects_initialize) (struct general_dsp_data            *gin, string pname);
typedef unsigned long (__stdcall * fennec_visuals_initialize) (struct general_visualization_data  *gin, string pname);
typedef unsigned long (__stdcall * fennec_output_initialize)  (struct general_output_data         *gin, string pname);
typedef unsigned long (__stdcall * fennec_videoout_initialize)(struct general_videoout_data       *gin, string pname);




#endif /* </ !header_plugin > */

/*-----------------------------------------------------------------------------
  eof.
-----------------------------------------------------------------------------*/


