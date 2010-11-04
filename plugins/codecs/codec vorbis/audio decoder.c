/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (Vorbis).
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

size_t dec_callback_read(void *buffer, size_t size, size_t amount, void *file)
{
    return sys_file_read((t_sys_file_handle)file, buffer, (DWORD)size * amount);
}

int dec_callback_seek(void *file, ogg_int64_t newpos, int set) 
{
	if(set == SEEK_END)
	{
		sys_file_seek((t_sys_file_handle)file, sys_file_getsize((t_sys_file_handle)file));
	}else{
		sys_file_seek((t_sys_file_handle)file, (unsigned long)newpos);
	}
	return (int)newpos;
}

int dec_callback_close(void *file)
{
	sys_file_close((t_sys_file_handle)file);
	return 1;	

}

long dec_callback_tell(void *file)
{
	return sys_file_tell((t_sys_file_handle)file);
}


int decoder_load(unsigned long id, const string sname)
{
    vorbis_info* vorbisinfo = 0;
	ov_callbacks callbacks;

	memset(&pstreams[id].vorbisfile, 0, sizeof(pstreams[id].vorbisfile));
   
	callbacks.close_func = dec_callback_close;
	callbacks.read_func  = dec_callback_read;
	callbacks.seek_func  = dec_callback_seek;
	callbacks.tell_func  = dec_callback_tell;

	/* open file */

	pstreams[id].frequency     = 0;
	pstreams[id].bitspersample = 0;
	pstreams[id].channels      = 0;

	pstreams[id].hstream = sys_file_openbuffering(sname, v_sys_file_forread);

	if(pstreams[id].hstream == v_error_sys_file_open)return 0;



	if(ov_open_callbacks((void *)pstreams[id].hstream, &pstreams[id].vorbisfile, 0, 0, callbacks) < 0)
    {
		sys_file_close(pstreams[id].hstream);
        return 0;
    }

    if(ov_streams(&pstreams[id].vorbisfile) != 1)
    {
        ov_clear(&pstreams[id].vorbisfile);
        sys_file_close(pstreams[id].hstream);
        return 0;
    }

    vorbisinfo = ov_info(&pstreams[id].vorbisfile, -1);

    if(!vorbisinfo)
    {
        ov_clear(&pstreams[id].vorbisfile);
        sys_file_close(pstreams[id].hstream);
        return 0;
    }

    /*if(vorbisinfo->channels > 2)
    {
        ov_clear(&pstreams[id].vorbisfile);
        sys_file_close(pstreams[id].hstream);
        return 0;
    }*/

	pstreams[id].bitspersample = 64;
	pstreams[id].bitrate       = ov_bitrate(&pstreams[id].vorbisfile, -1) / 1000;
	pstreams[id].duration      = (unsigned long)(ov_time_total(&pstreams[id].vorbisfile, -1) * 1000);
	pstreams[id].frequency     = vorbisinfo->rate;
	pstreams[id].channels      = vorbisinfo->channels;

	return 1;
}

int decoder_close(unsigned long id)
{
	ov_clear(&pstreams[id].vorbisfile);
	//sys_file_close(pstreams[id].hstream);
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
    ov_pcm_seek(&pstreams[id].vorbisfile, (ogg_int64_t) (pos * ((double)ov_pcm_total(&pstreams[id].vorbisfile, -1))));
	return 1;
}

unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize)
{
    int ret;
    unsigned long read_size = 0;

    while(read_size < dsize)
    {

		ret = ov_read_double_pcm(&pstreams[id].vorbisfile, (double*)((unsigned char*)adata + read_size), dsize - read_size, &pstreams[id].bstream);


        if(!ret)
        {
            return read_size;

        }else if(ret < 0){
			/* ?? */

        }else{

            read_size += ret;
        }
    }

    return read_size;
}

int tagread(const string fname,  struct fennec_audiotag *rtag)
{	
	FILE*           hfile;
    OggVorbis_File  vorbisfile;
    vorbis_comment* filecomment;
	int             i;
	char            ftag[128];
	char            *fdata, *tempdata;
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

    hfile = _wfopen(fname, uni("rb"));
    if(!hfile)return 0;

	fdata = (char*)sys_mem_alloc(65536);

    memset(&vorbisfile, 0, sizeof(vorbisfile));

    if(ov_open(hfile, &vorbisfile, 0, 0) < 0)
    {
		sys_mem_free(fdata);
        fclose(hfile);
        return 0;
    }

    filecomment = ov_comment(&vorbisfile, -1);

	tempdata = (char*)sys_mem_alloc(65536);

	for(i=0; i<filecomment->comments; i++)
	{
		if(filecomment->comment_lengths[i] >= 65536)continue;

		memcpy(tempdata, filecomment->user_comments[i], filecomment->comment_lengths[i]);
		tempdata[filecomment->comment_lengths[i]] = 0;
		tempdata[65536-1] = 0;
		
		if(sscanf(tempdata, " %[^= ] = %[^=]", ftag, fdata) == 2)
        {
            if(!stricmp(ftag, "TITLE"))
			{
                ct = &rtag->tag_title;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "ALBUM")){

                ct = &rtag->tag_album;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "ARTIST")){

                ct = &rtag->tag_artist;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "ORIGINALARTIST")){

                ct = &rtag->tag_origartist;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "COMPOSER")){

                ct = &rtag->tag_composer;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "LYRICIST")){

                ct = &rtag->tag_lyricist;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "BANDNAME")){

                ct = &rtag->tag_band;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "COPYRIGHT")){

                ct = &rtag->tag_copyright;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "PUBLISHER")){

                ct = &rtag->tag_publish;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "ENCODEDBY")){

                ct = &rtag->tag_encodedby;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "GENRE")){

                ct = &rtag->tag_genre;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "YEAR")){

                ct = &rtag->tag_year;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "URL")){

                ct = &rtag->tag_url;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "OFFICIALARTISTURL")){

                ct = &rtag->tag_offiartisturl;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "COMMENT")){

                ct = &rtag->tag_comments;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "LYRIC")){

                ct = &rtag->tag_lyric;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "BPM")){

                ct = &rtag->tag_bpm;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}else if(!stricmp(ftag, "TRACKNUMBER")){

                ct = &rtag->tag_tracknum;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

            }else if(!stricmp(ftag, "GENRE")){

                ct = &rtag->tag_genre;
				ct->tsize = (unsigned int)strlen(fdata);
				goto tagset;

			}

			goto tagsetend;
tagset:
			if(ct->tsize)
			{
				ct->tdata = (string) sys_mem_alloc((ct->tsize + 1) * sizeof(letter));
				if(ct->tdata)
				{
					MultiByteToWideChar(CP_UTF8, 0, fdata, -1, ct->tdata, ct->tsize + 1);
					ct->tmode = tag_memmode_dynamic;
				}else{
					ct->tsize = 0;
				}
			}

tagsetend:;

		}
	}

	sys_mem_free(fdata);
	sys_mem_free(tempdata);

    ov_clear(&vorbisfile);
    fclose(hfile);

	return 1;
}


/*-----------------------------------------------------------------------------
 fennec.
-----------------------------------------------------------------------------*/
