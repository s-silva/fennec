/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (MPEG).
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

#include "main.h"

/* decoder ------------------------------------------------------------------*/

unsigned long __inline s_uitoi(const unsigned int v, const unsigned char *oint)
{
	register unsigned long k = 0;

	if(v >= 4)
	{
		k  = ((unsigned long)oint[3]);
		k |= ((unsigned long)oint[2]) << 7;
		k |= ((unsigned long)oint[1]) << 14;
		k |= ((unsigned long)oint[0]) << 21;

	}else{

		k  = ((unsigned long)oint[3]);
		k |= ((unsigned long)oint[2]) << 8;
		k |= ((unsigned long)oint[1]) << 16;
		k |= ((unsigned long)oint[0]) << 24;
	}

	return k;
}

unsigned int tag_sizefromfile(t_sys_file_handle ofile)
{
	char  thead[10];     /* tag header */

	sys_file_read(ofile, thead, 10);

	if( (thead[0] != 'I' && thead[0] != 'i') ||
		(thead[1] != 'D' && thead[1] != 'd') ||
		(thead[2] != '3'))
	{
		return 0;
	}

	return s_uitoi(4, thead + 6) + 10;
}

void decoder_initialize(unsigned long id)
{
	pstreams[id].decdata.buffer      = sys_mem_alloc(40000);
    pstreams[id].decdata.size        = 0;
    pstreams[id].decdata.xing.flags  = 0;
    pstreams[id].decdata.samplecount = 0;
    pstreams[id].decdata.timer       = mad_timer_zero;
    pstreams[id].decdata.length      = mad_timer_zero;
    pstreams[id].decdata.rate        = 0;
    pstreams[id].decdata.frames      = 0;
    pstreams[id].decdata.lengthsec   = 0;
    pstreams[id].decdata.bitrate     = 0;
    pstreams[id].decdata.frequency   = 0;
    pstreams[id].decdata.channels    = 2;
    pstreams[id].decdata.bits        = SAMPLE_DEPTH;
    pstreams[id].decdata.buflen      = 0;
}

void decoder_uninitialize(unsigned long id)
{
	if(pstreams[id].initialized)
	{
		finish(id);
		sys_mem_free(pstreams[id].decdata.buffer);
		sys_file_close(pstreams[id].hstream);
	}
}

int decoder_openfile(unsigned long id, const string fname)
{
	unsigned int tagsize;

	pstreams[id].hstream = sys_file_openbuffering(fname, v_sys_file_forread);
    
	
	tagsize = tag_sizefromfile(pstreams[id].hstream);
	
	sys_file_seek(pstreams[id].hstream, 0);

	sys_file_seek(pstreams[id].hstream, tagsize);
	
    mad_stream_init(&pstreams[id].decdata.stream);
    mad_frame_init (&pstreams[id].decdata.frame);
    mad_synth_init (&pstreams[id].decdata.synth);

    if(scan_header(id, &pstreams[id].decdata.frame.header, &pstreams[id].decdata.xing) == -1)
	{
		decoder_uninitialize(id);
        return 0;
    }

	
	sys_file_seek(pstreams[id].hstream, tagsize);


    pstreams[id].decdata.size             = sys_file_getsize(pstreams[id].hstream);
    pstreams[id].decdata.synth.pcm.length = 0;
    pstreams[id].decdata.samplecount      = 0;
    pstreams[id].decdata.timer            = mad_timer_zero;

    if(pstreams[id].decdata.xing.flags & XING_FRAMES)
	{
        pstreams[id].decdata.length = pstreams[id].decdata.frame.header.duration;
        mad_timer_multiply(&pstreams[id].decdata.length, pstreams[id].decdata.xing.frames);
    }else {
        mad_timer_set(&pstreams[id].decdata.length, 0,1, pstreams[id].decdata.frame.header.bitrate / 8);
        mad_timer_multiply(&pstreams[id].decdata.length, pstreams[id].decdata.size);
    }

    pstreams[id].decdata.rate      = 0;
    pstreams[id].decdata.frames    = 0;
    pstreams[id].decdata.lengthsec = mad_timer_count(pstreams[id].decdata.length, MAD_UNITS_SECONDS);
	pstreams[id].decdata.bitrate   = pstreams[id].decdata.frame.header.bitrate / 1000;
    pstreams[id].decdata.frequency = pstreams[id].decdata.frame.header.samplerate;
    pstreams[id].decdata.channels  = pstreams[id].decdata.frame.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;
    pstreams[id].decdata.bits      = SAMPLE_DEPTH;
    pstreams[id].decdata.buflen    = 0;
    return 1;
}

int decoder_readdata(unsigned long id, void *block, unsigned long *size)
{
    unsigned char *samples = block;
    unsigned int  nsamples;

	if(!pstreams[id].initialized)return 0;

    nsamples = (*size / (SAMPLE_DEPTH / 8)) >> (pstreams[id].decdata.channels == 1 ? 0 : 1);
    *size = 0;

    while(nsamples)
	{
        unsigned int count, bitrate;

        count = pstreams[id].decdata.synth.pcm.length - pstreams[id].decdata.samplecount;
        
		if (count > nsamples)
		{
            count = nsamples;
		}

        if(count)
		{
            mad_fixed_t const *ch1, *ch2;

            ch1 = pstreams[id].decdata.synth.pcm.samples[0] + pstreams[id].decdata.samplecount;
            ch2 = pstreams[id].decdata.synth.pcm.samples[1] + pstreams[id].decdata.samplecount;

            if (pstreams[id].decdata.channels == 1) /* mono */
			{
				ch2 = 0;
			}else{ 
				if(pstreams[id].decdata.synth.pcm.channels == 1)
				{
					ch2 = ch1;
				}
			}
            pack_pcm(&samples, count, ch1, ch2);

            pstreams[id].decdata.samplecount += count;
            nsamples                         -= count;

            if (nsamples == 0)break;
        }

		
        while (mad_frame_decode(&pstreams[id].decdata.frame, &pstreams[id].decdata.stream) == -1)
		{
            DWORD bytes;

            if(MAD_RECOVERABLE(pstreams[id].decdata.stream.error))continue;

            if(pstreams[id].decdata.stream.next_frame)
			{
				pstreams[id].decdata.buflen = (unsigned int)((pstreams[id].decdata.buffer + pstreams[id].decdata.buflen) - pstreams[id].decdata.stream.next_frame);
				memmove(pstreams[id].decdata.buffer, pstreams[id].decdata.stream.next_frame, pstreams[id].decdata.buflen);
            }

            bytes = sys_file_read(pstreams[id].hstream, pstreams[id].decdata.buffer + pstreams[id].decdata.buflen, 40000 - pstreams[id].decdata.buflen);
			if(!bytes)return 0;
            
			mad_stream_buffer(&pstreams[id].decdata.stream, pstreams[id].decdata.buffer, pstreams[id].decdata.buflen += bytes);
        }

        bitrate = pstreams[id].decdata.frame.header.bitrate / 1000;

        pstreams[id].decdata.rate += bitrate;
        pstreams[id].decdata.frames++;

        pstreams[id].decdata.bitrate = bitrate;

        mad_synth_frame(&pstreams[id].decdata.synth, &pstreams[id].decdata.frame);

        pstreams[id].decdata.samplecount = 0;

        mad_timer_add(&pstreams[id].decdata.timer, pstreams[id].decdata.frame.header.duration);
    }

    *size = (unsigned long)(samples - (unsigned char *)block);
    return 1;
}

void decoder_seek(unsigned long id, double fpos)
{
    double fraction = 0;
    unsigned long position = 0;

    fraction = fpos;
	if(fraction > 1.0f)fraction = 1.0f;
	if(fraction < 0.0f)fraction = 0.0f;;

    position = (unsigned long)(mad_timer_count(pstreams[id].decdata.length, MAD_UNITS_MILLISECONDS) * fraction);

    mad_timer_set(&pstreams[id].decdata.timer, position / 1000, position % 1000, 1000);

    if (pstreams[id].decdata.xing.flags & XING_TOC)
	{
        int percent, p1, p2;

        percent = (int) (fraction * 100);
        p1 = (percent < 100) ? pstreams[id].decdata.xing.toc[percent    ] : 0x100;
        p2 = (percent <  99) ? pstreams[id].decdata.xing.toc[percent + 1] : 0x100;

        fraction = (p1 + (p2 - p1) * (fraction * 100 - percent)) / 0x100;
    }

    sys_file_seek(pstreams[id].hstream, (DWORD)(pstreams[id].decdata.size * fraction));

    pstreams[id].decdata.buflen = sys_file_read(pstreams[id].hstream, pstreams[id].decdata.buffer, 40000);

    mad_stream_buffer(&pstreams[id].decdata.stream, pstreams[id].decdata.buffer, pstreams[id].decdata.buflen);

    mad_frame_mute(&pstreams[id].decdata.frame);
    mad_synth_mute(&pstreams[id].decdata.synth);

    if (fraction != 0) {
        int skip;

        skip = 2;
        do {
            if (mad_frame_decode(&pstreams[id].decdata.frame, &pstreams[id].decdata.stream) == 0) {
                mad_timer_add(&pstreams[id].decdata.timer, pstreams[id].decdata.frame.header.duration);
                if (--skip == 0)
                    mad_synth_frame(&pstreams[id].decdata.synth, &pstreams[id].decdata.frame);
            }
            else if (!MAD_RECOVERABLE(pstreams[id].decdata.stream.error))
                break;
        }
        while (skip);
    }

    pstreams[id].decdata.synth.pcm.length = 0;
    pstreams[id].decdata.samplecount      = 0;
}

unsigned long decoder_getduration_ms(unsigned long id)
{
	if (!(pstreams[id].decdata.xing.flags & XING_FRAMES) && pstreams[id].decdata.frames)
	{
        mad_timer_set(&pstreams[id].decdata.length,0,1,(pstreams[id].decdata.rate /  pstreams[id].decdata.frames) * (1000 / 8));
		mad_timer_multiply(&pstreams[id].decdata.length, pstreams[id].decdata.size);
       
		if(mad_timer_compare(pstreams[id].decdata.timer, pstreams[id].decdata.length) > 0)
        {
            pstreams[id].decdata.length = pstreams[id].decdata.timer;
            pstreams[id].decdata.size   = sys_file_getsize(pstreams[id].hstream);
        }

		return mad_timer_count(pstreams[id].decdata.length, MAD_UNITS_MILLISECONDS);
	}else{
		return pstreams[id].decdata.lengthsec * 1000;
	}
}

int decoder_score(const string fname, int foptions)
{
	char               id3sign[3];
	t_sys_file_handle  hfile;
	int                cscore = 5; /* for mpeg only */

	hfile = sys_file_openbuffering(fname, v_sys_file_forread);

	if(hfile == v_error_sys_file_open)return cscore;

	sys_file_read(hfile, id3sign, sizeof(id3sign));

	if((id3sign[0] == 'I') && (id3sign[1] == 'D') && (id3sign[2] == '3'))
		cscore++;

	sys_file_close(hfile);

	return cscore;
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
