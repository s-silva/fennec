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



int decoder_load(unsigned long id, const string sname)
{
	pstreams[id].fhandle = _wfopen(sname, uni("rb"));

	if(!pstreams[id].fhandle) return 0;

	pstreams[id].bitspersample = 64;
	pstreams[id].channels      = 0;
	pstreams[id].frequency     = (unsigned long)-1;
	pstreams[id].duration      = 0;
	pstreams[id].filebps       = 0;

	fseek(pstreams[id].fhandle, 0, SEEK_END);
	pstreams[id].filesize      = ftell(pstreams[id].fhandle);
	fseek(pstreams[id].fhandle, 0, SEEK_SET);
	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	if(pstreams[id].fhandle)
	{
		fclose(pstreams[id].fhandle);
		pstreams[id].fhandle = 0;
	}

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	int framesize = (pstreams[id].filebps/8) * pstreams[id].channels;
	fseek(pstreams[id].fhandle, ((long)(pos * (double)pstreams[id].filesize) / framesize) * framesize, SEEK_SET);
	return 0;
}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	double *odata = (double*)adata;
	int     rread, oval, i, j, dsamples;

	if(!pstreams[id].fhandle) return 0;

	if(!pstreams[id].filebps)
	{
		memset(adata, 0, dsize);
		return dsize;
	}

	rread    = (dsize * (pstreams[id].filebps / 8)) / sizeof(double);

	pstreams[id].duration = pstreams[id].filesize / (pstreams[id].filebps / 8) / (pstreams[id].frequency / 1000) / pstreams[id].channels;
	
	memset(adata, 0, dsize);

    oval = (int)fread(adata, 1, rread, pstreams[id].fhandle);

	dsamples = oval / (pstreams[id].filebps / 8);

	if(pstreams[id].filebps == 16)
	{
		int16_t *sbuffer = (int16_t*)adata;

		for(i=dsamples; i>0; )
		{
			i--;
			if(i<0) break;

			odata[i] = (double)sbuffer[i] / 32768.0;

		}
	}else if(pstreams[id].filebps == 8){
	
		int8_t *cbuffer = (int8_t*)adata;

		for(i=dsamples; i>0; )
		{
			i--;
			if(i<0) break;

			odata[i] = (double)cbuffer[i] / 128.0;
		}

	}else if(pstreams[id].filebps == 32){
	
		int32_t *ibuffer = (int32_t*)adata;

		for(i=dsamples; i>0; )
		{
			i--;
			if(i<0) break;

			odata[i] = (double)ibuffer[i] / 2147483648.0;
		}
	}
	


	return dsamples * sizeof(double);
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
