/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (Audio CD).
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
#include "soundlib/dlsbank.h"
#include "soundlib/mptrack.h"




int setting_preamp = 100;


int decoder_load(unsigned long id, const string fname)
{	
	static int import_midilib = 0;

	DWORD nbr = 0;
	char  sname[MAX_PATH];
	BOOL  usedef = 1;

	pstreams[id].bitspersample = 16;
	pstreams[id].frequency     = 44100;
	pstreams[id].channels      = 2;

	WideCharToMultiByte(CP_ACP, 0, fname, -1, sname, sizeof(sname), "?", &usedef);

	if(!import_midilib)
	{
		CTrackApp::ImportMidiConfig("C:\\mptrack.ini", 0);
	}

	pstreams[id].fhandle = CreateFile(sname, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, 0, 0);
    
	if(pstreams[id].fhandle == INVALID_HANDLE_VALUE)return 0;

	pstreams[id].fsize = GetFileSize(pstreams[id].fhandle, 0);

	if(!pstreams[id].fsize)
	{
		CloseHandle(pstreams[id].fhandle);
		return 0;
	}

	pstreams[id].fmem = (char*)malloc(pstreams[id].fsize);

	if(!pstreams[id].fmem)
	{
		CloseHandle(pstreams[id].fhandle);
		return 0;
	}

	ReadFile(pstreams[id].fhandle, pstreams[id].fmem, pstreams[id].fsize, &nbr, 0);

	CloseHandle(pstreams[id].fhandle);

	pstreams[id].csnd.Create((LPCBYTE)pstreams[id].fmem, pstreams[id].fsize);


	/* <midi stuff> */

	if (pstreams[id].csnd.m_nType == MOD_TYPE_MID)
	{
		CDLSBank *pCachedBank = NULL, *pEmbeddedBank = NULL;
		CHAR      szCachedBankFile[_MAX_PATH] = "";

		if (CDLSBank::IsDLSBank((LPCSTR)sname))
		{
			pEmbeddedBank = new CDLSBank();
			pEmbeddedBank->Open((LPCSTR)sname);
		}
		
		pstreams[id].csnd.m_nType = MOD_TYPE_IT;
		
		//BeginWaitCursor();
		
		LPMIDILIBSTRUCT lpMidiLib = CTrackApp::GetMidiLibrary();
		// Scan Instruments
		if (lpMidiLib) for (UINT nIns=1; nIns<=pstreams[id].csnd.m_nInstruments; nIns++) if (pstreams[id].csnd.Headers[nIns])
		{
			LPCSTR pszMidiMapName;
			INSTRUMENTHEADER *penv = pstreams[id].csnd.Headers[nIns];
			UINT nMidiCode;
			BOOL bEmbedded = FALSE;

			if (penv->nMidiChannel == 10)
				nMidiCode = 0x80 | (penv->nMidiDrumKey & 0x7F);
			else
				nMidiCode = penv->nMidiProgram & 0x7F;

			pszMidiMapName = lpMidiLib->MidiMap[nMidiCode];

			if (pEmbeddedBank)
			{
				UINT nDlsIns = 0, nDrumRgn = 0;
				UINT nProgram = penv->nMidiProgram;
				UINT dwKey = (nMidiCode < 128) ? 0xFF : (nMidiCode & 0x7F);
				if ((pEmbeddedBank->FindInstrument(	(nMidiCode >= 128),
													(penv->wMidiBank & 0x3FFF),
													nProgram, dwKey, &nDlsIns))
				 || (pEmbeddedBank->FindInstrument(	(nMidiCode >= 128),	0xFFFF,
													(nMidiCode >= 128) ? 0xFF : nProgram,
													dwKey, &nDlsIns)))
				{
					if (dwKey < 0x80) nDrumRgn = pEmbeddedBank->GetRegionFromKey(nDlsIns, dwKey);
					if (pEmbeddedBank->ExtractInstrument(&pstreams[id].csnd, nIns, nDlsIns, nDrumRgn))
					{
						if ((dwKey >= 24) && (dwKey < 100))
						{
							lstrcpyn(penv->name, szMidiPercussionNames[dwKey-24], sizeof(penv->name));
						}
						bEmbedded = TRUE;
					}
				}
			}
			if ((pszMidiMapName) && (pszMidiMapName[0]) && (!bEmbedded))
			{
				// Load From DLS Bank
				if (CDLSBank::IsDLSBank(pszMidiMapName))
				{
					CDLSBank *pDLSBank = NULL;
					
					if ((pCachedBank) && (!lstrcmpi(szCachedBankFile, pszMidiMapName)))
					{
						pDLSBank = pCachedBank;
					} else
					{
						if (pCachedBank) delete pCachedBank;
						pCachedBank = new CDLSBank;
						strcpy(szCachedBankFile, pszMidiMapName);
						if (pCachedBank->Open(pszMidiMapName)) pDLSBank = pCachedBank;
					}
					if (pDLSBank)
					{
						UINT nDlsIns = 0, nDrumRgn = 0;
						UINT nProgram = penv->nMidiProgram;
						UINT dwKey = (nMidiCode < 128) ? 0xFF : (nMidiCode & 0x7F);
						if ((pDLSBank->FindInstrument(	(nMidiCode >= 128),
														(penv->wMidiBank & 0x3FFF),
														nProgram, dwKey, &nDlsIns))
						 || (pDLSBank->FindInstrument(	(nMidiCode >= 128), 0xFFFF,
														(nMidiCode >= 128) ? 0xFF : nProgram,
														dwKey, &nDlsIns)))
						{
							if (dwKey < 0x80) nDrumRgn = pDLSBank->GetRegionFromKey(nDlsIns, dwKey);
							pDLSBank->ExtractInstrument(&pstreams[id].csnd, nIns, nDlsIns, nDrumRgn);
							if ((dwKey >= 24) && (dwKey < 24+61))
							{
								lstrcpyn(penv->name, szMidiPercussionNames[dwKey-24], sizeof(penv->name));
							}
						}
					}
				} else
				{
					// Load from Instrument or Sample file
					CHAR szName[_MAX_FNAME], szExt[_MAX_EXT];
					CFile f;

					if (f.Open(pszMidiMapName, CFile::modeRead))
					{
						DWORD len = f.GetLength();
						LPBYTE lpFile;
						if ((len) && ((lpFile = (LPBYTE)GlobalAllocPtr(GHND, len)) != NULL))
						{
							f.Read(lpFile, len);
							pstreams[id].csnd.ReadInstrumentFromFile(nIns, lpFile, len);
							_splitpath(pszMidiMapName, NULL, NULL, szName, szExt);
							strncat(szName, szExt, sizeof(szName));
							penv = pstreams[id].csnd.Headers[nIns];
							if (!penv->filename[0]) lstrcpyn(penv->filename, szName, sizeof(penv->filename));
							if (!penv->name[0])
							{
								if (nMidiCode < 128)
								{
									lstrcpyn(penv->name, szMidiProgramNames[nMidiCode], sizeof(penv->name));
								} else
								{
									UINT nKey = nMidiCode & 0x7F;
									if (nKey >= 24)
										lstrcpyn(penv->name, szMidiPercussionNames[nKey-24], sizeof(penv->name));
								}
							}
						}
						f.Close();
					}
				}
			}
		}
		if (pCachedBank) delete pCachedBank;
		if (pEmbeddedBank) delete pEmbeddedBank;
		//EndWaitCursor();
	}
	// Convert to MOD/S3M/XM/IT
	switch(pstreams[id].csnd.m_nType)
	{
	case MOD_TYPE_MOD:
	case MOD_TYPE_S3M:
	case MOD_TYPE_XM:
	case MOD_TYPE_IT:
		break;

	case MOD_TYPE_AMF0:
	case MOD_TYPE_MTM:
	case MOD_TYPE_669:
		pstreams[id].csnd.m_nType = MOD_TYPE_MOD;
		break;

	case MOD_TYPE_MED:
	case MOD_TYPE_OKT:
	case MOD_TYPE_AMS:
	case MOD_TYPE_MT2:
		pstreams[id].csnd.m_nType = MOD_TYPE_XM;
		if ((pstreams[id].csnd.m_nDefaultTempo == 125) && (pstreams[id].csnd.m_nDefaultSpeed == 6) && (!pstreams[id].csnd.m_nInstruments))
		{
			pstreams[id].csnd.m_nType = MOD_TYPE_MOD;
			for (UINT i=0; i<MAX_PATTERNS; i++)
				if ((pstreams[id].csnd.Patterns[i]) && (pstreams[id].csnd.PatternSize[i] != 64))
					pstreams[id].csnd.m_nType = MOD_TYPE_XM;
		}
		break;
	case MOD_TYPE_FAR:
	case MOD_TYPE_PTM:
	case MOD_TYPE_STM:
	case MOD_TYPE_DSM:
	case MOD_TYPE_AMF:
	case MOD_TYPE_PSM:
		pstreams[id].csnd.m_nType = MOD_TYPE_S3M;
		break;
	default:
		pstreams[id].csnd.m_nType = MOD_TYPE_IT;
	}

	/* </midi stuff> */

	CSoundFile::gdwSoundSetup   |= SNDMIX_DIRECTTODISK;
	CSoundFile::gdwSoundSetup   |= SNDMIX_REVERB;
	CSoundFile::gdwSoundSetup   |= SNDMIX_ENABLEMMX;
	CSoundFile::gdwMixingFreq   = 44100;
	CSoundFile::gnBitsPerSample = 16;
	CSoundFile::gnChannels      = 2;
	CSoundFile::gdwSysInfo      |= SYSMIX_ENABLEMMX;
	CSoundFile::gnAGC   = 128;
	pstreams[id].csnd.m_nGlobalVolume = 256;
	pstreams[id].csnd.m_nSongPreAmp   = 100;

	pstreams[id].csnd.SetReverbParameters(90, 6);
pstreams[id].csnd.InitializeDSP(0);
	

	pstreams[id].csnd.ResetChannels();
	CSoundFile::InitPlayer(TRUE);
	pstreams[id].csnd.SetRepeatCount(0);

	pstreams[id].duration = pstreams[id].csnd.GetSongTime() * 1000;
	return 1;
}


int decoder_close(unsigned long id)
{
	if(pstreams[id].fmem)
	{
		pstreams[id].csnd.Destroy();
		free(pstreams[id].fmem);
		pstreams[id].fmem = 0;
	}
	return 1;
}


int decoder_seek(unsigned long id, double pos)
{
	pstreams[id].csnd.SetCurrentPos((UINT)(pos * ((double)pstreams[id].csnd.GetMaxPosition())) );

	return 1;
}


unsigned long decoder_read(unsigned long id, void* adata, unsigned long isize)
{
	UINT           samples_read;
	short         *sdata = (short*)adata;
	unsigned long  i, c;
	float          ts;

	samples_read = pstreams[id].csnd.Read(adata, isize);

	c = isize / sizeof(short);

	for(i=0; i<c; i++)
	{
		ts = (float)sdata[i] * 4.5f;

		if(ts > 32767.0f)       ts = 32767.0f;
		else if(ts < -32767.0f) ts = -32767.0f;

		sdata[i] = (short)ts;
	}

	return samples_read * 4;
}


/* tagging ------------------------------------------------------------------*/


int tagread(const string fname, struct fennec_audiotag* rtag)
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
		if(rtag->tag_filepath.tsize      ) { sys_mem_free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
		if(rtag->tag_filename.tsize      ) { sys_mem_free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; }
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


int tagwrite(const string fname, struct fennec_audiotag* rtag)
{
	return 0;
}

void Log(LPCSTR format,...)
{
	
}

BOOL SoundDeviceCallback(DWORD dwUser)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
// CMappedFile

CMappedFile::CMappedFile()
//------------------------
{
	m_hFMap = NULL;
	m_lpData = NULL;
}


CMappedFile::~CMappedFile()
//-------------------------
{
}


BOOL CMappedFile::Open(LPCSTR lpszFileName)
//-----------------------------------------
{
	return m_File.Open(lpszFileName, CFile::modeRead|CFile::typeBinary);
}


void CMappedFile::Close()
//-----------------------
{
	if (m_lpData) Unlock();
	m_File.Close();
}


DWORD CMappedFile::GetLength()
//----------------------------
{
	return m_File.GetLength();
}


LPBYTE CMappedFile::Lock(DWORD dwMaxLen)
//--------------------------------------
{
	DWORD dwLen = GetLength();
	LPBYTE lpStream;

	if (!dwLen) return NULL;
	if ((dwMaxLen) && (dwLen > dwMaxLen)) dwLen = dwMaxLen;
	HANDLE hmf = CreateFileMapping(
							(HANDLE)m_File.m_hFile,
							NULL,
							PAGE_READONLY,
							0, 0,
							NULL
							);
	if (hmf)
	{
		lpStream = (LPBYTE)MapViewOfFile(
								hmf,
								FILE_MAP_READ,
								0, 0,
								0
							);
		if (lpStream)
		{
			m_hFMap = hmf;
			m_lpData = lpStream;
			return lpStream;
		}
		CloseHandle(hmf);
	}
	//if (dwLen > CTrackApp::gMemStatus.dwTotalPhys) return NULL;
	if ((lpStream = (LPBYTE)GlobalAllocPtr(GHND, dwLen)) == NULL) return NULL;
	m_File.Read(lpStream, dwLen);
	m_lpData = lpStream;
	return lpStream;
}


BOOL CMappedFile::Unlock()
//------------------------
{
	if (m_hFMap)
	{
		if (m_lpData)
		{
			UnmapViewOfFile(m_lpData);
			m_lpData = NULL;
		}
		CloseHandle(m_hFMap);
		m_hFMap = NULL;
	}
	if (m_lpData)
	{
		GlobalFreePtr(m_lpData);
		m_lpData = NULL;
	}
	return TRUE;
}

LPMIDILIBSTRUCT CTrackApp::glpMidiLibrary = NULL;


BOOL CTrackApp::ImportMidiConfig(LPCSTR lpszConfigFile, BOOL bNoWarn)
{
	if (!glpMidiLibrary)
	{
		glpMidiLibrary = new MIDILIBSTRUCT;
		if (!glpMidiLibrary) return FALSE;
		memset(glpMidiLibrary, 0, sizeof(MIDILIBSTRUCT));
	}

	for (UINT iMidi=0; iMidi<256; iMidi++)
	{
		glpMidiLibrary->MidiMap[iMidi] = new CHAR[_MAX_PATH];

		memset(glpMidiLibrary->MidiMap[iMidi], 0, sizeof(CHAR) * _MAX_PATH);
		GetSystemDirectory(glpMidiLibrary->MidiMap[iMidi], _MAX_PATH);
		strcat(glpMidiLibrary->MidiMap[iMidi], "\\drivers\\gm.dls");
	}

	return FALSE;
}
/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

