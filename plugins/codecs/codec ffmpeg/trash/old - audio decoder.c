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

#include "plugin.h"
#include "subtitles/subtitles.h"

struct video_dec  ffvid_dec;
int vid_frame_get_for_audio(unsigned long id, AVPacket *packet, int* newrun);

void frame_queue_init(struct queue_frame *q);
void frame_queue_uninit(struct queue_frame *q);
void frame_queue_push(struct queue_frame *q, void *pkt, double pos);
int frame_queue_pop_from_pts(struct queue_frame *q, double pts);
unsigned long getactive(struct queue_frame *q);
int vid_frame_decode(unsigned long id, AVPacket *packet, int *newrun);
double get_smallest_pos_gap(struct queue_frame *q, double gpts);

double gpos = 0;

CRITICAL_SECTION cs;


uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;
int      lastpush = 0, lastpushframe = 0;
int      video_thread_run = 1;
int      video_thread_paused = 1, video_thread_created = 0;
int      redirect_video_skipping = 0;
int      use_thread = 1;

double   current_sync_seconds = 0.0;

int      count_audio_streams = 0;
int      count_video_streams = 0;

float    dvdrpos = 0.0;

int      last_id = 0;

int our_get_buffer(struct AVCodecContext *c, AVFrame *pic)
{
  int ret = avcodec_default_get_buffer(c, pic);
  uint64_t *pts = av_malloc(sizeof(uint64_t));
  *pts = global_video_pkt_pts;
  pic->opaque = pts;
  return ret;
}
void our_release_buffer(struct AVCodecContext *c, AVFrame *pic)
{
  if(pic) av_freep(&pic->opaque);
  avcodec_default_release_buffer(c, pic);
}


int decoder_load(unsigned long id, const string sname)
{
	unsigned  int i;
	letter        bname[v_sys_maxpath];
	char          afname[v_sys_maxpath];
	BOOL          useddef = 1;

	last_id = id;

	str_cpy(bname, sname);
	
	if(!video_thread_created)
	{
		InitializeCriticalSection(&cs);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE) video_thread, 0, 0, 0);
		video_thread_created = 1;
	}

	EnterCriticalSection(&cs);


	lastpushframe = 0;

	pstreams[id].dvd_buffer = 0;
	pstreams[id].pts_goes_wrong = 0;
	


	WideCharToMultiByte(CP_ACP, 0, sname, -1, afname, sizeof(afname), "?", &useddef);


	detect_subtitle_files(sname);
	set_primary_subtitle_file(0);
	//subtitles_load(sname, &pstreams[id].text_subtitles, 0);

	av_register_all();


	if(str_cmp(bname + 3, uni("root.vob")) == 0) /* DVD playback from root */
	{
		if(MessageBox(0, uni("For TESTING PURPOSES only!\nSkip playback (recommended)?"), uni("Not Implemented"), MB_ICONINFORMATION | MB_YESNO) == IDNO)
		{
			if(dvdplay_open(id, &pstreams[id].avf_context, 0) != 0)
				goto point_error;
			//if(dvdplay_open(id, &pstreams[id].audio.avf_context, 0) != 0)
			///	goto point_error;
		}else{
			goto point_error;
		}

	}else{

		if(av_open_input_file(&pstreams[id].avf_context, afname, 0, 0, 0) != 0)
			goto point_error;

		if(av_open_input_file(&pstreams[id].audio.avf_context, afname, 0, 0, 0)!=0)
			goto point_error;

	}


	if(!pstreams[id].avf_context) goto point_error;

	if(av_find_stream_info(pstreams[id].avf_context) < 0)
		return 0;

	pstreams[id].sel_stream_audio = pstreams[id].sel_stream_video = -1;

	/* default selection would be the first stream we find */

	count_audio_streams = count_video_streams = 0;
	
	for(i=0; i<pstreams[id].avf_context->nb_streams; i++)
	{
		if(pstreams[id].avf_context->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO && pstreams[id].sel_stream_video < 0)
		{
			pstreams[id].sel_stream_video = i;
			count_video_streams++;
		}

		if(pstreams[id].avf_context->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO && pstreams[id].sel_stream_audio < 0)
		{
			pstreams[id].sel_stream_audio = i;
			count_audio_streams++;
		}
	}

	if(!pstreams[id].sel_stream_video && !pstreams[id].sel_stream_audio) goto point_error;

	pstreams[id].avc_audio_context = pstreams[id].avf_context->streams[pstreams[id].sel_stream_audio]->codec;

	pstreams[id].bitspersample = 16;
	pstreams[id].channels      = pstreams[id].avc_audio_context->channels;
	pstreams[id].frequency     = pstreams[id].avc_audio_context->sample_rate;

	//pstreams[id].avc_codec_audio = avcodec_find_decoder(pstreams[id].avc_audio_context->codec_id);
	//if(!pstreams[id].avc_codec_audio) return 0;

	//avcodec_open(pstreams[id].avc_audio_context, pstreams[id].avc_codec_audio);

	/* get video stream */
	pstreams[id].avc_video_context = pstreams[id].avf_context->streams[pstreams[id].sel_stream_video]->codec;
	pstreams[id].avc_codec_video   = avcodec_find_decoder(pstreams[id].avc_video_context->codec_id);
	
	pstreams[id].avc_video_context->get_buffer     = our_get_buffer;
    pstreams[id].avc_video_context->release_buffer = our_release_buffer;

	pstreams[id].video_stream = pstreams[id].avf_context->streams[pstreams[id].sel_stream_video];
	pstreams[id].audio_stream = pstreams[id].avf_context->streams[pstreams[id].sel_stream_audio];
	
	//if(!pstreams[id].avc_codec_audio) return 0;
	avcodec_open(pstreams[id].avc_video_context, pstreams[id].avc_codec_video);

	pstreams[id].ob_size  = 0;
	pstreams[id].ob_start = 0;
	pstreams[id].running  = 1;
 
	pstreams[id].vid_audio_based_last_frame = 0;

	pstreams[id].newrun = 1;

	pstreams[id].last_w = pstreams[id].last_h = 0;
	pstreams[id].frame_got_new = 0;

	pstreams[id].current_pos = 0;
	pstreams[id].start_pos = -1;
	pstreams[id].lastp = 0;
	pstreams[id].seekpts = 0;

	frame_queue_init(&pstreams[id].vid_queue);
	
	timeBeginPeriod(1);

	/* second audio */


	if(!pstreams[id].dvd_buffer)
	{
		
		if(av_find_stream_info(pstreams[id].audio.avf_context) < 0)
			goto point_error;

		pstreams[id].audio.sel_stream_audio = -1;

		for(i=0; i<pstreams[id].audio.avf_context->nb_streams; i++)
		{
			if(pstreams[id].audio.avf_context->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO && pstreams[id].audio.sel_stream_audio < 0)
				pstreams[id].audio.sel_stream_audio = i;
		}

		pstreams[id].audio.audio_stream = pstreams[id].audio.avf_context->streams[pstreams[id].audio.sel_stream_audio];
		
		pstreams[id].audio.avc_audio_context = pstreams[id].audio.avf_context->streams[pstreams[id].audio.sel_stream_audio]->codec;

		pstreams[id].audio.avc_codec_audio = avcodec_find_decoder(pstreams[id].audio.avc_audio_context->codec_id);
		if(!pstreams[id].audio.avc_codec_audio) goto point_error;

		avcodec_open(pstreams[id].audio.avc_audio_context, pstreams[id].audio.avc_codec_audio);
		
		pstreams[id].audio.ob_size  = 0;
		pstreams[id].audio.ob_start = 0;

		pstreams[id].duration      = (unsigned long)(pstreams[id].audio.avf_context->duration / AV_TIME_BASE) * 1000;
	}
	pstreams[id].buf = 0;

	video_thread_paused = 0;

	pstreams[id].loaded = 1;

	LeaveCriticalSection(&cs);
	return 1;

point_error:
	LeaveCriticalSection(&cs);
	return 0;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;
	if(!pstreams[id].loaded) return 0;

	EnterCriticalSection(&cs);

	pstreams[id].initialized = 0;
	pstreams[id].loaded      = 0;

	free_loaded_subtitles();

	frame_queue_uninit(&pstreams[id].vid_queue);

	//if(pstreams[id].sel_stream_audio >= 0)
	//	avcodec_close(pstreams[id].avf_context->streams[pstreams[id].sel_stream_audio]->codec);

	if(pstreams[id].sel_stream_video >= 0)
		avcodec_close(pstreams[id].avf_context->streams[pstreams[id].sel_stream_video]->codec);

	av_close_input_file(pstreams[id].avf_context);

	if(!pstreams[id].dvd_buffer)
	{
		if(pstreams[id].audio.sel_stream_audio >= 0)
			avcodec_close(pstreams[id].audio.avf_context->streams[pstreams[id].audio.sel_stream_audio]->codec);

		av_close_input_file(pstreams[id].audio.avf_context);
	}

	timeEndPeriod(1);	  

	if(pstreams[id].dvd_buffer) sys_mem_free(pstreams[id].dvd_buffer);

	pstreams[id].initialized = 0;

	LeaveCriticalSection(&cs);
	return 1;
}

int decoder_seek(unsigned long id, double *posr)
{
	int64_t spos;
	int     sflag = 0;
	double  pos = *posr, lpos;
	AVRational      avr;

	if(!pstreams[id].loaded)return 0;

	if(!pstreams[id].dvd_buffer)
	{
		avr.num = 1;
		avr.den = AV_TIME_BASE;

		EnterCriticalSection(&cs);

		lpos = pstreams[id].audio_pos / (double)(pstreams[id].avf_context->duration / AV_TIME_BASE);
		if(lpos > pos) sflag = AVSEEK_FLAG_BACKWARD;
		else             sflag = AVSEEK_FLAG_ANY;

		
		//avformat_seek_file(pstreams[id].audio.avf_context, -1,  INT64_MIN, spos, INT64_MAX, 0);
		//avformat_seek_file(pstreams[id].avf_context, -1,  INT64_MIN, spos, INT64_MAX, 0);
				

		spos = (int64_t)(pos * (double)pstreams[id].avf_context->duration);
		av_seek_frame(pstreams[id].avf_context, -1, spos, sflag);
		
		
		spos = (int64_t)(pos * (double)pstreams[id].avf_context->duration);
		av_seek_frame(pstreams[id].audio.avf_context, -1, spos, sflag);
	
		pstreams[id].lastp = -1;

		pos = ((double)spos);
		pos /= (double)AV_TIME_BASE;
		*posr = pos;
		
		frame_queue_uninit(&pstreams[id].vid_queue);
		frame_queue_init(&pstreams[id].vid_queue);

		LeaveCriticalSection(&cs);
	}else{
		
		EnterCriticalSection(&cs);


		dvddec_seek(pstreams[id].dvd_info, (float)pos);

		LeaveCriticalSection(&cs);
	}
	return 1;
}

DWORD video_thread(LPVOID lparam)
{
	int             frame_size = 0, buffer_point = 0, bytecount = 0;
	int             delay = 1;
	AVPacket        packet;
	unsigned long   id = 0;
	int             nodecode;

point_start:
	Sleep(1);
	//return 0;

	redirect_video_skipping = 0;
	nodecode = 0;

	EnterCriticalSection(&cs);


	if(!video_thread_run || !pstreams || !use_thread)
	{
		LeaveCriticalSection(&cs);
		return 1;
	}

	if(video_thread_paused || !pstreams[id].loaded)
	{
		LeaveCriticalSection(&cs);
		goto point_start;
	}

	if(!pstreams[id].initialized || !pstreams[id].running)
	{
		LeaveCriticalSection(&cs);
		goto point_start;
	}

	if(pstreams[id].vid_queue.count > 8)
	{
		if(getactive(&pstreams[id].vid_queue) > (pstreams[id].vid_queue.count / 2))
		{
			if(get_smallest_pos_gap(&pstreams[id].vid_queue, pstreams[id].audio_pos) > 2.5)
				redirect_video_skipping = 1;

			LeaveCriticalSection(&cs);
			goto point_start;
		}
	}

	if(av_read_frame(pstreams[id].avf_context, &packet) >= 0)
	{
		if(!packet.size)
		{
			pstreams[id].running = 0;
			LeaveCriticalSection(&cs);
			goto point_start;
		}

		if(packet.stream_index == pstreams[id].sel_stream_video)
		{
			pstreams[id].newrun = 1;
			if(!nodecode)
				vid_frame_decode(id, &packet, &pstreams[id].newrun);
		
		}

		av_free_packet(&packet);
	}else{
		pstreams[id].running = 0;
		LeaveCriticalSection(&cs);
		goto point_start;
	}

	LeaveCriticalSection(&cs);

	goto point_start;

}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	unsigned long   frame_size = 0, buffer_point = 0, bytecount = 0;
	unsigned long   len, bsize;
	AVPacket        packet;
	int16_t        *fbuffer;
	
	if(!pstreams[id].loaded)return 0;


	if(pstreams[id].dvd_buffer)
		return dsize;


	
	if(pstreams[id].audio.ob_size)
	{
		buffer_point = min((unsigned long)pstreams[id].audio.ob_size, dsize);
		memcpy(adata, (uint8_t*)pstreams[id].audio.obuffer + pstreams[id].audio.ob_start, buffer_point);
		memset((uint8_t*)pstreams[id].audio.obuffer + pstreams[id].audio.ob_start, 0, buffer_point);
		pstreams[id].audio.ob_size -= buffer_point;

		pstreams[id].audio.ob_start += buffer_point;

		if(buffer_point >= dsize) return buffer_point;
	}

	for(;;)
	{
		if(redirect_video_skipping)
		{
			if(av_read_frame(pstreams[id].avf_context, &packet) >= 0)
				av_free_packet(&packet);
		}

		if(av_read_frame(pstreams[id].audio.avf_context, &packet) >= 0)
		{
			if(packet.stream_index == pstreams[id].sel_stream_video && use_thread == 0)
			{
				int decodefrm = 1;
				pstreams[id].newrun = 1;

				/* lets say the highest frame rate is 40fps, then
				   for 1/4 seconds, there're 10 frames... so we're keeping
				   about 1/4 sec time gap buffer (about 1/2 second normally)*/

				if(getactive(&pstreams[id].vid_queue) > 10)
				{
					decodefrm = 0;
				}
				
				if(decodefrm)
					vid_frame_decode(id, &packet, &pstreams[id].newrun);
			}

			if(packet.stream_index == pstreams[id].audio.sel_stream_audio)
			{
				frame_size = packet.size + FF_INPUT_BUFFER_PADDING_SIZE;
				fbuffer = sys_mem_alloc(frame_size);
				memset(fbuffer, 0, frame_size);
				memcpy(fbuffer, packet.data, packet.size);

				
				bsize = sizeof(pstreams[id].audio.obuffer);

				memset(pstreams[id].audio.obuffer, 0, sizeof(pstreams[id].audio.obuffer));
				len = avcodec_decode_audio2(pstreams[id].audio.avc_audio_context, (int16_t*)pstreams[id].audio.obuffer, &bsize, (uint8_t*)fbuffer, frame_size);
				av_free_packet(&packet);
				sys_mem_free(fbuffer);

				//if(len < 0) break;
				if(len <= 0) return 0;

				bytecount = bsize;
				if(buffer_point + bytecount > dsize) bytecount = dsize - buffer_point;

				memcpy(adata + buffer_point, pstreams[id].audio.obuffer, bytecount);

				if(packet.pts != AV_NOPTS_VALUE)
					pstreams[id].audio_pos = av_q2d(pstreams[id].audio.audio_stream->time_base) * packet.pts;
			

				if(buffer_point + bytecount >= dsize)
				{
					pstreams[id].audio.ob_start = bytecount;
					pstreams[id].audio.ob_size  = bsize - bytecount;
					buffer_point = dsize;
					break;
				}else{
					buffer_point += bytecount;
				}
			}
		}else{
			return buffer_point;
		}
	}
	return dsize;
}


int decoder_tagread(const string fname,  struct fennec_audiotag *rtag)
{
	if(!rtag)return 0;

	if(!fname)
	{
		if(rtag->tag_title.tsize         ) { sys_mem_free(rtag->tag_title.tdata         ); rtag->tag_title.tsize         = 0; }
		if(rtag->tag_album.tsize         ) { sys_mem_free(rtag->tag_album.tdata         ); rtag->tag_album.tsize         = 0; }
		if(rtag->tag_artist.tsize        ) { sys_mem_free(rtag->tag_artist.tdata        ); rtag->tag_artist.tsize        = 0; }
		if(rtag->tag_origartist.tsize    ) { sys_mem_free(rtag->tag_origartist.tdata    ); rtag->tag_origartist.tsize    = 0; }
		if(rtag->tag_composer.tsize      ) { sys_mem_free(rtag->tag_composer.tdata      ); rtag->tag_composer.tsize      = 0; }
		if(rtag->tag_lyricist.tsize      ) { sys_mem_free(rtag->tag_lyricist.tdata      ); rtag->tag_lyricist.tsize      = 0; }
		if(rtag->tag_band.tsize          ) { sys_mem_free(rtag->tag_band.tdata          ); rtag->tag_band.tsize          = 0; }
		if(rtag->tag_copyright.tsize     ) { sys_mem_free(rtag->tag_copyright.tdata     ); rtag->tag_copyright.tsize     = 0; }
		if(rtag->tag_publish.tsize       ) { sys_mem_free(rtag->tag_publish.tdata       ); rtag->tag_publish.tsize       = 0; }
		if(rtag->tag_encodedby.tsize     ) { sys_mem_free(rtag->tag_encodedby.tdata     ); rtag->tag_encodedby.tsize     = 0; }
		if(rtag->tag_genre.tsize         ) { sys_mem_free(rtag->tag_genre.tdata         ); rtag->tag_genre.tsize         = 0; }
		if(rtag->tag_year.tsize          ) { sys_mem_free(rtag->tag_year.tdata          ); rtag->tag_year.tsize          = 0; }
		if(rtag->tag_url.tsize           ) { sys_mem_free(rtag->tag_url.tdata           ); rtag->tag_url.tsize           = 0; }
		if(rtag->tag_offiartisturl.tsize ) { sys_mem_free(rtag->tag_offiartisturl.tdata ); rtag->tag_offiartisturl.tsize = 0; }
	/*  if(rtag->tag_filepath.tsize      ) { sys_mem_free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
	    if(rtag->tag_filename.tsize      ) { sys_mem_free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; } */
		if(rtag->tag_comments.tsize      ) { sys_mem_free(rtag->tag_comments.tdata      ); rtag->tag_comments.tsize      = 0; }
		if(rtag->tag_lyric.tsize         ) { sys_mem_free(rtag->tag_lyric.tdata         ); rtag->tag_lyric.tsize         = 0; }
		if(rtag->tag_bpm.tsize           ) { sys_mem_free(rtag->tag_bpm.tdata           ); rtag->tag_bpm.tsize           = 0; }
		if(rtag->tag_tracknum.tsize      ) { sys_mem_free(rtag->tag_tracknum.tdata      ); rtag->tag_tracknum.tsize      = 0; }
		
		return 1;
	}

	rtag->tag_title.tsize         = 0;
	rtag->tag_album.tsize         = 0;
	rtag->tag_artist.tsize        = 0;
	rtag->tag_origartist.tsize    = 0;
	rtag->tag_composer.tsize      = 0;
	rtag->tag_lyricist.tsize      = 0;
	rtag->tag_band.tsize          = 0;
	rtag->tag_copyright.tsize     = 0;
	rtag->tag_publish.tsize       = 0;
	rtag->tag_encodedby.tsize     = 0;
	rtag->tag_genre.tsize         = 0;
	rtag->tag_year.tsize          = 0;
	rtag->tag_url.tsize           = 0;
	rtag->tag_offiartisturl.tsize = 0;
	rtag->tag_filepath.tsize      = 0;
	rtag->tag_filename.tsize      = 0;
	rtag->tag_comments.tsize      = 0;
	rtag->tag_lyric.tsize         = 0;
	rtag->tag_bpm.tsize           = 0;
	rtag->tag_tracknum.tsize      = 0;

	return 1;
}

int decoder_tagwrite(const string fname,  struct fennec_audiotag *rtag)
{

	return 1;
}


/*
 * to get aspect ratio, pass height = 0, aspect ratio will be passed through width (as a double)
 */
int callc video_decoder_getsize(unsigned long id, int *width, int *height)
{
	if(!pstreams[id].loaded)return 0;
	if(!width) return 0;

	if(height)
	{
		if(!pstreams[id].avc_video_context)
		{
			*width = *height = 0;
			return 0;
		}

		*width  = pstreams[id].avc_video_context->width;
		*height = pstreams[id].avc_video_context->height;
	}else{
		/* aspect ratio */
		if(!pstreams[id].avc_video_context)
		{
			*((double*)width) = 0.0;
			return 0;
		}
		
		*((double*)width) = av_q2d(pstreams[id].avc_video_context->sample_aspect_ratio);
	}
	return 1;
}


void frame_queue_init(struct queue_frame *q)
{
	q->memcount = 10;
	q->count = 0;
	q->list = malloc(q->memcount * sizeof(qlist));

	memset(q->list, 0, q->memcount * sizeof(qlist));
}

void frame_queue_uninit(struct queue_frame *q)
{
	unsigned long i;

	for(i=0; i<q->count; i++)
	{
		if(q->list[i].pkt)
			av_free(q->list[i].pkt);

		q->list[i].pkt = 0;
	}
	free(q->list);
	q->list = 0;
}

void frame_queue_push(struct queue_frame *q, void *pkt, double pos)
{
	unsigned long i;

	lastpushframe++;

	for(i=0; i<q->count; i++)
	{
		if(!q->list[i].pkt)
		{
			q->list[i].pkt = pkt;
			q->list[i].pushi = lastpushframe;
			q->list[i].pos = pos;
			lastpush = i;
			return;
		}
	}

	if(q->count >= q->memcount)
	{
		int lm = q->memcount;
		q->memcount += 10;
		q->list = realloc(q->list, q->memcount * sizeof(qlist));
		memset(&q->list[lm], 0, ((q->memcount - lm) - 1) * sizeof(qlist));
	}

	q->list[q->count].pkt = pkt;
	q->list[q->count].pos = pos;
	q->list[q->count].pushi = lastpushframe;
	lastpush = q->count;

	q->count++;
}

int frame_queue_pop_from_pts(struct queue_frame *q, double pts)
{
	unsigned long i;
	static int leasti = -1;
	double  leastn = pts;

	if(leasti != -1)
	{
		if(q->list[leasti].pkt)
			av_free(q->list[leasti].pkt);
				
		q->list[leasti].pkt = 0;

		leasti = -1;
	}

	if(q->count > 1)
	{
		for(i=0; i<q->count; i++)
		{
			if(q->list[i].pos <= leastn)
			{
				if(q->list[i].pkt)
				{
					leastn = q->list[i].pos;
					leasti = i;
				}
			}
		}
	}else{
		return -1;
	}
	return leasti;
}

double get_smallest_pos_gap(struct queue_frame *q, double gpts)
{
	unsigned long i;
	double        cgap = -1;

	for(i=0; i<q->count; i++)
	{
		if((q->list[i].pos > cgap) || (cgap <= 0.0))
		{
			cgap = q->list[i].pos;
		}
	}

	return gpts - cgap;
}

unsigned long getactive(struct queue_frame *q)
{
	unsigned long i, c = 0;
	if(!q->list)return 0;
	for(i=0; i<q->count; i++)
	{
		if(q->list[i].pkt)c++;
	}
	return c;
}

int getmin(struct queue_frame *q)
{
	unsigned long i;
	static int leasti = -1;
	double  leastn = 1000000000;

	if(leasti != -1)
	{
		if(q->list[leasti].pkt)
			av_free(q->list[leasti].pkt);
				
		q->list[leasti].pkt = 0;
		leasti = -1;
	}

	if(q->count > 1)
	{
		for(i=0; i<q->count; i++)
		{
			if(q->list[i].pos <= leastn)
			{
				if(q->list[i].pkt)
				{
					leastn = q->list[i].pos;
					leasti = i;
				}
			}
		}
	}else{
		return -1;
	}

	return leasti;
}

int vid_frame_decode(unsigned long id, AVPacket *packet, int *newrun)
{
	static AVFrame            *pFrame = 0; 
	static AVFrame            *pFrameRGB = 0;
	static int                 numBytes;
	static int                 i, delay;
	static int                 frameFinished = 0;
	void                      *buffer;
	static struct SwsContext  *sws_context;
	static double              last_pst = 0, d, lastd = 1;

	if(!pstreams[id].loaded)return 0;
	if(!packet) return 0;

	EnterCriticalSection(&cs);

	pFrame    = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();

	numBytes = avpicture_get_size(PIX_FMT_RGB32, pstreams[id].avc_video_context->width, pstreams[id].avc_video_context->height);
	buffer   = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
			
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB32, pstreams[id].avc_video_context->width, pstreams[id].avc_video_context->height);

	global_video_pkt_pts = packet->pts;
	avcodec_decode_video(pstreams[id].avc_video_context, pFrame, &frameFinished, packet->data, packet->size);

	if(frameFinished)
	{
		int64_t pts;
		double dpts;

		sws_context = sws_getContext(pstreams[id].avc_video_context->width, pstreams[id].avc_video_context->height, pstreams[id].avc_video_context->pix_fmt, pstreams[id].avc_video_context->width, pstreams[id].avc_video_context->height, PIX_FMT_RGB32, SWS_BICUBIC, 0, 0, 0);
		sws_scale(sws_context, pFrame->data, pFrame->linesize, 0, pstreams[id].avc_video_context->height, pFrameRGB->data, pFrameRGB->linesize);
		
		sws_freeContext(sws_context);

		if(packet->dts == AV_NOPTS_VALUE && pFrame->opaque && *(uint64_t*)pFrame->opaque != AV_NOPTS_VALUE) {
			pts = *(uint64_t *)pFrame->opaque;
		}else if(packet->dts != AV_NOPTS_VALUE){
			pts = packet->dts;
		}else{
			pts = 0;
		}

		if(pstreams[id].lastp >= 0)
		{
			if(pts < pstreams[id].lastp)
				pts = pstreams[id].lastp + (int64_t)av_q2d(pstreams[id].video_stream->r_frame_rate);
		}

		pstreams[id].lastp = pts;
		
		av_free(pFrameRGB);
		av_free(pFrame);

		dpts = (double)pts;
		dpts *= av_q2d(pstreams[id].video_stream->time_base);

		frame_queue_push(&pstreams[id].vid_queue, buffer, dpts);
	}else{
		av_free(pFrameRGB);
		av_free(pFrame);
		av_free(buffer);
	}

	LeaveCriticalSection(&cs);
	return 1;
}

int callc video_decoder_trans_info(int id, long dt_l, double dt_d, void *data)
{
	switch(id)
	{
	case video_set_buffersize: /* audio buffer size for sync */
		current_sync_seconds = dt_d;
		break;

	case video_set_subfile:
		set_custom_primary_subtitle_file((const string)data);
		break;

	case video_get_subfile:
		
		break;

	case video_set_sub_language:
		if(dt_d < 1.0)
			set_primary_subtitle_file(dt_l);
		else
			set_secondary_subtitle_file(dt_l);
		return 1;


	case video_get_sub_language_current:
		if(data)
		{
			string sfname = get_subtitle_file_name(dt_l);
			if(sfname)
				str_cpy(data, sfname);
		}else{
			if(dt_l)
			{
				return get_current_subtitle_file_id(1);
			}else{
				return get_current_subtitle_file_id(0);
			}
			return 0;
		}
		return 0;

	case video_get_sub_language_count:
		return get_subtitle_file_count();

	case video_set_video_stream:
		break;

	case video_get_video_stream_current:
		break;

	case video_get_video_stream_count:
		return count_video_streams;

	case video_set_audio_stream:
		break;

	case video_get_audio_stream_current:
		break;

	case video_get_audio_stream_count:
		return count_audio_streams;

	case video_set_movie_chapter:
		break;

	case video_get_movie_chapter_current:
		break;

	case video_get_movie_chapter_count:
		break;

	case video_set_movie_title:
		break;

	case video_get_movie_title_current:
		break;

	case video_get_movie_title_count:
		break;

	case video_set_movie_angle:
		break;

	case video_get_movie_angle_current:
		break;

	case video_get_movie_angle_count:
		break;

	case video_set_movie_navigation:
		break;

	case video_get_movie_navigation_current:
		break;

	case video_get_movie_navigation_count:
		break;
	}
	return 1;
}

int callc video_decoder_getframe_sync(unsigned long id, void **buff, double audiosec)
{
	static int  lasti = -1;
	unsigned long i, ret = 10;
	double     p = 0, pts = 0, ptd;
	static double lframe = 0, lpts = 0, lptd = 20;

	if(!pstreams[id].loaded)return 0;

	EnterCriticalSection(&cs);

	if(pstreams[id].dvd_buffer)
		pts = audiosec;
	else
		pts = pstreams[id].audio_pos - current_sync_seconds + 0.03;

	if(pts < 0.0)
		pstreams[id].pts_goes_wrong = 1;

	if(pstreams[id].pts_goes_wrong)
		pts = audiosec - current_sync_seconds + 0.03;

	i = frame_queue_pop_from_pts(&pstreams[id].vid_queue, pts);

	if(i == -1)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}
	
	ptd = (pstreams[id].vid_queue.list[i].pos - lframe) * 1000.0;
	
	//ret = ptd;

	lframe = pstreams[id].vid_queue.list[i].pos;
	

	*buff = pstreams[id].vid_queue.list[i].pkt;
	lasti = i;

	p = pstreams[id].vid_queue.list[i].pos;

	for(i=0; i<pstreams[id].vid_queue.count; i++)
	{
		if(pstreams[id].vid_queue.list[i].pos < p)
		{
			if(pstreams[id].vid_queue.list[i].pkt)
				av_free(pstreams[id].vid_queue.list[i].pkt);
				
			pstreams[id].vid_queue.list[i].pkt = 0;
		}
	}

	LeaveCriticalSection(&cs);

	if(ptd < 0) ptd = lptd;
	if(ptd > 1000)ptd = lptd;

	lptd = ptd;

	if(*buff)
		return 10;
	else
		return 0;
}


int callc video_decoder_getframe(unsigned long id, void **buff)
{
	double as;

	if(!pstreams[id].loaded)return 0;
	
	if(pstreams[id].start_pos == -1) pstreams[id].start_pos = (double)timeGetTime();
	as = ((double)timeGetTime() - pstreams[id].start_pos) / 1000.0;
	return video_decoder_getframe_sync(id, buff, as);
}


/*

*/
int callc video_decoder_seek(unsigned long id, double *pos, int iframe, int rel)
{
	if(!pos)
	{
		if(rel)
		{
			/* relative frame seek */
		}else{
			/* absolute frame seek */
		}
	}else{
		/* fraction based seeking */
	}

	return 1;

}


string callc video_decoder_getsubtitle(unsigned long id, float ctime)
{
	if(!pstreams[id].loaded)return 0;
	if(!pstreams[id].initialized) return 0;

	if(ctime >= 0.0f)
	{
		return get_line(1, (float)(pstreams[id].audio_pos - current_sync_seconds) * 1000.0f);
	}else{
		return get_line(0, (float)(pstreams[id].audio_pos - current_sync_seconds) * 1000.0f);
	}
	/*if(!pstreams[id].text_subtitles.count) return 0;

	if(ctime >= 0)
	{
		return subtitles_get(&pstreams[id].text_subtitles, ctime);
	}else{

		return subtitles_get(&pstreams[id].text_subtitles, (float)(pstreams[id].audio_pos - current_sync_seconds));
	}
	*/
	return L"";
}

int decoder_score(const string sfile, int res)
{
	unsigned int      i;
	char              afname[v_sys_maxpath];
	BOOL              useddef = 1;
	AVFormatContext  *avfcontext;

	WideCharToMultiByte(CP_ACP, 0, sfile, -1, afname, sizeof(afname), "?", &useddef);

	av_register_all();

	if(av_open_input_file(&avfcontext, afname, 0, 0, 0) != 0) return 0;

	if(!avfcontext) return 0;

	if(av_find_stream_info(avfcontext) < 0) return 0;


	for(i=0; i<avfcontext->nb_streams; i++)
	{
		if(avfcontext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) return 8000;
	}

	av_close_input_file(avfcontext);
	return 1000;
}

/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
