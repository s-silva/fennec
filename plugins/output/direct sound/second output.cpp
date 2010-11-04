#include "main.h"

struct output_player   players[1];
int                    players_count = 1;

#define max_players_count 1
#define def_output_bps 16

/*
 * Initialize function will initialize the playback engine,
 * similarly 'uninitialize' will do the opposite (+ stopping
 * all loaded players). To initialize a player, use load function
 * ('stop' will do the unloading stuff).
 *
 * Playing multiple files simultaneously is not guaranteed cuz some
 * input plug-ins won't allow this.
 */


int subengine_initialize(void)
{
	int i;

	for(i=0; i<max_players_count; i++)
	{
		if(i == 0) /* preview */
		{
			players[i].engine.audio_out_default = preview_audio_out_default;
			memcpy(&players[i].engine.audio_out_device, &preview_audio_out_device, sizeof(GUID));

		}else{

			players[i].engine.audio_out_default = 1;
		}

		players[i].engine.window_audio = 0;
		players[i].engine.thread_wlpos = 0;

		players[i].audio.initialized = 1;
		players[i].audio.loaded      = 0;
		players[i].audio.state       = v_audio_playerstate_notinit;
		players[i].audio.ds          = 0;

		players[i].audio.input_plugin  = fennec_invalid_index;
		players[i].audio.stream_handle = fennec_invalid_index;
	}
	return 0;
}


int subengine_uninitialize(void)
{
	int i;

	for(i=0; i<max_players_count; i++)
	{
		if(players[i].audio.loaded)
			subengine_stop(i);
	}
	return 0;
}


int subengine_load(int pid, const string spath)
{
	HRESULT hr = DS_OK;
	int     perror_on_decoder = 1; /* error on decoder, not the format */


	if(players[pid].audio.loaded)
		subengine_stop(pid);

	players[pid].audio.input_plugin = sfennec.audio.input.selectinput(spath);


	sfennec.audio.input.plugins.initialize(players[pid].audio.input_plugin);
	sfennec.audio.input.plugins.loadfile(players[pid].audio.input_plugin, spath, &players[pid].audio.stream_handle);

	if(players[pid].audio.stream_handle == fennec_invalid_index) goto point_error;

	str_cpy(players[pid].audio.current_file, spath);


	if(!sfennec.audio.input.plugins.getformat(players[pid].audio.input_plugin, players[pid].audio.stream_handle, &players[pid].audio.frequency, &players[pid].audio.bps, &players[pid].audio.channels)) goto point_error;

	if(!players[pid].audio.frequency || !players[pid].audio.bps || !players[pid].audio.channels) goto point_error;


	players[pid].audio.outchannels = players[pid].audio.channels;


	
	players[pid].audio.prebuffersize = (sfennec.settings.general->audio_output.buffer_memory * players[pid].audio.frequency * players[pid].audio.channels * (players[pid].audio.bps / 8) ) / 1000;
	if(!players[pid].audio.prebuffersize) goto point_error;

	players[pid].audio.prebuffer = (char*) sys_mem_alloc(players[pid].audio.prebuffersize + 100);

	players[pid].audio.buffersize = (sfennec.settings.general->audio_output.buffer_memory * players[pid].audio.frequency * players[pid].audio.channels * (def_output_bps / 8) ) / 1000;

	/* now we've done with decoder stuff */

	perror_on_decoder = 0;

	/* direct sound */

	if(players[pid].engine.audio_out_default)
	{
		if(DirectSoundCreate(0, &(players[pid].audio.ds), 0)) goto point_error;
	}else{
		if(DirectSoundCreate(&players[pid].engine.audio_out_device, &(players[pid].audio.ds), 0)) goto point_error;
	}

	memset(&players[pid].audio.wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));

	players[pid].audio.wfx.Format.cbSize          = sizeof(WAVEFORMATEXTENSIBLE);
	players[pid].audio.wfx.Format.nSamplesPerSec  = players[pid].audio.frequency;
	players[pid].audio.wfx.Format.wBitsPerSample  = (WORD)def_output_bps;
	players[pid].audio.wfx.Format.nChannels       = (WORD)players[pid].audio.outchannels;
	players[pid].audio.wfx.Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
	players[pid].audio.wfx.Format.nAvgBytesPerSec = players[pid].audio.frequency * players[pid].audio.outchannels * (def_output_bps / 8);
	players[pid].audio.wfx.Format.nBlockAlign     = (WORD)(players[pid].audio.outchannels * (def_output_bps / 8));

	players[pid].audio.wfx.Samples.wReserved           = 0;
	players[pid].audio.wfx.Samples.wSamplesPerBlock    = 0;
	players[pid].audio.wfx.Samples.wValidBitsPerSample = def_output_bps;

	players[pid].audio.wfx.dwChannelMask = (1 << players[pid].audio.outchannels) - 1;


	if(players[pid].audio.output_float)
		players[pid].audio.wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ;
	else
		players[pid].audio.wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	
	memset(players[pid].audio.nrc, 0, sizeof(players[pid].audio.nrc));

	memset(&players[pid].audio.dsb, 0, sizeof(DSBUFFERDESC));

    players[pid].audio.dsb.dwSize        = sizeof(DSBUFFERDESC);
    players[pid].audio.dsb.dwFlags       = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    players[pid].audio.dsb.dwBufferBytes = players[pid].audio.buffersize;
    players[pid].audio.dsb.lpwfxFormat   = (LPWAVEFORMATEX)&players[pid].audio.wfx;


	hr = players[pid].audio.ds->SetCooperativeLevel(window_audio, DSSCL_PRIORITY);
	
	if(!SUCCEEDED(hr)) goto point_error;

	hr = players[pid].audio.ds->CreateSoundBuffer(&players[pid].audio.dsb, &players[pid].audio.buffer, 0);

	if(!SUCCEEDED(hr)) goto point_error;

	players[pid].audio.written_samples = 0;
	players[pid].audio.dur_ms          = sfennec.audio.input.plugins.getduration_ms(players[pid].audio.input_plugin, players[pid].audio.stream_handle);

	players[pid].audio.loaded = 1;
	players[pid].audio.state = v_audio_playerstate_loaded;

	players[pid].engine.thread_wlpos     = 0;
	players[pid].audio.stopping = 0;

	// audio_getduration_ms(); /* set cached value */

	{
		unsigned long  blen;
		void          *bufferst;

		if(players[pid].audio.buffer->Lock(0, 0, &bufferst, &blen, 0, 0, DSBLOCK_ENTIREBUFFER) == DS_OK)
		{
			unsigned long        rlen = players[pid].audio.prebuffersize;
			
			//local_readdata(players[pid].audio.input_plugin, players[pid].audio.stream_handle, &rlen, players[pid].audio.prebuffer, 1, &players[pid].audio);
			//local_downsample(bufferst, splayer.prebuffer, rlen);
			sfennec.audio.input.plugins.readdata(players[pid].audio.input_plugin, players[pid].audio.stream_handle, &rlen, players[pid].audio.prebuffer);
			local_def_downsample(bufferst, players[pid].audio.prebuffer, rlen);


			players[pid].audio.written_samples += rlen / (players[pid].audio.bps / 8);

			players[pid].audio.buffer->Unlock(bufferst, blen, 0, 0);
		}
	}

	return 1;

point_error:
	if(!perror_on_decoder)
	{
		if(hr == DSERR_NODRIVER)
		{
			MessageBox(0, sfennec.language_text[oooo_e_audio_no_device], sfennec.language_text[oooo_error], MB_ICONEXCLAMATION);
		}else if(hr != DS_OK){
			MessageBox(0, sfennec.language_text[oooo_e_files_invalid_format], sfennec.language_text[oooo_error], MB_ICONEXCLAMATION);
		}
	}else{
	}
	return 0;
}


int subengine_play(int pid)
{
	if(!players[pid].audio.initialized) return 0;

	if(players[pid].audio.loaded)
	{
		players[pid].audio.buffer->Play(0, 0, DSBPLAY_LOOPING);
	}else{
		subengine_load(pid, players[pid].audio.current_file);
		if(players[pid].audio.loaded)
			players[pid].audio.buffer->Play(0, 0, DSBPLAY_LOOPING);
		else
			return 0;
	}

	players[pid].audio.state = v_audio_playerstate_playing;
	return 0;
}


int subengine_pause(int pid)
{
	if(!players[pid].audio.initialized) return 0;

	if(players[pid].audio.loaded)
	{
		players[pid].audio.buffer->Stop();
	}

	players[pid].audio.state = v_audio_playerstate_paused;
	return 0;
}


int subengine_stop(int pid)
{
	if(players[pid].audio.input_plugin != fennec_invalid_index)
	{
		//EnterCriticalSection(&cst);

		players[pid].audio.state = v_audio_playerstate_stopped;

		if(players[pid].audio.loaded)
		{
			players[pid].audio.buffer->Stop();
			players[pid].audio.buffer->Release();
		}

		if(players[pid].audio.ds)
			players[pid].audio.ds->Release();

		players[pid].audio.loaded = 0;

		if(players[pid].audio.stream_handle != fennec_invalid_index)
			sfennec.audio.input.plugins.unloadfile(players[pid].audio.input_plugin, players[pid].audio.stream_handle);
	
		players[pid].audio.stream_handle = fennec_invalid_index;
		players[pid].audio.input_plugin = fennec_invalid_index;

		sys_mem_free(players[pid].audio.prebuffer);

		players[pid].audio.state = v_audio_playerstate_stopped;
		
		//LeaveCriticalSection(&cst);
	}
	return 1;
}


int subengine_seek(int pid, double pos)
{
	sfennec.audio.input.plugins.setposition(players[pid].audio.input_plugin, players[pid].audio.stream_handle, pos);
	players[pid].audio.written_samples = (int64_t)(((double)players[pid].audio.dur_ms * (double)players[pid].audio.channels * (double)players[pid].audio.frequency / 1000.0) * pos);

	return 0;
}


double subengine_getduration(int pid)
{
	return 0;
}


double subengine_getposition(int pid)
{
	if(!players[pid].audio.initialized)return 0.0;
	if(!players[pid].audio.loaded)return 0.0;

	if(players[pid].audio.dur_ms)
		return ( (double)(players[pid].audio.written_samples / players[pid].audio.channels) / ((double)players[pid].audio.frequency / 1000.0) ) / (double)players[pid].audio.dur_ms;
	else
		return 0.0;

	return 0.0;
}


/*
 * cid - channel id (this time, don't wanna limit this to stereo).
 * -1 = all
 */
double subengine_getvolume(int pid, int cid)
{
	return 0;
}


int subengine_setvolume(int pid, int cid, double vol)
{
	return 0;
}




int subengine_thread(int tdata)
{
	void             *bufferst;
	unsigned long     blen;
	int               pid;

#define def_upsampled_val(v)   (( (v) * players[pid].audio.bps) / def_output_bps)
#define def_downsampled_val(v) (( (v) * def_output_bps) / players[pid].audio.bps)

	for(pid=0; pid<max_players_count; pid++)
	{
		//for(;;)
		{
			//EnterCriticalSection(&cst);

			//if(thread_audio_terminate)
			//{
			//	LeaveCriticalSection(&cst);
			//	DeleteCriticalSection(&cst);
			//	thread_audio_terminate = 0;
			//	break;
			//}
			if(!players[pid].audio.loaded) goto point_epl;

			if(players[pid].audio.buffer->Lock(0, 0, &bufferst, &blen, 0, 0, DSBLOCK_ENTIREBUFFER) == DS_OK)
			{
				unsigned long        npos, rlen = 0, ret = 0;

				players[pid].audio.buffer->GetCurrentPosition(&npos, 0);

				if(players[pid].audio.stopping)
				{
					unsigned long btime = players[pid].audio.prebuffersize / (((players[pid].audio.bps / 8) * players[pid].audio.channels * players[pid].audio.frequency) / 1000);
					unsigned long ntime = timeGetTime();

					if(ntime > players[pid].audio.stop_start_time + btime)
					{
						//LeaveCriticalSection(&cst);

						subengine_stop(pid);

						goto point_eepl;
					}
				}






				if(npos)
				{
					if(npos > players[pid].engine.thread_wlpos)
					{
						rlen = def_upsampled_val(npos - players[pid].engine.thread_wlpos);

						if(!players[pid].audio.stopping)
							ret = local_def_readdata(pid, players[pid].audio.input_plugin, players[pid].audio.stream_handle, &rlen, players[pid].audio.prebuffer + def_upsampled_val(players[pid].engine.thread_wlpos));
						else
							memset(players[pid].audio.prebuffer + def_upsampled_val(players[pid].engine.thread_wlpos), 0, rlen);
						
						local_def_downsample((char*)bufferst + players[pid].engine.thread_wlpos, players[pid].audio.prebuffer + def_upsampled_val(players[pid].engine.thread_wlpos), rlen);
						players[pid].engine.thread_wlpos += def_downsampled_val(rlen);

						if((!rlen || !ret) && !players[pid].audio.stopping)
						{
							players[pid].audio.stopping        = 1;
							players[pid].audio.stop_at         = players[pid].engine.thread_wlpos;
							players[pid].audio.stop_start_time = timeGetTime();
							goto stop_playback;
						}
						

					}else if(npos < players[pid].engine.thread_wlpos){
						if(players[pid].engine.thread_wlpos < players[pid].audio.buffersize)
						{
							rlen = def_upsampled_val(players[pid].audio.buffersize - players[pid].engine.thread_wlpos);

							if(rlen > 0)
							{
								if(!players[pid].audio.stopping)
									ret = local_def_readdata(pid, players[pid].audio.input_plugin, players[pid].audio.stream_handle, &rlen, players[pid].audio.prebuffer + def_upsampled_val(players[pid].engine.thread_wlpos));
								else
									memset(players[pid].audio.prebuffer + def_upsampled_val(players[pid].engine.thread_wlpos), 0, rlen);

								local_def_downsample((char*)bufferst + players[pid].engine.thread_wlpos, players[pid].audio.prebuffer + def_upsampled_val(players[pid].engine.thread_wlpos), rlen);
								
								if((!rlen || !ret) && !players[pid].audio.stopping)
								{
									players[pid].audio.stopping        = 1;
									players[pid].audio.stop_at         = players[pid].engine.thread_wlpos;
									players[pid].audio.stop_start_time = timeGetTime();
									goto stop_playback;
								}
							}
						}
						

						rlen = def_upsampled_val(npos);

						players[pid].engine.thread_wlpos = 0;

						if(rlen > 0)
						{
							if(!players[pid].audio.stopping)
								ret = local_def_readdata(pid, players[pid].audio.input_plugin, players[pid].audio.stream_handle, &rlen, players[pid].audio.prebuffer);
							else
								memset(players[pid].audio.prebuffer, 0, rlen);

							local_def_downsample(bufferst, players[pid].audio.prebuffer, rlen);
						}

						players[pid].engine.thread_wlpos = def_downsampled_val(rlen);

						if((!rlen || !ret) && !players[pid].audio.stopping)
						{
							players[pid].audio.stopping        = 1;
							players[pid].audio.stop_at         = players[pid].engine.thread_wlpos;
							players[pid].audio.stop_start_time = timeGetTime();
							goto stop_playback;
						}
					}
				}


	stop_playback:
				players[pid].audio.buffer->Unlock(bufferst, blen, 0, 0);
				
			}

	point_epl:
			//LeaveCriticalSection(&cst);

			//Sleep(5);

	point_eepl:;
		}
	}

	return 0;
}

int local_def_readdata(int playerid, unsigned long pid, unsigned long fid, unsigned long *dsize, void* odata)
{
	int tmp_retval   = 1;

	tmp_retval = sfennec.audio.input.plugins.readdata(pid, fid, dsize, odata);

	players[playerid].audio.written_samples += *dsize / (players[playerid].audio.bps / 8);
	return tmp_retval;
}