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


int encoder_setdefaults(unsigned long id)
{
	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	str_cat(pestreams[id].filepath, uni(".ape"));
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

	if(pestreams[id].smode == fennec_encoder_file)
	{
		if(fcreatemode == fennec_encode_openmode_createnew)
		{
			

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
		pestreams[id].aenc->Finish(NULL, 0, 0);
		SAFE_DELETE(pestreams[id].aenc)

		pestreams[id].firstwrite = 1;
	}

	return 1;
}

int encoder_set(unsigned long id)
{	

	return 1;
}

int encoder_write(unsigned long id, void* rbuffer, unsigned long bsize)
{	
	int             rv;
	unsigned long   i, scount;
	short          *shortbuf;
	fennec_sample  *fsbuf = (fennec_sample*) rbuffer;

	if(pestreams[id].firstwrite)
	{
		int  clevel = COMPRESSION_LEVEL_NORMAL, iclevel;

		pestreams[id].aenc = CreateIAPECompress();

		FillWaveFormatEx(&pestreams[id].wfx, pestreams[id].cfrequency, 16, pestreams[id].cchannels);

		
		fsettings.plugin_settings_getnum("monkey", "compression", &iclevel, 0, 0);

		switch(iclevel)
		{
		case 0:  clevel = COMPRESSION_LEVEL_FAST;        break;
		case 1:  clevel = COMPRESSION_LEVEL_NORMAL;      break;
		case 2:  clevel = COMPRESSION_LEVEL_HIGH;        break;
		case 3:  clevel = COMPRESSION_LEVEL_EXTRA_HIGH;  break;
		case 4:  clevel = COMPRESSION_LEVEL_INSANE;      break;

		default: clevel = COMPRESSION_LEVEL_NORMAL; break;
		}

		rv = pestreams[id].aenc->Start(pestreams[id].filepath, &pestreams[id].wfx, MAX_AUDIO_BYTES_UNKNOWN, COMPRESSION_LEVEL_HIGH, NULL, CREATE_WAV_HEADER_ON_DECOMPRESSION);

		if(rv != 0)
		{
			SAFE_DELETE(pestreams[id].aenc)
			/* error */
			return 0;
		}

		pestreams[id].firstwrite = 0;
	}

	scount = bsize / sizeof(fennec_sample);

	shortbuf = (short*) sys_mem_alloc(scount * sizeof(short));

	for(i=0; i<scount; i++) shortbuf[i] = (short)(fsbuf[i] * 32767.0);

	pestreams[id].aenc->AddData((unsigned char*)shortbuf, scount * sizeof(short) );

	sys_mem_free(shortbuf);
	return 1;
}


/* --------------------------------------------------------------------------*/


int proc_settings_fileencoder(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{
	case WM_INITDIALOG:

		/*
		COMPRESSION_LEVEL_FAST
		COMPRESSION_LEVEL_NORMAL
		COMPRESSION_LEVEL_HIGH
		COMPRESSION_LEVEL_EXTRA_HIGH
		COMPRESSION_LEVEL_INSANE
		*/

		SendDlgItemMessage(hwnd, IDC_TYPE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("Fast"));
		SendDlgItemMessage(hwnd, IDC_TYPE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("Normal"));
		SendDlgItemMessage(hwnd, IDC_TYPE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("High"));
		SendDlgItemMessage(hwnd, IDC_TYPE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("Extra High"));
		SendDlgItemMessage(hwnd, IDC_TYPE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("Insane"));
		
		{
			int idx = 1;
			fsettings.plugin_settings_getnum("monkey", "compression", &idx, 0, 0);
			SendDlgItemMessage(hwnd, IDC_TYPE, CB_SETCURSEL, (WPARAM)idx, 0);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:

			{
				int idx = 1;

				idx = (int)SendDlgItemMessage(hwnd, IDC_TYPE, CB_GETCURSEL, 0, 0);
				
				fsettings.plugin_settings_setnum("monkey", "compression", idx);
			}

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

