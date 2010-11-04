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

#include "fennec main.h"
#include "plugins.h"



/* data ---------------------------------------------------------------------*/

int                         externalout_initialized = 0;
t_sys_library_handle        externalout_handle = 0;
struct general_output_data  current_out_data;




/* code ---------------------------------------------------------------------*/


/*
 * initialize external output plug-in.
 */
int output_plugin_initialize(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_output_initialize   hfunc;
	letter                     otitle[v_sys_maxpath];
	letter                     fpath[v_sys_maxpath];

	if(externalout_initialized)output_plugin_uninitialize();

	fennec_get_abs_path(fpath, fname);

	fhandle = sys_library_load(fpath);

	if(fhandle == v_error_sys_library_load)return -1;

	hfunc = (fennec_output_initialize) sys_library_getaddress(fhandle, out_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return -2;
	}

	shared_functions_fill(v_shared_function_mask_audio_input   |
						  v_shared_function_mask_audio_output  |
						  v_shared_function_mask_general       |
						  v_shared_function_mask_simple        |
						  v_shared_function_mask_settings, &fennec);

	current_out_data.shared = &fennec;
	hfunc(&current_out_data, otitle);

	externalout_handle = fhandle;

	externalout_initialized = 1;
	return 0;
}


/*
 * uninitialize current output plug-in.
 */
int output_plugin_uninitialize()
{
	if(!externalout_initialized)return 1; /* warning: already uninitialized */
	if(externalout_handle)
	{
		if(current_out_data.audio_uninitialize)current_out_data.audio_uninitialize();	
		sys_pass();
		sys_library_free(externalout_handle);
	}

	externalout_handle = 0;
	externalout_initialized = 0;
	return 0;
}


/*
 * analyze plug-in.
 */
int output_plugin_checkfile(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_output_initialize   hfunc;
	letter                     fpath[v_sys_maxpath];

	fennec_get_abs_path(fpath, fname);

	fhandle = sys_library_load(fpath);

	if(fhandle == v_error_sys_library_load)return 0;

	hfunc = (fennec_output_initialize) sys_library_getaddress(fhandle, out_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return 0;
	}

	sys_library_free(fhandle);
	return 1;
}


/*---------------------------------------------------------------------------*/


/*
 * show about.
 */
int external_output_about(void* vdata)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.about)
			return current_out_data.about(vdata);	

	return 0;
}


/*
 * show settings.
 */
int external_output_settings(void* vdata)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.settings)
			return current_out_data.settings(vdata);	

	return 0;
}


/*
 * call initialization routine.
 */
int external_output_initialize(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_initialize)
			return current_out_data.audio_initialize();	

	return 0;
}


/*
 * call uninitialization routine.
 */
int external_output_uninitialize(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_uninitialize)
			return current_out_data.audio_uninitialize();	

	return 0;
}


/*
 * load file/stream.
 */
int external_output_load(const string spath)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_load)
			return current_out_data.audio_load(spath);	

	return 0;
}


/*
 * add player (not used in fennec player).
 */
int external_output_add(unsigned long idx, const string spath)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_add)
			return current_out_data.audio_add(idx, spath);	

	return 0;
}


/*
 * start playback.
 */
int external_output_play(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_play)
			return current_out_data.audio_play();	

	return 0;
}


/*
 * pause playback.
 */
int external_output_pause(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_pause)
			return current_out_data.audio_pause();	

	return 0;
}


/*
 * stop playback.]
 */
int external_output_stop(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_stop)
			return current_out_data.audio_stop();	

	return 0;
}


/*
 * seek (fractional, 0.0 to 1.0).
 */
int external_output_setposition(double cpos)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_setposition)
			return current_out_data.audio_setposition(cpos);	

	return 0;
}


/*
 * seek (set position in milliseconds).
 */
int external_output_setposition_ms(double mpos)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_setposition_ms)
			return current_out_data.audio_setposition_ms(mpos);	

	return 0;
}


/*
 * get current (player) state (playing, paused etc.).
 */
int external_output_getplayerstate(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getplayerstate)
			return current_out_data.audio_getplayerstate();	

	return 0;
}


/*
 * get current position (fraction).
 */
double external_output_getposition(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getposition)
			return current_out_data.audio_getposition();	

	return 0;
}


/*
 * get current position in milliseconds.
 */
double external_output_getposition_ms(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getposition_ms)
			return current_out_data.audio_getposition_ms();	

	return 0;
}


/*
 * get duration in milliseconds.
 */
double external_output_getduration_ms(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getduration_ms)
			return current_out_data.audio_getduration_ms();	

	return 0;
}


/*
 * get peak power (output range: between 0 and 10,000).
 */
int external_output_getpeakpower(unsigned long* pleft, unsigned long* pright)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getpeakpower)
			return current_out_data.audio_getpeakpower(pleft, pright);	

	return 0;
}


/*
 * set volume fraction (left and right separately).
 */
int external_output_setvolume(double vl, double vr)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_setvolume)
			return current_out_data.audio_setvolume(vl, vr);	

	return 0;
}


/*
 * get volume fraction.
 */
int external_output_getvolume(double* vl, double* vr)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getvolume)
			return current_out_data.audio_getvolume(vl, vr);	

	return 0;
}


/*
 * copy current buffer.
 */
int external_output_getcurrentbuffer(void* dout, unsigned long* dsize)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getcurrentbuffer)
			return current_out_data.audio_getcurrentbuffer(dout, dsize);	

	return 0;
}


/*
 * copy current buffer in float (between 0.0 and +/-32,768 :-( ).
 */
int external_output_getfloatbuffer(float* dout, unsigned long scount, unsigned long channel)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getfloatbuffer)
			return current_out_data.audio_getfloatbuffer(dout, scount, channel);	

	return 0;
}


/*
 * get data (frequency, channels).
 */
int external_output_getdata(unsigned int di, void* ret)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_getdata)
			return current_out_data.audio_getdata(di, ret);	

	return 0;
}


/*
 * toggle play/pause.
 */
int external_output_playpause(void)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_playpause)
			return current_out_data.audio_playpause();	

	return 0;
}


/*
 * do a preview action.
 */
int external_output_preview_action(int actionid, void *adata)
{
	if(!externalout_initialized)return 0;

	if(externalout_handle)
		if(current_out_data.audio_preview_action)
			return current_out_data.audio_preview_action(actionid, adata);	

	return 0;
}


/*-----------------------------------------------------------------------------
 fennec, july 2007.
-----------------------------------------------------------------------------*/

