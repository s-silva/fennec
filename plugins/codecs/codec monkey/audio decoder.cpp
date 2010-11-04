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


int  ret_nulldata = 0;


/* code ---------------------------------------------------------------------*/



int decoder_load(unsigned long id, const string sname)
{
	int rv = 0;


	pstreams[id].adec = CreateIAPEDecompress(sname, &rv);

	if(pstreams[id].adec == NULL) /* error */
		return 0;
	
	pstreams[id].bitspersample = pstreams[id].adec->GetInfo(APE_INFO_BITS_PER_SAMPLE);
	pstreams[id].channels      = pstreams[id].adec->GetInfo(APE_INFO_CHANNELS);
	pstreams[id].frequency     = pstreams[id].adec->GetInfo(APE_INFO_SAMPLE_RATE);
	pstreams[id].duration      = pstreams[id].adec->GetInfo(APE_INFO_LENGTH_MS);
	pstreams[id].block_size    = pstreams[id].adec->GetInfo(APE_INFO_BLOCK_ALIGN);

	pstreams[id].block_count   = pstreams[id].adec->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS);

	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	delete pstreams[id].adec;

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	ret_nulldata = 1;
	Sleep(0);

	pstreams[id].adec->Seek( (int)(pos * (double)pstreams[id].block_count));
	
	ret_nulldata = 0;
	return 0;
}


unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize)
{
	int  rsize, ablocks;

	if(ret_nulldata)
	{
		memset(adata, 0, dsize);
		return dsize;
	}

	ablocks = dsize /  pstreams[id].block_size;

	pstreams[id].adec->GetData((char*)adata, ablocks, &rsize);

    return rsize * pstreams[id].block_size;
}


int decoder_tagread(const string fname,  struct fennec_audiotag *rtag)
{
	CAPETag          *ape_tag;
	IAPEDecompress   *adec;
	int               rv = 0;
	struct fennec_audiotag_item  *ctag;

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
	


	adec = CreateIAPEDecompress(fname, &rv);

	if(adec == NULL) /* error */
		return 0;

	ape_tag = (CAPETag *) adec->GetInfo(APE_INFO_TAG);

	if(!ape_tag->GetHasID3Tag() && !ape_tag->GetHasAPETag())
	{
		delete adec;
		return 0;
	}
	

	ctag = &rtag->tag_title;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		rv = ape_tag->GetFieldString(APE_TAG_FIELD_TITLE, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}



	ctag = &rtag->tag_album;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_ALBUM, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_artist;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_ARTIST, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	/*ctag = &rtag->tag_origartist;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_TITLE, ctag->tdata, (int*)&ctag->tsize);*/

	ctag = &rtag->tag_composer;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_COMPOSER, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	/*ctag = &rtag->tag_lyricist;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_TITLE, ctag->tdata, (int*)&ctag->tsize);*/

	ctag = &rtag->tag_copyright;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_COPYRIGHT, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_publish;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_PUBLISHER_URL, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_encodedby;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_TOOL_NAME, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_genre;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_GENRE, ctag->tdata, (int*)&ctag->tsize);
	
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_year;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_YEAR, ctag->tdata, (int*)&ctag->tsize);
		
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_url;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_BUY_URL, ctag->tdata, (int*)&ctag->tsize);
		
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_offiartisturl;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_ARTIST_URL, ctag->tdata, (int*)&ctag->tsize);
		
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_comments;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_COMMENT, ctag->tdata, (int*)&ctag->tsize);
		
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	ctag = &rtag->tag_lyric;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_LYRICS, ctag->tdata, (int*)&ctag->tsize);
		
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}


	/*ctag = &rtag->tag_bpm;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_TITLE, ctag->tdata, (int*)&ctag->tsize);*/
	
	ctag = &rtag->tag_tracknum;
		ctag->tsize = 1024;
		ctag->tdata = (string)sys_mem_alloc(ctag->tsize * sizeof(letter));
		ape_tag->GetFieldString(APE_TAG_FIELD_TRACK, ctag->tdata, (int*)&ctag->tsize);
		
	if(rv == -1 || !ctag->tsize)
	{
		sys_mem_free(ctag->tdata);
		ctag->tsize = 0;
	}

	rtag->tag_title.tsize         *= sizeof(letter);
	rtag->tag_album.tsize         *= sizeof(letter);
	rtag->tag_artist.tsize        *= sizeof(letter);
	rtag->tag_origartist.tsize    *= sizeof(letter);
	rtag->tag_composer.tsize      *= sizeof(letter);
	rtag->tag_lyricist.tsize      *= sizeof(letter);
	rtag->tag_band.tsize          *= sizeof(letter);
	rtag->tag_copyright.tsize     *= sizeof(letter);
	rtag->tag_publish.tsize       *= sizeof(letter);
	rtag->tag_encodedby.tsize     *= sizeof(letter);
	rtag->tag_genre.tsize         *= sizeof(letter);
	rtag->tag_year.tsize          *= sizeof(letter);
	rtag->tag_url.tsize           *= sizeof(letter);
	rtag->tag_offiartisturl.tsize *= sizeof(letter);
	rtag->tag_filepath.tsize      *= sizeof(letter);
	rtag->tag_filename.tsize      *= sizeof(letter);
	rtag->tag_comments.tsize      *= sizeof(letter);
	rtag->tag_lyric.tsize         *= sizeof(letter);
	rtag->tag_bpm.tsize           *= sizeof(letter);
	rtag->tag_tracknum.tsize      *= sizeof(letter);



	delete adec;

	return 1;
}

int decoder_tagwrite(const string fname,  struct fennec_audiotag *rtag)
{
	CAPETag                      *ape_tag;
	IAPEDecompress               *adec;
	int                           rv = 0;

	if(!fname)return 0;


	adec = CreateIAPEDecompress(fname, &rv);

	if(adec == NULL) /* error */
		return 0;

	ape_tag = (CAPETag *) adec->GetInfo(APE_INFO_TAG);

	ape_tag->SetIgnoreReadOnly(true);

	if(rtag)
	{
	
		
		if(rtag->tag_title.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_TITLE, rtag->tag_title.tdata);
		else                      ape_tag->SetFieldString(APE_TAG_FIELD_TITLE, uni(" "));

		if(rtag->tag_album.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_ALBUM, rtag->tag_album.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_ALBUM);
		
		if(rtag->tag_artist.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_ARTIST, rtag->tag_artist.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_ARTIST);

		if(rtag->tag_composer.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_COMPOSER, rtag->tag_composer.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_COMPOSER);

		if(rtag->tag_copyright.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_COPYRIGHT, rtag->tag_copyright.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_COPYRIGHT);

		if(rtag->tag_publish.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_PUBLISHER_URL, rtag->tag_publish.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_PUBLISHER_URL);

		if(rtag->tag_encodedby.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_TOOL_NAME, rtag->tag_encodedby.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_TOOL_NAME);

		if(rtag->tag_genre.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_GENRE, rtag->tag_genre.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_GENRE);

		if(rtag->tag_year.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_YEAR, rtag->tag_year.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_YEAR);

		if(rtag->tag_url.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_BUY_URL, rtag->tag_url.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_BUY_URL);

		if(rtag->tag_offiartisturl.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_ARTIST_URL, rtag->tag_offiartisturl.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_ARTIST_URL);

		if(rtag->tag_comments.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_COMMENT, rtag->tag_comments.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_COMMENT);

		if(rtag->tag_lyric.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_LYRICS, rtag->tag_lyric.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_LYRICS);

		if(rtag->tag_tracknum.tsize) ape_tag->SetFieldString(APE_TAG_FIELD_TRACK, rtag->tag_tracknum.tdata);
		else                      ape_tag->RemoveField(APE_TAG_FIELD_TRACK);


	}else{
		
		ape_tag->ClearFields();
	}

	ape_tag->Save();

	delete adec;
	return 1;
}


/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
