#include "main.h"

GUID                 audio_out_device;
int                  audio_out_default = 1;
GUID                 preview_audio_out_device;
int                  preview_audio_out_default = 1;

struct audioplayer   splayer;
struct fennec        sfennec;         /* fennec shared functions */
CRITICAL_SECTION     cst;
HWND                 window_audio = 0;
HANDLE               hthread_audip;
HINSTANCE            plugin_instance;

int                  thread_audio_terminate;
unsigned long        thread_wlpos = 0;
int                  device_combo_index = 0;
int                  preview_device_combo_index = 0;

/*
 * initialize plug-in.
 */
unsigned long __stdcall fennec_initialize_output(struct general_output_data *gin, string pname)
{
	memcpy(&sfennec, gin->shared, sizeof(struct fennec));

    gin->settings                 = show_settings;
    gin->about                    = show_about;
    gin->audio_initialize         = audio_initialize;
    gin->audio_uninitialize       = audio_uninitialize;
    gin->audio_load               = audio_load;
    gin->audio_add                = audio_add;
    gin->audio_play               = audio_play;
    gin->audio_playpause          = audio_playpause;
    gin->audio_pause              = audio_pause;
    gin->audio_stop               = audio_stop;
    gin->audio_setposition        = audio_setposition;
    gin->audio_setposition_ms     = audio_setposition_ms;
    gin->audio_getplayerstate     = audio_getplayerstate;
    gin->audio_getposition        = audio_getposition;
    gin->audio_getposition_ms     = audio_getposition_ms;
    gin->audio_getduration_ms     = audio_getduration_ms;
    gin->audio_getpeakpower       = audio_getpeakpower;
    gin->audio_setvolume          = audio_setvolume;
    gin->audio_getvolume          = audio_getvolume;
    gin->audio_getcurrentbuffer   = audio_getcurrentbuffer;
    gin->audio_getfloatbuffer     = audio_getfloatbuffer;
    gin->audio_getdata            = audio_getdata;
	gin->audio_preview_action     = audio_preview_action;

	audio_initialize();
	subengine_initialize();
	return 1;
}


/*
 * show settings dialog.
 */
int show_settings(void* vdata)
{
	DialogBox(plugin_instance, MAKEINTRESOURCE(dialog_settings), (HWND)vdata, DialogProc);
	return 1;
}


/*
 * show about dialog.
 */
int show_about(void* vdata)
{
	MessageBox(0, uni("DirectX audio output plug-in for Fennec Player."), uni("About"), MB_ICONINFORMATION);
	return 1;
}


/*
 * initialize buffers and devices.
 */
int audio_initialize(void)
{
	DWORD  tp = 0, ti;

	if(!window_audio)
		window_audio = CreateWindow(uni("STATIC"), uni("window audio - fennec ds"), WS_POPUP, 0, 0, 0, 0, 0, 0, GetModuleHandle(0), 0);

	splayer.initialized = 1;
	splayer.loaded      = 0;
	splayer.state       = v_audio_playerstate_notinit;
	splayer.ds          = 0;

	splayer.input_plugin  = fennec_invalid_index;
	splayer.stream_handle = fennec_invalid_index;

	audio_setvolume(sfennec.settings.general->player.volume[0], sfennec.settings.general->player.volume[1]);

	splayer.public_prebuffer_length  = public_prebuffer_size;
	splayer.public_prebuffer_clength = 0;
	sfennec.call_function(call_fennec_memory_alloc, splayer.public_prebuffer_length, 0, (void*)&splayer.public_prebuffer) ;

	InitializeCriticalSection(&cst);
	

	thread_audio_terminate = 0;

	hthread_audip = CreateThread(0, 0, thread_audio, &tp, 0, &ti);
	SetThreadPriority(hthread_audip, setting_priority_to_sys(sfennec.settings.general->general.threads_priority));


	CoInitializeEx(0, COINIT_APARTMENTTHREADED );


	/* get last device GUID */
	{
		audio_out_default = sfennec.call_function(load_setting_int, 1 /* default */, (void*)"output.dx.device-default", 0);

		audio_out_device.Data1 = sfennec.call_function(load_setting_int, 0, (void*)"output.dx.guid1", 0);
		audio_out_device.Data2 = sfennec.call_function(load_setting_int, 0, (void*)"output.dx.guid2", 0);
		audio_out_device.Data3 = sfennec.call_function(load_setting_int, 0, (void*)"output.dx.guid3", 0);

		audio_out_device.Data4[0] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.0", 0);
		audio_out_device.Data4[1] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.1", 0);
		audio_out_device.Data4[2] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.2", 0);
		audio_out_device.Data4[3] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.3", 0);
		audio_out_device.Data4[4] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.4", 0);
		audio_out_device.Data4[5] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.5", 0);
		audio_out_device.Data4[6] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.6", 0);
		audio_out_device.Data4[7] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.guid4.7", 0);


		/* preview */

		preview_audio_out_default = sfennec.call_function(load_setting_int, 1 /* default */, (void*)"output.dx.preview.device-default", 0);

		preview_audio_out_device.Data1 = sfennec.call_function(load_setting_int, 0, (void*)"output.dx.preview.guid1", 0);
		preview_audio_out_device.Data2 = sfennec.call_function(load_setting_int, 0, (void*)"output.dx.preview.guid2", 0);
		preview_audio_out_device.Data3 = sfennec.call_function(load_setting_int, 0, (void*)"output.dx.preview.guid3", 0);

		preview_audio_out_device.Data4[0] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.0", 0);
		preview_audio_out_device.Data4[1] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.1", 0);
		preview_audio_out_device.Data4[2] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.2", 0);
		preview_audio_out_device.Data4[3] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.3", 0);
		preview_audio_out_device.Data4[4] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.4", 0);
		preview_audio_out_device.Data4[5] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.5", 0);
		preview_audio_out_device.Data4[6] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.6", 0);
		preview_audio_out_device.Data4[7] = (char)sfennec.call_function(load_setting_int,  0, (void*)"output.dx.preview.guid4.7", 0);


	}
	return 1;
}


/*
 * uninitialize buffers and devices initialized.
 */
int audio_uninitialize(void)
{
	audio_stop();
	
	subengine_uninitialize();

	splayer.state = v_audio_playerstate_notinit;

	splayer.initialized = 0;

	EnterCriticalSection(&cst);

	thread_audio_terminate = 1;

	LeaveCriticalSection(&cst);

	while(thread_audio_terminate)Sleep(0);

	if(splayer.public_prebuffer)
	{
		sfennec.call_function(call_fennec_memory_free, 0, splayer.public_prebuffer, 0);
		splayer.public_prebuffer = 0;
	}

	if(window_audio)
		DestroyWindow(window_audio);

	CoUninitialize();
	return 1;
}


/*
 * load file/stream for playback.
 */
int audio_load(const string spath)
{
	HRESULT hr = DS_OK;
	int     perror_on_decoder = 1, perror_badformat = 0; /* error on decoder, not the format */

	if(!spath[0])
	{
		/* str_cpy(splayer.current_file, uni("")); */
		return 0;
	}

	if(splayer.loaded)
		audio_stop();

	EnterCriticalSection(&cst);

	/* str_cpy(splayer.current_file, uni("")); */

	splayer.state = v_audio_playerstate_buffering;

	splayer.output_bps    = sfennec.settings.general->audio_output.bit_depth;
	splayer.output_float  = sfennec.settings.general->audio_output.bit_depth_float;
	splayer.output_signed = sfennec.settings.general->audio_output.bit_depth_signed;

	splayer.input_plugin = sfennec.audio.input.selectinput(spath);

	perror_badformat = 1;

	if(splayer.input_plugin == fennec_invalid_index) goto point_error;

	perror_badformat = 0;

	sfennec.audio.input.plugins.initialize(splayer.input_plugin);
	sfennec.audio.input.plugins.loadfile(splayer.input_plugin, spath, &splayer.stream_handle);

	if(splayer.stream_handle == fennec_invalid_index) goto point_error;

	str_cpy(splayer.current_file, spath);
	
	if(!sfennec.audio.input.plugins.getformat(splayer.input_plugin, splayer.stream_handle, &splayer.frequency, &splayer.bps, &splayer.channels)) goto point_error;

	if(!splayer.frequency || !splayer.bps || !splayer.channels) goto point_error;

	

	splayer.outchannels = sfennec.call_function(call_dsp_getoutput_channels, splayer.channels, 0, 0);



	splayer.prebuffersize = (sfennec.settings.general->audio_output.buffer_memory * splayer.frequency * splayer.channels * (splayer.bps / 8) ) / 1000;
	if(!splayer.prebuffersize) goto point_error;

	splayer.prebuffer = (char*) sys_mem_alloc(splayer.prebuffersize + 100);

	splayer.buffersize = (sfennec.settings.general->audio_output.buffer_memory * splayer.frequency * splayer.channels * (splayer.output_bps / 8) ) / 1000;

	/* now we've done with decoder stuff */

	perror_on_decoder = 0;

	/* direct sound */

	if(audio_out_default)
	{
		if(DirectSoundCreate(0, &(splayer.ds), 0)) goto point_error;
	}else{
		if(DirectSoundCreate(&audio_out_device, &(splayer.ds), 0)) goto point_error;
	}

	memset(&splayer.wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));

	splayer.wfx.Format.cbSize          = sizeof(WAVEFORMATEXTENSIBLE);
	splayer.wfx.Format.nSamplesPerSec  = splayer.frequency;
	splayer.wfx.Format.wBitsPerSample  = (WORD)splayer.output_bps;
	splayer.wfx.Format.nChannels       = (WORD)splayer.outchannels;
	splayer.wfx.Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
	splayer.wfx.Format.nAvgBytesPerSec = splayer.frequency * splayer.outchannels * (splayer.output_bps / 8);
	splayer.wfx.Format.nBlockAlign     = (WORD)(splayer.outchannels * (splayer.output_bps / 8));

	splayer.wfx.Samples.wReserved           = 0;
	splayer.wfx.Samples.wSamplesPerBlock    = 0;
	splayer.wfx.Samples.wValidBitsPerSample = splayer.output_bps;

	splayer.wfx.dwChannelMask = (1 << splayer.outchannels) - 1;

	/* mono fix */
	if(splayer.wfx.Format.nChannels == 1)
	{
		splayer.wfx.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	}

	/* 7.1 fix */
	if(splayer.wfx.Format.nChannels == 8)
	{
		splayer.wfx.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER |
									SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT  |
									SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
	}

	if(splayer.output_float)
		splayer.wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ;
	else
		splayer.wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;



	

	memset(splayer.nrc, 0, sizeof(splayer.nrc));

	memset(&splayer.dsb, 0, sizeof(DSBUFFERDESC));

    splayer.dsb.dwSize        = sizeof(DSBUFFERDESC);
    splayer.dsb.dwFlags       = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    splayer.dsb.dwBufferBytes = splayer.buffersize;
    splayer.dsb.lpwfxFormat   = (LPWAVEFORMATEX)&splayer.wfx;


	hr = splayer.ds->SetCooperativeLevel(window_audio, DSSCL_PRIORITY);
	
	if(!SUCCEEDED(hr)) goto point_error;

	hr = splayer.ds->CreateSoundBuffer(&splayer.dsb, &splayer.buffer, 0);

	if(!SUCCEEDED(hr)) goto point_error;

	splayer.written_samples = 0;
	splayer.dur_ms          = sfennec.audio.input.plugins.getduration_ms(splayer.input_plugin, splayer.stream_handle);

	sfennec.audio.equalizer.reset();

	splayer.loaded = 1;
	splayer.state = v_audio_playerstate_loaded;


	audio_setvolume((double)splayer.main_volume / 10000.0, (double)splayer.main_volume / 10000.0);

	thread_wlpos     = 0;
	splayer.stopping = 0;

	audio_getduration_ms(); /* set cached value */

	splayer.buffer_times = 0;

	{
		unsigned long  blen;
		void          *bufferst;

		if(splayer.buffer->Lock(0, 0, &bufferst, &blen, 0, 0, DSBLOCK_ENTIREBUFFER) == DS_OK)
		{
			unsigned long        rlen = splayer.prebuffersize;
			
			local_readdata(splayer.input_plugin, splayer.stream_handle, &rlen, splayer.prebuffer, 1, &splayer);
			local_downsample(bufferst, splayer.prebuffer, rlen);

			splayer.buffer->Unlock(bufferst, blen, 0, 0);
		}
	}

	LeaveCriticalSection(&cst);
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

		if(perror_badformat)
		{
			MessageBox(0, sfennec.language_text[oooo_e_files_invalid_file], sfennec.language_text[oooo_error], MB_ICONEXCLAMATION);
			str_cpy(splayer.current_file, uni(""));
		}
	}
	LeaveCriticalSection(&cst);
	return 0;
}


/*
 * I can't remember :o)
 */
int audio_add(dword idx, const string spath)
{
	
	return 1;
}


/*
 * start playback.
 */
int audio_play(void)
{
	if(!splayer.initialized) return 0;

	if(splayer.loaded)
	{
		splayer.buffer->Play(0, 0, DSBPLAY_LOOPING);
	}else{
		audio_load(splayer.current_file);
		if(splayer.loaded)
			splayer.buffer->Play(0, 0, DSBPLAY_LOOPING);
		else
			return 0;
	}

	splayer.start_time = timeGetTime();

	splayer.state = v_audio_playerstate_playing;
	return 1;
}


/*
 * switch between play and pause.
 */
int audio_playpause(void)
{
	if(splayer.state == v_audio_playerstate_playing)
	{
		audio_pause();
	}else{
		audio_play();
	}
	return 1;
}


/*
 * pause currently playing media.
 */
int audio_pause(void)
{
	if(!splayer.initialized) return 0;

	if(splayer.loaded)
	{
		splayer.buffer->Stop();
	}

	splayer.state = v_audio_playerstate_paused;
	return 1;
}


/*
 * stop playback and release allocated buffers & so...
 */
int audio_stop(void)
{
	if(splayer.input_plugin != fennec_invalid_index)
	{
		EnterCriticalSection(&cst);

		splayer.state = v_audio_playerstate_stopped;

		if(splayer.loaded)
		{
			splayer.buffer->Stop();
			splayer.buffer->Release();
		}

		if(splayer.ds)
			splayer.ds->Release();

		splayer.loaded = 0;

		if(splayer.stream_handle != fennec_invalid_index)
			sfennec.audio.input.plugins.unloadfile(splayer.input_plugin, splayer.stream_handle);
	
		splayer.stream_handle = fennec_invalid_index;
		splayer.input_plugin = fennec_invalid_index;

		sys_mem_free(splayer.prebuffer);

		splayer.state = v_audio_playerstate_stopped;
		
		LeaveCriticalSection(&cst);
	}
	return 1;
}


/*
 * seek (0 to 1.0 = end).
 */
int audio_setposition(double cpos)
{
	if(!splayer.loaded) return 0;

	if(splayer.state = v_audio_playerstate_playing)
		splayer.buffer->Stop();

	//sfennec.setposition_ex(splayer.input_plugin, splayer.stream_handle, &cpos); /* returns in seconds */
	if(sfennec.audio.input.plugins.setposition(splayer.input_plugin, splayer.stream_handle, cpos) != -1)
	{
		//splayer.written_samples = (int64_t)(cpos * (double)splayer.channels * (double)splayer.frequency);//(int64_t)(((double)splayer.dur_ms * (double)splayer.channels * (double)splayer.frequency / 1000.0) * cpos);
		splayer.written_samples = (int64_t)(((double)splayer.dur_ms * (double)splayer.channels * (double)splayer.frequency / 1000.0) * cpos);
	}

	if(splayer.state = v_audio_playerstate_playing)
		splayer.buffer->Play(0, 0, DSBPLAY_LOOPING);
	return 1;
}


/*
 * seek in milliseconds.
 */
int audio_setposition_ms(double mpos)
{
	double cpos = mpos / (double)splayer.dur_ms;
	if(cpos > 1.0)cpos = 1.0;
	else if(cpos < 0.0)cpos = 0.0;
	audio_setposition(cpos);
	return 1;
}


/*
 * get current state of the engine.
 */
int audio_getplayerstate(void)
{
	return splayer.state;
}


/*
 * get current position fraction (0.0 - 1.0).
 */
double audio_getposition(void)
{
	if(!splayer.initialized)return 0.0;
	if(!splayer.loaded)return 0.0;

	if(splayer.dur_ms)
		return audio_getposition_ms() / (double)splayer.dur_ms;
	else
		return 0.0;
}


/*
 * get current position in milliseconds.
 */
double audio_getposition_ms(void)
{
	if(!splayer.initialized)return 0.0;
	if(!splayer.loaded)return 0.0;

	return (double)(splayer.written_samples / splayer.channels) / ((double)splayer.frequency / 1000.0);
}


/*
 * get duration in milliseconds.
 */
double audio_getduration_ms(void)
{

	/* get duration */

//	splayer.dur_ms = sfennec.audio.input.plugins.getduration_ms(splayer.input_plugin, splayer.stream_handle);

	return splayer.dur_ms;
}


/*
 * this is not used - but this is to get the current peak level easily,
 */
int audio_getpeakpower(DWORD* pleft, DWORD* pright)
{
	return 1;
}


/*
 * set volume & pan (stereo).
 */
int audio_setvolume(double vl, double vr)
{
	double v;
	splayer.main_volume = (unsigned short)(((vl + vr) / 2.0) * 10000.0);


	if(splayer.loaded)
	{
		v = (double)splayer.main_volume / 10000.0;
		v = 1.0 - v;
		v = (exp(log(10.0) * v) - 1) / 9.0;
		v *= DSBVOLUME_MIN;

		splayer.buffer->SetVolume((long)v);
	}
	
	return 1;
}


/*
 * get volume/pan (stereo).
 */
int audio_getvolume(double* vl, double* vr)
{
	if(vl)*vl = (double)splayer.main_volume / 10000.0;
	if(vr)*vr = (double)splayer.main_volume / 10000.0;
	return 1;
}


/*
 * get currently playing buffer pointer & size.
 * this ain't a cool function cuz it leads to many buffer leaks.
 */
int audio_getcurrentbuffer(void* dout, dword* dsize)
{

	return 0;
}


/*
 * get a buffer filled with float samples.
 */
int audio_getfloatbuffer(float* dout, dword scount, dword channel)
{
	unsigned long     npos, i, j, byps;

	if(splayer.state != v_audio_playerstate_playing) return 0;

	splayer.buffer->GetCurrentPosition(&npos, 0);

	byps = splayer.bps / 8;

	if(splayer.bps == 64){

		double *buf = (double*)splayer.prebuffer;

		for(i=0; i<scount; i++)
		{
			j = (i * splayer.channels) + (channel - 1) + (upsampled_val(npos) / byps);
			j %= (splayer.prebuffersize / byps);

			dout[i] = (float)buf[j];
		}

		return 1;
	}

	return 0;
}


/*
 * get information about the engine.
 */
int audio_getdata(unsigned int id, void* ret)
{
	switch(id)
	{
	case audio_v_data_frequency:
		memcpy(ret, &splayer.frequency, sizeof(int));
		break;

	case audio_v_data_bitspersample:
		memcpy(ret, &splayer.bps, sizeof(int));
		break;

	case audio_v_data_channels:
		memcpy(ret, &splayer.channels, sizeof(int));
		break;

	case audio_v_data_ids_pid:
		return splayer.input_plugin;

	case audio_v_data_ids_fid:
		return splayer.stream_handle;

	case audio_v_data_position_for_video: /* input: (double*) */
		{
			DWORD npos = 0;	
			double buffersize_sec;	
			int64_t buffercalc1, buffercalc;
			buffersize_sec = ((double)splayer.buffersize / (double)splayer.channels) / (double)splayer.frequency;
			
			splayer.buffer->GetCurrentPosition(&npos, 0);
			/*
			double apos;
			double buffersize_sec;
			
			buffersize_sec = ((double)splayer.buffersize / (double)splayer.channels) / (double)splayer.frequency;
			buffersize_sec /= (splayer.output_bps / 8);

			*((double*)ret) = (splayer.buffer_times * buffersize_sec) + apos ;
			*/

			buffercalc1 = splayer.buffersize / (splayer.output_bps / 8);
			buffercalc = splayer.written_samples % buffercalc1;
			buffercalc = splayer.written_samples - buffercalc;
			buffercalc += (npos / (splayer.output_bps / 8));
			buffercalc += buffercalc1; /* the data we read at loading (full buffer size) */
	
			*((double*)ret) = (double)(buffercalc / splayer.channels) / ((double)splayer.frequency);
		}
		return 1;

	default:
		return 0;
	}
	return 1;
}


/*
 * call audio preview (second output).
 */
int audio_preview_action(int actionid, void *adata)
{
	switch(actionid)
	{
	case audio_preview_play:
		
		subengine_play(0);
		break;

	case audio_preview_pause:

		subengine_pause(0);
		break;

	case audio_preview_stop:

		subengine_stop(0);
		break;

	case audio_preview_load:

		subengine_load(0, (const string)adata);
		break;

	case audio_preview_seek:

		subengine_seek(0, *((double*)adata));

		break;

	case audio_preview_volume:

		MessageBox(0, uni(""), uni("Preview - Volume"), MB_ICONINFORMATION);
		break;

	case audio_preview_getpos:	
		*((double*)adata) = subengine_getposition(0);
		break;

	default: return 0;
	}

	return 1;
}




/* audio playback thread ----------------------------------------------------*/




DWORD WINAPI thread_audio(LPVOID lpParam) 
{ 
	void             *bufferst;
	unsigned long     blen;

	for(;;)
	{
		/* TODO: put this into the bottom of the loop */ subengine_thread(0);

		EnterCriticalSection(&cst);

		if(thread_audio_terminate)
		{
			LeaveCriticalSection(&cst);
			DeleteCriticalSection(&cst);
			thread_audio_terminate = 0;
			break;
		}
		if(!splayer.loaded) goto point_epl;

		if(splayer.buffer->Lock(0, 0, &bufferst, &blen, 0, 0, DSBLOCK_ENTIREBUFFER) == DS_OK)
		{
			unsigned long        npos, rlen = 0, ret = 0;

			splayer.buffer->GetCurrentPosition(&npos, 0);

			if(splayer.stopping)
			{
				unsigned long btime = splayer.prebuffersize / (((splayer.bps / 8) * splayer.channels * splayer.frequency) / 1000);
				unsigned long ntime = timeGetTime();

				if(ntime > splayer.stop_start_time + btime)
				{
					LeaveCriticalSection(&cst);

					if(sfennec.settings.general->player.playlist_repeat_single)
					{
						//audio_setposition(0.0);
						audio_stop();
						audio_play();
					}else{
						audio_stop();
						if(sfennec.settings.general->player.auto_switching)sfennec.audio.output.playlist.next();
					}

					goto point_eepl;
				}
			}






			if(npos)
			{
				if(npos > thread_wlpos)
				{
					rlen = upsampled_val(npos - thread_wlpos);

					if(!splayer.stopping)
						ret = local_readdata(splayer.input_plugin, splayer.stream_handle, &rlen, splayer.prebuffer + upsampled_val(thread_wlpos), 0, &splayer);
					else
						memset(splayer.prebuffer + upsampled_val(thread_wlpos), 0, rlen);
					
					local_downsample((char*)bufferst + thread_wlpos, splayer.prebuffer + upsampled_val(thread_wlpos), rlen);
					thread_wlpos += downsampled_val(rlen);

					if((!rlen || !ret) && !splayer.stopping)
					{
						splayer.stopping        = 1;
						splayer.stop_at         = thread_wlpos;
						splayer.stop_start_time = timeGetTime();
						goto stop_playback;
					}
					

				}else if(npos < thread_wlpos){
					if(thread_wlpos < splayer.buffersize)
					{
						rlen = upsampled_val(splayer.buffersize - thread_wlpos);

						if(rlen > 0)
						{
							if(!splayer.stopping)
								ret = local_readdata(splayer.input_plugin, splayer.stream_handle, &rlen, splayer.prebuffer + upsampled_val(thread_wlpos), 0, &splayer);
							else
								memset(splayer.prebuffer + upsampled_val(thread_wlpos), 0, rlen);

							local_downsample((char*)bufferst + thread_wlpos, splayer.prebuffer + upsampled_val(thread_wlpos), rlen);
							
							if((!rlen || !ret) && !splayer.stopping)
							{
								splayer.stopping        = 1;
								splayer.stop_at         = thread_wlpos;
								splayer.stop_start_time = timeGetTime();
								goto stop_playback;
							}
						}
					}
					

					rlen = upsampled_val(npos);

					thread_wlpos = 0;

					if(rlen > 0)
					{
						if(!splayer.stopping)
							ret = local_readdata(splayer.input_plugin, splayer.stream_handle, &rlen, splayer.prebuffer, 0, &splayer);
						else
							memset(splayer.prebuffer, 0, rlen);

						local_downsample(bufferst, splayer.prebuffer, rlen);
					}

					thread_wlpos = downsampled_val(rlen);

					if((!rlen || !ret) && !splayer.stopping)
					{
						splayer.stopping        = 1;
						splayer.stop_at         = thread_wlpos;
						splayer.stop_start_time = timeGetTime();
						goto stop_playback;
					}
				}
			}


stop_playback:
			splayer.buffer->Unlock(bufferst, blen, 0, 0);
			
		}

point_epl:
		LeaveCriticalSection(&cst);

		Sleep(5);

point_eepl:;
	}
    return 0; 
} 
 

/* local functions ----------------------------------------------------------*/



int local_readdata(unsigned long pid, unsigned long fid, unsigned long *dsize, void* odata, int posset, struct audioplayer *pl)
{
	unsigned long tmp_dpointer = 0;
	unsigned long tmp_rsize;
	int           tmp_retval   = 1, lastfill = 0;
	unsigned long tmp_bsize;

	if(pl->outchannels != sfennec.call_function(call_dsp_getoutput_channels, pl->channels, 0, 0))
		return 0;

	if(!sfennec.settings.general->dsp_plugins.enable)
	{
		ZeroMemory(odata, *dsize); /* error correction, if plugin ain't gonna give us any data, let there be silence */
		tmp_retval = sfennec.audio.input.plugins.readdata(pid, fid, dsize, odata);
		sfennec.audio.equalizer.equalize(odata, 0, pl->channels, pl->frequency, pl->bps, *dsize);
		
		pl->written_samples += *dsize / (pl->bps / 8);
		return tmp_retval;
	}

	tmp_rsize = *dsize;

	if(posset)
		lastfill = 0;

	if(pl->public_prebuffer_clength && !posset)
	{
		tmp_dpointer = pl->public_prebuffer_clength;
	}

	for(;;)
	{
		if(pl->public_prebuffer_length < tmp_rsize)
		{
			pl->public_prebuffer_length = tmp_rsize;
			sfennec.call_function(call_fennec_memory_realloc, pl->public_prebuffer_length, pl->public_prebuffer, (void*)&pl->public_prebuffer);
		}


		if(tmp_dpointer >= tmp_rsize)
		{
			memcpy(odata, pl->public_prebuffer, tmp_rsize);

			if((tmp_dpointer > tmp_rsize) && !posset)
			{
				pl->public_prebuffer_clength = tmp_dpointer - tmp_rsize;
				memcpy(pl->public_prebuffer, pl->public_prebuffer + tmp_rsize, pl->public_prebuffer_clength);
			}else{
				pl->public_prebuffer_clength = 0;
			}

			if(!posset)
				tmp_dpointer = pl->public_prebuffer_clength;
			break;
		}else{
			if(tmp_dpointer && pl->public_prebuffer_clength)
			{
				memcpy(odata, pl->public_prebuffer, pl->public_prebuffer_clength);
				tmp_rsize -= pl->public_prebuffer_clength;
				odata = ((char*)odata) + pl->public_prebuffer_clength;
				pl->public_prebuffer_clength = 0;
				tmp_dpointer = 0;
			}
		}

		if(lastfill)
		{
			memcpy(odata, pl->public_prebuffer, tmp_dpointer);
			*dsize = tmp_dpointer;
			break;
		}

		tmp_bsize    =  tmp_rsize;
		ZeroMemory(pl->public_prebuffer + tmp_dpointer, tmp_bsize); /* error correction, if plugin ain't gonna give us any data, let there be silence */
		tmp_retval   =  sfennec.audio.input.plugins.readdata(pid, fid, &tmp_bsize, pl->public_prebuffer + tmp_dpointer);
		


		sfennec.audio.equalizer.equalize(pl->public_prebuffer + tmp_dpointer, 0, pl->channels, pl->frequency, pl->bps, tmp_rsize);
		
		if(!posset)
			pl->written_samples += tmp_rsize / (pl->bps / 8);

		pl->public_prebuffer = (char*)sfennec.audio.dsp.process(0, &tmp_bsize, pl->frequency, pl->bps, pl->channels, pl->public_prebuffer, tmp_dpointer, pl->public_prebuffer_length);

		if(tmp_bsize > pl->public_prebuffer_length)pl->public_prebuffer_length = tmp_bsize;

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


/*
 * enumerate device names.
 */
BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext )
{
  HWND   hcombo = *(HWND *)lpContext;
  LPGUID lpTemp = 0;
  int    i;
 
  if(lpGUID != 0)
  {
    if( (lpTemp = (LPGUID)malloc(sizeof(GUID)) ) == 0)
    return 1;
 
    memcpy(lpTemp, lpGUID, sizeof(GUID));
  }

  i = (int)SendMessage(hcombo, CB_GETCOUNT, 0, 0);
  
  if(lpGUID)
	if(memcmp(&audio_out_device, lpGUID, sizeof(GUID)) == 0) device_combo_index = i;

  if(lpGUID)
	if(memcmp(&preview_audio_out_device, lpGUID, sizeof(GUID)) == 0) preview_device_combo_index = i;

  SendMessage(hcombo, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)lpszDesc );
  SendMessage(hcombo, CB_SETITEMDATA, i, (LPARAM)lpTemp);
  return 1;
}


/*
 * settings dialog callback functon.
 */
int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				HWND hcombo = GetDlgItem(hwnd, combo_devices);
				int i = (int)SendMessage(hcombo, CB_GETCURSEL, 0, 0);
				void *d;

				if(i >= 0)
				{
					d = (void*) SendMessage(hcombo, CB_GETITEMDATA, i, 0);

					if(d)
					{
						memcpy(&audio_out_device, d, sizeof(GUID));
					
						sfennec.call_function(save_setting_int,  audio_out_device.Data1, (void*)"output.dx.guid1", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data2, (void*)"output.dx.guid2", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data3, (void*)"output.dx.guid3", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[0], (void*)"output.dx.guid4.0", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[1], (void*)"output.dx.guid4.1", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[2], (void*)"output.dx.guid4.2", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[3], (void*)"output.dx.guid4.3", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[4], (void*)"output.dx.guid4.4", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[5], (void*)"output.dx.guid4.5", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[6], (void*)"output.dx.guid4.6", 0);
						sfennec.call_function(save_setting_int,  audio_out_device.Data4[7], (void*)"output.dx.guid4.7", 0);

						sfennec.call_function(save_setting_int, 0, (void*)"output.dx.device-default", 0);

						audio_out_default = 0;
					}else{

						sfennec.call_function(save_setting_int, 1, (void*)"output.dx.device-default", 0);
						audio_out_default = 1;
					}
					
				}

				/* preview */

				hcombo = GetDlgItem(hwnd, combo_devices2);
				i = (int)SendMessage(hcombo, CB_GETCURSEL, 0, 0);

				if(i >= 0)
				{
					d = (void*) SendMessage(hcombo, CB_GETITEMDATA, i, 0);

					if(d)
					{
						memcpy(&preview_audio_out_device, d, sizeof(GUID));
					
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data1, (void*)"output.dx.preview.guid1", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data2, (void*)"output.dx.preview.guid2", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data3, (void*)"output.dx.preview.guid3", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[0], (void*)"output.dx.preview.guid4.0", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[1], (void*)"output.dx.preview.guid4.1", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[2], (void*)"output.dx.preview.guid4.2", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[3], (void*)"output.dx.preview.guid4.3", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[4], (void*)"output.dx.preview.guid4.4", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[5], (void*)"output.dx.preview.guid4.5", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[6], (void*)"output.dx.preview.guid4.6", 0);
						sfennec.call_function(save_setting_int,  preview_audio_out_device.Data4[7], (void*)"output.dx.preview.guid4.7", 0);

						sfennec.call_function(save_setting_int, 0, (void*)"output.dx.preview.device-default", 0);

						preview_audio_out_default = 0;
					}else{

						sfennec.call_function(save_setting_int, 1, (void*)"output.dx.preview.device-default", 0);
						preview_audio_out_default = 1;
					}
					
				}


				/* --- */

			}


		case IDCANCEL:
			{
				HWND hcombo = GetDlgItem(hwnd, combo_devices);
				int i, c = (int)SendMessage(hcombo, CB_GETCOUNT, 0, 0);
				void *d;

				for(i=0; i<c; i++)
				{
					d = (void*)SendMessage(hcombo, CB_GETITEMDATA, i, 0);
					if(d) free(d);
				}

			}

			/* preview */

			{
				HWND hcombo = GetDlgItem(hwnd, combo_devices2);
				int i, c = (int)SendMessage(hcombo, CB_GETCOUNT, 0, 0);
				void *d;

				for(i=0; i<c; i++)
				{
					d = (void*)SendMessage(hcombo, CB_GETITEMDATA, i, 0);
					if(d) free(d);
				}

			}

			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			HWND hcombo = GetDlgItem(hwnd, combo_devices);

			DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc, (VOID*)&hcombo);

		
			if(!audio_out_default)
				SendMessage(hcombo, CB_SETCURSEL, device_combo_index, 0);
			else
				SendMessage(hcombo, CB_SETCURSEL, 0, 0);
			

			/* preview */
			
			hcombo = GetDlgItem(hwnd, combo_devices2);

			DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc, (VOID*)&hcombo);

			if(!preview_audio_out_default)
				SendMessage(hcombo, CB_SETCURSEL, preview_device_combo_index, 0);
			else
				SendMessage(hcombo, CB_SETCURSEL, 0, 0);

		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}


/*
 * DLL callback (just to get the module handle).
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH) plugin_instance = hinstDLL;
	return 1;
}