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



int decoder_load(unsigned long id, const string sname)
{
	HRESULT hr;

	pstreams[id].bitspersample = 16;
	pstreams[id].channels      = 1;
	pstreams[id].frequency     = 44100;
	pstreams[id].duration      = 0;
	
	if(DirectSoundCaptureCreate(0, &pstreams[id].ds, 0)) return 0;

	pstreams[id].wfx.wFormatTag      = WAVE_FORMAT_PCM;
	pstreams[id].wfx.nChannels       = pstreams[id].channels;
	pstreams[id].wfx.nSamplesPerSec  = pstreams[id].frequency;
	pstreams[id].wfx.wBitsPerSample  = pstreams[id].bitspersample;
	pstreams[id].wfx.nBlockAlign     = (pstreams[id].bitspersample / 8) * pstreams[id].channels;
	pstreams[id].wfx.nAvgBytesPerSec = pstreams[id].frequency * (pstreams[id].bitspersample / 8) * pstreams[id].channels;
	pstreams[id].wfx.cbSize          = sizeof(WAVEFORMATEX);

	pstreams[id].bsize = (float)pstreams[id].wfx.nAvgBytesPerSec;

	pstreams[id].dbs.dwSize        = sizeof(DSCBUFFERDESC);
	pstreams[id].dbs.dwFlags       = 0;
	pstreams[id].dbs.dwBufferBytes = pstreams[id].bsize;
	pstreams[id].dbs.dwReserved    = 0;
	pstreams[id].dbs.lpwfxFormat   = &pstreams[id].wfx;
	pstreams[id].dbs.dwFXCount     = 0;
	pstreams[id].dbs.lpDSCFXDesc   = 0;


 
	pstreams[id].cap = 0;
 
	hr = pstreams[id].ds->CreateCaptureBuffer(&pstreams[id].dbs, &pstreams[id].cap, 0);
	if(!SUCCEEDED(hr)) return 0;

	pstreams[id].pos = 0;


	pstreams[id].cap->Start(DSCBSTART_LOOPING);
	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].initialized)return 0;

	if(pstreams[id].cap)pstreams[id].cap->Stop();
	if(pstreams[id].cap)pstreams[id].cap->Release();
	if(pstreams[id].ds)pstreams[id].ds->Release();

	pstreams[id].ds  = 0;
	pstreams[id].cap = 0;

	pstreams[id].initialized = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	return 0;
}


unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize)
{
	HRESULT hr;
	void* pbCaptureData  = 0;
	DWORD dwCaptureLength;
	void* pbCaptureData2 = 0;
	DWORD dwCaptureLength2;
	UINT  dwDataWrote;
	DWORD dwReadPos;
	LONG  lLockSize;
	 
	if (FAILED (hr = pstreams[id].cap->GetCurrentPosition(0, &dwReadPos)))
		return dsize;
	 
	// Lock everything between our private cursor 
	// and the read cursor, allowing for wraparound.
	 
	lLockSize = dsize;
	if (FAILED(hr = pstreams[id].cap->Lock( 
		pstreams[id].pos, lLockSize, 
		&pbCaptureData, &dwCaptureLength, 
		&pbCaptureData2, &dwCaptureLength2, 0L)))
		return dsize;
	 
	// Write the data. This is done in two steps
	// to account for wraparound.

	memset(adata, 0, dsize);
	memcpy(adata, pbCaptureData, dwCaptureLength);
	 
	if (pbCaptureData2 != NULL)
	{
		memcpy(((char*)adata) + dwCaptureLength, pbCaptureData2, dwCaptureLength2);
	}
	 
	// Unlock the capture buffer.
	 
	pstreams[id].cap->Unlock( pbCaptureData, dwCaptureLength, 
		pbCaptureData2, dwCaptureLength2  );
	  
	// Move the capture offset along.
	 
	pstreams[id].pos += dwCaptureLength; 
	pstreams[id].pos %= pstreams[id].bsize; 
	pstreams[id].pos += dwCaptureLength2; 
	pstreams[id].pos %= pstreams[id].bsize; 

	return dsize ;
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
