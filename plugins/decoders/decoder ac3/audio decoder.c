/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0
 Copyright (C) 2008 Chase <c-h@users.sf.net>

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



#define static_sample_buffer_size    2048*4


/* code ---------------------------------------------------------------------*/



int decoder_load(unsigned long id, const string sname)
{
	pstreams[id].infile =  sys_file_openbuffering(sname, v_sys_file_forread);

	if(sys_file_read(pstreams[id].infile, pstreams[id].mpa_data, 5) != 5)
	{
		goto err;
	}

	AC3_GetAudioInfo(&pstreams[id].frame_size, &pstreams[id].sample_rate, pstreams[id].mpa_data);

	pstreams[id].buffer        = 0;
	pstreams[id].buffersize    = 0;
	pstreams[id].buffer_left   = 0;
	pstreams[id].bufferpt      = 0;

	pstreams[id].bitspersample = 16;
	pstreams[id].channels      = 2;
	pstreams[id].frequency     = pstreams[id].sample_rate;
	pstreams[id].duration      = 0;
	return 1;

err:
	sys_file_close(pstreams[id].infile);
	return 0;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	sys_file_close(pstreams[id].infile);
	if(pstreams[id].buffer)free(pstreams[id].buffer);

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	return 0;
}

int search_sync_word(t_sys_file_handle fp)
{
	unsigned char syncword[2];
	int i = 0;

	if(sys_file_read(fp, syncword, 2) != 2)
		return 0;
	do 
	{
		if(syncword[0] == 0x0B && syncword[1] == 0x77)
		{
			return 1;
		}

		i++;

		syncword[0] = syncword[1];
	} 
	while(sys_file_read(fp, syncword + 1, 1));
	return 1;
}

int ac3_dec_get_frame(unsigned long id, void* data)
{
	UINT32  datalen, dret, flen;
	
	dret = sys_file_read(pstreams[id].infile, pstreams[id].mpa_data + 5, pstreams[id].frame_size - 5);
	
	if(dret != pstreams[id].frame_size - 5) return 0;
	
	flen = AC3_SampleConvert(data, &datalen, pstreams[id].mpa_data, pstreams[id].frame_size);
	
	if(search_sync_word(pstreams[id].infile) == 0)
		return 0;
	
	memset(pstreams[id].mpa_data, 0, 1792);
	pstreams[id].mpa_data[0] = 0x0B;
	pstreams[id].mpa_data[1] = 0x77;

	if(sys_file_read(pstreams[id].infile, pstreams[id].mpa_data + 2, 3) != 3) return 0;
		
	if(AC3_GetAudioInfo(&pstreams[id].frame_size, &pstreams[id].sample_rate, pstreams[id].mpa_data) == 0) return 0;
	return datalen;
}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	unsigned long         i = 0;
	unsigned long         lenret = 0;
	static unsigned char  ptbuff[static_sample_buffer_size];

	if(pstreams[id].buffersize && (pstreams[id].bufferpt < pstreams[id].buffersize))
	{
		memmove((char*)adata, pstreams[id].buffer + pstreams[id].bufferpt, min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt));
		i += min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt);
		pstreams[id].bufferpt += min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt);

		if(i >= dsize)return dsize;
	}

	while(1)
	{
		lenret = ac3_dec_get_frame(id, ptbuff);

		if(!lenret)break;

		memmove((char*)adata + i, ptbuff, min(dsize - i, lenret));
		i += lenret;

		if(i > dsize)
		{
			if(!pstreams[id].buffer)
			{
				pstreams[id].buffer = (char*)malloc(lenret);
				pstreams[id].bufferallocsize = lenret;
				pstreams[id].buffersize = lenret;
			}else{
				if(lenret > pstreams[id].bufferallocsize)
				{
					pstreams[id].buffer = (char*) realloc(pstreams[id].buffer, lenret);
					pstreams[id].bufferallocsize = lenret;
				}
				pstreams[id].buffersize = lenret;
			}
			memcpy(pstreams[id].buffer, ptbuff, lenret);

			pstreams[id].bufferpt = lenret - (i - dsize);
		}

		if(i >= dsize)return dsize;
	}
	return i;
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


/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
