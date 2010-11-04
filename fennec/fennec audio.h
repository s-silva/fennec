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

#include "systems/system.h"
#include "tagging.h"
#include "plugins.h"



typedef double fennec_sample;

#define fennec_sample_size sizeof(fennec_sample)
#define fennec_sample_bits 64










void  SetDecibelBandValue(int bnid, int chid, float value);
void  SetDecibelPreampValue(int chid, float value);
float GetDecibelPreampValue(int chid);
float GetDecibelBandValue(int bnid, int chid);
void  InitializeEqualizer(void);
int   SetEqualizer(int rvs, float* bsets, int bcount);
void  EqualizeBuffer(void* dbuf, int chan, int freq, int bps, unsigned long dlen);
int   EqualizeBufferEx(void* outbuf, const void* inbuf, int chan, int freq, int bps, unsigned long dlen);

void  fft_float(unsigned NumSamples, int InverseTransform, float *RealIn, float *ImagIn, float *RealOut, float *ImagOut);

/* rules applied ------------------------------------------------------------*/

#define audio_v_playerstate_null                0 /* error */
#define audio_v_playerstate_notinit             1 /* player ain't initialized */
#define audio_v_playerstate_init                2 /* initialized */
#define audio_v_playerstate_loaded              3 /* loaded */
#define audio_v_playerstate_buffering           4 /* writing buffer (not playing) */
#define audio_v_playerstate_stopped             5 /* stopped */
#define audio_v_playerstate_paused              6 /* paused */
#define audio_v_playerstate_opening             7 /* opening (or buffering for the first time) */
#define audio_v_playerstate_playing             8 /* playing */
#define audio_v_playerstate_playingandbuffering 9 /* buffering while playing */

#define audio_v_data_frequency                  1 /* [int] current frequency */
#define audio_v_data_bitspersample              2 /* [int] */
#define audio_v_data_channels                   3 /* [int] */
#define audio_v_data_bitrate                    4 /* [int] */
#define audio_v_data_ids_pid                    5 /* return */
#define audio_v_data_ids_fid                    6 /* return */
#define audio_v_data_position_for_video         7 /* [double*] in seconds */

/* wave decoder */



/* equalizer */

int equalizer_initialize(int frequency);


/* internal output */

int     internal_output_initialize(void);
int     internal_output_uninitialize(void);
int     internal_output_loadfile(const string fname);
int     internal_output_play(void);
int     internal_output_pause(void);
int     internal_output_stop(void);
int     internal_output_playpause(void);
int     internal_output_setposition(double cpos);
int     internal_output_setposition_ms(double mpos);
double  internal_output_getposition();
unsigned long   internal_output_getposition_ms();
unsigned long   internal_output_getduration_ms();
int     internal_output_getplayerstate(void);
int     internal_output_getpeakpower(unsigned long* pleft, unsigned long* pright);
int     internal_output_setvolume(double lvol, double rvol);
int     internal_output_getdata(unsigned int di, void* ret);

/*
int   internal_output_playlist_clear(void);
int   internal_output_playlist_add(const char* fname, int checkfile, int gettags);
int   internal_output_playlist_move(unsigned long idd, unsigned long ids);
int   internal_output_playlist_setshuffle(int shuffle, int doaction);
int   internal_output_playlist_shuffle(int entirelist);
int   internal_output_playlist_unshuffle(void);
int   internal_output_playlist_remove(unsigned long idx);
int   internal_output_playlist_mark(unsigned long idx);
int   internal_output_playlist_unmark(unsigned long idx);
int   internal_output_playlist_ismarked(unsigned long idx);
int   internal_output_playlist_getinfo(unsigned long idx, char* sname, char* sartist, char* salbum, char* sgenre, unsigned long* durationms, unsigned long* fsize);
int   internal_output_playlist_next(void);
int   internal_output_playlist_previous(void);
int   internal_output_playlist_switch(unsigned long idx);
int   internal_output_playlist_switch_list(unsigned long idx);
unsigned long internal_output_playlist_getcurrentindex(void);
unsigned long internal_output_playlist_getrealindex(unsigned long idx);
unsigned long internal_output_playlist_getlistindex(unsigned long ridx);
unsigned long internal_output_playlist_getcount(void);
int   internal_output_playlist_exchange(unsigned long idd, unsigned long ids);
*/

const char* internal_output_playlist_getsource(unsigned long idx);
int   internal_output_getvolume(double* lvol, double* rvol);
int   internal_output_getcurrentbuffer(void* dout, unsigned long* dsize);
int   internal_output_getfloatbuffer(float* dout, unsigned long scount, unsigned long channel);

/* audio output */

int     audio_initialize(void);
int     audio_uninitialize(void);
int     audio_load(const string spath);
int     audio_loadfile(const string spath);
int     audio_loadfile_soft(int *pchanged, const string spath);
int     audio_add(unsigned long idx, const string spath);
int     audio_play(void);
int     audio_pause(void);
int     audio_stop(void);
int     audio_setposition(double cpos);
int     audio_setposition_ms(double mpos);
int     audio_getplayerstate(void);
double  audio_getposition(double* cpos);
double  audio_getposition_ms(void);
double  audio_getduration_ms(void);
int     audio_getpeakpower(unsigned long* pleft, unsigned long* pright);
int     audio_setvolume(double vl, double vr);
int     audio_getvolume(double* vl, double* vr);
int     audio_getcurrentbuffer(void* dout, unsigned long* dsize);
int     audio_getfloatbuffer(float* dout, unsigned long scount, unsigned long channel);
int     audio_getdata(unsigned int di, void* ret);
int     audio_preview_action(int actionid, void *adata);
int     audio_get_current_ids(unsigned long *pid, unsigned long *fid);


int     audio_playlist_initialize(void);
int     audio_playlist_uninitialize(void);
int     audio_playlist_switch_list(unsigned long idx);
int     audio_playlist_clear(void);
int     audio_playlist_add(const string fname, int checkfile, int gettags);
int     audio_playlist_move(unsigned long idd, unsigned long ids, int mmode);
int     audio_playlist_setshuffle(int shuffle, int doaction);
int     audio_playlist_shuffle(int entirelist);
int     audio_playlist_unshuffle(void);
int     audio_playlist_remove(unsigned long idx);
int     audio_playlist_mark(unsigned long idx);
int     audio_playlist_unmark(unsigned long idx);
int     audio_playlist_ismarked(unsigned long idx);
int     audio_playlist_getinfo(unsigned long idx, string sname, string sartist, string salbum, string sgenre, unsigned long* durationms, unsigned long* fsize);
int     audio_playlist_next(void);
int     audio_playlist_previous(void);
int     audio_playlist_switch(unsigned long idx);
unsigned long   audio_playlist_getcurrentindex(void);
unsigned long   audio_playlist_getrealindex(unsigned long idx);
unsigned long   audio_playlist_getlistindex(unsigned long ridx);
unsigned long   audio_playlist_getcount(void);
int     audio_playlist_exchange(unsigned long idd, unsigned long ids);

const string audio_playlist_getsource(unsigned long idx);

int     audio_input_initialize(void);
int     audio_input_uninitialize(void);
int     audio_input_selectinput(const string fname);
int     audio_input_plugin_initialize(unsigned long pid);
int     audio_input_plugin_uninitialize(unsigned long pid, int force);
int     audio_input_plugin_mark_usage(unsigned long pid, int isused);
int     audio_input_plugin_loadfile(unsigned long pid, const string sfile, unsigned long* fid);
int     audio_input_plugin_getformat(unsigned long pid, unsigned long fid, unsigned long* freq, unsigned long* bps, unsigned long* nchannels);
int     audio_input_plugin_readdata(unsigned long pid, unsigned long fid, unsigned long* dsize, void* odata);
unsigned long   audio_input_plugin_getduration_ms(unsigned long pid, unsigned long fid);
int     audio_input_plugin_unloadfile(unsigned long pid, unsigned long fid);
int     audio_input_plugin_setposition(unsigned long pid, unsigned long fid, double spos);
int     audio_input_plugin_setposition_ex(unsigned long pid, unsigned long fid, double *spos);
int     audio_input_getextensionsinfo(unsigned long id, string ext, string info);
int     audio_input_tagread_known(unsigned long pid, const string fname, struct fennec_audiotag* rtag);
int     audio_input_tagwrite(const string fname, struct fennec_audiotag* rtag);
unsigned long audio_input_tagread(const string fname, struct fennec_audiotag* rtag);
t_sys_library_handle audio_input_gethandle(const string fname);
int	    audio_input_drive_eject(char driveid);
int     audio_input_drive_load(char driveid);
int     audio_input_drive_gettracks(char driveid);

/* internal input */

int     internal_input_initialize(void);
int     internal_input_uninitialize(void);
int     internal_input_selectinput(const string fname);
int     internal_input_plugin_initialize(unsigned long pid);
int     internal_input_plugin_uninitialize(unsigned long pid, int force);
int     internal_input_plugin_mark_usage(unsigned long pid, int isused);
int     internal_input_plugin_loadfile(unsigned long pid, const string sfile, unsigned long* fid);
int     internal_input_plugin_getformat(unsigned long pid, unsigned long fid, unsigned long* freq, unsigned long* bps, unsigned long* nchannels);
int     internal_input_plugin_readdata(unsigned long pid, unsigned long fid, unsigned long* dsize, void* odata);
int     internal_input_plugin_unloadfile(unsigned long pid, unsigned long fid);
int     internal_input_plugin_setposition(unsigned long pid, unsigned long fid, double spos);
int     internal_input_plugin_setposition_ex(unsigned long pid, unsigned long fid, double *spos);
unsigned long   internal_input_plugin_getduration_ms(unsigned long pid, unsigned long fid);
int     internal_input_getextensionsinfo(unsigned long id, string ext, string info);
int     internal_input_tagread_known(unsigned long pid, const string fname, struct fennec_audiotag* rtag);
int     internal_input_tagwrite(const string fname, struct fennec_audiotag* rtag);
int     internal_input_show_about(unsigned long pid, void *pdata);
int     internal_input_show_settings(unsigned long pid, void *pdata);
unsigned long internal_input_tagread(const string fname, struct fennec_audiotag* rtag);
t_sys_library_handle internal_input_gethandle(const string fname);
int internal_input_drive_eject(char driveid);
int internal_input_drive_load(char driveid);
int internal_input_drive_gettracks(char driveid);
int internal_input_plugin_getvideo(unsigned long pid, unsigned long fid, struct video_dec **vdec);
int internal_input_plugin_score(unsigned long pid, const string sfile);


#define audio_input_tagfree(id, tag)(internal_input_tagread_known(id, 0, tag))

/* encoder */

int           encoder_initialize(void);
int           encoder_uninitialize(void);
unsigned int  encoder_getencoderscount(void);
string        encoder_getpath(unsigned int id);
string        encoder_getname(unsigned int id);
unsigned long encoder_plugin_file_create(unsigned int id, string fname);
int           encoder_plugin_file_write(unsigned int id, unsigned long fid, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize);
int           encoder_plugin_file_close(unsigned int id, unsigned long fid);
int           encoder_plugin_uninitialize(unsigned int id);
int           encoder_plugin_initialize(unsigned int id);
int           encoder_plugin_file_settings(unsigned int id, unsigned long fid, void* odata);
int           encoder_plugin_encoder_settings(unsigned int id, unsigned long fid, void* odata);
int           encoder_plugin_about(unsigned int id, unsigned long fid, void* odata);
int           encoder_plugin_global_file_settings(unsigned int id, void* odata);
int           encoder_plugin_global_encoder_settings(unsigned int id, void* odata);
int           encoder_plugin_global_about(unsigned int id, void* odata);

/* dsp */

int dsp_initialize(void);
int dsp_reinitialize(void); /* initialize recently added plugins only */
int dsp_uninitialize(void);
int dsp_initialize_index(unsigned long pid, unsigned long id);
int dsp_uninitialize_index(unsigned long pid, unsigned long id);
void* dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize);
int dsp_showabout(unsigned int pid, void* pdata);
int dsp_showconfig(unsigned int pid, void* pdata);
int dsp_sendmessage(unsigned int pid, int id, int a, void* b, double d);
int __stdcall dsp_getmessage(int id, int a, void* b, double d);
int dsp_getoutput_channels(int inchannels);

/* visualization */

/* equalizer */

int   equalizer_set_bands(int channel, int bands, float *values);
int   equalizer_get_bands(int channel, int bands, float *values);
int   equalizer_set_preamp(int channel, float value);
float equalizer_get_preamp(int channel);
int   equalizer_reset(void);
int   equalizer_switch_current(int state);
int   equalizer_equalize(void *outbuf, const void *inbuf, int nchan, int freq, int bpsample, unsigned int dlength);
void* equalize_buffer_variable_init(void *eqp, int channels, int frequency);
int   equalize_buffer_variable_uninit(void *eqd);
int   equalize_buffer_variable(void *inout, const void *in, int channels, int frequency, int bps, unsigned long datalength, void *eqd);


int    media_library_initialize(void);
int    media_library_uninitialize(void);
int    media_library_add_file(const string spath);
int    media_library_save(const string spath);
int    media_library_load(const string spath);
void  *media_library_get(unsigned long id);
int    media_library_translate(unsigned long id, struct fennec_audiotag *at);
int    media_library_get_dir_names(unsigned long id, unsigned long pdir, string *dname);
int    media_library_get_dir_files(unsigned long id, unsigned long pdir, string cdir, uint32_t *tid);
int    media_library_add_dir(const string fpath);
int    media_library_advanced_function(int fid, int fdata, void *rf);
string media_library_getsource(unsigned long id);


int videoout_initialize(const string fname);
int videoout_uninitialize();
int videoout_checkfile(const string fname);
int videoout_display(void *data, int mode, void *modedata);
int videoout_setinfo(int frame_w, int frame_h, int frame_rate, int colors);
int videoout_setinfo_ex(int id, void *data, int (*afunc)());
int videoout_pushtext(int textmode, int addmode, void *data, int x, int y, int w, int h);
int videoout_settings(void* fdata);
int videoout_refresh(int rlevel);
int videoout_about(void* fdata);
unsigned long __stdcall videoout_getdata(int id, int cid, void *mdata, void *sdata);

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/