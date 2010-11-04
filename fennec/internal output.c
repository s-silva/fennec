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
#include "fennec audio.h"
#include "plugins.h"
#include <time.h>
#include <math.h>




/* structs ------------------------------------------------------------------*/

struct internal_output_player
{
	int              initialized;                      /* player initialized */
	int              state;                            /* player state, playing/paused... */

	int              input_plugin;                     /* internal input plugin handle */
	unsigned long    stream_handle;                    /* internal input stream (file...) handle */

	unsigned long    samplerate, channels, bitdepth;   /* input format information */
	int              output_bitdepth;                  /* output bit depth */
	int              output_float;                     /* floating point output? */
	int              output_signed;                    /* signed? */


	unsigned long    buffersize;                       /* buffer size (output bitdepth related) */
	unsigned long    prebuffersize;                    /* prebuffer size (real bitdepth related) */

	char            *buffer;                           /* audio output buffer */
	char            *prebuffer;                        /* input buffer */
	char            *dsp_prebuffer;                    /* prebuffer used in DSP effect processing */

	unsigned long    dsp_prebuffer_length;             /* length of the dsp_prebuffer */
	unsigned long    dsp_prebuffer_clength;            /* current length (pointer like) of dsp_prebuffer */

	letter           current_file[v_sys_maxpath];      /* current file */

	double           volume[16];                       /* volume storage for maximum of 16 channels */

	int              loaded, stopping;
	uint64_t         written_samples;
	unsigned long    bufferpos;
	unsigned long    stop_at;
	unsigned long    stop_start_time;


	unsigned long    duration;

	t_sys_media_handle  audio_handle;                  /* audio output handle */
};





/* defines ------------------------------------------------------------------*/

#define upsampled_val(v)   (( (v) * player.bitdepth) / player.output_bitdepth)
#define downsampled_val(v) (( (v) * player.output_bitdepth) / player.bitdepth)





/* declarations -------------------------------------------------------------*/

sys_thread_function_header(thread_audio_engine);

int local_downsample(void* dout, void *idata, unsigned long dsize);
int local_readdata(unsigned long pid, unsigned long fid, unsigned long *dsize, void* odata, int posset, struct internal_output_player *pl);





/* data ---------------------------------------------------------------------*/

struct internal_output_player    player;
int                              internal_output_initialized = 0;
unsigned long                    dsp_prebuffer_size          = 32768;
int                              thread_audio_terminate      = 0;
t_sys_thread_handle              internal_output_thread_audio_engine;
t_sys_thread_share               internal_output_share;

/* code ---------------------------------------------------------------------*/



/*
 * initialize internal output engine.
 */
int internal_output_initialize(void)
{
	int i;

	if(internal_output_initialized)return 0;

	player.initialized   = 1;
	player.state         = v_audio_playerstate_notinit;
	player.input_plugin  = (unsigned long)fennec_invalid_index;
	player.stream_handle = (unsigned long)fennec_invalid_index;
	player.loaded        = 0;

	/* mute all */

	for(i=0; i<16; i++)
		player.volume[i] = settings.player.volume[i];

	/* allocate dsp prebuffer */

	player.dsp_prebuffer_length  = dsp_prebuffer_size;
	player.dsp_prebuffer_clength = 0;
	player.dsp_prebuffer         = (char*)sys_mem_alloc(player.dsp_prebuffer_length);

	/* however, the thread's going to call sharing routines */

	sys_thread_share_create(&internal_output_share);

	/* the thread should run! */

	thread_audio_terminate = 0;

	internal_output_thread_audio_engine = sys_thread_call((t_sys_thread_function) thread_audio_engine);

	/* set priority of the thread just created */

	sys_thread_setpriority(internal_output_thread_audio_engine, setting_priority_to_sys(settings.general.threads_priority));


	if(internal_output_thread_audio_engine == v_error_sys_thread_call)
	{
		/* multithreading is not supported, use the same thread (would this work?) */
		sys_thread_function_caller(thread_audio_engine);
	}

	/* well... */

	internal_output_initialized = 1;
	return 1;
}


/*
 * uninitialize internal output engine
 * and everything associated with it.
 */
int internal_output_uninitialize(void)
{
	if(!internal_output_initialized) return 0;

	internal_output_stop();

	player.state       = v_audio_playerstate_notinit;
	player.initialized = 0;

	/* happy holidays */

	sys_thread_share_request(&internal_output_share);
	thread_audio_terminate = 1;
	sys_thread_share_release(&internal_output_share);

	/* just wait awhile */

	while(thread_audio_terminate) sys_pass();

	/* clear the DSP buffer */

	if(player.dsp_prebuffer)
	{
		sys_mem_free(player.dsp_prebuffer);
		player.dsp_prebuffer = 0;
	}


	internal_output_initialized = 0;
	return 1;
}


/*
 * load file/stream.
 */
int internal_output_loadfile(const string spath)
{
	int perror_on_decoder = 1;

	/* history, oh, fading away */

	if(player.state != v_audio_playerstate_init)
		audio_stop();

	
	/* "this is my turn" */

	sys_thread_share_request(&internal_output_share);

	/* this state would be a message to the user */

	player.state = v_audio_playerstate_opening;


	/* load default values */

	player.output_bitdepth = settings.audio_output.bit_depth;
	player.output_float    = settings.audio_output.bit_depth_float;
	player.output_signed   = settings.audio_output.bit_depth_signed;


	/* select input plugin */

	player.input_plugin = audio_input_selectinput(spath);

	/* no! */

	if(player.input_plugin == fennec_invalid_index) goto point_error;


	/* initialize the plugin */

	audio_input_plugin_initialize(player.input_plugin);
	audio_input_plugin_loadfile(player.input_plugin, spath, &player.stream_handle);

	/* file corrupted? ain't implemented? or ... */

	if(player.stream_handle == fennec_invalid_index) goto point_error;

	/* cool to be remembered */

	str_cpy(player.current_file, spath);
	

	/* get format information */


	if(!audio_input_plugin_getformat(player.input_plugin, player.stream_handle, &player.samplerate, &player.bitdepth, &player.channels)) goto point_error;

	/* zero samples a second with zero channels and/or zero bit depth? what a cool sound! */

	if(!player.samplerate || !player.bitdepth || !player.channels) goto point_error;


	/* allocate memory */

	player.prebuffersize = (settings.audio_output.buffer_memory * player.samplerate * player.channels * (player.bitdepth / 8) ) / 1000;
	player.buffersize    = (settings.audio_output.buffer_memory * player.samplerate * player.channels * (player.output_bitdepth/ 8) ) / 1000;

	if(!player.buffersize || !player.prebuffersize) goto point_error;

	player.prebuffer = (char*) sys_mem_alloc(player.prebuffersize);
	player.buffer    = (char*) sys_mem_alloc(player.buffersize);
	
	/* now we've done with decoder stuff */

	perror_on_decoder = 0;

	/* start working with audio output system */

	
	{
		unsigned int devi = v_sys_media_driver_default;

		if(settings.audio_output.output_device_id != -1 && settings.audio_output.output_device_id != 0)
			devi = settings.audio_output.output_device_id - 1; /* cuz the default device replaces 0 */

		player.audio_handle = sys_media_stream_init(devi, player.samplerate, player.channels, player.output_bitdepth);
	
		if(player.audio_handle == v_error_sys_media_stream_init) goto point_error;
	
	}

	/* reset equalizer history */

	equalizer_reset();

	/* the state (remember, the message) */

	player.state           = v_audio_playerstate_loaded;
	player.loaded          = 1;
	player.written_samples = 0;
	player.bufferpos       = 0;
	player.stopping        = 0;

	{
		unsigned long  rlen = player.prebuffersize;

		local_readdata(player.input_plugin, player.stream_handle, &rlen, player.prebuffer, 1, &player);
		local_downsample(player.buffer, player.prebuffer, rlen);
	}

	sys_thread_share_release(&internal_output_share);

	return 1;


	/* - - - - - - - - - - - - - - - */

point_error:

	if(!perror_on_decoder)
		MessageBox(0, text(oooo_e_files_invalid_format), text(oooo_error), MB_ICONEXCLAMATION);

	sys_thread_share_release(&internal_output_share);
	return 0;
}


/*
 * start playback.
 */
int internal_output_play(void)
{
	if(!player.initialized) return 0;

	if(player.loaded)
	{
		if(player.state != v_audio_playerstate_paused)
			sys_media_stream_write(player.audio_handle, player.buffer, player.buffersize, v_sys_media_infinite_repeats);		
	
		sys_media_stream_play(player.audio_handle);

	}else{
		internal_output_loadfile(player.current_file);

		if(player.loaded)
		{
			if(player.state != v_audio_playerstate_paused)
				sys_media_stream_write(player.audio_handle, player.buffer, player.buffersize, v_sys_media_infinite_repeats);		
			
			sys_media_stream_play(player.audio_handle);

		}else{

			return 0;
		}
	}

	player.state = v_audio_playerstate_playing;

	return 1;
}


/*
 * pause playback.
 */
int internal_output_pause(void)
{
	if(player.state == v_audio_playerstate_playing)
		sys_media_stream_pause(player.audio_handle);

	player.state = v_audio_playerstate_paused;
	return 1;
}


/*
 * toggle play/pause.
 */
int internal_output_playpause(void)
{
	if(player.state == v_audio_playerstate_playing)
	{
		internal_output_pause();
	}else{
		internal_output_play();
	}

	return 1;
}


/*
 * unload file/stream.
 */
int internal_output_unload(void)
{
	return 1;
}


/*
 * stop playback.
 */
int internal_output_stop(void)
{
	if(player.input_plugin != fennec_invalid_index)
	{
		sys_thread_share_request(&internal_output_share);

		player.state = v_audio_playerstate_stopped;

		if(player.loaded)
		{
			if(player.audio_handle != v_error_sys_media_main_init)
			{
				sys_media_stream_clear(player.audio_handle);
				sys_media_stream_uninit(player.audio_handle);
			}
		}

		player.loaded = 0;

		if(player.stream_handle != fennec_invalid_index)
			audio_input_plugin_unloadfile(player.input_plugin, player.stream_handle);
	
		player.stream_handle = (unsigned long)fennec_invalid_index;
		player.input_plugin  = (unsigned long)fennec_invalid_index;

		sys_mem_free(player.prebuffer);
		sys_mem_free(player.buffer);

		player.state = v_audio_playerstate_stopped;
		
		sys_thread_share_release(&internal_output_share);
	}
	return 1;
}


/*
 * set volume fraction.
 */
int internal_output_setvolume(double lvol, double rvol)
{
	player.volume[0] = lvol;
	player.volume[1] = rvol;

	settings.player.volume[0] = lvol;
	settings.player.volume[1] = rvol;
	return 1;
}


/*
 * get info!.
 */
int internal_output_getdata(unsigned int id, void* ret)
{
	switch(id)
	{
	case audio_v_data_frequency:
		memcpy(ret, &player.samplerate, sizeof(int));
		break;

	case audio_v_data_bitspersample:
		memcpy(ret, &player.buffer, sizeof(int));
		break;

	case audio_v_data_channels:
		memcpy(ret, &player.channels, sizeof(int));
		break;

	case audio_v_data_ids_pid:
		return player.input_plugin;

	case audio_v_data_ids_fid:
		return player.stream_handle;

	default:
		return 0;
	}
	return 1;
}


/*
 * get volume.
 */
int internal_output_getvolume(double* lvol, double* rvol)
{
	if(!internal_output_initialized) return 0;

	*lvol = player.volume[0];
	*rvol = player.volume[1];
	return 1;
}


/*
 * set position fraction.
 * where most of the attention is needed :-)
 */
int internal_output_setposition(double cpos)
{
	audio_input_plugin_setposition(player.input_plugin, player.stream_handle, cpos);
	player.written_samples = (int64_t)(((double)player.duration * (double)player.channels * (double)player.samplerate / 1000.0) * cpos);
	return 1;
}


/*
 * set position in milliseconds.
 */
int internal_output_setposition_ms(double mpos)
{
	if(player.duration)
		return internal_output_setposition(mpos / (double)player.duration);
	else
		return 0;
}


/*
 * get position fraction.
 */
double internal_output_getposition(void)
{
	if(player.duration)
		return internal_output_getposition_ms() / (double)player.duration;
	else
		return 0.0;
}


/*
 * get position in milliseconds.
 */
unsigned long internal_output_getposition_ms(void)
{
	if(!internal_output_initialized || !player.loaded) return 0;

	return (unsigned long)((double)(player.written_samples / player.channels) / ((double)player.samplerate / 1000.0));
}


/*
 * get duration in milliseconds.
 */
unsigned long  internal_output_getduration_ms(void)
{
	if(!internal_output_initialized || !player.loaded) return 0;

	return player.duration;
}


/*
 * get current player state.
 */
int internal_output_getplayerstate(void)
{
	if(!internal_output_initialized) return v_audio_playerstate_notinit;
	return player.state;
}


/*
 * copy currnetly playing buffer to a requested memory buffer.
 */
int internal_output_getcurrentbuffer(void* dout, unsigned long* dsize)
{

	return 1;
}


/*
 */
int internal_output_getfloatbuffer(float* dout, unsigned long scount, unsigned long channel)
{
	unsigned long     npos, i, j, byps;

	if(player.state != v_audio_playerstate_playing) return 0;

	npos  = sys_media_stream_buffertimer_byte(player.audio_handle);

	byps = player.bitdepth / 8;

	if(player.bitdepth == 64){

		double *buf = (double*)player.prebuffer;

		for(i=0; i<scount; i++)
		{
			j = (i * player.channels) + (channel - 1) + (upsampled_val(npos) / byps);
			j %= (player.prebuffersize / byps);

			dout[i] = (float)buf[j];
		}

		return 1;
	}

	return 0;
}


/*
 * return peak power value between 10,000.
 */
int internal_output_getpeakpower(unsigned long* pleft, unsigned long* pright)
{
	return 0;
}

/* thread -------------------------------------------------------------------*/


/*
 */
sys_thread_function_header(thread_audio_engine)
{
	unsigned long   npos, rlen = 0, ret = 0;


	for(;;)
	{
		sys_thread_share_request(&internal_output_share);

		if(thread_audio_terminate)
		{
			sys_thread_share_release(&internal_output_share);
			sys_thread_share_close(&internal_output_share);
			thread_audio_terminate = 0;
			break;
		}

		if(!player.loaded || player.state != v_audio_playerstate_playing)
		{
			sys_thread_share_release(&internal_output_share);
			sys_sleep(10);
			continue;
		}


		npos  = sys_media_stream_buffertimer_byte(player.audio_handle);
		npos %= player.buffersize;

		if(player.stopping)
		{
			unsigned long btime = player.prebuffersize / (((player.bitdepth / 8) * player.channels * player.samplerate) / 1000);

			if((sys_timer_getms() - player.stop_start_time > btime))
			{
				sys_thread_share_release(&internal_output_share);
				
				if(settings.player.playlist_repeat_single)
				{
					internal_output_setposition(0.0);
				}else{
					internal_output_stop();
					if(settings.player.auto_switching)
						audio_playlist_next();
				}

				continue;
			}
		}


		/* write buffers */


		if(npos)
		{
			if(npos > player.bufferpos)
			{
				rlen = upsampled_val(npos - player.bufferpos);

				if(!player.stopping)
					ret = local_readdata(player.input_plugin, player.stream_handle, &rlen, player.prebuffer + upsampled_val(player.bufferpos), 0, &player);
				else
					memset(player.prebuffer + upsampled_val(player.bufferpos), 0, rlen);
				
				local_downsample(player.buffer + player.bufferpos, player.prebuffer + upsampled_val(player.bufferpos), rlen);
				player.bufferpos += downsampled_val(rlen);

				if((!rlen || !ret) && !player.stopping)
				{
					player.stopping        = 1;
					player.stop_at         = player.bufferpos;
					player.stop_start_time = sys_timer_getms();

					goto stop_playback;
				}
				

			}else if(npos < player.bufferpos){
				if(player.bufferpos < player.buffersize)
				{
					rlen = upsampled_val(player.buffersize - player.bufferpos);

					if(rlen > 0)
					{
						if(!player.stopping)
							ret = local_readdata(player.input_plugin, player.stream_handle, &rlen, player.prebuffer + upsampled_val(player.bufferpos), 0, &player);
						else
							memset(player.prebuffer + upsampled_val(player.bufferpos), 0, rlen);

						local_downsample(player.buffer + player.bufferpos, player.prebuffer + upsampled_val(player.bufferpos), rlen);
						
						if((!rlen || !ret) && !player.stopping)
						{
							player.stopping        = 1;
							player.stop_at         = player.bufferpos;
							player.stop_start_time = sys_timer_getms();

							goto stop_playback;
						}
					}
				}
				

				rlen = upsampled_val(npos);

				player.bufferpos = 0;

				if(rlen > 0)
				{
					if(!player.stopping)
						ret = local_readdata(player.input_plugin, player.stream_handle, &rlen, player.prebuffer, 0, &player);
					else
						memset(player.prebuffer, 0, rlen);

					local_downsample(player.buffer, player.prebuffer, rlen);
				}

				player.bufferpos = downsampled_val(rlen);

				if((!rlen || !ret) && !player.stopping)
				{
					player.stopping        = 1;
					player.stop_at         = player.bufferpos;
					player.stop_start_time = sys_timer_getms();

					goto stop_playback;
				}

				player.duration = audio_input_plugin_getduration_ms(player.input_plugin, player.stream_handle);
					
			}
		}



stop_playback:

		sys_thread_share_release(&internal_output_share);

		sys_sleep(10);
	}

	sys_thread_function_return();
}










int local_downsample(void* dout, void *idata, unsigned long dsize)
{
	fennec_sample     *ifdata = (fennec_sample*)idata, vol;
	unsigned long      i, msize = dsize / sizeof(fennec_sample);


	vol = pow(player.volume[0], 2);

	/* downsampling */

	if(player.output_bitdepth == 16)
	{

		/* 16 bit signed output */

		short    *sout = (short*)dout;
		double    mval = 32767.0, din;
		short     dout;
		uint16_t  cw;

		/* set rounding mode to nearst */

		__asm fnstcw cw
		cw |= ~((uint16_t)(3 << 10));
		__asm fldcw cw


		/* convert */

		for(i=0; i<msize; i++)
		{
			din = (double)ifdata[i] * vol;

			/* round to the nearst value */

			__asm
			{
				fld   din
				fmul  mval
				fistp dout
			}

			sout[i] = dout;
		}

	}else if(player.output_bitdepth == 8){

		if(!player.output_signed)
		{
			/* 8 bit signed output */

			uint8_t *sout = (uint8_t*)dout;
			double   mval = 127.0, din;
			int32_t  dout;
			uint16_t cw;

			/* set rounding mode to nearst */

			__asm fnstcw cw
			cw |= ~((uint16_t)(3 << 10));
			__asm fldcw cw

			/* convert */

			for(i=0; i<msize; i++)
			{
				din = (double)ifdata[i] * vol;

				/* round to the nearst value */

				__asm
				{
					fld   din
					fmul  mval
					fistp dout
				}

				sout[i] = (uint8_t)(dout + 127);
			}
		}else{

			/* 8 bit unsigned output */


			int8_t *sout = (int8_t*)dout;

			for(i=0; i<msize; i++)
			{
				sout[i] = (int8_t)(ifdata[i] * vol * 127.0);
			}
		}
	}
	return 1;
}


int local_readdata(unsigned long pid, unsigned long fid, unsigned long *dsize, void* odata, int posset, struct internal_output_player *pl)
{
	unsigned long tmp_dpointer = 0;
	unsigned long tmp_rsize;
	int           tmp_retval   = 1, lastfill = 0;
	unsigned long tmp_bsize;

	if(!settings.dsp_plugins.enable)
	{
		tmp_retval = audio_input_plugin_readdata(pid, fid, dsize, odata);

		equalizer_equalize(odata, 0, pl->channels, pl->samplerate, pl->bitdepth, *dsize);
		
		pl->written_samples += *dsize / (pl->bitdepth / 8);
		return tmp_retval;
	}

	tmp_rsize = *dsize;

	if(posset)
		lastfill = 0;

	if(pl->dsp_prebuffer_clength && !posset)
	{
		tmp_dpointer = pl->dsp_prebuffer_clength;
	}

	for(;;)
	{
		if(pl->dsp_prebuffer_length < tmp_rsize)
		{
			pl->dsp_prebuffer_length = tmp_rsize;
			pl->dsp_prebuffer = (char*) sys_mem_realloc(pl->dsp_prebuffer, pl->dsp_prebuffer_length);
		}


		if(tmp_dpointer >= tmp_rsize)
		{
			memcpy(odata, pl->dsp_prebuffer, tmp_rsize);

			if((tmp_dpointer > tmp_rsize) && !posset)
			{
				pl->dsp_prebuffer_clength = tmp_dpointer - tmp_rsize;
				memcpy(pl->dsp_prebuffer, pl->dsp_prebuffer + tmp_rsize, pl->dsp_prebuffer_clength);
			}else{
				pl->dsp_prebuffer_clength = 0;
			}

			if(!posset)
				tmp_dpointer = pl->dsp_prebuffer_clength;
			break;
		}else{
			if(tmp_dpointer && pl->dsp_prebuffer_clength)
			{
				memcpy(odata, pl->dsp_prebuffer, pl->dsp_prebuffer_clength);
				tmp_rsize -= pl->dsp_prebuffer_clength;
				odata = ((char*)odata) + pl->dsp_prebuffer_clength;
				pl->dsp_prebuffer_clength = 0;
				tmp_dpointer = 0;
			}
		}

		if(lastfill)
		{
			memcpy(odata, pl->dsp_prebuffer, tmp_dpointer);
			*dsize = tmp_dpointer;
			break;
		}

		tmp_bsize    =  tmp_rsize;
		tmp_retval   =  audio_input_plugin_readdata(pid, fid, &tmp_bsize, pl->dsp_prebuffer + tmp_dpointer);
		

		equalizer_equalize(pl->dsp_prebuffer + tmp_dpointer, 0, pl->channels, pl->samplerate, pl->bitdepth, tmp_rsize);
		
		if(!posset)
			pl->written_samples += tmp_rsize / (pl->bitdepth / 8);

		pl->dsp_prebuffer = (char*)dsp_process(0, &tmp_bsize, pl->samplerate, pl->bitdepth, pl->channels, pl->dsp_prebuffer, tmp_dpointer, pl->dsp_prebuffer_length);

		if(tmp_bsize > pl->dsp_prebuffer_length)pl->dsp_prebuffer_length = tmp_bsize;

		tmp_dpointer = tmp_dpointer + tmp_bsize;

		if((tmp_retval == fennec_input_nodata) || !tmp_retval || !tmp_rsize)
		{
			lastfill = 1;
		}
	}

	if(lastfill)
	{
		return 0;
	}else{
		return 1;
	}
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
