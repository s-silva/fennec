#include "main.h"

/*-----------------------------------------------------------------------------
 
 this code may look messy; I could mark todos all around the source, if I could
 advice m$ not to use identifiers like 'g_wszWMxxxxx' ;-D
 ...
 anyway, this is just for m$ and C++.
 ...
 note: the buffer caching system (to handle variable size sample input
       automatically; see html tags "buffer caching" in this source file)
	   can cause an over usage of memory (only if the wm team decided to do so)
	   I've let the buffer to increase (resize itself equal to the maximum
	   input sample buffer size) to save reallocation time.
-----------------------------------------------------------------------------*/

int decoder_load(unsigned long id, const string sname)
{
	HRESULT               hres;
	IWMOutputMediaProps*  ppProps;
	WM_MEDIA_TYPE*        wmt = 0;
	DWORD                 wmpz = 0;
	WAVEFORMATEX          wfx;
	DWORD                 i, outcount = 0;
	IWMHeaderInfo*        wminfo;
	WORD                  wmistream = 0;
    WMT_ATTR_DATATYPE     Type;
	WORD                  wmilen;

	CoInitialize(0);

	hres = WMCreateSyncReader(0, 0, &pstreams[id].wmreader);
	if(FAILED(hres))return 0;

	hres = pstreams[id].wmreader->Open(sname);

	pstreams[id].wmreader->GetOutputCount(&outcount);

	for(i=0; i<outcount; i++)
	{
		
		hres = pstreams[id].wmreader->GetOutputProps(i, &ppProps);
		if(FAILED(hres))
		{
			ppProps->Release();
			continue;
		}

		hres = ppProps->GetMediaType(0, &wmpz);
		if(FAILED(hres))
		{
			ppProps->Release();
			continue;
		}

		wmt = (WM_MEDIA_TYPE*) malloc(wmpz);

		hres = ppProps->GetMediaType(wmt, &wmpz);

		if(WMMEDIATYPE_Audio != wmt->majortype)
		{
			ppProps->Release();
			free(wmt);
			continue;
		}

		memcpy(&wfx, wmt->pbFormat, wmt->cbFormat);

		pstreams[id].channels      = wfx.nChannels;
		pstreams[id].frequency     = wfx.nSamplesPerSec;
		pstreams[id].bitspersample = wfx.wBitsPerSample;

		pstreams[id].wmaudioout = i;

		free(wmt);

		ppProps->Release();
		break;
	}
	pstreams[id].buffer = 0;
	pstreams[id].buffersize = 0;

	/* get information */

	hres = pstreams[id].wmreader->QueryInterface(IID_IWMHeaderInfo, (VOID **)&wminfo);
	if(FAILED(hres))return 0;

	wmistream = 0;

	hres = wminfo->GetAttributeByName(&wmistream, g_wszWMDuration, &Type, 0, &wmilen);

	if(hres == S_OK)
	{
		QWORD dur;
		wminfo->GetAttributeByName(&wmistream, g_wszWMDuration, &Type, (BYTE*)&dur,&wmilen);
		pstreams[id].duration = (DWORD)(dur / 10000);
	}

	wminfo->Release();
	return 1;
}

int decoder_close(unsigned long id)
{
	if(pstreams[id].buffer)free(pstreams[id].buffer);
	pstreams[id].wmreader->Close();
	pstreams[id].wmreader->Release();
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	if(!pstreams[id].duration)return 0;
	pstreams[id].wmreader->SetRange((QWORD)(pos * 10000.0f * ((double)(pstreams[id].duration))), 0);
	pstreams[id].buffersize = 0;
	pstreams[id].bufferpt = 0;
	return 1;
}

unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize)
{
	HRESULT      hres = 0;
	DWORD        i = 0;
	INSSBuffer*  ppSample;
	QWORD        pcnsSampleTime;
	QWORD        pcnsDuration;
	DWORD        pdwFlags;
	DWORD        pdwOutputNum;
	WORD         pwStreamNum;
	DWORD        lenret = 0;
	unsigned char* ptbuff;

	if(pstreams[id].buffersize && (pstreams[id].bufferpt < pstreams[id].buffersize))
	{
		memmove((char*)adata, pstreams[id].buffer + pstreams[id].bufferpt, min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt));
		i += min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt);
		pstreams[id].bufferpt += min(dsize, pstreams[id].buffersize - pstreams[id].bufferpt);

		if(i >= dsize)return dsize;
	}

	while(1)
	{
		hres = pstreams[id].wmreader->GetNextSample((WORD)pstreams[id].wmaudioout, &ppSample, &pcnsSampleTime, &pcnsDuration, &pdwFlags, &pdwOutputNum, &pwStreamNum);
		if(hres == NS_E_NO_MORE_SAMPLES || FAILED(hres))return 0;

		ppSample->GetBufferAndLength(&ptbuff, &lenret);
		
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

		ppSample->Release();

		if(i >= dsize)return dsize;
	}
    return i;
}

string WM_GetTagString(IWMHeaderInfo* wminfo, WORD* wmis, LPCWSTR tname)
{
    WMT_ATTR_DATATYPE     Type;
    string                wmivalue;
	WORD                  wmilen;
	HRESULT               hres;

	hres = wminfo->GetAttributeByName(wmis, tname, &Type, 0, &wmilen);

	if(hres != S_OK)return 0;

	wmivalue = (string) malloc(wmilen + sizeof(letter));

	wminfo->GetAttributeByName(wmis, tname, &Type, (BYTE*)wmivalue, &wmilen);

	return wmivalue;
}

int WM_SetTagString(IWMHeaderInfo* wminfo, WORD* wmis, LPCWSTR tname, const string tdata)
{
	HRESULT               hres;
    WMT_ATTR_DATATYPE     Type = WMT_TYPE_STRING;

	if(tdata)
	{
		hres = wminfo->SetAttribute(*wmis, tname, Type, (const BYTE*)tdata, (WORD)str_size(tdata));
		if(hres == S_OK)return 1;
	}else{
		hres = wminfo->SetAttribute(*wmis, tname, Type, 0, 0);
		if(hres == S_OK)return 1;
	}
	return 0;
}



int tagwrite(const string fname,  struct fennec_audiotag *wtag)
{	

	HRESULT               hres;
	IWMHeaderInfo*        wminfo;
	WORD                  wmistream = 0;
	BOOL                  sdef = 1;
	IWMMetadataEditor    *pEdit = 0;
	IWMHeaderInfo        *pInfo = 0;
	int                   twrote = 1;

	if(!wtag)return 0;

	CoInitialize(0);

	hres = WMCreateEditor(&pEdit);
	hres = pEdit->Open(fname);

	pEdit->QueryInterface(IID_IWMHeaderInfo, (VOID **)(&wminfo));


	if(wtag->tag_title.tsize         ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMTitle          ,wtag->tag_title.tdata         ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMTitle          , 0); }

	if(wtag->tag_album.tsize         ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMAlbumTitle     ,wtag->tag_album.tdata         ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMAlbumTitle     , 0); }

	if(wtag->tag_artist.tsize        ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMAuthor         ,wtag->tag_artist.tdata        ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMAuthor         , 0); }
	
	if(wtag->tag_origartist.tsize    ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMOriginalArtist ,wtag->tag_origartist.tdata    ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMOriginalArtist , 0); }
	
	if(wtag->tag_composer.tsize      ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMComposer       ,wtag->tag_composer.tdata      ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMComposer       , 0); }
	
	if(wtag->tag_lyricist.tsize      ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMLyrics         ,wtag->tag_lyricist.tdata      ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMWriter         , 0); }
 
	/* band: not used */
	
	if(wtag->tag_copyright.tsize     ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMCopyright      ,wtag->tag_copyright.tdata     ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMCopyright      , 0); }
	
	if(wtag->tag_publish.tsize       ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMPublisher      ,wtag->tag_publish.tdata       ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMPublisher      , 0); }
	
	if(wtag->tag_encodedby.tsize     ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMEncodedBy      ,wtag->tag_encodedby.tdata     ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMEncodedBy      , 0); }
	
	if(wtag->tag_genre.tsize         ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMGenre          ,wtag->tag_genre.tdata         ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMGenre          , 0); }
	
	if(wtag->tag_year.tsize          ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMYear           ,wtag->tag_year.tdata          ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMYear           , 0); }
	
	if(wtag->tag_url.tsize           ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMUserWebURL     ,wtag->tag_url.tdata           ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMUserWebURL     , 0); }
	
	if(wtag->tag_offiartisturl.tsize ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMAuthorURL      ,wtag->tag_offiartisturl.tdata ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMAuthorURL      , 0); }
	
	if(wtag->tag_comments.tsize      ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMDescription    ,wtag->tag_comments.tdata      ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMDescription    , 0); }
	
	if(wtag->tag_lyric.tsize         ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMLyrics         ,wtag->tag_lyric.tdata         ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMLyrics         , 0); }

	if(wtag->tag_bpm.tsize           ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMBeatsPerMinute ,wtag->tag_bpm.tdata           ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMBeatsPerMinute , 0); }

	if(wtag->tag_tracknum.tsize      ){ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMTrack          ,wtag->tag_tracknum.tdata      ); }
	else{ twrote = WM_SetTagString(wminfo, &wmistream, g_wszWMTrack          , 0); }

	if(!twrote)
	{
		MessageBox(0, uni("Couldn't set tag(s), if you're setting tags on currently playing media, try stopping it."), uni("Error on tagging"), MB_ICONQUESTION);
	}

	pEdit->Flush();
	pEdit->Close();
	wminfo->Release();
	pEdit->Release();

	return 1;
}

int tagread(const string fname, struct fennec_audiotag *rtag)
{ 
	struct fennec_audiotag_item* ct;
	HRESULT               hres;
	IWMHeaderInfo*        wminfo;
	WORD                  wmistream = 0;
	IWMSyncReader*        wmreader;
	BOOL                  sdef = 1;

	if(!rtag)return 0;

	CoInitialize(0);

	if(!fname)
	{
		if(rtag->tag_title.tsize         ) { free(rtag->tag_title.tdata         ); rtag->tag_title.tsize         = 0; }
		if(rtag->tag_album.tsize         ) { free(rtag->tag_album.tdata         ); rtag->tag_album.tsize         = 0; }
		if(rtag->tag_artist.tsize        ) { free(rtag->tag_artist.tdata        ); rtag->tag_artist.tsize        = 0; }
		if(rtag->tag_origartist.tsize    ) { free(rtag->tag_origartist.tdata    ); rtag->tag_origartist.tsize    = 0; }
		if(rtag->tag_composer.tsize      ) { free(rtag->tag_composer.tdata      ); rtag->tag_composer.tsize      = 0; }
		if(rtag->tag_lyricist.tsize      ) { free(rtag->tag_lyricist.tdata      ); rtag->tag_lyricist.tsize      = 0; }
		if(rtag->tag_band.tsize          ) { free(rtag->tag_band.tdata          ); rtag->tag_band.tsize          = 0; }
		if(rtag->tag_copyright.tsize     ) { free(rtag->tag_copyright.tdata     ); rtag->tag_copyright.tsize     = 0; }
		if(rtag->tag_publish.tsize       ) { free(rtag->tag_publish.tdata       ); rtag->tag_publish.tsize       = 0; }
		if(rtag->tag_encodedby.tsize     ) { free(rtag->tag_encodedby.tdata     ); rtag->tag_encodedby.tsize     = 0; }
		if(rtag->tag_genre.tsize         ) { free(rtag->tag_genre.tdata         ); rtag->tag_genre.tsize         = 0; }
		if(rtag->tag_year.tsize          ) { free(rtag->tag_year.tdata          ); rtag->tag_year.tsize          = 0; }
		if(rtag->tag_url.tsize           ) { free(rtag->tag_url.tdata           ); rtag->tag_url.tsize           = 0; }
		if(rtag->tag_offiartisturl.tsize ) { free(rtag->tag_offiartisturl.tdata ); rtag->tag_offiartisturl.tsize = 0; }
      //if(rtag->tag_filepath.tsize      ) { free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
      //if(rtag->tag_filename.tsize      ) { free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; }
		if(rtag->tag_comments.tsize      ) { free(rtag->tag_comments.tdata      ); rtag->tag_comments.tsize      = 0; }
		if(rtag->tag_lyric.tsize         ) { free(rtag->tag_lyric.tdata         ); rtag->tag_lyric.tsize         = 0; }
		if(rtag->tag_bpm.tsize           ) { free(rtag->tag_bpm.tdata           ); rtag->tag_bpm.tsize           = 0; }
		if(rtag->tag_tracknum.tsize      ) { free(rtag->tag_tracknum.tdata      ); rtag->tag_tracknum.tsize      = 0; }

	}else{

		hres = WMCreateSyncReader(0, 0, &wmreader);
		if(FAILED(hres))return 0;

		hres = wmreader->Open(fname);

		/* get information */

		hres = wmreader->QueryInterface(IID_IWMHeaderInfo, (VOID **)&wminfo);
		if(FAILED(hres))return 0;

		wmistream = 0;


		ct = &rtag->tag_title;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMTitle);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;

		ct = &rtag->tag_album;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMAlbumTitle);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_artist;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMAuthor);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_origartist;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMOriginalArtist);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_composer;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMComposer);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_lyricist;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMWriter);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_band;
		ct->tdata  = 0;
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_copyright;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMCopyright);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_publish;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMPublisher);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_encodedby;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMEncodedBy);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_genre;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMGenre);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_year;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMYear);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_url;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMUserWebURL);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_offiartisturl;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMAuthorURL);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_comments;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMDescription);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_lyric;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMLyrics);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) + 2 : 0;
	
		ct = &rtag->tag_bpm;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMBeatsPerMinute);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
	
		ct = &rtag->tag_tracknum;
		ct->tdata  = WM_GetTagString(wminfo, &wmistream, g_wszWMTrack);
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;

		wminfo->Release();
		wmreader->Close();
		wmreader->Release();

	}
	return 1;
}










