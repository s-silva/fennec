/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (AAC).
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
#include <commctrl.h>

HINSTANCE hInst;


unsigned int setting_type     = 1; /* wave */
unsigned int setting_bps      = 0; /* default */
unsigned int setting_channels = 0; /* default */


string sbitrates[] = {uni("8 kbps"), uni("16 kbps"), uni("24 kbps"), uni("32 kbps"), uni("40 kbps"), uni("48 kbps"),
	                 uni("56 kbps"), uni("64 kbps"), uni("72 kbps"), uni("96 kbps"), uni("128 kbps"), uni("192 kbps"),
					 uni("250 kbps")};
int   ibitrates[] = {8, 16, 24, 32, 40, 48, 56, 64, 72, 96, 128, 192, 250};


string squalities[] = {uni("Highest"), uni("High"), uni("Normal"), uni("Low"), uni("Lowest")};
int    iqualities[] = {130, 120, 100, 90, 70};



BOOL WINAPI DllMain(HINSTANCE hinstDLL, unsigned long fdwReason, LPVOID lpReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
		hInst = hinstDLL;

    return TRUE;
}

int encoder_setdefaults(unsigned long id)
{

	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	int v = 0;

	if(fsettings.plugin_settings_getnum("aac", "mp4", &v, 0, 0))v = 0;

	pestreams[id].ismp4 = v;

	if(pestreams[id].ismp4)
		str_cat(pestreams[id].filepath, uni(".mp4"));
	else
		str_cat(pestreams[id].filepath, uni(".aac"));

	return 1;
}

int encoder_deleteextension(unsigned long id, string fpath)
{
	int i = (int)str_len(fpath);

	if(i - 3 <= 0)return 0;
	i -= 3;

	fpath[i] = 0;
	return 1;
}

int encoder_initialize(unsigned long id, int fcreatemode)
{
	pestreams[id].firstwrite = 1;

	if(pestreams[id].smode == fennec_encoder_file && !pestreams[id].ismp4)
	{
		if(fcreatemode == fennec_encode_openmode_createnew)
		{
			pestreams[id].fhandle = sys_file_createstream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;

		}else if(fcreatemode == fennec_encode_openmode_create){

			pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seteof(pestreams[id].fhandle);

		}else if(fcreatemode == fennec_encode_openmode_append){

			pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seek(pestreams[id].fhandle, sys_file_getsize(pestreams[id].fhandle));

		}else{

			/* unknown mode! */
			return 0;
		}
	}

	
	return 1;
}


int encoder_uninitialize(unsigned long id)
{
	if(!pestreams[id].firstwrite)
	{
		faacEncClose(pestreams[id].enchandle);

		sys_mem_free(pestreams[id].floatbuffer);
		sys_mem_free(pestreams[id].obuffer);
		sys_mem_free(pestreams[id].cachebuffer);


		if(!pestreams[id].ismp4)
		{
			sys_file_close(pestreams[id].fhandle);
		}else{
			MP4Close(pestreams[id].mp4file);
		}

		pestreams[id].firstwrite = 1;
	}
	return 1;
}

int encoder_set(unsigned long id)
{	

	return 1;
}

int local_write_doublebuffer(unsigned long id, double *dbuff, unsigned long scount, int bytesencoded, int framesize)
{
	unsigned int   i;

	if(pestreams[id].fb_size <= scount * sizeof(float) || !pestreams[id].floatbuffer)
	{
		pestreams[id].fb_size = scount * sizeof(float);
		pestreams[id].floatbuffer = (float*) sys_mem_realloc(pestreams[id].floatbuffer, pestreams[id].fb_size);
	}


	for(i=0; i<scount; i++)
		pestreams[id].floatbuffer[i] = (float)(dbuff[i] * 32767.0);


	bytesencoded = faacEncEncode(pestreams[id].enchandle,
								(int32_t *)pestreams[id].floatbuffer,
								scount,
								pestreams[id].obuffer,
								pestreams[id].maxbytesout);

	if(pestreams[id].ismp4)
	{
		if(bytesencoded)
		{
			unsigned int samples_left = pestreams[id].totalsamples - pestreams[id].encodedsamples + framesize;
			MP4Duration dur           = samples_left > framesize ? framesize : samples_left;
			MP4Duration ofs           = pestreams[id].encodedsamples > 0 ? 0 : framesize;


			MP4WriteSample(pestreams[id].mp4file, pestreams[id].mp4track, pestreams[id].obuffer, bytesencoded, dur, ofs, 1);

			pestreams[id].encodedsamples += (unsigned int)dur;
		}

	}else{
		if(bytesencoded)
			sys_file_write(pestreams[id].fhandle, pestreams[id].obuffer, bytesencoded);
	}
}

int encoder_write(unsigned long id, void* rbuffer, unsigned long bsize)
{	
	int             bytesencoded = 0;
	double         *sbuffer = (double*) rbuffer;
	unsigned char  *bbuffer = (unsigned char*) rbuffer;
	unsigned int    framesize = 0;

	if(pestreams[id].firstwrite)
	{
		int v = 0;

		pestreams[id].enchandle = faacEncOpen(pestreams[id].cfrequency, pestreams[id].cchannels,
											  &pestreams[id].samplesin, &pestreams[id].maxbytesout);

		pestreams[id].obuffer     = (unsigned char*)sys_mem_alloc(pestreams[id].maxbytesout);
		pestreams[id].cachebuffer = (unsigned char*)sys_mem_alloc(pestreams[id].samplesin * 10);
		pestreams[id].fb_size     = 0;
		pestreams[id].floatbuffer = 0;

		pestreams[id].econfig = faacEncGetCurrentConfiguration(pestreams[id].enchandle);

		pestreams[id].econfig->inputFormat = FAAC_INPUT_FLOAT;

		pestreams[id].econfig->aacObjectType = LOW; //objectType;

		if(pestreams[id].ismp4)
		{
			pestreams[id].econfig->mpegVersion   = MPEG4;
			pestreams[id].econfig->outputFormat  = 0;
			pestreams[id].econfig->allowMidside  = 1;
			pestreams[id].econfig->shortctl      = SHORTCTL_NORMAL;

			if(fsettings.plugin_settings_getnum("aac", "bitrate", &v, 0, 0))v = 0xa;
			pestreams[id].econfig->bitRate       = (ibitrates[v] * 1000) / pestreams[id].cchannels;

			v = 0x2;
			if(fsettings.plugin_settings_getnum("aac", "quality", &v, 0, 0))v = 0x2;
			pestreams[id].econfig->quantqual     = iqualities[v];

			pestreams[id].econfig->bandWidth     = pestreams[id].cfrequency / 2;

		}else{

			pestreams[id].econfig->mpegVersion   = MPEG2;

			pestreams[id].econfig->useTns        = 0;
			pestreams[id].econfig->shortctl      = SHORTCTL_NORMAL;
			pestreams[id].econfig->useLfe        = 0;
			pestreams[id].econfig->allowMidside  = 1;

			if(fsettings.plugin_settings_getnum("aac", "bitrate", &v, 0, 0))v = 0xa;
			pestreams[id].econfig->bitRate       = (ibitrates[v] * 1000) / pestreams[id].cchannels;

			v = 0x2;
			if(fsettings.plugin_settings_getnum("aac", "quality", &v, 0, 0))v = 0x2;
			pestreams[id].econfig->quantqual     = iqualities[v];

			pestreams[id].econfig->bandWidth     = 0;
		}


		faacEncSetConfiguration(pestreams[id].enchandle, pestreams[id].econfig);

		/* mp4 stuff */

		if(pestreams[id].ismp4)
		{
			unsigned char *ASC       = 0;
			unsigned long  ASCLength = 0;
			char           afname[v_sys_maxpath];
			BOOL           usedef = 1;

			WideCharToMultiByte(CP_ACP, 0, pestreams[id].filepath, -1, afname, sizeof(afname), "?", &usedef);

#			ifdef MP4_CREATE_EXTENSIBLE_FORMAT

				pestreams[id].mp4file = MP4Create(afname, 0, 0);
#			else

				pestreams[id].mp4file = MP4Create(afname, 0, 0, 0);
#			endif


			MP4SetTimeScale(pestreams[id].mp4file, 90000);
			pestreams[id].mp4track = MP4AddAudioTrack(pestreams[id].mp4file, pestreams[id].cfrequency, MP4_INVALID_DURATION, MP4_MPEG4_AUDIO_TYPE);
			MP4SetAudioProfileLevel(pestreams[id].mp4file, 0x0F);

			faacEncGetDecoderSpecificInfo(pestreams[id].enchandle, &ASC, &ASCLength);

			MP4SetTrackESConfiguration(pestreams[id].mp4file, pestreams[id].mp4track, ASC, ASCLength);


			free(ASC);

			MP4SetMetadataTool(pestreams[id].mp4file, "Fennec Player 1.1 (libfaac)");
			
		}



		pestreams[id].totalsamples   = 0;
		pestreams[id].encodedsamples = 0;
		pestreams[id].cachesize      = 0;
		pestreams[id].firstwrite     = 0;
	}

	
	/* if(pestreams[id].cbitspersample == 64) standard */
	{
		unsigned int i = 0;
		unsigned int sbytes = pestreams[id].cbitspersample / 8;


		framesize = pestreams[id].samplesin / pestreams[id].cchannels;

		if(pestreams[id].ismp4)
			pestreams[id].totalsamples += bsize / sbytes;

		while(1)
		{
			if((int)(bsize / sbytes) - (int)i < (int)pestreams[id].samplesin)
			{
				if((int)(bsize / sbytes) - (int)i > 0)
				{
					memcpy(pestreams[id].cachebuffer + pestreams[id].cachesize, sbuffer + i, (bsize / sbytes) - i);
					pestreams[id].cachesize += (bsize / sbytes) - i;
				}
				break;
			}

			if(pestreams[id].cachesize)
			{
				memcpy(pestreams[id].cachebuffer + pestreams[id].cachesize, sbuffer + i, (pestreams[id].samplesin * sbytes) - pestreams[id].cachesize);
				
				local_write_doublebuffer(id, (double*)pestreams[id].cachebuffer, pestreams[id].samplesin, bytesencoded, framesize);


				pestreams[id].cachesize = 0;

				i += pestreams[id].samplesin - pestreams[id].cachesize;

			}else{

				local_write_doublebuffer(id, (double*)(sbuffer + i), pestreams[id].samplesin, bytesencoded, framesize);

				i += pestreams[id].samplesin;
			}
		}
	}

	return 1;
}

int proc_settings_fileencoder(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{

#	define addit(h, a)   (SendDlgItemMessage(hwnd, h, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)a))
#	define selit(h, n)   (SendDlgItemMessage(hwnd, h, CB_SETCURSEL, (WPARAM)n, 0))
#	define getsel(h)     (SendDlgItemMessage(hwnd, h, CB_GETCURSEL, 0, 0))
#	define items_i(x)    (sizeof(x) / sizeof((x)[0]))
#	define checkset(h, s)(CheckDlgButton(hwnd, h, s ? BST_CHECKED : BST_UNCHECKED))
#	define checkget(h)   (IsDlgButtonChecked(hwnd, h) == BST_CHECKED)

	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:

			fsettings.plugin_settings_setnum("aac", "bitrate", (int)getsel(combo_bitrate));
			fsettings.plugin_settings_setnum("aac", "quality", (int)getsel(combo_quality));
			fsettings.plugin_settings_setnum("aac", "mp4", (int)checkget(check_mp4));

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int i;
			int          v = 0;
			
			for(i = 0;  i < items_i(sbitrates);  i++)
			{
				addit(combo_bitrate, sbitrates[i]);
			}

			if(fsettings.plugin_settings_getnum("aac", "bitrate", &v, 0, 0))v = 0xa;

			selit(combo_bitrate, v);

			for(i = 0;  i < items_i(squalities);  i++)
			{
				addit(combo_quality, squalities[i]);
			}

			if(fsettings.plugin_settings_getnum("aac", "quality", &v, 0, 0))v = 0x2;

			selit(combo_quality, v);

			if(fsettings.plugin_settings_getnum("aac", "mp4", &v, 0, 0))v = 0;

			checkset(check_mp4, v);

		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 fennec.
-----------------------------------------------------------------------------*/
