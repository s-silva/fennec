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




/* code ---------------------------------------------------------------------*/

void alertv(unsigned long val)
{
	char      db[100];
	memset(db, 0, sizeof(db));
	itoa(val, db, 10);
	MessageBoxA(0, db, "ha", 32);
}

int decoder_load(unsigned long id, const string sname)
{
	unsigned long fz;
	REXInfo rx;
	unsigned long sc, i;

	pstreams[id].fhandle = _wfopen(sname, uni("rb"));

	if(!pstreams[id].fhandle) return 0;

	fseek(pstreams[id].fhandle, 0, SEEK_END);
	fz = ftell(pstreams[id].fhandle);
	fseek(pstreams[id].fhandle, 0, SEEK_SET);

	if(!fz)
	{
		fclose(pstreams[id].fhandle);
		pstreams[id].fhandle = 0;
		return 0;
	}

	pstreams[id].buffer = sys_mem_alloc(fz);
	fread(pstreams[id].buffer, 1, fz, pstreams[id].fhandle);

	//if(!IsREXLoadedByThisInstance())
		

	//if(!IsREXLoadedByThisInstance()) Beep(1000, 100);

	REXCreate(&pstreams[id].hrex, pstreams[id].buffer, fz, 0, 0);

	
	REXGetInfo(pstreams[id].hrex, sizeof(rx), &rx);

	pstreams[id].bitspersample = 64;
	pstreams[id].channels      = rx.fChannels;
	pstreams[id].frequency     = rx.fSampleRate;
	pstreams[id].duration      = rx.fPPQLength / (rx.fTempo / 1000) * 4;

	pstreams[id].slcs = rx.fSliceCount;

	pstreams[id].cpt = 0;

	sc = (2 * rx.fSampleRate);

	pstreams[id].obsize = (pstreams[id].duration / 1000 * pstreams[id].frequency) * rx.fChannels * sizeof(double);

	pstreams[id].ob[0] = sys_mem_alloc(sizeof(float) * sc);
	pstreams[id].ob[1] = sys_mem_alloc(sizeof(float) * sc);

	pstreams[id].obuffer = sys_mem_alloc(sizeof(double) * sc * 2);

	REXSetOutputSampleRate(pstreams[id].hrex, pstreams[id].frequency);

	REXSetPreviewTempo(pstreams[id].hrex, rx.fTempo);
	REXStartPreview(pstreams[id].hrex);

	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	REXStopPreview(pstreams[id].hrex);
	REXDelete(&pstreams[id].hrex);

	if(pstreams[id].fhandle)
	{
		fclose(pstreams[id].fhandle);
		pstreams[id].fhandle = 0;
	}

	if(pstreams[id].buffer) sys_mem_free(pstreams[id].buffer);
	if(pstreams[id].obuffer) sys_mem_free(pstreams[id].obuffer);
	if(pstreams[id].ob[0]) sys_mem_free(pstreams[id].ob[0]);
	if(pstreams[id].ob[1]) sys_mem_free(pstreams[id].ob[1]);

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	return 0;
}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	int i = 0; 
	int sc = dsize / sizeof(double) / 2;

	REXRenderPreviewBatch(pstreams[id].hrex, sc, pstreams[id].ob);
	//REXRenderSlice(pstreams[id].hrex, 1, sc * 2, ob);

	if(pstreams[id].cpt >= pstreams[id].obsize) return 0;

	if(pstreams[id].cpt + dsize > pstreams[id].obsize)
	{
		dsize = (pstreams[id].obsize - pstreams[id].cpt) / sizeof(double);
	}else{
		dsize = dsize;
	}

	for(i=0; i<sc; i++)
		pstreams[id].obuffer[i * 2] = pstreams[id].ob[0][i];// / 32.768f;;
	
	for(i=0; i<sc; i++)
		pstreams[id].obuffer[(i * 2) + 1] = pstreams[id].ob[1][i];// / 32.768f;;

	memcpy(adata, pstreams[id].obuffer, dsize);
	pstreams[id].cpt += dsize;
    return dsize;
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
