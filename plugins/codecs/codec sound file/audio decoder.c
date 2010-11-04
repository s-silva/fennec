/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (Sound Files).
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

int decoder_load(unsigned long id, const string sname)
{
	char  afname[v_sys_maxpath];
	BOOL  useddef = 1;

	WideCharToMultiByte(CP_ACP, 0, sname, -1, afname, sizeof(afname), "?", &useddef);

	pstreams[id].sndfile = sf_open(afname, SFM_READ, &pstreams[id].sfileinfo);

	if(!pstreams[id].sndfile)return 0;

	pstreams[id].bitspersample = 64;
	pstreams[id].channels      = pstreams[id].sfileinfo.channels;
	pstreams[id].frequency     = pstreams[id].sfileinfo.samplerate;
	pstreams[id].duration      = (unsigned long)((pstreams[id].sfileinfo.frames * 2 * pstreams[id].sfileinfo.channels) / ((pstreams[id].frequency * pstreams[id].channels * (2 /*pstreams[id].bitspersample / 8*/)) / 1000));
	pstreams[id].bitrate       = (pstreams[id].frequency * pstreams[id].channels * (pstreams[id].bitspersample / 8)) / 1000; /* / 1000 to get KBps */
	
	return 1;
}

int decoder_close(unsigned long id)
{
	sf_close(pstreams[id].sndfile);
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	sf_seek(pstreams[id].sndfile, (int)((double)pstreams[id].sfileinfo.frames * pos), SEEK_SET);
	return 1;
}

unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize)
{
	return (unsigned long)sf_read_double(pstreams[id].sndfile, (double*)adata, dsize / sizeof(fennec_sample)) * sizeof(fennec_sample);
}

int tagread(const string fname,  struct fennec_audiotag *rtag)
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

	{
		unsigned int c = (unsigned int)str_len(fname);
		unsigned int i = c;
		unsigned int p = 0;

		while(i)
		{
			if(fname[i] == uni('\\') || fname[i] == uni('/'))break;
			if(!p && (fname[i] == uni('.')))p = i;
			i--;
		}

		if(i)i++;

		if(c - i > 0)
		{
			rtag->tag_title.tsize = (c - i) * sizeof(letter);
			rtag->tag_title.tdata = sys_mem_alloc(rtag->tag_title.tsize + sizeof(letter));
			str_cpy(rtag->tag_title.tdata, fname + i);
			rtag->tag_title.tmode = tag_memmode_dynamic;
			if(p && (p - i) > 0)rtag->tag_title.tdata[p - i] = 0;
		}
	}
	return 1;
}

/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
