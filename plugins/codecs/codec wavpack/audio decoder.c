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
	char error [128];
	char  fname[v_sys_maxpath];
	BOOL  useddef = 1;

	WideCharToMultiByte(CP_ACP, 0, sname, -1, fname, sizeof(fname), "?", &useddef);

	pstreams[id].wpc = WavpackOpenFileInput(fname, error, OPEN_NORMALIZE, 23);
	if(!pstreams[id].wpc) return 0;
	
	pstreams[id].bitspersample = 64;
	pstreams[id].channels      = WavpackGetNumChannels(pstreams[id].wpc);
	pstreams[id].frequency     = WavpackGetSampleRate(pstreams[id].wpc);
	pstreams[id].numsamples    = WavpackGetNumSamples(pstreams[id].wpc);
	pstreams[id].duration      = (pstreams[id].numsamples / (pstreams[id].frequency / 1000));
	pstreams[id].bps           = WavpackGetBitsPerSample(pstreams[id].wpc);
	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	if(pstreams[id].wpc)
		WavpackCloseFile(pstreams[id].wpc);

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	WavpackSeekSample(pstreams[id].wpc, (uint32_t)((double)pstreams[id].numsamples * pos));
	return 0;
}


unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	int32_t        *idata;
	short          *sdata, *sodata = (short*) adata;
	fennec_sample  *fdata = (fennec_sample*) adata, t;
	unsigned long   i, j, c, rsamples, fv = sizeof(fennec_sample), scount = (dsize / sizeof(fennec_sample));

	idata = (int32_t*) sys_mem_alloc(dsize / 2);
	sdata = (short*)idata;

	rsamples = (unsigned long) WavpackUnpackSamples(pstreams[id].wpc, idata, (uint32_t) scount / pstreams[id].channels);
	rsamples *= pstreams[id].channels;

	switch(WavpackGetBytesPerSample(pstreams[id].wpc))
	{
	case 2:
		for(i=0; i<rsamples; i++)
		{
			fdata[i] = (double)idata[i] / 32768.0;
		}
		break;

	case 4:
		for(i=0; i<rsamples; i++)
		{
			fdata[i] = (double)idata[i] / 2147483648.0;
		}
		break;

	}


	sys_mem_free(idata);

	return rsamples * sizeof(fennec_sample);
}


int decoder_tagread(const string fname,  struct fennec_audiotag *rtag)
{
	WavpackContext                *wpc;
	char                           afname[v_sys_maxpath], error[128], cdata[4096];
	BOOL                           useddef = 1;
	struct fennec_audiotag_item   *ctag;
	char                          *tiname;
	int                            i;

	WideCharToMultiByte(CP_ACP, 0, fname, -1, afname, sizeof(afname), "?", &useddef);

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


	wpc = WavpackOpenFileInput(afname, error, OPEN_TAGS, 0);
	if(!wpc) return 0;
	

	for(i=0; ;i++)
	{
		switch(i)
		{
		case 0:  ctag = &rtag->tag_title         ; tiname = "Title";              break;
		case 1:  ctag = &rtag->tag_album         ; tiname = "Album";              break;
		case 2:  ctag = &rtag->tag_artist        ; tiname = "Artist";             break;
		case 3:  ctag = &rtag->tag_origartist    ; tiname = "OriginalArtist";     break;
		case 4:  ctag = &rtag->tag_composer      ; tiname = "Composer";           break;
		case 5:  ctag = &rtag->tag_lyricist      ; tiname = "Lyricist";           break;
		case 6:  ctag = &rtag->tag_band          ; tiname = "Band";               break;
		case 7:  ctag = &rtag->tag_copyright     ; tiname = "Copyright";          break;
		case 8:  ctag = &rtag->tag_publish       ; tiname = "Publisher";          break;
		case 9:  ctag = &rtag->tag_encodedby     ; tiname = "Tool";               break;
		case 10: ctag = &rtag->tag_genre         ; tiname = "Genre";              break;
		case 11: ctag = &rtag->tag_year          ; tiname = "Year";               break;
		case 12: ctag = &rtag->tag_url           ; tiname = "URL";                break;
		case 13: ctag = &rtag->tag_offiartisturl ; tiname = "OfficialArtistURL";  break;
		case 14: ctag = &rtag->tag_comments      ; tiname = "Comment";            break;
		case 15: ctag = &rtag->tag_lyric         ; tiname = "Lyrics";             break;
		case 16: ctag = &rtag->tag_bpm           ; tiname = "BPM";                break;
		case 17: ctag = &rtag->tag_tracknum      ; tiname = "Track";              break;
		default: goto point_tagread_end;                                          break;
		}
		
		WavpackGetTagItem(wpc, tiname, cdata, sizeof(cdata));
		ctag->tsize = MultiByteToWideChar(CP_ACP, 0, cdata, -1, 0, 0);
		ctag->tsize *= sizeof(letter);
		ctag->tdata = sys_mem_alloc(ctag->tsize + sizeof(letter));
		MultiByteToWideChar(CP_ACP, 0, cdata, -1, ctag->tdata, (ctag->tsize / sizeof(letter)) + 1);
	}

point_tagread_end:

	WavpackCloseFile(wpc);

	return 1;
}

int decoder_tagwrite(const string fname,  struct fennec_audiotag *rtag)
{
	WavpackContext                *wpc;
	char                           afname[v_sys_maxpath], error[128], cdata[4096];
	BOOL                           useddef = 1;
	char                          *tiname;
	struct fennec_audiotag_item   *ctag;
	int                            i;


	WideCharToMultiByte(CP_ACP, 0, fname, -1, afname, sizeof(afname), "?", &useddef);


	wpc = WavpackOpenFileInput(afname, error, OPEN_EDIT_TAGS, 0);
	if(!wpc) return 0;
	
	for(i=0; ;i++)
	{
		switch(i)
		{
		case 0:  ctag = &rtag->tag_title         ; tiname = "Title";              break;
		case 1:  ctag = &rtag->tag_album         ; tiname = "Album";              break;
		case 2:  ctag = &rtag->tag_artist        ; tiname = "Artist";             break;
		case 3:  ctag = &rtag->tag_origartist    ; tiname = "OriginalArtist";     break;
		case 4:  ctag = &rtag->tag_composer      ; tiname = "Composer";           break;
		case 5:  ctag = &rtag->tag_lyricist      ; tiname = "Lyricist";           break;
		case 6:  ctag = &rtag->tag_band          ; tiname = "Band";               break;
		case 7:  ctag = &rtag->tag_copyright     ; tiname = "Copyright";          break;
		case 8:  ctag = &rtag->tag_publish       ; tiname = "Publisher";          break;
		case 9:  ctag = &rtag->tag_encodedby     ; tiname = "Tool";               break;
		case 10: ctag = &rtag->tag_genre         ; tiname = "Genre";              break;
		case 11: ctag = &rtag->tag_year          ; tiname = "Year";               break;
		case 12: ctag = &rtag->tag_url           ; tiname = "URL";                break;
		case 13: ctag = &rtag->tag_offiartisturl ; tiname = "OfficialArtistURL";  break;
		case 14: ctag = &rtag->tag_comments      ; tiname = "Comment";            break;
		case 15: ctag = &rtag->tag_lyric         ; tiname = "Lyrics";             break;
		case 16: ctag = &rtag->tag_bpm           ; tiname = "BPM";                break;
		case 17: ctag = &rtag->tag_tracknum      ; tiname = "Track";              break;
		default: goto point_tagwrite_end;                                         break;
		}

		if(ctag->tsize)
		{
			WideCharToMultiByte(CP_ACP, 0, ctag->tdata, -1, cdata, sizeof(cdata), "?", &useddef);

			WavpackAppendTagItem(wpc, tiname, cdata, strlen(cdata));

		}else{
			WavpackDeleteTagItem(wpc, tiname);
		}
	}

point_tagwrite_end:

	WavpackWriteTag(wpc);
	WavpackCloseFile(wpc);
	return 1;
}


/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
