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



#define        static_sample_buffer_size (SAMPLES_PER_WRITE * FLAC_PLUGIN__MAX_SUPPORTED_CHANNELS * (24/8) * 2)




/* code ---------------------------------------------------------------------*/

ULONGLONG FileSize(string fileName)
{
	LARGE_INTEGER res;
	HANDLE hFile = CreateFile(fileName, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) return 0;
	res.LowPart = GetFileSize(hFile, &res.HighPart);
	CloseHandle(hFile);
	return res.QuadPart;
}



int decoder_load(unsigned long id, const string sname)
{
	char  fsname[1024];
	BOOL  useddef = 1;


	pstreams[id].dec = FLAC__file_decoder_new();

	pstreams[id].filesize = FileSize(sname);
	if(!pstreams[id].filesize) return 0;

	WideCharToMultiByte(CP_ACP, 0, sname, -1, fsname, sizeof(fsname), "?", &useddef);


	if(!FLAC_plugin__decoder_init(pstreams[id].dec, fsname, pstreams[id].filesize, &pstreams[id].finfo, &pstreams[id].output)) return 0;



	pstreams[id].bitspersample = pstreams[id].finfo.bits_per_sample;
	pstreams[id].channels      = pstreams[id].finfo.channels;
	pstreams[id].frequency     = pstreams[id].finfo.sample_rate;
	pstreams[id].duration      = pstreams[id].finfo.length_in_msec;
	pstreams[id].buffer_left   = 0;
	pstreams[id].buffer        = 0;
	pstreams[id].buffersize    = 0;
	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	FLAC_plugin__decoder_finish(pstreams[id].dec);
	FLAC_plugin__decoder_delete(pstreams[id].dec);
	if(pstreams[id].buffer)free(pstreams[id].buffer);

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	pstreams[id].finfo.seek_to = (int)((double)pos * (double)pstreams[id].duration);
	return 0;
}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	unsigned long    i = 0;
	unsigned long    lenret = 0;
	unsigned char    ptbuff[static_sample_buffer_size];

	if(pstreams[id].finfo.seek_to != -1)
		FLAC_plugin__seek(pstreams[id].dec, &pstreams[id].finfo);

	if(pstreams[id].buffersize && (pstreams[id].bufferpt < pstreams[id].buffersize))
	{
		memmove((char*)adata, pstreams[id].buffer + pstreams[id].bufferpt, min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt));
		i += min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt);
		pstreams[id].bufferpt += min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt);

		if(i >= dsize)return dsize;
	}

	while(1)
	{
		lenret = FLAC_plugin__decode(pstreams[id].dec, &pstreams[id].finfo, ptbuff);

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
	FLAC__StreamMetadata  *tags;
	char                   fsname[1024];
	BOOL                   useddef = 1;
	struct fennec_audiotag_item  *ct;


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


	WideCharToMultiByte(CP_ACP, 0, fname, -1, fsname, sizeof(fsname), "?", &useddef);

	FLAC_plugin__tags_get(fsname, &tags);


	ct = &rtag->tag_artist;

	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "PERFORMER");
	if(ct->tdata)
	{
		ct->tsize = str_len(ct->tdata);
	}else{
		/* try "artist" */
		ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "ARTIST");
		if(ct->tdata)ct->tsize = str_len(ct->tdata);
	}

	ct = &rtag->tag_title;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "TITLE");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_album;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "ALBUM");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_origartist;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "ORIGINALARTIST");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_composer;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "COMPOSER");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_lyricist;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "LYRICIST");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_band;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "BAND");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_copyright;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "COPYRIGHT");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_publish;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "PUBLISH");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_encodedby;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "ENCODEDBY");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_genre;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "GENRE");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_year;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "DATE");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_url;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "URL");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_offiartisturl;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "OFFICIALARTISTURL");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_comments;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "DESCRIPTION");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_lyric;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "LYRICS");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_bpm;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "BPM");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	ct = &rtag->tag_tracknum;
	ct->tdata = FLAC_plugin__tags_get_tag_ucs2(tags, "TRACKNUMBER");
	if(ct->tdata)ct->tsize = str_len(ct->tdata);

	
	FLAC_plugin__tags_destroy(&tags);
	return 1;
}

int decoder_tagwrite(const string fname,  struct fennec_audiotag *rtag)
{
	FLAC__StreamMetadata  *tags;
	char                   fsname[1024];
	BOOL                   useddef = 1;
	struct fennec_audiotag_item  *ct;

	if(!fname) return 0;
	


	WideCharToMultiByte(CP_ACP, 0, fname, -1, fsname, sizeof(fsname), "?", &useddef);

	if(!rtag)
	{
		FLAC_plugin__tags_get(fsname, &tags);
		FLAC_plugin__tags_delete_all(tags);
		FLAC_plugin__tags_set(fsname, tags);
		FLAC_plugin__tags_destroy(&tags);
		return 1;
	}


	FLAC_plugin__tags_get(fsname, &tags);

	if(rtag->tag_artist.tsize)         FLAC_plugin__tags_set_tag_ucs2(tags, "PERFORMER", rtag->tag_artist.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "PERFORMER");

	if(rtag->tag_title.tsize)          FLAC_plugin__tags_set_tag_ucs2(tags, "TITLE", rtag->tag_title.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "TITLE");

	if(rtag->tag_album.tsize)          FLAC_plugin__tags_set_tag_ucs2(tags, "ALBUM", rtag->tag_album.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "ALBUM");

	if(rtag->tag_origartist.tsize)     FLAC_plugin__tags_set_tag_ucs2(tags, "ORIGINALARTIST", rtag->tag_origartist.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "ORIGINALARTIST");

	if(rtag->tag_composer.tsize)       FLAC_plugin__tags_set_tag_ucs2(tags, "COMPOSER", rtag->tag_composer.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "COMPOSER");

	if(rtag->tag_lyricist.tsize)       FLAC_plugin__tags_set_tag_ucs2(tags, "LYRICIST", rtag->tag_lyricist.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "LYRICIST");

	if(rtag->tag_band.tsize)           FLAC_plugin__tags_set_tag_ucs2(tags, "BAND", rtag->tag_band.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "BAND");

	if(rtag->tag_copyright.tsize)      FLAC_plugin__tags_set_tag_ucs2(tags, "COPYRIGHT", rtag->tag_copyright.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "COPYRIGHT");

	if(rtag->tag_publish.tsize)        FLAC_plugin__tags_set_tag_ucs2(tags, "PUBLISH", rtag->tag_publish.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "PUBLISH");

	if(rtag->tag_encodedby.tsize)      FLAC_plugin__tags_set_tag_ucs2(tags, "ENCODEDBY", rtag->tag_encodedby.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "ENCODEDBY");

	if(rtag->tag_genre.tsize)          FLAC_plugin__tags_set_tag_ucs2(tags, "GENRE", rtag->tag_genre.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "GENRE");

	if(rtag->tag_year.tsize)           FLAC_plugin__tags_set_tag_ucs2(tags, "DATE", rtag->tag_year.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "DATE");

	if(rtag->tag_url.tsize)            FLAC_plugin__tags_set_tag_ucs2(tags, "URL", rtag->tag_url.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "URL");

	if(rtag->tag_offiartisturl.tsize)  FLAC_plugin__tags_set_tag_ucs2(tags, "OFFICIALARTISTURL", rtag->tag_offiartisturl.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "OFFICIALARTISTURL");

	if(rtag->tag_comments.tsize)       FLAC_plugin__tags_set_tag_ucs2(tags, "DESCRIPTION", rtag->tag_comments.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "DESCRIPTION");

	if(rtag->tag_lyric.tsize)          FLAC_plugin__tags_set_tag_ucs2(tags, "LYRICS", rtag->tag_lyric.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "LYRICS");

	if(rtag->tag_bpm.tsize)            FLAC_plugin__tags_set_tag_ucs2(tags, "BPM", rtag->tag_bpm.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "BPM");

	if(rtag->tag_tracknum.tsize)       FLAC_plugin__tags_set_tag_ucs2(tags, "TRACKNUMBER", rtag->tag_tracknum.tdata, 0);
	else                               FLAC_plugin__tags_delete_tag(tags,   "TRACKNUMBER");



	FLAC_plugin__tags_set(fsname, tags);

	FLAC_plugin__tags_destroy(&tags);
	return 1;
}


/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
