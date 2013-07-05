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

#include "fennec_main.h"
#include "fennec_audio.h"




/* defines ------------------------------------------------------------------*/

#define  isinternalout() (settings.output_plugins.selected[0] == 0 ? 1 : 0)


/* global functions ---------------------------------------------------------*/


/*
 * Initialize audio engine(s).
 */

int audio_initialize(void)
{
	int ret = 0;

	if(settings.dsp_plugins.enable)
		dsp_initialize();

	equalizer_initialize(-1);

	if(isinternalout())
	{
		ret = internal_output_initialize();

		report("initialized internal output (ret): %d", rt_stepping, ret);
		
		/* dsp stuff */

		//if(settings.dsp_plugins.enablenextstartup)
		//	settings.dsp_plugins.enable = 1;


	}else{

		if(output_plugin_initialize(settings.output_plugins.selected) < 0)
		{
			/* error! initialize internal output instead. */

			reportx("error in external output plugin (%s), resetting.", rt_error, settings.output_plugins.selected);

			settings.output_plugins.selected[0];
			audio_initialize();

			ret = 0;
		}

		report("initialized external output successfully", rt_stepping);

		ret = 1;
	}

	return ret;
}


/*
 * Uninitialize audio output engine(s).
 */

int audio_uninitialize(void)
{
	int ret = 0;

	report("uninitializing audio output...", rt_stepping);

	audio_stop();
	sys_pass();
	dsp_uninitialize();

	if(isinternalout())
	{
		ret = internal_output_uninitialize();
		//dsp_uninitialize();

	}else{

		ret = output_plugin_uninitialize();
	}

	report("uninitialized audio output (ret): %d", rt_stepping, ret);

	return ret;
}


/*
 * Load files.
 */

int audio_load(const string spath)
{
	int ret = 0;

	audio_playlist_clear();
	audio_playlist_add(spath, 0, 0);

	ret = audio_playlist_switch(0);

	//str_cpy(settings.player.last_file, spath);
		
	fennec_refresh(fennec_v_refresh_force_high);

	return ret;
}


int audio_loadfile_soft(int *pchanged, const string spath)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = internal_output_loadfile(spath);
	}else{
		ret = external_output_load(spath);
	}

	reportx("(audio_loadfile) loading file: %s, ret: %d", rt_stepping, spath, ret);

	if(!*pchanged)
	{
		fennec_refresh(fennec_v_refresh_force_less);
	}else{
		fennec_refresh(fennec_v_refresh_force_high);
		*pchanged = 0;
	}

	//str_cpy(settings.player.last_file, spath);

	return ret;
}


/*
 * actual routines.
 */
int audio_loadfile(const string spath)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = internal_output_loadfile(spath);
	}else{
		ret = external_output_load(spath);
	}

	//str_cpy(settings.player.last_file, spath);

	reportx("(audio_loadfile) loading file: %s, ret: %d", rt_stepping, spath, ret);

	fennec_refresh(fennec_v_refresh_force_high);
	
	return ret;
}


/*
 * Add file/stream... into playlist (first 'add' to an empty player would
 * do the same routine of 'load')
 */

int audio_add(unsigned long idx, const string spath)
{

	return 0;
}


/*
 * Start (actually toggle) playback.
 */

int audio_play(void)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = internal_output_playpause();
		fennec_refresh(fennec_v_refresh_force_not);
	
	}else{
	
		ret = external_output_playpause();
		fennec_refresh(fennec_v_refresh_force_not);
	}
	
	/* if video is available, request to show video window */

	{
		unsigned long pid, fid;
		struct video_dec  *vid;
		double  aspect = 0;
		int     w, h;

		audio_get_current_ids(&pid, &fid);
		internal_input_plugin_getvideo(pid, fid, &vid);

		if(vid)
		{
			if(vid->video_decoder_getsize)
			{

				vid->video_decoder_getsize(fid, (int*)&aspect, 0);
				vid->video_decoder_getsize(fid, &w, &h);

				if(aspect > 0.0)
					h = (int)((double)h / aspect);


				skins_function_getdata(set_video_window, 0, (int)(((double)w / (double)h) * 1000000.0));
			}else{
				skins_function_getdata(set_video_window, 0, 0);
			}

		}else{
			skins_function_getdata(set_video_window, 0, 0);
		}
	
	}

	return ret;
}


/*
 * Pause!.
 */

int audio_pause(void)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = internal_output_pause();
		fennec_refresh(fennec_v_refresh_force_not);

	}else{

		ret = external_output_pause();
		fennec_refresh(fennec_v_refresh_force_not);
	}

	return ret;
}


/*
 * Stop!
 */

int audio_stop(void)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = internal_output_stop();
		fennec_refresh(fennec_v_refresh_force_not);

	}else{

		ret = external_output_stop();
		fennec_refresh(fennec_v_refresh_force_not);
	}

	inform_other_programs(0);

	report("stopped media (ret): %d", rt_stepping, ret);
	return ret;
}


/*
 * Get some data.
 */

int audio_getdata(unsigned int di, void* ret)
{
	int rv = 0;

	if(isinternalout())
	{
		rv = internal_output_getdata(di, ret);

	}else{

		rv = external_output_getdata(di, ret);
	}

	return rv;
}


/*
 * Set volume.
 * 'vl' - volume left, 'vr' - volume right.
 * Both should be supplied with values between
 * 0.0 and 1.0
 */

int audio_setvolume(double vl, double vr)
{
	int ret = 0;

	if(isinternalout())
	{
		settings.player.volume[0] = vl;
		settings.player.volume[1] = vr;
		ret = (int)internal_output_setvolume(vl, vr);

	}else{

		settings.player.volume[0] = vl;
		settings.player.volume[1] = vr;
		ret = (int)external_output_setvolume(vl, vr);
	}

	return ret;
}


/*
 * Get current volume.
 * 'vl' - volume left, 'vr' - volume right.
 * (fractions) 0.0 to 1.0
 */

int audio_getvolume(double* vl, double* vr)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = (int)internal_output_getvolume(vl, vr);
	
	}else{
	
		ret = (int)external_output_getvolume(vl, vr);
	}

	return ret;
}


/*
 * Set position fraction ('cpos').
 * 'cpos' - a value between 0.0 and 1.0.
 */

int audio_setposition(double cpos)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = (int)internal_output_setposition(cpos);

	}else{

		ret = (int)external_output_setposition(cpos);
	}

	return ret;
}


/*
 * Set position in milliseconds.
 */

int audio_setposition_ms(double mpos)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = (int)internal_output_setposition_ms(mpos);

	}else{

		ret = (int)external_output_setposition_ms(mpos);
	}

	return ret;
}


/*
 * Get position fraction (0.0 to 1.0).
 */

double audio_getposition(double* cpos)
{
	double ret = 0;

	if(isinternalout())
	{
		*cpos = (double)internal_output_getposition();
		ret = 1;

	}else{

		*cpos = (double)external_output_getposition();
		ret = 1;
	}

	return ret;
}


/*
 * Get position in milliseconds.
 */

double audio_getposition_ms(void)
{
	double ret = 0;

	if(isinternalout())
	{
		ret = (double)internal_output_getposition_ms();

	}else{

		ret = (double)external_output_getposition_ms();
	}

	return ret;
}


/* 
 * Get current duration in milliseconds
 */

double audio_getduration_ms(void)
{
	double ret = 0;

	if(isinternalout())
	{
		ret = (double)internal_output_getduration_ms();

	}else{

		ret = (double)external_output_getduration_ms();
	}

	return ret;
}


/*
 * Get player state (playing, paused...).
 */

int audio_getplayerstate(void)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = internal_output_getplayerstate();

	}else{

		ret = external_output_getplayerstate();
	}

	return ret;
}


/*
 * Get current peak power.
 * Values between 0 and 10,000
 */

int audio_getpeakpower(unsigned long* pleft, unsigned long* pright)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = internal_output_getpeakpower(pleft, pright);

	}else{

		ret = external_output_getpeakpower(pleft, pright);
	}

	return ret;
}


/*
 * Copy current buffer to 'dout'.
 */

int audio_getcurrentbuffer(void* dout, unsigned long* dsize)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = internal_output_getcurrentbuffer(dout, dsize);

	}else{

		ret = external_output_getcurrentbuffer(dout, dsize);
	}

	return ret;
}



/*
 * Get float buffer (sorry, sample values between 0.0 to -,+ 32768,
 * but it's generic, not like 16bit)
 *
 * Try using minus 'channel' (it might return FFT data)
 */

int audio_getfloatbuffer(float* dout, unsigned long scount, unsigned long channel)
{
	int ret = 0;

	if(isinternalout())
	{
		ret = internal_output_getfloatbuffer(dout, scount, channel);

	}else{

		ret = external_output_getfloatbuffer(dout, scount, channel);
	}

	return ret;
}

int audio_get_current_ids(unsigned long *pid, unsigned long *fid)
{
	if(isinternalout())
	{
		if(pid) *pid = internal_output_getdata(audio_v_data_ids_pid, 0);
		if(fid) *fid = internal_output_getdata(audio_v_data_ids_fid, 0);
	}else{
		if(pid) *pid = external_output_getdata(audio_v_data_ids_pid, 0);
		if(fid) *fid = external_output_getdata(audio_v_data_ids_fid, 0);
	}
	return 1;
}


/* input interface ----------------------------------------------------------*/


/*
 * Initialize input engine.
 */

int audio_input_initialize(void)
{

	return internal_input_initialize();
}


/*
 * Uninitialize input engine and
 * all initialized plugins and streams.
 */

int audio_input_uninitialize(void)
{

	return internal_input_uninitialize();
}


/*
 * Select input plugin index by source path.
 * (by extension/headed/file analyzation).
 */

int audio_input_selectinput(const string fname)
{

	return internal_input_selectinput(fname);
}


/*
 * Initialize plugins separately.
 */

int audio_input_plugin_initialize(unsigned long pid)
{

	return internal_input_plugin_initialize(pid);
}


/*
 * Uninitialze plugins separately.
 */

int audio_input_plugin_uninitialize(unsigned long pid, int force)
{

	return internal_input_plugin_uninitialize(pid, force);
}


/*
 * Mark plugin usage (give a push).
 */

int audio_input_plugin_mark_usage(unsigned long pid, int isused)
{

	return internal_input_plugin_mark_usage(pid, isused);
}


/*
 * Load files/streams.
 */

int audio_input_plugin_loadfile(unsigned long pid, const string sfile, unsigned long* fid)
{

	return internal_input_plugin_loadfile(pid, sfile, fid);
}


/*
 * Get raw output format.
 * frequency, channels count, bits per sample.
 */

int audio_input_plugin_getformat(unsigned long pid, unsigned long fid, unsigned long* freq, unsigned long* bps, unsigned long* nchannels)
{

	return internal_input_plugin_getformat(pid, fid, freq, bps, nchannels);
}


/*
 * Read raw data (interpolated)
 */

int audio_input_plugin_readdata(unsigned long pid, unsigned long fid, unsigned long* dsize, void* odata)
{

	return internal_input_plugin_readdata(pid, fid, dsize, odata);
}


/*
 * Get duration in milliseconds.
 */

unsigned long audio_input_plugin_getduration_ms(unsigned long pid, unsigned long fid)
{

	return internal_input_plugin_getduration_ms(pid, fid);
}


/*
 * Set position (fraction - 0.0 to 1.0)
 */

int audio_input_plugin_setposition(unsigned long pid, unsigned long fid, double spos)
{

	return internal_input_plugin_setposition(pid, fid, spos);
}


int audio_input_plugin_setposition_ex(unsigned long pid, unsigned long fid, double *spos)
{

	return internal_input_plugin_setposition_ex(pid, fid, spos);
}


/*
 * Unload stream from a plugin.
 * 'pid' - plugin id, 'fid' - file id.
 */

int audio_input_plugin_unloadfile(unsigned long pid, unsigned long fid)
{

	return internal_input_plugin_unloadfile(pid, fid);
}


/*
 * Get plugin's supported extensions and associated descriptions.
 * Return zero for the maximum 'id'.
 */

int audio_input_getextensionsinfo(unsigned long id, string ext, string info)
{

	return internal_input_getextensionsinfo(id, ext, info);
}


/*
 * Read tags.
 */

unsigned long audio_input_tagread(const string fname, struct fennec_audiotag* rtag)
{

	return internal_input_tagread(fname, rtag);
}


/*
 * Get system library handle of the plugin that
 * supports 'fname'.
 * Return -1 - error.
 */

t_sys_library_handle audio_input_gethandle(const string fname)
{
	return internal_input_gethandle(fname);
}


/*
 * Read tags from a given plugin, actually
 * this is the function to free memory that
 * was allocated for the tag; to do that,
 * just pass the tag and set fname to zero.
 */

int audio_input_tagread_known(unsigned long pid, const string fname, struct fennec_audiotag* rtag)
{

	return internal_input_tagread_known(pid, fname, rtag);
}


/*
 * Write tags into 'fname'.
 */

int audio_input_tagwrite(const string fname, struct fennec_audiotag* rtag)
{

	return internal_input_tagwrite(fname, rtag);
}


/*
 * Eject removeable media drives.
 */

int audio_input_drive_eject(char driveid)
{

	return internal_input_drive_eject(driveid);
}


/*
 * Load removeable media drives.
 */

int audio_input_drive_load(char driveid)
{

	return internal_input_drive_load(driveid);
}


/*
 * Get number of tracks in a disc.
 */

int audio_input_drive_gettracks(char driveid)
{

	return internal_input_drive_gettracks(driveid);
}



/* audio preview (play from another sound card) -----------------------------*/
/* this interface will only be used to perform basic playback so don't
   make this bulky. */



/*
 * Play/stop/seek... the file.
 * Loading could easily be done using
 * the audio input interface (just like playing a file normally).
 */

int audio_preview_action(int actionid, void *adata)
{
	int ret = 0;

	if(isinternalout())
	{
		/* ret = internal_output_preview_action(actionid, adata); */
	}else{
		ret = external_output_preview_action(actionid, adata);
	}

	return ret;
}






/*-----------------------------------------------------------------------------
 fennec. 2005 - Sept. 2007.
-----------------------------------------------------------------------------*/
