/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (AAC).
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

/* data */



int fill_buffer(aac_buffer *b)
{
    int bread;

    if (b->bytes_consumed > 0)
    {
        if (b->bytes_into_buffer)
        {
            memmove((void*)b->buffer, (void*)(b->buffer + b->bytes_consumed),
                b->bytes_into_buffer*sizeof(unsigned char));
        }

        if (!b->at_eof)
        {
            bread = sys_file_read(b->infile, (void*)(b->buffer + b->bytes_into_buffer), b->bytes_consumed);

            if (bread != b->bytes_consumed)
                b->at_eof = 1;

            b->bytes_into_buffer += bread;
        }

        b->bytes_consumed = 0;

        if (b->bytes_into_buffer > 3)
        {
            if (memcmp(b->buffer, "TAG", 3) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 11)
        {
            if (memcmp(b->buffer, "LYRICSBEGIN", 11) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 8)
        {
            if (memcmp(b->buffer, "APETAGEX", 8) == 0)
                b->bytes_into_buffer = 0;
        }
    }

    return 1;
}

void advance_buffer(aac_buffer *b, int bytes)
{
    b->file_offset += bytes;
    b->bytes_consumed = bytes;
    b->bytes_into_buffer -= bytes;
}

static int adts_sample_rates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350,0,0,0};

int adts_parse(aac_buffer *b, int *bitrate, float *length)
{
    int frames, frame_length;
    int t_framelength = 0;
    int samplerate;
    float frames_per_sec, bytes_per_frame;

    /* Read all frames to ensure correct time and bitrate */
    for (frames = 0; /* */; frames++)
    {
        fill_buffer(b);

        if (b->bytes_into_buffer > 7)
        {
            /* check syncword */
            if (!((b->buffer[0] == 0xFF)&&((b->buffer[1] & 0xF6) == 0xF0)))
                break;

            if (frames == 0)
                samplerate = adts_sample_rates[(b->buffer[2]&0x3c)>>2];

            frame_length = ((((unsigned int)b->buffer[3] & 0x3)) << 11)
                | (((unsigned int)b->buffer[4]) << 3) | (b->buffer[5] >> 5);

            t_framelength += frame_length;

            if (frame_length > b->bytes_into_buffer)
                break;

            advance_buffer(b, frame_length);
        } else {
            break;
        }
    }

    frames_per_sec = (float)samplerate/1024.0f;
    if (frames != 0)
        bytes_per_frame = (float)t_framelength/(float)(frames*1000);
    else
        bytes_per_frame = 0;
    *bitrate = (int)(8. * bytes_per_frame * frames_per_sec + 0.5);
    if (frames_per_sec != 0)
        *length = (float)frames/frames_per_sec;
    else
        *length = 1;

    return 1;
}

unsigned long srates[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000
};

/* functions */

int get_aac_track(mp4ff_t *infile)
{
    /* find AAC track */
    int i, rc;
    int numTracks = mp4ff_total_tracks(infile);

    for (i = 0; i < numTracks; i++)
    {
        unsigned char *buff = NULL;
        int buff_size = 0;
        mp4AudioSpecificConfig mp4ASC;

        mp4ff_get_decoder_config(infile, i, &buff, &buff_size);

        if (buff)
        {
            rc = NeAACDecAudioSpecificConfig(buff, buff_size, &mp4ASC);
            free(buff);

            if (rc < 0)
                continue;
            return i;
        }
    }

    /* can't decode this */
    return -1;
}

uint32_t write_callback(void *user_data, void *buffer, uint32_t length)
{
    return (uint32_t)sys_file_write((t_sys_file_handle)user_data, buffer, length);
}

uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
    return (uint32_t)sys_file_read((t_sys_file_handle)user_data, buffer, length);
}

uint32_t truncate_callback(void *user_data)
{
	return (uint32_t)sys_file_seteof((t_sys_file_handle)user_data);
}

uint32_t seek_callback(void *user_data, uint64_t position)
{
    return (uint32_t)sys_file_seek((t_sys_file_handle)user_data, (long)position);
}

int decoder_load_mp4(unsigned long id)
{
	pstreams[id].mp4cb = malloc(sizeof(mp4ff_callback_t));

	pstreams[id].mp4cb->write     = write_callback;
    pstreams[id].mp4cb->read      = read_callback;
    pstreams[id].mp4cb->seek      = seek_callback;
	pstreams[id].mp4cb->truncate  = truncate_callback;
    pstreams[id].mp4cb->user_data = pstreams[id].mp4file;

	pstreams[id].hdecoder = NeAACDecOpen();

	pstreams[id].config = NeAACDecGetCurrentConfiguration(pstreams[id].hdecoder);

    pstreams[id].config->outputFormat = FAAD_FMT_DOUBLE;
	pstreams[id].config->downMatrix   = 0;

    NeAACDecSetConfiguration(pstreams[id].hdecoder, pstreams[id].config);

	pstreams[id].infile = mp4ff_open_read(pstreams[id].mp4cb);

	if(!pstreams[id].infile)return 0;

	pstreams[id].track = get_aac_track(pstreams[id].infile);

	if(pstreams[id].track < 0)
	{
		return 0;
	}

	pstreams[id].buffer = 0;
	pstreams[id].buffer_size = 0;

	mp4ff_get_decoder_config(pstreams[id].infile, pstreams[id].track, &pstreams[id].buffer, &pstreams[id].buffer_size);

	if(NeAACDecInit2(pstreams[id].hdecoder, pstreams[id].buffer, pstreams[id].buffer_size, &pstreams[id].frequency, &pstreams[id].channels) < 0)
    {
        return 0;
    }

	pstreams[id].timescale      = mp4ff_time_scale(pstreams[id].infile, pstreams[id].track);
    pstreams[id].framesize      = 1024;
    pstreams[id].use_aac_length = 0;

	if(pstreams[id].buffer)
    {
        if(NeAACDecAudioSpecificConfig(pstreams[id].buffer, pstreams[id].buffer_size, &pstreams[id].mp4asc) >= 0)
        {
            if(pstreams[id].mp4asc.frameLengthFlag  == 1) pstreams[id].framesize = 960;
            if(pstreams[id].mp4asc.sbr_present_flag == 1) pstreams[id].framesize *= 2;
        }

        free(pstreams[id].buffer);
    }

	pstreams[id].numsamples = mp4ff_num_samples(pstreams[id].infile, pstreams[id].track);

	pstreams[id].duration = (unsigned long)(mp4ff_get_track_duration(pstreams[id].infile, pstreams[id].track) / pstreams[id].frequency) * 1000;
	
	pstreams[id].ok = 1;
	return 1;
}

int decoder_load_aac(unsigned long id)
{
	int     tagsize = 0;
    int     bread, fileread = 0;
    int     header_type = 0;
    int     bitrate = 0;
    float   length = 0;

#define MAX_CHANNELS 6


	sys_file_seek(pstreams[id].mp4file, 0);

	pstreams[id].b.infile = pstreams[id].mp4file;

	pstreams[id].b.buffer = (unsigned char*)malloc(FAAD_MIN_STREAMSIZE * MAX_CHANNELS);
    memset(pstreams[id].b.buffer, 0, FAAD_MIN_STREAMSIZE * MAX_CHANNELS);

	bread = sys_file_read(pstreams[id].b.infile, pstreams[id].b.buffer, FAAD_MIN_STREAMSIZE * MAX_CHANNELS);
    pstreams[id].b.bytes_into_buffer = bread;
    pstreams[id].b.bytes_consumed    = 0;
    pstreams[id].b.file_offset       = 0;

	pstreams[id].fsize = sys_file_getsize(pstreams[id].mp4file);

    if (!memcmp(pstreams[id].b.buffer, "ID3", 3))
    {
        /* high bit is not used */
        tagsize = (pstreams[id].b.buffer[6] << 21) | (pstreams[id].b.buffer[7] << 14) |
            (pstreams[id].b.buffer[8] <<  7) | (pstreams[id].b.buffer[9] <<  0);

        tagsize += 10;
        advance_buffer(&pstreams[id].b, tagsize);
        fill_buffer(&pstreams[id].b);
    }

	pstreams[id].hdecoder = NeAACDecOpen();

	pstreams[id].config = NeAACDecGetCurrentConfiguration(pstreams[id].hdecoder);

    pstreams[id].config->outputFormat = FAAD_FMT_DOUBLE;
	pstreams[id].config->downMatrix   = 0;

    NeAACDecSetConfiguration(pstreams[id].hdecoder, pstreams[id].config);


	header_type = 0;
    if ((pstreams[id].b.buffer[0] == 0xFF) && ((pstreams[id].b.buffer[1] & 0xF6) == 0xF0))
    {
        adts_parse(&pstreams[id].b, &bitrate, &length);
        sys_file_seek(pstreams[id].b.infile, tagsize);

        bread = sys_file_read(pstreams[id].b.infile, pstreams[id].b.buffer, FAAD_MIN_STREAMSIZE*MAX_CHANNELS);
        if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
            pstreams[id].b.at_eof = 1;
        else
            pstreams[id].b.at_eof = 0;
        pstreams[id].b.bytes_into_buffer = bread;
        pstreams[id].b.bytes_consumed = 0;
        pstreams[id].b.file_offset = tagsize;

        header_type = 1;
    } else if (memcmp(pstreams[id].b.buffer, "ADIF", 4) == 0) {
        int skip_size = (pstreams[id].b.buffer[4] & 0x80) ? 9 : 0;
        bitrate = ((unsigned int)(pstreams[id].b.buffer[4 + skip_size] & 0x0F)<<19) |
            ((unsigned int)pstreams[id].b.buffer[5 + skip_size]<<11) |
            ((unsigned int)pstreams[id].b.buffer[6 + skip_size]<<3) |
            ((unsigned int)pstreams[id].b.buffer[7 + skip_size] & 0xE0);

        length = (float)fileread;
        if (length != 0)
        {
            length = ((float)length*8.f)/((float)bitrate) + 0.5f;
        }

        bitrate = (int)((float)bitrate/1000.0f + 0.5f);

        header_type = 2;
    }

	pstreams[id].duration = (unsigned long)(length * 1000.0f);

	fill_buffer(&pstreams[id].b);

	bread = NeAACDecInit(pstreams[id].hdecoder, pstreams[id].b.buffer, pstreams[id].b.bytes_into_buffer, &pstreams[id].frequency, &pstreams[id].channels);

	pstreams[id].bitspersample = 64;

	advance_buffer(&pstreams[id].b, bread);
    fill_buffer(&pstreams[id].b);

	pstreams[id].ok = 1;
	return 1;
}

int decoder_load(unsigned long id, const string sname, int fw)
{
	unsigned char header[8];

	pstreams[id].ok = 0;

	if(fw) pstreams[id].mp4file = CreateFile(sname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
	else   pstreams[id].mp4file = sys_file_openbuffering(sname, v_sys_file_forread);

	if(pstreams[id].mp4file == v_error_sys_file_open)return 0;

	sys_file_read(pstreams[id].mp4file, header, 8);

	pstreams[id].ismp4 = 0;

	if(header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
        pstreams[id].ismp4 = 1;

	sys_file_seek(pstreams[id].mp4file, 0);

	pstreams[id].sampleid      = 0;
	pstreams[id].initial       = 1;
	pstreams[id].bitspersample = 64;

	pstreams[id].tmpbufferlen  = 0;
	pstreams[id].tmpbuffersize = 0;
	pstreams[id].tmpbuffer     = 0;

	if(pstreams[id].ismp4)
		return decoder_load_mp4(id);
	else
		return decoder_load_aac(id);
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].ok)return 0;

	NeAACDecClose(pstreams[id].hdecoder);
	if(pstreams[id].ismp4)mp4ff_close(pstreams[id].infile);
	if(pstreams[id].ismp4)free(pstreams[id].mp4cb);
	sys_file_close(pstreams[id].mp4file);

	if(pstreams[id].tmpbuffer)
		sys_mem_free(pstreams[id].tmpbuffer);

	pstreams[id].ok = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	if(!pstreams[id].ok)return 0;

	pstreams[id].tmpbufferlen = 0;
	
	if(pstreams[id].ismp4)
		pstreams[id].sampleid = (long)((double)pstreams[id].numsamples * pos);

	return 1;
}

unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize)
{
	int          rc;
    long         dur;
	unsigned int i = min(dsize, pstreams[id].tmpbufferlen);
	unsigned int sample_count = 0;
    unsigned int delay = 0;

	if(!pstreams[id].ok)return 0;

	if(pstreams[id].tmpbufferlen)
	{
		memcpy(adata, pstreams[id].tmpbuffer, min(dsize, pstreams[id].tmpbufferlen));
		if(dsize < pstreams[id].tmpbufferlen)
			memmove(pstreams[id].tmpbuffer, pstreams[id].tmpbuffer + dsize, pstreams[id].tmpbufferlen - dsize);
		pstreams[id].tmpbufferlen -= min(dsize, pstreams[id].tmpbufferlen);
		
	}
			
	if(pstreams[id].sampleid >= pstreams[id].numsamples && pstreams[id].ismp4)
		return i;

	if(dsize > pstreams[id].tmpbuffersize)
	{
		pstreams[id].tmpbuffersize = dsize;
		pstreams[id].tmpbuffer = (char*) sys_mem_realloc(pstreams[id].tmpbuffer, pstreams[id].tmpbuffersize + 4096);
	}

	while(i < dsize)
	{

		if(pstreams[id].ismp4)
		{

			pstreams[id].buffer      = 0;
			pstreams[id].buffer_size = 0;

			dur = mp4ff_get_sample_duration(pstreams[id].infile, pstreams[id].track, pstreams[id].sampleid);
			rc  = mp4ff_read_sample(pstreams[id].infile, pstreams[id].track, pstreams[id].sampleid, &pstreams[id].buffer,  &pstreams[id].buffer_size);

			if(rc == 0)
			{
				return 1;
			}
			
			pstreams[id].sample_buffer = NeAACDecDecode(pstreams[id].hdecoder, &pstreams[id].frameinfo, pstreams[id].buffer, pstreams[id].buffer_size);
			
		
			if(pstreams[id].buffer) free(pstreams[id].buffer);

			if(pstreams[id].sampleid == 0)dur = 0;

			if (pstreams[id].use_aac_length || (pstreams[id].timescale != pstreams[id].frequency))
			{
				sample_count = pstreams[id].frameinfo.samples;
			} else {
				sample_count = (unsigned int)(dur * pstreams[id].frameinfo.channels);

				if(!pstreams[id].use_aac_length && !pstreams[id].initial && (pstreams[id].sampleid < pstreams[id].numsamples / 2) && (sample_count != pstreams[id].frameinfo.samples))
				{
					pstreams[id].use_aac_length = 1;
					sample_count = pstreams[id].frameinfo.samples;
				}
			}

			if(sample_count)
			{
				if((i + (sample_count * (pstreams[id].bitspersample / 8))) <= dsize)
				{
				
					memcpy((char*)adata + i, pstreams[id].sample_buffer, sample_count * (pstreams[id].bitspersample / 8));
					
					i += sample_count * (pstreams[id].bitspersample / 8);

				}else{

					memcpy((char*)adata + i, pstreams[id].sample_buffer, dsize - i);
					
					memcpy(pstreams[id].tmpbuffer, (char*)pstreams[id].sample_buffer + (dsize - i), (sample_count * (pstreams[id].bitspersample / 8)) - (dsize - i));
					pstreams[id].tmpbufferlen = (sample_count * (pstreams[id].bitspersample / 8)) - (dsize - i);

					if(pstreams[id].tmpbufferlen > pstreams[id].tmpbuffersize)
					{
						pstreams[id].tmpbuffersize = pstreams[id].tmpbufferlen;
						pstreams[id].tmpbuffer = (char*) sys_mem_realloc(pstreams[id].tmpbuffer, pstreams[id].tmpbuffersize + 4096);
					}

					i = dsize;
				}
			}

			pstreams[id].sampleid++;

			if(pstreams[id].sampleid >= pstreams[id].numsamples)
				return i;

			if(pstreams[id].initial)pstreams[id].initial = 0;
		}else{

			pstreams[id].sample_buffer = NeAACDecDecode(pstreams[id].hdecoder, &pstreams[id].frameinfo, pstreams[id].b.buffer, pstreams[id].b.bytes_into_buffer);
			
			advance_buffer(&pstreams[id].b, pstreams[id].frameinfo.bytesconsumed);
////////////////////////////////////
			if(pstreams[id].buffer) free(pstreams[id].buffer);

			if(pstreams[id].sampleid == 0)dur = 0;

			if (pstreams[id].use_aac_length || (pstreams[id].timescale != pstreams[id].frequency))
			{
				sample_count = pstreams[id].frameinfo.samples;
			} else {
				sample_count = (unsigned int)(dur * pstreams[id].frameinfo.channels);

				if(!pstreams[id].use_aac_length && !pstreams[id].initial && (pstreams[id].sampleid < pstreams[id].numsamples / 2) && (sample_count != pstreams[id].frameinfo.samples))
				{
					pstreams[id].use_aac_length = 1;
					sample_count = pstreams[id].frameinfo.samples;
				}
			}

			if(sample_count)
			{
				if((i + (sample_count * (pstreams[id].bitspersample / 8))) <= dsize)
				{
				
					memcpy((char*)adata + i, pstreams[id].sample_buffer, sample_count * (pstreams[id].bitspersample / 8));
					
					i += sample_count * (pstreams[id].bitspersample / 8);

				}else{

					memcpy((char*)adata + i, pstreams[id].sample_buffer, dsize - i);
					
					memcpy(pstreams[id].tmpbuffer, (char*)pstreams[id].sample_buffer + (dsize - i), (sample_count * (pstreams[id].bitspersample / 8)) - (dsize - i));
					pstreams[id].tmpbufferlen = (sample_count * (pstreams[id].bitspersample / 8)) - (dsize - i);

					if(pstreams[id].tmpbufferlen > pstreams[id].tmpbuffersize)
					{
						pstreams[id].tmpbuffersize = pstreams[id].tmpbufferlen;
						pstreams[id].tmpbuffer = (char*) sys_mem_realloc(pstreams[id].tmpbuffer, pstreams[id].tmpbuffersize + 4096);
					}
					
					i = dsize;
				}
			}

			pstreams[id].sampleid++;

			if(pstreams[id].initial)pstreams[id].initial = 0;
///////////////////////////////////////
			fill_buffer(&pstreams[id].b);

			if (pstreams[id].b.bytes_into_buffer == 0)return i;

		}
	}
    return i;
}

void validate_tag(struct fennec_audiotag_item *ct)
{
	string  mem;
	int     msize;

	if(ct->tdata)
	{
		msize = (int)(strlen((char*)ct->tdata) + 1) * sizeof(letter); /* would this work for UTF-8 ? */
		mem = sys_mem_alloc(msize);
		
		MultiByteToWideChar(CP_UTF8, 0, (char*)ct->tdata, -1, mem, msize / sizeof(letter));
		
		free(ct->tdata); /* don't use 'sys_mem_free' */

		ct->tdata = mem;
		ct->tsize = (unsigned int)str_size(mem);

	}else{
		ct->tsize = 0;
	}

	ct->tmode = tag_memmode_dynamic;
}

int32_t mod_meta_find_by_name(const mp4ff_t *f, const char *item, letter **value)
{
	return mp4ff_meta_find_by_name(f, item, (char**)value);
}

int mtag_compare_sign(unsigned char *buf, char *sign)
{
	int i;
	for(i=0; i<4; i++)
		if(tolower(buf[i]) != tolower((unsigned char)sign[i]))return 0;
	return 1;
}

int mtag_finddata(unsigned char *buf, unsigned char size)
{
	int i;
	for(i=0; i<size - 4; i++)
	{	if( buf[i]   == (unsigned char)'d' &&
			buf[i+1] == (unsigned char)'a' &&
			buf[i+2] == (unsigned char)'t' &&
			buf[i+3] == (unsigned char)'a')return i;
	}
	return 0;
}

int mtag_4byte2int(unsigned char *buf)
{
	int num = (int)(unsigned char)buf[3];
	num += ((int)(unsigned char)buf[2]) << 8;
	num += ((int)(unsigned char)buf[1]) << 16;
	num += ((int)(unsigned char)buf[0]) << 24;
	return num;
}

void mtag_setitem(char *dat, int size, struct fennec_audiotag_item *ct)
{
	ct->tdata = sys_mem_alloc(size * sizeof(letter));
	
	ct->tmode = tag_memmode_dynamic;

	MultiByteToWideChar(CP_UTF8, 0, (char*)dat, -1, ct->tdata, size);
	ct->tsize = str_size(ct->tdata);
}

void* get_tagarray(const string fname, struct fennec_audiotag *rtag)
{
	unsigned char *buf;
	int            buflen = 32;
	char          *signs[] = {"moov", "udta", "meta", "ilst"};
	int            signindex = 0;
	unsigned long  nbread;
	char           atombuf[8];
	uint32_t       atomsize;
	int            bufi = 0;
	char          *dbuf;

	t_sys_file_handle f = sys_file_openbuffering(fname, v_sys_file_forread);

	buf = (unsigned char*)sys_mem_alloc(buflen);

	nbread = sys_file_read(f, buf, buflen);

	if(!mtag_compare_sign(buf + 4, "ftyp"))
	{
		sys_file_close(f);
		return 0;
	}

	while(nbread == buflen)
	{
		sys_file_seek(f, bufi);
		nbread = sys_file_read(f, buf, buflen);

		atomsize = mtag_4byte2int(buf);
		bufi += 4;
		if(atomsize == 0)continue;
		
		if(mtag_compare_sign(buf + 4, signs[signindex]))
		{
			signindex++;
			if(signindex >= 4)
			{
				buflen = atomsize;
				buf = sys_mem_realloc(buf, buflen);
				sys_file_seek(f, bufi + 4);
				nbread = sys_file_read(f, buf, buflen);
				break;
			}
			bufi += 4;
		}else{
			bufi += atomsize - 4;
		}

		//if(bufi + 8 >= buflen)
		//{
		//	nbread = sys_file_read(f, buf, buflen);
		//	bufi = 
		//	//sys_file_close(f);
		//	return 0;
		//}
	}



	bufi = 0;

	for(;;)
	{
		atomsize = mtag_4byte2int(buf + bufi);
		bufi += 4;
		if(atomsize == 0)continue;
		if(bufi + atomsize > buflen)break;
		dbuf = buf + mtag_finddata(buf + bufi, atomsize) + 12 + bufi;//buf + bufi + 4;
		

		if(mtag_compare_sign(buf + bufi, "©art"))
		{
			mtag_setitem(dbuf, atomsize - 8, &rtag->tag_artist);

		}else if(mtag_compare_sign(buf + bufi, "©alb")){
			mtag_setitem(dbuf, atomsize - 8, &rtag->tag_album);

		}else if(mtag_compare_sign(buf + bufi, "©nam")){
			mtag_setitem(dbuf, atomsize - 8, &rtag->tag_title);

		}else if(mtag_compare_sign(buf + bufi, "©day")){
			mtag_setitem(dbuf, atomsize - 8, &rtag->tag_year);

		}else if(mtag_compare_sign(buf + bufi, "©gen") || mtag_compare_sign(buf + bufi, "gnre")){
			mtag_setitem(dbuf, atomsize - 8, &rtag->tag_genre);
		}


		bufi += atomsize - 4;
		if(bufi + 8 > buflen)break;
	}

	sys_mem_free(buf);
	sys_file_close(f);
	return 0;
}



int tagread(const string fname,  struct fennec_audiotag *rtag)
{	
	struct fennec_audiotag_item *ct;

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
	  //if(rtag->tag_filepath.tsize      ) { sys_mem_free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
	  //if(rtag->tag_filename.tsize      ) { sys_mem_free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; }
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

	get_tagarray(fname, rtag);

	return 1;

	/*

	decoder_load(0, fname, 0);

	if(!pstreams[0].ok || !pstreams[0].ismp4)return 0;

	ct = &rtag->tag_title;
	mod_meta_find_by_name(pstreams[0].infile, "title", &ct->tdata);
	validate_tag(ct);

	ct = &rtag->tag_album;
	mod_meta_find_by_name(pstreams[0].infile, "album", &ct->tdata);
	validate_tag(ct);

	ct = &rtag->tag_artist;
	mod_meta_find_by_name(pstreams[0].infile, "artist", &ct->tdata);
	validate_tag(ct);

	//ct = &rtag->tag_origartist;
	//mod_meta_find_by_name(pstreams[0].infile, "original artist", &ct->tdata);
	//validate_tag(ct);

	//ct = &rtag->tag_composer;
	////mod_meta_find_by_name(pstreams[0].infile, "composer", &ct->tdata);
	validate_tag(ct);

	ct = &rtag->tag_lyricist;
	mod_meta_find_by_name(pstreams[0].infile, "writer", &ct->tdata);
	validate_tag(ct);

	//ct = &rtag->tag_band;
	//mod_meta_find_by_name(pstreams[0].infile, "band", &ct->tdata);
	//validate_tag(ct);

	//ct = &rtag->tag_copyright;
	//mod_meta_find_by_name(pstreams[0].infile, "copyright", &ct->tdata);
	//validate_tag(ct);

	//ct = &rtag->tag_publish;
	//mod_meta_find_by_name(pstreams[0].infile, "publish", &ct->tdata);
	//validate_tag(ct);

	//ct = &rtag->tag_encodedby;
	//mod_meta_find_by_name(pstreams[0].infile, "encoded by", &ct->tdata);
	//validate_tag(ct);

	ct = &rtag->tag_genre;
	mod_meta_find_by_name(pstreams[0].infile, "genre", &ct->tdata);
	validate_tag(ct);

	ct = &rtag->tag_year;
	mod_meta_find_by_name(pstreams[0].infile, "date", &ct->tdata);
	validate_tag(ct);

	//ct = &rtag->tag_url;
	//mod_meta_find_by_name(pstreams[0].infile, "url", &ct->tdata);
	//validate_tag(ct);

	//ct = &rtag->tag_offiartisturl;
	//mod_meta_find_by_name(pstreams[0].infile, "official artist url", &ct->tdata);
	//validate_tag(ct);

	ct = &rtag->tag_comments;
	mod_meta_find_by_name(pstreams[0].infile, "comment", &ct->tdata);
	validate_tag(ct);

	//ct = &rtag->tag_lyric;
	//mod_meta_find_by_name(pstreams[0].infile, "lyrics", &ct->tdata);
	//validate_tag(ct);

	ct = &rtag->tag_bpm;
	mod_meta_find_by_name(pstreams[0].infile, "tempo", &ct->tdata);
	validate_tag(ct);

	ct = &rtag->tag_tracknum;
	mod_meta_find_by_name(pstreams[0].infile, "track", &ct->tdata);
	validate_tag(ct);

	decoder_close(0);*/
	
	return 1;
}

void tag_remove(mp4ff_metadata_t *tag, const char *item)
{
	unsigned int i;

	for(i=0; i<tag->count; i++)
	{
		if(stricmp(tag->tags[i].item, item) == 0)
		{
			free(tag->tags[i].item);
			free(tag->tags[i].value);

			tag->tags[i].item  = tag->tags[tag->count - 1].item;
			tag->tags[i].value = tag->tags[tag->count - 1].value;

			tag->count--;
		}
	}
}

int32_t mod_field(mp4ff_metadata_t *tags, const char *item, const string value)
{
	int32_t  rv;
	char    *uvalue; /* UTF-8 value */
	int      ssize;

	ssize = (int)(str_len(value) + 1) * 4; /* maximum of 4 bytes per character */

	uvalue = sys_mem_alloc(ssize);

	WideCharToMultiByte(CP_UTF8, 0, value, -1, uvalue, ssize, 0, 0);

	/* the function below duplicates the value; if it doesn't we're on danger! */
	rv = mp4ff_tag_set_field(tags, item, uvalue);
	sys_mem_free(uvalue);
	return rv;
}


int tagwrite(const string fname,  struct fennec_audiotag *wtag)
{
	struct fennec_audiotag_item *ct;
	mp4ff_metadata_t             tag;
	unsigned int                 i, n;

	if(!wtag)return 0;

	decoder_load(0, fname, 1);

	if(!pstreams[0].ok || !pstreams[0].ismp4)return 0;

	n = mp4ff_meta_get_num_items(pstreams[0].infile);

	tag.tags = (mp4ff_tag_t*) malloc(sizeof(mp4ff_tag_t) * (n + 1));

	for(i=0; i<n; i++)
	{
		mp4ff_meta_get_by_index(pstreams[0].infile, i, &tag.tags[i].item, &tag.tags[i].value);
	}

	tag.count = n;

	ct = &wtag->tag_title;
	if(ct->tsize)
		mod_field(&tag, "title", ct->tdata);
	else
		tag_remove(&tag, "title");

	ct = &wtag->tag_album;
	if(ct->tsize)
		mod_field(&tag, "album", ct->tdata);
	else
		tag_remove(&tag, "album");

	ct = &wtag->tag_artist;
	if(ct->tsize)
		mod_field(&tag, "artist", ct->tdata);
	else
		tag_remove(&tag, "artist");

	ct = &wtag->tag_origartist;
	if(ct->tsize)
		mod_field(&tag, "original artist", ct->tdata);
	else
		tag_remove(&tag, "original artist");

	ct = &wtag->tag_composer;
	if(ct->tsize)
		mod_field(&tag, "composer", ct->tdata);
	else
		tag_remove(&tag, "composer");

	ct = &wtag->tag_lyricist;
	if(ct->tsize)
		mod_field(&tag, "writer", ct->tdata);
	else
		tag_remove(&tag, "writer");

	ct = &wtag->tag_band;
	if(ct->tsize)
		mod_field(&tag, "band", ct->tdata);
	else
		tag_remove(&tag, "band");

	ct = &wtag->tag_copyright;
	if(ct->tsize)
		mod_field(&tag, "copyright", ct->tdata);
	else
		tag_remove(&tag, "copyright");

	ct = &wtag->tag_publish;
	if(ct->tsize)
		mod_field(&tag, "publish", ct->tdata);
	else
		tag_remove(&tag, "publish");

	ct = &wtag->tag_encodedby;
	if(ct->tsize)
		mod_field(&tag, "encoded by", ct->tdata);
	else
		tag_remove(&tag, "encoded by");

	ct = &wtag->tag_genre;
	if(ct->tsize)
		mod_field(&tag, "genre", ct->tdata);
	else
		tag_remove(&tag, "genre");

	ct = &wtag->tag_year;
	if(ct->tsize)
		mod_field(&tag, "date", ct->tdata);
	else
		tag_remove(&tag, "date");

	ct = &wtag->tag_url;
	if(ct->tsize)
		mod_field(&tag, "url", ct->tdata);
	else
		tag_remove(&tag, "url");

	ct = &wtag->tag_offiartisturl;
	if(ct->tsize)
		mod_field(&tag, "official artist url", ct->tdata);
	else
		tag_remove(&tag, "official artist url");

	ct = &wtag->tag_comments;
	if(ct->tsize)
		mod_field(&tag, "comment", ct->tdata);
	else
		tag_remove(&tag, "comment");

	ct = &wtag->tag_lyric;
	if(ct->tsize)
		mod_field(&tag, "lyrics", ct->tdata);
	else
		tag_remove(&tag, "lyrics");

	ct = &wtag->tag_bpm;
	if(ct->tsize)
		mod_field(&tag, "tempo", ct->tdata);
	else
		tag_remove(&tag, "tempo");

	ct = &wtag->tag_tracknum;
	if(ct->tsize)
		mod_field(&tag, "track", ct->tdata);
	else
		tag_remove(&tag, "track");
		


	mp4ff_meta_update(pstreams[0].mp4cb, &tag);

	free(tag.tags);

	decoder_close(0);
	return 1;
}


int callc decoder_score(const string sfile, int res)
{
	return 2000; /* the other plug-ins will take care of this */
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
