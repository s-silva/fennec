/**----------------------------------------------------------------------------

 Fennec FFMPEG Decoder Interface.
 Copyright (C) 2010 Chase <c-h@users.sf.net>

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

#include "media decoder.h"

AVCodecContext    *avcodec_opts[CODEC_TYPE_NB];
AVFormatContext   *avformat_opts;
struct SwsContext *sws_opts;

static int sws_flags = SWS_BICUBIC;

double  current_sync_seconds = 0.2;
struct  video_dec  ffvid_dec;

/* variables */

int aud_run = 0;
int current_eof = 0;

int process_quiting = 0;
static AVInputFormat *file_iformat;
static const char *input_filename;
static int fs_screen_width;
static int fs_screen_height;
static int screen_width = 0;
static int screen_height = 0;
static int frame_width = 0;
static int frame_height = 0;
static enum PixelFormat frame_pix_fmt = PIX_FMT_NONE;
static int audio_disable;
static int video_disable;
static int wanted_audio_stream= 0;
static int wanted_video_stream= 0;
static int wanted_subtitle_stream= -1;
static int seek_by_bytes;
static int display_disable;
static int show_status = 1;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = AV_NOPTS_VALUE;
static int debug = 0;
static int debug_mv = 0;
static int step = 0;
static int thread_count = 1;
static int workaround_bugs = 1;
static int fast = 0;
static int genpts = 0;
static int lowres = 0;
static int idct = FF_IDCT_AUTO;
static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;
static int error_recognition = FF_ER_CAREFUL;
static int error_concealment = 3;
static int decoder_reorder_pts= 0;

CRITICAL_SECTION  cs_video, cs_decode, cs_events, cs_pictureq, cs_subpictureq;

/* current context */
static int is_full_screen;
static VideoState *cur_stream;
static int64_t audio_callback_time;

static AVPacket flush_pkt;

//static SDL_Surface *screen;
int    screen = 0;

int    audio_format_ready = 0;
struct plugin_stream *cstream = 0;

int    video_width = 0, video_height = 0;
void  *video_frame = 0;

int    quit_thread_events = 0;

int           last_seek = 0;
unsigned long last_seek_time;
double        last_seek_pos;
int           audio_first_time = 1;

#define sys_cs_create(c) InitializeCriticalSection(c)
#define sys_cs_enter(c) EnterCriticalSection(c)
#define sys_cs_leave(c) LeaveCriticalSection(c)
#define sys_cs_destroy(c) DeleteCriticalSection(c)

static void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));

	sys_cs_create(&q->mutex);
    q->cond = SDL_CreateCond();
}

static void packet_queue_flush(PacketQueue *q)
{
    AVPacketList *pkt, *pkt1;
	
	sys_cs_enter(&q->mutex);
    for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
	sys_cs_leave(&q->mutex);
}


static void packet_queue_end(PacketQueue *q)
{
    packet_queue_flush(q);
	sys_cs_destroy(&q->mutex);
    SDL_DestroyCond(q->cond);
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pkt1;

    /* duplicate the packet */
    if (pkt!=&flush_pkt && av_dup_packet(pkt) < 0)
        return -1;

    pkt1 = av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;


	sys_cs_enter(&q->mutex);

    if (!q->last_pkt)

        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);
    /* XXX: should duplicate packet data in DV case */
    SDL_CondSignal(q->cond);

	sys_cs_leave(&q->mutex);
    return 0;
}

static void packet_queue_abort(PacketQueue *q)
{
	sys_cs_enter(&q->mutex);
    q->abort_request = 1;
    SDL_CondSignal(q->cond);
	sys_cs_leave(&q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret;

	sys_cs_enter(&q->mutex);

    for(;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;

		if(pkt1)
		{
			if(pkt1->pkt.size == 0)
			{
				if(current_eof == 1)
				{
					current_eof = 2;
				}
			}
		}

        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
			sys_cs_leave(&q->mutex);
			SDL_CondSignal(q->cond);
			sys_cs_enter(&q->mutex);
			
			if(current_eof == 1)
			{
				current_eof = 2;
				break;
			}

        }
    }
	sys_cs_leave(&q->mutex);
    return ret;
}

static void free_subpicture(SubPicture *sp)
{
    int i;

    for (i = 0; i < sp->sub.num_rects; i++)
    {
        av_freep(&sp->sub.rects[i]->pict.data[0]);
        av_freep(&sp->sub.rects[i]->pict.data[1]);
        av_freep(&sp->sub.rects[i]);
    }

    av_free(sp->sub.rects);

    memset(&sp->sub, 0, sizeof(AVSubtitle));
}

static void video_image_display(VideoState *is)
{
    VideoPicture *vp;
    SubPicture *sp;
    AVPicture pict;
    float aspect_ratio;
    int width, height, x, y;
    //SDL_Rect rect;
    int i;

    vp = &is->pictq[is->pictq_rindex];
    if (vp->bmp)
	{
        /* XXX: use variable in the frame */
        if (is->video_st->sample_aspect_ratio.num)
            aspect_ratio = av_q2d(is->video_st->sample_aspect_ratio);
        else if (is->video_st->codec->sample_aspect_ratio.num)
            aspect_ratio = av_q2d(is->video_st->codec->sample_aspect_ratio);
        else
            aspect_ratio = 0;
        if (aspect_ratio <= 0.0)
            aspect_ratio = 1.0;
        aspect_ratio *= (float)is->video_st->codec->width / is->video_st->codec->height;
        /* if an active format is indicated, then it overrides the
           mpeg format */


		/*
        if (is->subtitle_st)
        {
            if (is->subpq_size > 0)
            {
                sp = &is->subpq[is->subpq_rindex];

                if (vp->pts >= sp->pts + ((float) sp->sub.start_display_time / 1000))
                {
                    SDL_LockYUVOverlay (vp->bmp);

                    pict.data[0] = vp->bmp->pixels[0];
                    pict.data[1] = vp->bmp->pixels[2];
                    pict.data[2] = vp->bmp->pixels[1];

                    pict.linesize[0] = vp->bmp->pitches[0];
                    pict.linesize[1] = vp->bmp->pitches[2];
                    pict.linesize[2] = vp->bmp->pitches[1];

                    for (i = 0; i < sp->sub.num_rects; i++)
                        blend_subrect(&pict, sp->sub.rects[i],
                                      vp->bmp->w, vp->bmp->h);

                    SDL_UnlockYUVOverlay (vp->bmp);
                }
            }
        }
		*/

        /* XXX: we suppose the screen has a 1.0 pixel ratio */
        height = is->height;
        width = ((int)rint(height * aspect_ratio)) & ~1;
        if (width > is->width) {
            width = is->width;
            height = ((int)rint(width / aspect_ratio)) & ~1;
        }
        x = (is->width - width) / 2;
        y = (is->height - height) / 2;
        if (!is->no_background) {
            /* fill the background */
            //            fill_border(is, x, y, width, height, QERGB(0x00, 0x00, 0x00));
        } else {
            is->no_background = 0;
        }
        //rect.x = is->xleft + x;
        //rect.y = is->ytop  + y;
        //rect.w = width;
        //rect.h = height;

		video_frame = vp->rgb;

    } else {

    }
}

static void video_audio_display(VideoState *s)
{

}

static int video_open(VideoState *is)
{
    is->width = 320;
    is->height = 240;
	screen = 1;
    return 0;
}

/* display the current picture, if any */
static void video_display(VideoState *is)
{
    if(!screen)
        video_open(cur_stream);
  //  if (is->audio_st && is->show_audio)
  //      video_audio_display(is);
   else if (is->video_st)
        video_image_display(is);
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque)
{
    SDL_Event ev;
    ev.type = FF_REFRESH_EVENT;
    ev.user.data1 = opaque;
    SDL_PushEvent(&ev);
    return 0; /* 0 means stop timer */
}

/* schedule a video refresh in 'delay' ms */
static void schedule_refresh(VideoState *is, int delay)
{
    if(!delay) delay=1; //SDL seems to be buggy when the delay is 0
	//Sleep(delay);
	//sdl_refresh_timer_cb(delay, is);
    SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}

/* get the current audio clock value */
static double get_audio_clock(VideoState *is)
{
    double pts;
    int hw_buf_size, bytes_per_sec;
    pts = is->audio_clock;
    hw_buf_size = audio_write_get_buf_size(is);
    bytes_per_sec = 0;
    if (is->audio_st) {
        bytes_per_sec = is->audio_st->codec->sample_rate *
            2 * is->audio_st->codec->channels;
    }
    //if (bytes_per_sec)
    //    pts -= (double)0.2 /*hw_buf_size*/ / bytes_per_sec;
    return pts;// - 0.2;
}

/* get the current video clock value */
static double get_video_clock(VideoState *is)
{
    double delta;
    if (is->paused) {
        delta = 0;
    } else {
        delta = (av_gettime() - is->video_current_pts_time) / 1000000.0;
    }
    return is->video_current_pts + delta;
}

/* get the current external clock value */
static double get_external_clock(VideoState *is)
{
    int64_t ti;
    ti = av_gettime();
    return is->external_clock + ((ti - is->external_clock_time) * 1e-6);
}

/* get the current master clock value */
static double get_master_clock(VideoState *is)
{
    double val;

    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st)
            val = get_video_clock(is);
        else
            val = get_audio_clock(is);
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st)
            val = get_audio_clock(is);
        else
            val = get_video_clock(is);
    } else {
        val = get_external_clock(is);
    }
    return val;
}

/* seek in the stream */
static void stream_seek(VideoState *is, int64_t pos, int64_t rel)
{
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_rel = rel;
        if (seek_by_bytes)
            is->seek_flags |= AVSEEK_FLAG_BYTE;
        is->seek_req = 1;
    }
}

/* pause or resume the video */
static void stream_pause(VideoState *is)
{
    is->paused = !is->paused;
    if (!is->paused) {
        is->video_current_pts = get_video_clock(is);
        is->frame_timer += (av_gettime() - is->video_current_pts_time) / 1000000.0;
    }
}

static double compute_frame_delay(double frame_current_pts, VideoState *is)
{
    double actual_delay, delay, sync_threshold, ref_clock, diff;

    /* compute nominal delay */
    delay = frame_current_pts - is->frame_last_pts;
    if (delay <= 0 || delay >= 10.0) {
        /* if incorrect delay, use previous one */
        delay = is->frame_last_delay;
    } else {
        is->frame_last_delay = delay;
    }
    is->frame_last_pts = frame_current_pts;

    /* update delay to follow master synchronisation source */
    if (((is->av_sync_type == AV_SYNC_AUDIO_MASTER && is->audio_st) ||
         is->av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        ref_clock = get_master_clock(is);
        diff = frame_current_pts - ref_clock;

        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
        if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
            if (diff <= -sync_threshold)
                delay = 0;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }

    is->frame_timer += delay;
    /* compute the REAL delay (we need to do that to avoid
       long term errors */
    actual_delay = is->frame_timer - (av_gettime() / 1000000.0);
    if (actual_delay < 0.010) {
        /* XXX: should skip picture */
        actual_delay = 0.010;
    }

    return actual_delay;
}

/* called to display each frame */
static void video_refresh_timer(void *opaque)
{
    VideoState *is = opaque;
    VideoPicture *vp;

    SubPicture *sp, *sp2;

	if(process_quiting) return;

    if (is->video_st) {
        if (is->pictq_size == 0) {
            /* if no picture, need to wait */
            schedule_refresh(is, 1);
        } else {
            /* dequeue the picture */
            vp = &is->pictq[is->pictq_rindex];

            /* update current video pts */
            is->video_current_pts = vp->pts;
            is->video_current_pts_time = av_gettime();

            /* launch timer for next picture */
            schedule_refresh(is, (int)(compute_frame_delay(vp->pts, is) * 1000 + 0.5));

            if(is->subtitle_st) {
                if (is->subtitle_stream_changed) {
					sys_cs_enter(&cs_subpictureq);
                    while (is->subpq_size) {
                        free_subpicture(&is->subpq[is->subpq_rindex]);

                        /* update queue size and signal for next picture */
                        if (++is->subpq_rindex == SUBPICTURE_QUEUE_SIZE)
                            is->subpq_rindex = 0;

                        is->subpq_size--;
                    }
                    is->subtitle_stream_changed = 0;

                    SDL_CondSignal(is->subpq_cond);
					sys_cs_leave(&cs_subpictureq);
                } else {
                    if (is->subpq_size > 0) {
                        sp = &is->subpq[is->subpq_rindex];

                        if (is->subpq_size > 1)
                            sp2 = &is->subpq[(is->subpq_rindex + 1) % SUBPICTURE_QUEUE_SIZE];
                        else
                            sp2 = NULL;

                        if ((is->video_current_pts > (sp->pts + ((float) sp->sub.end_display_time / 1000)))
                                || (sp2 && is->video_current_pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
                        {
                            free_subpicture(sp);

                            /* update queue size and signal for next picture */
                            if (++is->subpq_rindex == SUBPICTURE_QUEUE_SIZE)
                                is->subpq_rindex = 0;

                            sys_cs_enter(&cs_subpictureq);
                            is->subpq_size--;
                            SDL_CondSignal(is->subpq_cond);
							sys_cs_leave(&cs_subpictureq);
                        }
                    }
                }
            }

            /* display picture */
            video_display(is);

            /* update queue size and signal for next picture */
            if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
                is->pictq_rindex = 0;

			sys_cs_enter(&cs_pictureq);
            is->pictq_size--;
            SDL_CondSignal(is->pictq_cond);
			sys_cs_leave(&cs_pictureq);
        }
    } else if (is->audio_st) {
        /* draw the next audio frame */

        schedule_refresh(is, 40);

        /* if only audio stream, then display the audio bars (better
           than nothing, just to test the implementation */

        /* display picture */
        video_display(is);
    } else {
        schedule_refresh(is, 100);
    }
    
}

/* allocate a picture (needs to do that in main thread to avoid
   potential locking problems */
static void alloc_picture(void *opaque)
{
    VideoState *is = opaque;
    VideoPicture *vp;

    vp = &is->pictq[is->pictq_windex];

	if (vp->bmp)
	{
		DeleteCriticalSection(&vp->cs);
		free(vp->rgb);
	}
	vp->bmp = 1;
    //if (vp->bmp)
    //    SDL_FreeYUVOverlay(vp->bmp);

    //vp->bmp = SDL_CreateYUVOverlay(is->video_st->codec->width,
    //                               is->video_st->codec->height,
    //                               SDL_YV12_OVERLAY,
    //                              screen); 
    vp->width = is->video_st->codec->width;
    vp->height = is->video_st->codec->height;
	InitializeCriticalSection(&vp->cs);

	vp->rgb = malloc(vp->width * vp->height * 4);

	sys_cs_enter(&cs_pictureq);
    vp->allocated = 1;
    SDL_CondSignal(is->pictq_cond);
	sys_cs_leave(&cs_pictureq);
}

/**
 *
 * @param pts the dts of the pkt / pts of the frame and guessed if not known
 */
static int queue_picture(VideoState *is, AVFrame *src_frame, double pts)
{
    VideoPicture *vp;
    int dst_pix_fmt;

    /* wait until we have space to put a new picture */
	sys_cs_enter(&cs_pictureq);
    while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE &&
           !is->videoq.abort_request) {

	   sys_cs_leave(&cs_pictureq);
		SDL_CondSignal(is->pictq_cond);
	   sys_cs_enter(&cs_pictureq);
    }
	sys_cs_leave(&cs_pictureq);

    if (is->videoq.abort_request)
        return -1;

    vp = &is->pictq[is->pictq_windex];

    /* alloc or resize hardware picture buffer */
    if (!vp->bmp ||
        vp->width != is->video_st->codec->width ||
        vp->height != is->video_st->codec->height) {
        SDL_Event ev;

        vp->allocated = 0;

        /* the allocation must be done in the main thread to avoid
           locking problems */
        ev.type = FF_ALLOC_EVENT;
        ev.user.data1 = is;
        SDL_PushEvent(&ev);

        /* wait until the picture is allocated */
		sys_cs_enter(&cs_pictureq);
        while (!vp->allocated && !is->videoq.abort_request) {
			sys_cs_leave(&cs_pictureq);
			SDL_CondSignal(is->pictq_mutex);
			sys_cs_enter(&cs_pictureq);

        }
		sys_cs_leave(&cs_pictureq);

        if (is->videoq.abort_request)
            return -1;
    }

    /* if the frame is not skipped, then display it */
    if (vp->bmp) {
        AVPicture pict;

        /* get a pointer on the bitmap */
       // SDL_LockYUVOverlay (vp->bmp);
		//EnterCriticalSection(&vp->cs);

		video_width = is->video_st->codec->width;
		video_height = is->video_st->codec->height;

        memset(&pict,0,sizeof(AVPicture));
		
#ifndef sff

		dst_pix_fmt = PIX_FMT_RGB32;
        pict.data[0] = vp->rgb;
		pict.linesize[0] = is->video_st->codec->width * 4;
#else
		dst_pix_fmt = PIX_FMT_YUV420P;
		pict.data[0] = vp->bmp->pixels[0];
        pict.data[1] = vp->bmp->pixels[2];
        pict.data[2] = vp->bmp->pixels[1];

        
		pict.linesize[0] = vp->bmp->pitches[0];
        pict.linesize[1] = vp->bmp->pitches[2];
        pict.linesize[2] = vp->bmp->pitches[1];
#endif
        sws_flags = av_get_int(sws_opts, "sws_flags", NULL);
        is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
            is->video_st->codec->width, is->video_st->codec->height,
            is->video_st->codec->pix_fmt,
            is->video_st->codec->width, is->video_st->codec->height,
            dst_pix_fmt, sws_flags, NULL, NULL, NULL);
        if (is->img_convert_ctx == NULL) {
            fprintf(stderr, "Cannot initialize the conversion context\n");
            exit(1);
        }
        sws_scale(is->img_convert_ctx, src_frame->data, src_frame->linesize,
                  0, is->video_st->codec->height, pict.data, pict.linesize);
        /* update the bitmap content */
        //SDL_UnlockYUVOverlay(vp->bmp);
		//LeaveCriticalSection(&vp->cs);

        vp->pts = pts;

        /* now we can update the picture count */
        if (++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)
            is->pictq_windex = 0;
		sys_cs_enter(&cs_pictureq);
        is->pictq_size++;
		sys_cs_leave(&cs_pictureq);
    }
    return 0;
}

/**
 * compute the exact PTS for the picture if it is omitted in the stream
 * @param pts1 the dts of the pkt / pts of the frame
 */
static int output_picture2(VideoState *is, AVFrame *src_frame, double pts1)
{
    double frame_delay, pts;

    pts = pts1;

    if (pts != 0) {
        /* update video clock with pts, if present */
        is->video_clock = pts;
    } else {
        pts = is->video_clock;
    }
    /* update video clock for next frame */
    frame_delay = av_q2d(is->video_st->codec->time_base);
    /* for MPEG2, the frame can be repeated, so we update the
       clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;

    return queue_picture(is, src_frame, pts);
}

static int video_thread(void *arg)
{
    VideoState *is = arg;
    AVPacket pkt1, *pkt = &pkt1;
    int len1, got_picture;
    AVFrame *frame= avcodec_alloc_frame();
    double pts;

    for(;;){

		//EnterCriticalSection(&cs_video);

		if(is->videoq.abort_request)
		{
			//LeaveCriticalSection(&cs_video);
			break;
		}

        while (is->paused && !is->videoq.abort_request) {
			//LeaveCriticalSection(&cs_video);
            Sleep(10);
			//EnterCriticalSection(&cs_video);
        }
        if (packet_queue_get(&is->videoq, pkt, 1) < 0)
		{
			//LeaveCriticalSection(&cs_video);
			break;
		}

        if(pkt->data == flush_pkt.data){
            avcodec_flush_buffers(is->video_st->codec);
			//LeaveCriticalSection(&cs_video);
            continue;
        }

        /* NOTE: ipts is the PTS of the _first_ picture beginning in
           this packet, if any */
        is->video_st->codec->reordered_opaque= pkt->pts;
        len1 = avcodec_decode_video2(is->video_st->codec,
                                    frame, &got_picture,
                                    pkt);

        if(   (decoder_reorder_pts || pkt->dts == AV_NOPTS_VALUE)
           && frame->reordered_opaque != AV_NOPTS_VALUE)
            pts= frame->reordered_opaque;
        else if(pkt->dts != AV_NOPTS_VALUE)
            pts= pkt->dts;
        else
            pts= 0;
        pts *= av_q2d(is->video_st->time_base);

//            if (len1 < 0)
//                break;
        if (got_picture) {
            if (output_picture2(is, frame, pts) < 0)
			{
				//LeaveCriticalSection(&cs_video);
				goto the_end;
			}
        }
        av_free_packet(pkt);
        if (step)
            if (cur_stream)
                stream_pause(cur_stream);

		Sleep(10);

		//LeaveCriticalSection(&cs_video);
    }
 the_end:
    av_free(frame);
    return 0;
}

static int subtitle_thread(void *arg)
{
    VideoState *is = arg;
    SubPicture *sp;
    AVPacket pkt1, *pkt = &pkt1;
    int len1, got_subtitle;
    double pts;
    int i, j;
    int r, g, b, y, u, v, a;

    for(;;) {
        while (is->paused && !is->subtitleq.abort_request) {
            Sleep(10);
        }
        if (packet_queue_get(&is->subtitleq, pkt, 1) < 0)
            break;

        if(pkt->data == flush_pkt.data){
            avcodec_flush_buffers(is->subtitle_st->codec);
            continue;
        }
		sys_cs_enter(&cs_subpictureq);
        while (is->subpq_size >= SUBPICTURE_QUEUE_SIZE &&
               !is->subtitleq.abort_request) {
			   sys_cs_leave(&cs_subpictureq);
			   SDL_CondSignal(is->subpq_cond);
			   sys_cs_enter(&cs_subpictureq);
        }
		sys_cs_leave(&cs_subpictureq);

        if (is->subtitleq.abort_request)
            goto the_end;

        sp = &is->subpq[is->subpq_windex];

       /* NOTE: ipts is the PTS of the _first_ picture beginning in
           this packet, if any */
        pts = 0;
        if (pkt->pts != AV_NOPTS_VALUE)
            pts = av_q2d(is->subtitle_st->time_base)*pkt->pts;

        len1 = avcodec_decode_subtitle2(is->subtitle_st->codec,
                                    &sp->sub, &got_subtitle,
                                    pkt);
//            if (len1 < 0)
//                break;
        if (got_subtitle && sp->sub.format == 0) {
            sp->pts = pts;

            for (i = 0; i < sp->sub.num_rects; i++)
            {
                for (j = 0; j < sp->sub.rects[i]->nb_colors; j++)
                {
                    RGBA_IN(r, g, b, a, (uint32_t*)sp->sub.rects[i]->pict.data[1] + j);
                    y = RGB_TO_Y_CCIR(r, g, b);
                    u = RGB_TO_U_CCIR(r, g, b, 0);
                    v = RGB_TO_V_CCIR(r, g, b, 0);
                    YUVA_OUT((uint32_t*)sp->sub.rects[i]->pict.data[1] + j, y, u, v, a);
                }
            }

            /* now we can update the picture count */
            if (++is->subpq_windex == SUBPICTURE_QUEUE_SIZE)
                is->subpq_windex = 0;
			sys_cs_enter(&cs_subpictureq);
            is->subpq_size++;
			sys_cs_leave(&cs_subpictureq);
        }
        av_free_packet(pkt);
//        if (step)
//            if (cur_stream)
//                stream_pause(cur_stream);
    }
 the_end:
    return 0;
}

/* copy samples for viewing in editor window */
static void update_sample_display(VideoState *is, short *samples, int samples_size)
{
   /* int size, len, channels;

    channels = is->audio_st->codec->channels;

    size = samples_size / sizeof(short);
    while (size > 0) {
        len = SAMPLE_ARRAY_SIZE - is->sample_array_index;
        if (len > size)
            len = size;
        memcpy(is->sample_array + is->sample_array_index, samples, len * sizeof(short));
        samples += len;
        is->sample_array_index += len;
        if (is->sample_array_index >= SAMPLE_ARRAY_SIZE)
            is->sample_array_index = 0;
        size -= len;
    } */
}

/* return the new audio buffer size (samples can be added or deleted
   to get better sync if video or external master clock) */
static int synchronize_audio(VideoState *is, short *samples,
                             int samples_size1, double pts)
{
    int n, samples_size;
    double ref_clock;

    n = 2 * is->audio_st->codec->channels;
    samples_size = samples_size1;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (((is->av_sync_type == AV_SYNC_VIDEO_MASTER && is->video_st) ||
         is->av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
        double diff, avg_diff;
        int wanted_size, min_size, max_size, nb_samples;

        ref_clock = get_master_clock(is);
        diff = get_audio_clock(is) - ref_clock;

        if (diff < AV_NOSYNC_THRESHOLD) {
            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
            if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                is->audio_diff_avg_count++;
            } else {
                /* estimate the A-V difference */
                avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

                if (fabs(avg_diff) >= is->audio_diff_threshold) {
                    wanted_size = samples_size + ((int)(diff * is->audio_st->codec->sample_rate) * n);
                    nb_samples = samples_size / n;

                    min_size = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
                    max_size = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
                    if (wanted_size < min_size)
                        wanted_size = min_size;
                    else if (wanted_size > max_size)
                        wanted_size = max_size;

                    /* add or remove samples to correction the synchro */
                    if (wanted_size < samples_size) {
                        /* remove samples */
                        samples_size = wanted_size;
                    } else if (wanted_size > samples_size) {
                        uint8_t *samples_end, *q;
                        int nb;

                        /* add samples */
                        nb = (samples_size - wanted_size);
                        samples_end = (uint8_t *)samples + samples_size - n;
                        q = samples_end + n;
                        while (nb > 0) {
                            memcpy(q, samples_end, n);
                            q += n;
                            nb -= n;
                        }
                        samples_size = wanted_size;
                    }
                }
            }
        } else {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            is->audio_diff_avg_count = 0;
            is->audio_diff_cum = 0;
        }
    }

    return samples_size;
}

/* decode one audio frame and returns its uncompressed size */
static int audio_decode_frame(VideoState *is, double *pts_ptr)
{
    AVPacket *pkt_temp = &is->audio_pkt_temp;
    AVPacket *pkt = &is->audio_pkt;
    AVCodecContext *dec= is->audio_st->codec;
    int n, len1, data_size;
    double pts;

    for(;;) {
        /* NOTE: the audio packet can contain several frames */

        while (pkt_temp->size > 0) {
            data_size = sizeof(is->audio_buf1);
            len1 = avcodec_decode_audio3(dec,
                                        (int16_t *)is->audio_buf1, &data_size,
                                        pkt_temp);

            if (len1 < 0) {
                /* if error, we skip the frame */
                pkt_temp->size = 0;
                break;
            }

            pkt_temp->data += len1;
            pkt_temp->size -= len1;
            if (data_size <= 0)
                continue;

            if (dec->sample_fmt != is->audio_src_fmt) {
                if (is->reformat_ctx)
                    av_audio_convert_free(is->reformat_ctx);
                is->reformat_ctx= av_audio_convert_alloc(SAMPLE_FMT_S16, 1,
                                                         dec->sample_fmt, 1, NULL, 0);
                if (!is->reformat_ctx) {
                    fprintf(stderr, "Cannot convert %s sample format to %s sample format\n",
                        avcodec_get_sample_fmt_name(dec->sample_fmt),
                        avcodec_get_sample_fmt_name(SAMPLE_FMT_S16));
                        break;
                }
                is->audio_src_fmt= dec->sample_fmt;
            }

            if (is->reformat_ctx) {
                const void *ibuf[6]= {is->audio_buf1};
                void *obuf[6]= {is->audio_buf2};
                int istride[6]= {av_get_bits_per_sample_format(dec->sample_fmt)/8};
                int ostride[6]= {2};
                int len= data_size/istride[0];
                if (av_audio_convert(is->reformat_ctx, obuf, ostride, ibuf, istride, len)<0) {
                    printf("av_audio_convert() failed\n");
                    break;
                }
                is->audio_buf= is->audio_buf2;
                /* FIXME: existing code assume that data_size equals framesize*channels*2
                          remove this legacy cruft */
                data_size= len*2;
            }else{
                is->audio_buf= is->audio_buf1;
            }

            /* if no pts, then compute it */
            pts = is->audio_clock;
            *pts_ptr = pts;
            n = 2 * dec->channels;
            is->audio_clock += (double)data_size /
                (double)(n * dec->sample_rate);
#if defined(DEBUG_SYNC)
            {
                static double last_clock;
                printf("audio: delay=%0.3f clock=%0.3f pts=%0.3f\n",
                       is->audio_clock - last_clock,
                       is->audio_clock, pts);
                last_clock = is->audio_clock;
            }
#endif
            return data_size;
        }

        /* free the current packet */
        if (pkt->data)
            av_free_packet(pkt);

        if (is->paused || is->audioq.abort_request) {
            return -1;
        }

        /* read next packet */
        if (packet_queue_get(&is->audioq, pkt, 0) < 0)
            return -1;
        if(pkt->data == flush_pkt.data){
            avcodec_flush_buffers(dec);
            continue;
        }

		if(current_eof == 2)
			if(pkt->size <= 0)
				return -1;


        pkt_temp->data = pkt->data;
        pkt_temp->size = pkt->size;

        /* if update the audio clock with the pts */
        if (pkt->pts != AV_NOPTS_VALUE) {
            is->audio_clock = av_q2d(is->audio_st->time_base)*pkt->pts;
        }
    }
}

/* get the current audio output buffer size, in samples. With SDL, we
   cannot have a precise information */
static int audio_write_get_buf_size(VideoState *is)
{
    return is->audio_buf_size - is->audio_buf_index;
}


/* prepare a new audio buffer */
static int sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    VideoState *is = opaque;
    int audio_size, len1, oldlen = len;
    double pts;

    audio_callback_time = av_gettime();

    while (len > 0) {
        if (is->audio_buf_index >= is->audio_buf_size) {
           audio_size = audio_decode_frame(is, &pts);

           if (audio_size < 0) {
                /* if error, just output silence */
               is->audio_buf = is->audio_buf1;
               is->audio_buf_size = 1024;
               memset(is->audio_buf, 0, is->audio_buf_size);
           } else {
               if (is->show_audio)
                   update_sample_display(is, (int16_t *)is->audio_buf, audio_size);
               audio_size = synchronize_audio(is, (int16_t *)is->audio_buf, audio_size,
                                              pts);
               is->audio_buf_size = audio_size;
           }
           is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
            len1 = len;
        memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }

	return oldlen - len;
}

/* open a given stream. Return 0 if OK */
static int stream_component_open(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *enc;
    AVCodec *codec;
    SDL_AudioSpec wanted_spec, spec;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    enc = ic->streams[stream_index]->codec;

    /* prepare audio output */
    if (enc->codec_type == CODEC_TYPE_AUDIO) {
        if (enc->channels > 0) {
            enc->request_channels = FFMIN(2, enc->channels);
        } else {
            enc->request_channels = 2;
        }
    }

    codec = avcodec_find_decoder(enc->codec_id);
    enc->debug_mv = debug_mv;
    enc->debug = debug;
    enc->workaround_bugs = workaround_bugs;
    enc->lowres = lowres;
    if(lowres) enc->flags |= CODEC_FLAG_EMU_EDGE;
    enc->idct_algo= idct;
    if(fast) enc->flags2 |= CODEC_FLAG2_FAST;
    enc->skip_frame= skip_frame;
    enc->skip_idct= skip_idct;
    enc->skip_loop_filter= skip_loop_filter;
    enc->error_recognition= error_recognition;
    enc->error_concealment= error_concealment;

    //set_context_opts(enc, avcodec_opts[enc->codec_type], 0);

    if (!codec ||
        avcodec_open(enc, codec) < 0)
        return -1;

    /* prepare audio output */
    if (enc->codec_type == CODEC_TYPE_AUDIO) {
        wanted_spec.freq = enc->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = enc->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = sdl_audio_callback;
        wanted_spec.userdata = is;

		cstream->frequency = enc->sample_rate;
		cstream->bitspersample = 16;
		cstream->channels = enc->channels;
		cstream->duration = (unsigned long)(ic->duration / 1000LL);
        /*if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
            fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
            return -1;
        }*/
        is->audio_hw_buf_size = (200 * enc->sample_rate * enc->channels * 2) / 1000;
        is->audio_src_fmt= SAMPLE_FMT_S16;
    }

    if(thread_count>1)
        avcodec_thread_init(enc, thread_count);
    enc->thread_count= thread_count;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch(enc->codec_type) {
    case CODEC_TYPE_AUDIO:
        is->audio_stream = stream_index;
        is->audio_st = ic->streams[stream_index];
        is->audio_buf_size = 0;
        is->audio_buf_index = 0;

        /* init averaging filter */
        is->audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        is->audio_diff_avg_count = 0;
        /* since we do not have a precise anough audio fifo fullness,
           we correct audio sync only if larger than this threshold */
        is->audio_diff_threshold = 0.01;// 2.0 * SDL_AUDIO_BUFFER_SIZE / enc->sample_rate;

        memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
        packet_queue_init(&is->audioq);

		audio_format_ready = 1;
        //SDL_PauseAudio(0);
        break;
    case CODEC_TYPE_VIDEO:
        is->video_stream = stream_index;
        is->video_st = ic->streams[stream_index];

        is->frame_last_delay = 40e-3;
        is->frame_timer = (double)av_gettime() / 1000000.0;
        is->video_current_pts_time = av_gettime();

        packet_queue_init(&is->videoq);
        is->video_tid = SDL_CreateThread(video_thread, is);
        break;
    case CODEC_TYPE_SUBTITLE:
        is->subtitle_stream = stream_index;
        is->subtitle_st = ic->streams[stream_index];
        packet_queue_init(&is->subtitleq);

        is->subtitle_tid = SDL_CreateThread(subtitle_thread, is);
        break;
    default:
        break;
    }
    return 0;
}

static void stream_component_close(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *enc;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    enc = ic->streams[stream_index]->codec;

    switch(enc->codec_type) {
    case CODEC_TYPE_AUDIO:
        packet_queue_abort(&is->audioq);

       // SDL_CloseAudio();

        packet_queue_end(&is->audioq);
        if (is->reformat_ctx)
            av_audio_convert_free(is->reformat_ctx);
        break;
    case CODEC_TYPE_VIDEO:

        packet_queue_abort(&is->videoq);

        /* note: we also signal this mutex to make sure we deblock the
           video thread in all cases */

		sys_cs_enter(&cs_pictureq);
        SDL_CondSignal(is->pictq_cond);
		sys_cs_leave(&cs_pictureq);


        SDL_WaitThread(is->video_tid, NULL);

        packet_queue_end(&is->videoq);
        break;
    case CODEC_TYPE_SUBTITLE:
        packet_queue_abort(&is->subtitleq);

        /* note: we also signal this mutex to make sure we deblock the
           video thread in all cases */

		sys_cs_enter(&cs_subpictureq);
        is->subtitle_stream_changed = 1;

        SDL_CondSignal(is->subpq_cond);

		sys_cs_leave(&cs_subpictureq);

        SDL_WaitThread(is->subtitle_tid, NULL);

        packet_queue_end(&is->subtitleq);
        break;
    default:
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    avcodec_close(enc);
    switch(enc->codec_type) {
    case CODEC_TYPE_AUDIO:
        is->audio_st = NULL;
        is->audio_stream = -1;
        break;
    case CODEC_TYPE_VIDEO:
        is->video_st = NULL;
        is->video_stream = -1;
        break;
    case CODEC_TYPE_SUBTITLE:
        is->subtitle_st = NULL;
        is->subtitle_stream = -1;
        break;
    default:
        break;
    }
}

/* since we have only one decoding thread, we can use a global
   variable instead of a thread local variable */
static VideoState *global_video_state;

static int decode_interrupt_cb(void)
{
    return (global_video_state && global_video_state->abort_request);
}

/* this thread gets the stream from the disk or the network */
static int decode_thread(void *arg)
{
    VideoState *is = arg;
    AVFormatContext *ic;
    int err, i, ret, video_index, audio_index, subtitle_index;
    AVPacket pkt1, *pkt = &pkt1;
    AVFormatParameters params, *ap = &params;
    int eof=0;

    video_index = -1;
    audio_index = -1;
    subtitle_index = -1;
    is->video_stream = -1;
    is->audio_stream = -1;
    is->subtitle_stream = -1;

    global_video_state = is;
    url_set_interrupt_cb(decode_interrupt_cb);

    memset(ap, 0, sizeof(*ap));

    ap->width = frame_width;
    ap->height= frame_height;
    ap->time_base.den = 25;
	ap->time_base.num = 1;
    ap->pix_fmt = frame_pix_fmt;

    err = av_open_input_file(&ic, is->filename, is->iformat, 0, ap);
    if (err < 0) {
        //print_error(is->filename, err);
        ret = -1;
        goto fail;
    }
    is->ic = ic;

    if(genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    err = av_find_stream_info(ic);
    if (err < 0) {
        fprintf(stderr, "%s: could not find codec parameters\n", is->filename);
        ret = -1;
        goto fail;
    }
    if(ic->pb)
        ic->pb->eof_reached= 0; //FIXME hack, ffplay maybe should not use url_feof() to test for the end


    /* if seeking requested, we execute it */
    if (start_time != AV_NOPTS_VALUE) {
        int64_t timestamp;

        timestamp = start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0) {
            fprintf(stderr, "%s: could not seek to position %0.3f\n",
                    is->filename, (double)timestamp / AV_TIME_BASE);
        }
    }

    for(i = 0; i < ic->nb_streams; i++) {
        AVCodecContext *enc = ic->streams[i]->codec;
        ic->streams[i]->discard = AVDISCARD_ALL;
        switch(enc->codec_type) {
        case CODEC_TYPE_AUDIO:
            if (wanted_audio_stream-- >= 0 && !audio_disable)
                audio_index = i;
            break;
        case CODEC_TYPE_VIDEO:
            if (wanted_video_stream-- >= 0 && !video_disable)
                video_index = i;
            break;
        case CODEC_TYPE_SUBTITLE:
            if (wanted_subtitle_stream-- >= 0 && !video_disable)
                subtitle_index = i;
            break;
        default:
            break;
        }
    }
    if (show_status) {
        dump_format(ic, 0, is->filename, 0);
    }

    /* open the streams */
    if (audio_index >= 0) {
        stream_component_open(is, audio_index);
    }

    if (video_index >= 0) {
        stream_component_open(is, video_index);
    } else {
        if (!display_disable)
            is->show_audio = 1;
    }

    if (subtitle_index >= 0) {
        stream_component_open(is, subtitle_index);
    }

    if (is->video_stream < 0 && is->audio_stream < 0) {
        fprintf(stderr, "%s: could not open codecs\n", is->filename);
        ret = -1;
        goto fail;
    }

    for(;;) {

		//EnterCriticalSection(&cs_decode);

		if (is->abort_request){
			//LeaveCriticalSection(&cs_decode);
            break;
		}
        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
                av_read_pause(ic);
            else
                av_read_play(ic);
        }
        if (is->seek_req) {
            int64_t seek_target= is->seek_pos;
            int64_t seek_min= is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max= is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
//FIXME the +-2 is due to rounding being not done in the correct direction in generation
//      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0) {
                fprintf(stderr, "%s: error while seeking\n", is->ic->filename);
            }else{
                if (is->audio_stream >= 0) {
                    packet_queue_flush(&is->audioq);
                    packet_queue_put(&is->audioq, &flush_pkt);
                }
                if (is->subtitle_stream >= 0) {
                    packet_queue_flush(&is->subtitleq);
                    packet_queue_put(&is->subtitleq, &flush_pkt);
                }
                if (is->video_stream >= 0) {
                    packet_queue_flush(&is->videoq);
                    packet_queue_put(&is->videoq, &flush_pkt);
                }
            }
            is->seek_req = 0;
            eof= 0;
        }

        /* if the queue are full, no need to read more */
        if (is->audioq.size > MAX_AUDIOQ_SIZE ||
            is->videoq.size > MAX_VIDEOQ_SIZE ||
            is->subtitleq.size > MAX_SUBTITLEQ_SIZE) {
            /* wait 10 ms */
            Sleep(10);
			//LeaveCriticalSection(&cs_decode);
            continue;
        }
        if(url_feof(ic->pb) || eof) {
            if(is->video_stream >= 0){
                av_init_packet(pkt);
                pkt->data=NULL;
                pkt->size=0;
                pkt->stream_index= is->video_stream;
                packet_queue_put(&is->videoq, pkt);

				current_eof = 1;
            }
            Sleep(10);
			//LeaveCriticalSection(&cs_decode);
            continue;
		}else{
			current_eof = 0;
		}

        ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF)
                eof=1;
            if (url_ferror(ic->pb))
                break;
            Sleep(100); /* wait for user ev */
			//LeaveCriticalSection(&cs_decode);
            continue;
        }
        if (pkt->stream_index == is->audio_stream) {
            packet_queue_put(&is->audioq, pkt);
        } else if (pkt->stream_index == is->video_stream) {
            packet_queue_put(&is->videoq, pkt);
        } else if (pkt->stream_index == is->subtitle_stream) {
            packet_queue_put(&is->subtitleq, pkt);
        } else {
            av_free_packet(pkt);
        }

		//LeaveCriticalSection(&cs_decode);
    }
    /* wait until the end */
    while (!is->abort_request) {
        Sleep(100);
    }

    ret = 0;
 fail:
    /* disable interrupting */
    global_video_state = NULL;

    /* close each stream */
    if (is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);
    if (is->video_stream >= 0)
        stream_component_close(is, is->video_stream);
    if (is->subtitle_stream >= 0)
        stream_component_close(is, is->subtitle_stream);
    if (is->ic) {
        av_close_input_file(is->ic);
        is->ic = NULL; /* safety */
    }
    url_set_interrupt_cb(NULL);

    if (ret != 0) {
        SDL_Event ev;

        ev.type = FF_QUIT_EVENT;
        ev.user.data1 = is;
        SDL_PushEvent(&ev);
    }
    return 0;
}

static VideoState *stream_open(const char *filename, AVInputFormat *iformat)
{
    VideoState *is;

    is = av_mallocz(sizeof(VideoState));
    if (!is)
        return NULL;
    av_strlcpy(is->filename, filename, sizeof(is->filename));
    is->iformat = iformat;
    is->ytop = 0;
    is->xleft = 0;

    /* start video display */
	sys_cs_create(&cs_pictureq);
    is->pictq_cond = SDL_CreateCond();

	sys_cs_create(&cs_subpictureq);
    is->subpq_cond = SDL_CreateCond();

    /* add the refresh timer to draw the picture */
    schedule_refresh(is, 40);

    is->av_sync_type = av_sync_type;
    is->parse_tid = SDL_CreateThread(decode_thread, is);
    if (!is->parse_tid) {
        av_free(is);
        return NULL;
    }
    return is;
}

static void stream_close(VideoState *is)
{
    VideoPicture *vp;
    int i;
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    is->abort_request = 1;
    SDL_WaitThread(is->parse_tid, NULL);

    /* free all pictures */
    for(i=0;i<VIDEO_PICTURE_QUEUE_SIZE; i++) {
        vp = &is->pictq[i];
        if (vp->bmp) {
			free(vp->rgb);
			DeleteCriticalSection(&vp->cs);
            //SDL_FreeYUVOverlay(vp->bmp);
            vp->bmp = NULL;
        }
    }
	sys_cs_destroy(&cs_pictureq);
    SDL_DestroyCond(is->pictq_cond);
	sys_cs_destroy(&cs_subpictureq);
    SDL_DestroyCond(is->subpq_cond);
    if (is->img_convert_ctx)
        sws_freeContext(is->img_convert_ctx);
    av_free(is);
}

static void stream_cycle_channel(VideoState *is, int codec_type)
{
    AVFormatContext *ic = is->ic;
    int start_index, stream_index;
    AVStream *st;

    if (codec_type == CODEC_TYPE_VIDEO)
        start_index = is->video_stream;
    else if (codec_type == CODEC_TYPE_AUDIO)
        start_index = is->audio_stream;
    else
        start_index = is->subtitle_stream;
    if (start_index < (codec_type == CODEC_TYPE_SUBTITLE ? -1 : 0))
        return;
    stream_index = start_index;
    for(;;) {
        if (++stream_index >= is->ic->nb_streams)
        {
            if (codec_type == CODEC_TYPE_SUBTITLE)
            {
                stream_index = -1;
                goto the_end;
            } else
                stream_index = 0;
        }
        if (stream_index == start_index)
            return;
        st = ic->streams[stream_index];
        if (st->codec->codec_type == codec_type) {
            /* check that parameters are OK */
            switch(codec_type) {
            case CODEC_TYPE_AUDIO:
                if (st->codec->sample_rate != 0 &&
                    st->codec->channels != 0)
                    goto the_end;
                break;
            case CODEC_TYPE_VIDEO:
            case CODEC_TYPE_SUBTITLE:
                goto the_end;
            default:
                break;
            }
        }
    }
 the_end:
    stream_component_close(is, start_index);
    stream_component_open(is, stream_index);
}

static void toggle_pause(void)
{
    if(cur_stream)
        stream_pause(cur_stream);
    step = 0;
}

static void step_to_next_frame(void)
{
    if(cur_stream)
	{
        if (cur_stream->paused)
            stream_pause(cur_stream);
    }
    step = 1;
}

static void do_exit(void)
{
    int i;
	process_quiting = 1;

	EnterCriticalSection(&cs_video);
	cur_stream->videoq.abort_request = 1;
	LeaveCriticalSection(&cs_video);

	EnterCriticalSection(&cs_decode);
	cur_stream->abort_request = 1;
	LeaveCriticalSection(&cs_decode);

	EnterCriticalSection(&cs_events);
	quit_thread_events = 1;
	LeaveCriticalSection(&cs_events); 

	while(quit_thread_events)Sleep(0);

    if(cur_stream)
	{
        stream_close(cur_stream);
        cur_stream = NULL;
    }

    for (i = 0; i < CODEC_TYPE_NB; i++)
        av_free(avcodec_opts[i]);

    av_free(avformat_opts);
    av_free(sws_opts);

	DeleteCriticalSection(&cs_decode);
	DeleteCriticalSection(&cs_video);
	DeleteCriticalSection(&cs_events);
}

static void event_loop(void)
{
    SDL_Event ev;
    double incr, pos, frac;

    for(;;)	
	{
		Sleep(12);
		EnterCriticalSection(&cs_events);
		if(quit_thread_events)
		{
			quit_thread_events = 0;
			LeaveCriticalSection(&cs_events);
			return 0;
		}

        SDL_WaitEvent(&ev);
        switch(ev.type) 
		{
        case SDL_KEYDOWN:
            switch(ev.key.keysym.sym) {
            case SDLK_SPACE:
                toggle_pause();
                break;
            case SDLK_s: //S: Step to next frame
                step_to_next_frame();
                break;
            case SDLK_a:
                if (cur_stream)
                    stream_cycle_channel(cur_stream, CODEC_TYPE_AUDIO);
                break;
            case SDLK_v:
                if (cur_stream)
                    stream_cycle_channel(cur_stream, CODEC_TYPE_VIDEO);
                break;
            case SDLK_t:
                if (cur_stream)
                    stream_cycle_channel(cur_stream, CODEC_TYPE_SUBTITLE);
                break;
            default:
                break;
            }
            break;

        case SDL_QUIT:
        case FF_QUIT_EVENT:
            do_exit();
            break;
        case FF_ALLOC_EVENT:
            video_open(ev.user.data1);
            alloc_picture(ev.user.data1);
            break;
        case FF_REFRESH_EVENT:
            video_refresh_timer(ev.user.data1);
            break;
        default:
            break;
        }

		LeaveCriticalSection(&cs_events);
		Sleep(10);
    }
}


unsigned long ccthread(void *a)
{
	event_loop();
}

/* Called from the main */
int playfile(char *infile)
{
    int flags, i;

	quit_thread_events = 0;
	wanted_audio_stream = 0;
	wanted_video_stream = 0;
	wanted_subtitle_stream = -1;
    screen = 0;
	video_frame = 0;
	process_quiting = 0;

	input_filename = infile;

    /* register all codecs, demux and protocols */
    avcodec_register_all();
    avdevice_register_all();
    av_register_all();

    for(i=0; i<CODEC_TYPE_NB; i++){
        avcodec_opts[i]= avcodec_alloc_context2(i);
    }

    avformat_opts = avformat_alloc_context();
    sws_opts = sws_getContext(16,16,0, 16,16,0, sws_flags, NULL,NULL,NULL);
 
    flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

    if (SDL_Init (flags)) {
        MessageBoxA(0, SDL_GetError(), "ss", 32);
		return -1;
    }

    av_init_packet(&flush_pkt);
    flush_pkt.data= "FLUSH";

    cur_stream = stream_open(input_filename, file_iformat);

    CreateThread(0, 0, (LPTHREAD_START_ROUTINE) ccthread, 0, 0, 0);
	
    return 0;
}

/*
 * load a stream.
 */
int decoder_load(unsigned long id, const string sname)
{
	char          afname[v_sys_maxpath];
	BOOL          useddef = 1;

	audio_first_time = 1;
	current_eof = 0;

	WideCharToMultiByte(CP_ACP, 0, sname, -1, afname, sizeof(afname), "?", &useddef);
	
	InitializeCriticalSection(&cs_video);
	InitializeCriticalSection(&cs_decode);
	InitializeCriticalSection(&cs_events);

	audio_format_ready = 0;
	cstream = &pstreams[id];

	playfile(afname);

	while(!audio_format_ready) sys_sleep(0);

	detect_subtitle_files(sname);
	set_primary_subtitle_file(-1);

	pstreams[id].initialized = 1;
	pstreams[id].loaded = 1;
	return 0;
}


/*
 * close a loaded stream.
 */
int decoder_close(unsigned long id)
{
	
	if(!pstreams[id].initialized)return 0;

	free_subtitle_files();
	video_frame = 0;
	pstreams[id].initialized = 0;
	do_exit();
	return 0;
}


/*
 * set position
 */
int decoder_seek(unsigned long id, double *posr)
{
	if(cur_stream)
	{
		double frac = *posr;
        int64_t ts;

		if(last_seek)
		{
			unsigned long newstime = GetTickCount();

			if(last_seek_time < newstime)
			{
				if(last_seek_time + 200 > newstime)
				{
					return 0;
				}
			}
		}else{
			last_seek = 1;
			last_seek_time = GetTickCount();
		}

        ts = frac*cur_stream->ic->duration;
        if (cur_stream->ic->start_time != AV_NOPTS_VALUE)
            ts += cur_stream->ic->start_time;
        stream_seek(cur_stream, ts, 0);
    }
	return 1;
}


/*
 * read audio samples
 */
unsigned long audio_decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	int len;


	if(audio_first_time)
	{
		audio_first_time = 0;
		return dsize;
	}

	if(current_eof == 2) return 0;
	
	if(audio_format_ready)
	{
		aud_run = 1;
		
		len = sdl_audio_callback(cur_stream, adata, dsize);
		pstreams[id].audio_pos = cur_stream->audio_clock;
		return len;
	}
	return 0;//dsize;
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
		*width = video_width;
		*height = video_height;

	}else{
		/* aspect ratio */
		float aspect_ratio = 1.0f;

		if (cur_stream->video_st->sample_aspect_ratio.num)
            aspect_ratio = av_q2d(cur_stream->video_st->sample_aspect_ratio);
        else if (cur_stream->video_st->codec->sample_aspect_ratio.num)
            aspect_ratio = av_q2d(cur_stream->video_st->codec->sample_aspect_ratio);
        else
            aspect_ratio = 0;
        if (aspect_ratio <= 0.0)
            aspect_ratio = 1.0;
       // aspect_ratio *= (float)cur_stream->video_st->codec->width / cur_stream->video_st->codec->height;
       
		*((double*)width) = aspect_ratio;
	}
	return 1;
}


/*
 * get video data.
 */
int callc video_decoder_trans_info(int id, long dt_l, double dt_d, void *data)
{
	switch(id)
	{
	case video_set_buffersize:
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
		break;

	case video_set_audio_stream:
		break;

	case video_get_audio_stream_current:
		break;

	case video_get_audio_stream_count:
		break;

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


/*
 * get video frame.
 */
int callc video_decoder_getframe_sync(unsigned long id, void **buff, double audiosec)
{
	*buff = video_frame;
	return 10;
}


/*
 * get video frame.
 */
int callc video_decoder_getframe(unsigned long id, void **buff)
{
	return video_decoder_getframe_sync(id, buff, 0.0);
}

/*
 * will be the same as "general seek", but with more focusing on video
 * frames. stepping etc.
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


/*
 * get subtitle line.
 */
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
	return uni("");
}


/*
 * score streams.
 */
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
 
-----------------------------------------------------------------------------*/
