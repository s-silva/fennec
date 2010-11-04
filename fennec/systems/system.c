/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "system.h"

/* system specific codes - Microsoft (R) Windows*/

#ifdef system_microsoft_windows




	/* <data> */
	
	/* multimedia */
	
	typedef struct _mmwave
	{
		HWAVEOUT hwo;
		WAVEHDR  wh;
		int      hdr_init;
		int      wav_init;
		int      cleared;
	}mmwave;
	
	unsigned int  mmwave_size = 0;
	unsigned int  mmwave_count = 0;
	mmwave       *mmdata;
	
	/* </data> */




	/* <memory> */

	void *sys_mem_alloc(size_t sz)
	{
		void *ov = malloc(sz);
		if(!ov)
			MessageBox(0, uni("Out of memory!"), uni("Error, Fennec Player"), MB_ICONERROR);
		return ov;
	}

	
	void *sys_mem_realloc(void *imem, size_t sz)
	{
		void *ov = realloc(imem, sz);
		if(!ov)
			MessageBox(0, uni("Out of memory!"), uni("Error, Fennec Player"), MB_ICONERROR);
		return ov;
	}


	/* </memory> */




	/* < disc i/o functions > */

	unsigned long sys_file_read(HANDLE h, void* a, unsigned long z)
	{
		unsigned long nbr = 0;
		if(ReadFile(h, a, z, &nbr, 0))return nbr;
		return v_error_sys_file_read;
	}

	unsigned long sys_file_write(HANDLE h, void* a, unsigned long z)
	{
		unsigned long nbr = 0;
		if(WriteFile(h, a, z, &nbr, 0))return nbr;
		return v_error_sys_file_write;
	}

	/* </ disc i/o functions > */




	/* <timers> */
	
	int sys_timer_init(void)
	{
		TIMECAPS tcaps;
		UINT     tmres = 0;
	
		if(timeGetDevCaps(&tcaps, sizeof(TIMECAPS)) == TIMERR_NOERROR)
		{
			tmres = min(max(tcaps.wPeriodMin, 1), tcaps.wPeriodMax);
			timeBeginPeriod(tmres);
			return tmres;
		}
		return v_error_sys_timer_init;
	}
	
	/* </timers> */
	
	
	
	
	/* <multimedia> */
	
	unsigned int sys_media_stream_init(unsigned int d, unsigned int freq, unsigned int channels, unsigned int bps)
	{
		MMRESULT     mmr;
		WAVEFORMATEX wfx;
		unsigned int i;
		int          found_ea = 0;
	
		if(!mmwave_size) /* not initialized */
		{
			mmwave_size = 10;
			mmdata      = (mmwave*) sys_mem_alloc(sizeof(mmwave) * mmwave_size);
		}
	
		/* check for an uninitialized stream */
	
		for(i = 0;  i < mmwave_count;  i++)
		{
			if(mmdata[i].hwo == 0)
			{
				found_ea = 1;
				break;
			}
		}
	
		if(!found_ea)
		{
			if(mmwave_count + 1 >= mmwave_size) /* resize */
			{
				mmwave_size += 5;
				mmdata       = (mmwave*) sys_mem_realloc(mmdata, sizeof(mmwave) * mmwave_size);
			}
	
			mmwave_count++;
		}
	
		wfx.cbSize          = sizeof(WAVEFORMATEX);
		wfx.wFormatTag      = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec  = freq;
		wfx.nChannels       = (WORD)channels;
		wfx.wBitsPerSample  = (WORD)bps;
		wfx.nAvgBytesPerSec = ((bps / 8) * channels * freq);
		wfx.nBlockAlign     = (WORD)(channels * (bps / 8));
	
		if(!found_ea)
		{
			mmdata[mmwave_count].hdr_init = 0;
			mmdata[mmwave_count].wav_init = 0;
			mmdata[mmwave_count].cleared  = 0;
			mmr = waveOutOpen(&mmdata[mmwave_count].hwo, d, &wfx, 0, 0, CALLBACK_NULL);
		}else{
			mmdata[i].hdr_init = 0;
			mmdata[i].wav_init = 0;
			mmdata[i].cleared  = 0;
			mmr = waveOutOpen(&mmdata[i].hwo, d, &wfx, 0, 0, CALLBACK_NULL);
		}
	
		if(mmr == MMSYSERR_NOERROR)
		{
			if(!found_ea)
			{
				mmdata[mmwave_count].wav_init = 1;
				return mmwave_count;
			}else{
				mmdata[i].wav_init = 1;
				return i;
			}
		}else{
			return v_error_sys_media_stream_init;
		}
	}
	
	unsigned int sys_media_stream_write(unsigned int h, void* b, unsigned int z, unsigned int reps)
	{
		if(h > mmwave_count)return v_error_sys_media_stream_write;
	
		if(!mmdata[h].wav_init)return v_error_sys_media_main_write;
		if(mmdata[h].hdr_init)sys_media_stream_clear(h);

		memset(&mmdata[h].wh, 0, sizeof(WAVEHDR));

		mmdata[h].wh.lpData         = b;
		mmdata[h].wh.dwBufferLength = z;
		mmdata[h].wh.dwFlags        = WHDR_BEGINLOOP | WHDR_ENDLOOP;
		mmdata[h].wh.dwLoops        = reps;
	
		waveOutPrepareHeader(mmdata[h].hwo, &mmdata[h].wh, sizeof(WAVEHDR));
		waveOutWrite(mmdata[h].hwo, &mmdata[h].wh, sizeof(WAVEHDR));

		mmdata[h].cleared = 0;
	
		mmdata[h].hdr_init = 1;
		return z;
	}
	
	int sys_media_stream_play(unsigned long h)
	{
		if(h > mmwave_count)return 0;
		if(!mmdata[h].wav_init)return 0;
		if(!mmdata[h].hdr_init)return 0;
		waveOutRestart(mmdata[h].hwo);
		return 1;
	}
	
	int sys_media_stream_pause(unsigned long h)
	{
		if(h > mmwave_count)return 0;
		if(!mmdata[h].wav_init)return 0;
		if(!mmdata[h].hdr_init)return 0;
		waveOutPause(mmdata[h].hwo);
		return 1;
	}
	
	int sys_media_stream_stop(unsigned long h)
	{
		if(h > mmwave_count)return 0;
		if(!mmdata[h].wav_init)return 0;
		if(!mmdata[h].hdr_init)return 0;
		return 1;
	}
	
	int sys_media_stream_clear(unsigned long h)
	{
		if(h > mmwave_count)return 0;
		if(!mmdata[h].wav_init)return 0;
		if(!mmdata[h].hdr_init)return 0;
		if(mmdata[h].cleared)return 2;

		mmdata[h].cleared = 1;

		waveOutBreakLoop(mmdata[h].hwo);
		waveOutReset(mmdata[h].hwo);
		waveOutUnprepareHeader(mmdata[h].hwo, &mmdata[h].wh, sizeof(WAVEHDR));
		return 1;
	}
	
	unsigned long sys_media_stream_buffertimer_byte(unsigned long h)
	{
		MMTIME lpt;
		unsigned long  tb;
	
		if(h > mmwave_count)return 0;
		if(!mmdata[h].wav_init)return 0;
		if(!mmdata[h].hdr_init)return 0;
		if(!mmdata[h].hwo)return 0;
	
		lpt.wType = TIME_BYTES;
	
		waveOutGetPosition(mmdata[h].hwo, &lpt, sizeof(lpt));
		tb = lpt.u.cb - (((unsigned long)(lpt.u.cb / mmdata[h].wh.dwBufferLength)) * mmdata[h].wh.dwBufferLength);
		return tb;
	}
	
	int sys_media_stream_uninit(unsigned long h)
	{
		if(h > mmwave_count)return 0;
		if(!mmdata)return 0;
		if(!mmdata[h].wav_init)return 0;
		if(!mmdata[h].hdr_init)return 0;
	
		if(!sys_media_stream_clear(h))
		{
			return 0;
		}
		
		mmdata[h].hdr_init = 0;
			
		waveOutClose(mmdata[h].hwo);
	
		mmdata[h].hwo = 0;
	
		mmdata[h].wav_init = 0;
		return 1;
	}

	int sys_media_getinfo(unsigned int dvid, unsigned long *sformats, int *maxchannels)
	{
		WAVEOUTCAPS  wocaps;
		MMRESULT     mres;

		mres = waveOutGetDevCaps(dvid, &wocaps, sizeof(wocaps));

		if(mres != MMSYSERR_NOERROR)
		{
			switch(mres)
			{
			case MMSYSERR_NODRIVER:    return v_error_sys_media_getinfo_nodev;
			case MMSYSERR_BADDEVICEID: return v_error_sys_media_getinfo_badid;
			default:                   return v_error_sys_media_getinfo_other;
			}
		}

		*maxchannels = (int)wocaps.wChannels;
		*sformats    = wocaps.dwFormats;
		return 0;
	}

	/* check? (bool) */
	int sys_media_check(unsigned int dvid, unsigned long freq, unsigned int bps, unsigned int channels)
	{
		WAVEFORMATEX  wfx;

		wfx.cbSize = sizeof(WAVEFORMATEX);

		wfx.cbSize          = sizeof(WAVEFORMATEX);
		wfx.wFormatTag      = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec  = freq;
		wfx.nChannels       = (WORD)channels;
		wfx.wBitsPerSample  = (WORD)bps;
		wfx.nAvgBytesPerSec = ((bps / 8) * channels * freq);
		wfx.nBlockAlign     = (WORD)(channels * (bps / 8));
		
		return (waveOutOpen(0, dvid, &wfx, 0, 0, WAVE_FORMAT_QUERY) == 0 ? 1 : 0);
	}

	/* </multimedia> */
	
	
	
	
	/* <threading> */
	
	t_sys_thread_handle sys_thread_call(t_sys_thread_function cfunc)
	{
		unsigned long tpr = 0;
		unsigned long tid = 0;
		return (t_sys_thread_handle)CreateThread(0, 0, cfunc, &tpr, 0,&tid);
	}
	
	int sys_thread_setpriority(t_sys_thread_handle thand, int plevel)
	{
		if(SetThreadPriority(thand, plevel))return 1;
		return 0;
	}
	
	int sys_thread_exit(t_sys_thread_handle thand)
	{
		if(CloseHandle(thand))
		{
			return 1;
		}else{
			return v_error_sys_thread_exit;
		}
	}
	
	int sys_thread_share_create(t_sys_thread_share* cs)
	{
		InitializeCriticalSection(cs);
		return 1; 
	}
	
	int sys_thread_share_close(t_sys_thread_share* cs)
	{
		DeleteCriticalSection(cs);
		return 1;
	}
	
	int sys_thread_share_request(t_sys_thread_share* cs)
	{
		EnterCriticalSection(cs);
		return 1;
	}
	
	
	int sys_thread_share_release(t_sys_thread_share* cs)
	{
		LeaveCriticalSection(cs);
		return 1;
	}

	/* </threading> */
	
	
	
	
	/* <file system> */



	int sys_fs_file_exist(const string fpath)
	{
		WIN32_FIND_DATA wfd;
		HANDLE          shandle;
	
		shandle = FindFirstFile(fpath, &wfd);
	
		if(shandle == INVALID_HANDLE_VALUE)return 0;
		return 1;
	}

	
	int sys_fs_find_start(const string inpath, string outpath, size_t osize, t_sys_fs_find_handle* shandle)
	{
		WIN32_FIND_DATA wfd;
	
		*shandle = FindFirstFile(inpath, &wfd);
	
		if(*shandle == INVALID_HANDLE_VALUE)return 0;
	
		//memset(outpath, 0, osize);
		str_cpy(outpath, wfd.cFileName);
		return 1;
	}
	
	int sys_fs_find_next(string outpath, size_t osize, t_sys_fs_find_handle shandle)
	{
		WIN32_FIND_DATA wfd;
	
		if(FindNextFile(shandle, &wfd))
		{
			//memset(outpath, 0, osize);
			str_cpy(outpath, wfd.cFileName);
			return 1;
		}else{
			return 0;
		}
	}
	
	int sys_fs_find_close(t_sys_fs_find_handle shandle)
	{
		if(FindClose(shandle))
		{
			return 1;
		}else{
			return 0;
		}
	}

	
#endif /* </ system_microsoft_windows > */

/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/
