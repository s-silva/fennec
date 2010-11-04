/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0
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

#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"

#define inline __inline

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


//typedef struct qlist {
 //   void         *pkt;
//	double       pos;
//	int          pushi;
//} qlist;

//struct queue_frame
//{
//	qlist         *list;
//	unsigned long  count;
//	unsigned long  memcount;
//};

/*
 * decoder data.
 */
struct plugin_stream
{
	/* default */
	
	unsigned char     initialized;     /* initialized? do not modify this */
	unsigned long     frequency;       /* samples per second */
	unsigned long     channels;        /* channels */
	unsigned long     bitspersample;   /* bit depth (16 fixed, 64 float) */
	unsigned long     duration;        /* duration in ms */

	/* plugin defined */

	int               loaded;

	void          *buf;
	int64_t        lastp;
	double         seekpts;
	double         audio_pos;

	uint8_t       *dvd_buffer;
	void          *dvd_info;
	int            dvd_cpt;
	int            dvd_bufsize;

	struct
	{
		FILE            *sfile;
		ByteIOContext    io;
		int              io_buffer_size;
		uint8_t         *io_buffer;
		AVInputFormat   *fmt;
		AVFormatContext *ic;
		URLContext       url;
		URLProtocol      prot;
	}dvd;

	int            pts_goes_wrong; /* if this is true (initially false), the audio time will be taken for sync */
};



/*
 * encoder data.
 */
struct plugin_encoder_stream
{
	/* default */

	int                initialized;    /* initialized? do not modify this */
	int                bmode;          /* base mode, fennec_plugintype... */
	int                smode;          /* sub mode, fenne_encoder_file etc. */

	letter             filepath[v_sys_maxpath];
	t_sys_file_handle  fhandle;

	unsigned long      cfrequency;     /* current frequency (sample rate) */
	unsigned long      cbitspersample; /* current bit depth */
	unsigned long      cchannels;      /* current channels */

	int                firstwrite;     /* first write? */

	/* plugin defined */

};


/* external data */

extern struct plugin_stream          *pstreams;  /* decoders */
extern struct plugin_encoder_stream  *pestreams; /* encoders */


/* decoder functions */

int           decoder_load(unsigned long id, const string sname);
int           decoder_close(unsigned long id);
int           decoder_seek(unsigned long id, double *pos);
unsigned long audio_decoder_read(unsigned long id, void* adata, unsigned long dsize);
int           decoder_tagread(const string fname,  struct fennec_audiotag *rtag);
int           decoder_tagwrite(const string fname,  struct fennec_audiotag *rtag);

/* encoder functions */

int           encoder_appendextension(unsigned long id, string fpath);
int           encoder_deleteextension(unsigned long id, string fpath);
int           encoder_initialize(unsigned long id, int fcreatemode);
int           encoder_uninitialize(unsigned long id);
int           encoder_set(unsigned long id);
int           encoder_setdefaults(unsigned long id);
int           encoder_write(unsigned long id, void* rbuffer, unsigned long bsize);



/* general encoder functions */

int           callc  encoder_fennec_plugin_initialize(void);
int           callc  encoder_fennec_plugin_uninitialize(void);
unsigned long callc  encoder_file_create(string fname, unsigned long fcreatemode);
int           callc  encoder_file_write(unsigned long id, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize);
int           callc  encoder_file_close(unsigned long id);
int           callc  encoder_file_flush(unsigned long id);
int           callc  encoder_file_seek(unsigned long id, double spos);
double        callc  encoder_file_tell(unsigned long id, unsigned long tellmode);
int           callc  encoder_about(unsigned long id, void* odata);
int           callc  encoder_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize);
unsigned long callc  encode_initialize(void);
int           callc  encode_uninitialize(unsigned long id);
int           callc  encode_encode(unsigned long id, void* pcmin, void* eout, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize, unsigned long* osize, encode_realloc reallocfunc);
int           callc  settings_fileencoder(unsigned long id, void* odata);
int           callc  settings_encoder(unsigned long id, void* odata);
unsigned long        encoder_local_locateindex(void);

/* general decoder functions */

unsigned long callc fennec_initialize_input(struct general_input_data* gin);
int           callc fennec_plugin_initialize(void);
int           callc fennec_plugin_getformat(unsigned long id, unsigned long* ftype, void* fdata);
int           callc fennec_plugin_getposition(unsigned long id, unsigned long* ptype, void* pos);
int           callc fennec_plugin_setposition(unsigned long id, unsigned long ptype, void* pos);
int           callc fennec_plugin_getduration(unsigned long id, unsigned long* ptype, void* pos);
int           callc fennec_plugin_getstatus(unsigned long id, unsigned long* stype, void* stt);
int           callc fennec_plugin_read(unsigned long id, unsigned long dsize, unsigned long* dread, void* rdata);
int           callc fennec_plugin_about(void* pdata);
int           callc fennec_plugin_settings(void* pdata);
int           callc fennec_plugin_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize);
int           callc fennec_plugin_get_extension(int id, string out_ext, string out_desc, int* newdsc);
int           callc fennec_plugin_tagread (const string fname, struct fennec_audiotag* rtag);
int           callc fennec_plugin_close(unsigned long id);
int           callc fennec_plugin_uninitialize(void);
int           callc fennec_plugin_tagwrite (const string fname, struct fennec_audiotag* rtag);
unsigned long callc fennec_plugin_open(unsigned long otype, unsigned long osize, const string odata);
int           callc fennec_plugin_tagread(const string fname, struct fennec_audiotag* rtag);
int           callc fennec_plugin_score(const string fname, int options);
int           decoder_score(const string sfile, int res);

int callc video_decoder_getsize(unsigned long id, int *width, int *height);
int callc video_decoder_getframe(unsigned long id, void **buffer);
int callc video_decoder_getframe_sync(unsigned long id, void **buffer, double audiosec);
int callc video_decoder_seek(unsigned long id, double *pos, int iframe, int rel);
int callc video_decoder_trans_info(int id, long dt_l, double dt_d, void *data);
string callc video_decoder_getsubtitle(unsigned long id, float ctime);
DWORD video_thread(LPVOID lparam);
int dvddec_open(int driveid, void **info);
int dvddec_close(void *info);
int dvddec_read(void *info, void *buf); /* reads (1024 * DVD_VIDEO_LB_LEN) data into 'buf' */
int dvdplay_open(unsigned long id, AVFormatContext **ct, int driveid);
int dvddec_seek(void *info, float pos);


extern int video_thread_created;
/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
