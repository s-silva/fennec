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
#include "../include/settings.h"

/* defines ------------------------------------------------------------------*/

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

/* structs ------------------------------------------------------------------*/

struct fennec
{
	struct /* audio */
	{
		struct /* output */
		{

			int     (*initialize)(void);
			int     (*uninitialize)(void);
			int     (*load)(const string spath);
			int     (*add)(unsigned long idx, const string spath);
			int     (*play)(void);
			int     (*pause)(void);
			int     (*stop)(void);
			int     (*setposition)(double cpos);
			int     (*setposition_ms)(double mpos);
			int     (*getplayerstate)(void);
			double  (*getposition)(double* cpos);
			double  (*getposition_ms)(void);
			double  (*getduration_ms)(void);
			int     (*getpeakpower)(unsigned long* pleft, unsigned long* pright);
			int     (*setvolume)(double vl, double vr);
			int     (*getvolume)(double* vl, double* vr);
			int     (*getcurrentbuffer)(void* dout, unsigned long* dsize);
			int     (*getfloatbuffer)(float* dout, unsigned long scount, unsigned long channel);
			int     (*getdata)(unsigned int di, void* ret);

			struct /* playlist */
			{
				int         (*switch_list)(unsigned long idx);
				int         (*clear)(void);
				int         (*add)(string fname, int checkfile, int gettags);
				int         (*move)(unsigned long idd, unsigned long ids, int mmode);
				int         (*setshuffle)(int shuffle, int doaction);
				int         (*shuffle)(int entirelist);
				int         (*unshuffle)(void);
				int         (*remove)(unsigned long idx);
				int         (*mark)(unsigned long idx);
				int         (*unmark)(unsigned long idx);
				int         (*ismarked)(unsigned long idx);
				int         (*getinfo)(unsigned long idx, string sname, string sartist, string salbum, string sgenre, unsigned long* durationms, unsigned long* fsize);
				int         (*next)(void);
				int         (*previous)(void);
				int         (*switchindex)(unsigned long idx);
				unsigned long       (*getcurrentindex)(void);
				unsigned long       (*getrealindex)(unsigned long idx);
				unsigned long       (*getlistindex)(unsigned long ridx);
				unsigned long       (*getcount)(void);
				const string (*getsource)(unsigned long idx);
			}playlist;

		}output;

		struct /* input */
		{

			struct /* plugins */
			{
				int    (*initialize)(unsigned long pid);
				int    (*uninitialize)(unsigned long pid, int force);
				int    (*mark_usage)(unsigned long pid, int isused);
				int    (*loadfile)(unsigned long pid, const string sfile, unsigned long* fid);
				int    (*getformat)(unsigned long pid, unsigned long fid, unsigned long* freq, unsigned long* bps, unsigned long* nchannels);
				int    (*readdata)(unsigned long pid, unsigned long fid, unsigned long* dsize, void* odata);
				unsigned long  (*getduration_ms)(unsigned long pid, unsigned long fid);
				int    (*unloadfile)(unsigned long pid, unsigned long fid);
				int    (*setposition)(unsigned long pid, unsigned long fid, double spos);

			}plugins;

			int                    (*initialize)(void);
			int                    (*uninitialize)(void);
			int                    (*selectinput)(const string fname);
			int                    (*getextensionsinfo)(unsigned long id, string ext, string info);
			int                    (*tagread_known)(unsigned long pid, const string fname, struct fennec_audiotag* rtag);
			int                    (*tagwrite)(const string fname, struct fennec_audiotag* rtag);
			unsigned long          (*tagread)(const string fname, struct fennec_audiotag* rtag);
			t_sys_library_handle   (*gethandle)(const string fname);

		}input;

		struct
		{
			int   (*set_bands)(int channel, int bands, float *values);
			int   (*get_bands)(int channel, int bands, float *values);
			int   (*set_preamp)(int channel, float value);
			float (*get_preamp)(int channel);
			int   (*reset)(void);
			int   (*switch_current)(int state);
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
		int (*show_error)(int etype, const string text);

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
			int (*add)(unsigned long sid, void *val, unsigned long vsz);
			int (*set)(unsigned long sid, unsigned long iid, void *val);
			int (*get)(unsigned long sid, unsigned long iid, void *val, unsigned long *vsz);
			int (*remove)(unsigned long sid, unsigned long iid);
			int (*clean)(unsigned long sid);

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

/*-----------------------------------------------------------------------------
 fennec, july 2007.
-----------------------------------------------------------------------------*/