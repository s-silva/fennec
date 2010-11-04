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




/* code ---------------------------------------------------------------------*/

/*
  The data bundle we pass around with our reader to store file
  position and size etc. 
*/


/*
  Our implementations of the mpc_reader callback functions.
*/
mpc_int32_t read_impl(void *data, void *ptr, mpc_int32_t size)
{
    reader_data *d = (reader_data *) data;
    return fread(ptr, 1, size, d->file);
}

mpc_bool_t seek_impl(void *data, mpc_int32_t offset)
{
    reader_data *d = (reader_data *) data;
    return d->seekable ? !fseek(d->file, offset, SEEK_SET) : 0;
}

mpc_int32_t tell_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return ftell(d->file);
}

mpc_int32_t get_size_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return d->size;
}

mpc_bool_t canseek_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return d->seekable;
}


int decoder_load(unsigned long id, const string sname)
{

    pstreams[id].stream.file  = _wfopen(sname, uni("rb"));

	if(!pstreams[id].stream.file) return 0;


    pstreams[id].stream.seekable = 1;
	
	fseek(pstreams[id].stream.file, 0, SEEK_END);
	pstreams[id].stream.size = ftell(pstreams[id].stream.file); 
    fseek(pstreams[id].stream.file, 0, SEEK_SET);

    /* set up an mpc_reader linked to our function implementations */
    
    pstreams[id].reader.read     = read_impl;
    pstreams[id].reader.seek     = seek_impl;
    pstreams[id].reader.tell     = tell_impl;
    pstreams[id].reader.get_size = get_size_impl;
    pstreams[id].reader.canseek  = canseek_impl;
    pstreams[id].reader.data     = &pstreams[id].stream.file;

	mpc_streaminfo_init(&pstreams[id].info);

    if(mpc_streaminfo_read(&pstreams[id].info, &pstreams[id].reader) != ERROR_CODE_OK)
	{
		fclose(pstreams[id].stream.file);
        return 0;
    }

	mpc_decoder_setup(&pstreams[id].decoder, &pstreams[id].reader);

    if (!mpc_decoder_initialize(&pstreams[id].decoder, &pstreams[id].info))
	{
        fclose(pstreams[id].stream.file);
        return 0;
    }

	pstreams[id].sample_start  = 0;

	pstreams[id].bitspersample = 32;
	pstreams[id].channels      = pstreams[id].info.channels;
	pstreams[id].frequency     = pstreams[id].info.sample_freq;
	pstreams[id].duration      = (unsigned long)(pstreams[id].info.pcm_samples / (pstreams[id].info.sample_freq / 1000));
	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	mpc_decoder_seek_sample(&pstreams[id].decoder, (mpc_uint32_t)(pos * pstreams[id].info.pcm_samples) );
	return 0;
}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	int  cur = 0;

	if(pstreams[id].sample_start)
	{
		int z = min(dsize, (pstreams[id].sample_end - pstreams[id].sample_start));

		memcpy(adata + cur, ((char*)pstreams[id].sample_buffer) + pstreams[id].sample_start, z);
		pstreams[id].sample_start += z;
		cur += z;

		if(pstreams[id].sample_start >= pstreams[id].sample_end)
		{
			pstreams[id].sample_start = 0;
		}
		
		if(cur >= dsize)
			return cur;
	}
	
	for(;;)
	{
		int z;
		int i = mpc_decoder_decode(&pstreams[id].decoder, pstreams[id].sample_buffer, 0, 0);

		i *= sizeof(MPC_SAMPLE_FORMAT);
		i *= pstreams[id].info.channels;

		if(i == 0) break;
		
		z = min(i, dsize - cur);

		memcpy(adata + cur, pstreams[id].sample_buffer, z);

		cur += z;


		if(cur >= dsize)
		{
			if(i > z)
				pstreams[id].sample_start = z;
			pstreams[id].sample_end   = i;
			return cur;
		}
		
		pstreams[id].sample_start = 0;

		
	}

    return cur;
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
