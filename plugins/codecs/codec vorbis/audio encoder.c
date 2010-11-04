/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (Vorbis).
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

#define bitrate_mode_constant    1
#define bitrate_mode_variable    2

#define channel_mode_mono        1
#define channel_mode_stereo      2
#define channel_mode_source      3

int global_enc_bitrate_mode = bitrate_mode_variable;
int global_enc_bitrate = 128000;
int global_enc_quality = 7;
int global_enc_channel_mode = channel_mode_source;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
			hInst = hinstDLL;

    return TRUE;
}

int encoder_setdefaults(unsigned long id)
{
	int v = 0, c = 0;

	if(fsettings.plugin_settings_getnum("ogg-vorbis", "constant?"  , &c, 0, 0)) c = 1;

	if(c)
	{
		int brates[14] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
		
		if(fsettings.plugin_settings_getnum("ogg-vorbis", "bitrate-c", &v, 0, 0)) v = 8;

		if(v < 14)global_enc_bitrate = brates[v] * 1000;

	}else{

		int brates[14] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
		
		if(fsettings.plugin_settings_getnum("ogg-vorbis", "bitrate-v", &v, 0, 0)) v = 3;

		global_enc_bitrate_mode = bitrate_mode_variable;
		
		if(v < 14)global_enc_bitrate = brates[v] * 1000;

	}

	if(fsettings.plugin_settings_getnum("ogg-vorbis", "quality"  , &v, 0, 0)) v = 5;
	if(v >= 0 && v <= 10)
		global_enc_quality = v;

	if(fsettings.plugin_settings_getnum("ogg-vorbis", "channels" , &v, 0, 0)) v = 0;
		
	switch(v)
	{
	case 0: global_enc_channel_mode = channel_mode_source; break;
	case 1: global_enc_channel_mode = channel_mode_stereo; break;
	case 2: global_enc_channel_mode = channel_mode_mono;   break;
	default:global_enc_channel_mode = channel_mode_source; break;
	}

	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	str_cat(pestreams[id].filepath, uni(".ogg"));
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
	pestreams[id].g_eos = 1;
	pestreams[id].firstwrite = 1;

	if(pestreams[id].smode == fennec_encoder_file)
	{
		if(fcreatemode == fennec_encode_openmode_createnew)
		{
			pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seteof(pestreams[id].fhandle);

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
	if(!pestreams[id].initialized)return 0;

	sys_sleep(10);

	vorbis_analysis_wrote(&pestreams[id].vd, 0);

    /*
	vorbis does some data preanalysis, then divvies up blocks for
	more involved (potentially parallel) processing.  Get a single
	block for encoding now
	*/
    while(vorbis_analysis_blockout(&pestreams[id].vd, &pestreams[id].vb) == 1)
	{
		// analysis, assume we want to use bitrate management
		vorbis_analysis(&pestreams[id].vb, NULL);
		vorbis_bitrate_addblock(&pestreams[id].vb);

		while(vorbis_bitrate_flushpacket(&pestreams[id].vd, &pestreams[id].op))
		{
			// weld the packet into the bitstream
			ogg_stream_packetin(&pestreams[id].os, &pestreams[id].op);
	
			/* write out pages (if any) */
			while(!pestreams[id].g_eos)
			{
				int result = ogg_stream_pageout(&pestreams[id].os, &pestreams[id].og);
				if(result == 0)
					break;

	  			sys_file_write(pestreams[id].fhandle, pestreams[id].og.header, pestreams[id].og.header_len);
				sys_file_write(pestreams[id].fhandle, pestreams[id].og.body,   pestreams[id].og.body_len);
	
				// this could be set above, but for illustrative purposes, I do
				// it here (to show that vorbis does know where the stream ends)
				if(ogg_page_eos(&pestreams[id].og))
					pestreams[id].g_eos = 1;
			}
		}
	}
	
	// clean up and exit.  vorbis_info_clear() must be called last
	ogg_stream_clear    (&pestreams[id].os);
	vorbis_block_clear  (&pestreams[id].vb);
	vorbis_dsp_clear    (&pestreams[id].vd);
	vorbis_comment_clear(&pestreams[id].vc);
	vorbis_info_clear   (&pestreams[id].vi);
	sys_file_close(pestreams[id].fhandle);

	pestreams[id].initialized = 0;
	return 1;
}

int encoder_set(unsigned long id)
{	
	//lame_set_in_samplerate(pestreams[id].lameflags, pestreams[id].cfrequency);
	//lame_set_num_channels(pestreams[id].lameflags, pestreams[id].cchannels);
	return 1;
}

int encoder_write(unsigned long id, void* buffer, unsigned long bsize)
{	
	int i, vals, samplez, frames;	
	signed char *readbuffer = (signed char *)buffer;
	fennec_sample  *sbuffer = (fennec_sample*)buffer;


	if(pestreams[id].firstwrite)
	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;
		vorbis_info_init(&pestreams[id].vi);

		if(global_enc_channel_mode == channel_mode_mono)
		{
			if(global_enc_bitrate_mode == bitrate_mode_variable)
			{
				if(vorbis_encode_init_vbr(&pestreams[id].vi, 1, pestreams[id].cfrequency, ((float)global_enc_quality) / 10.0f))goto return_not_init;
			}else{
				if(vorbis_encode_init(&pestreams[id].vi, 1, pestreams[id].cfrequency, global_enc_bitrate, global_enc_bitrate, global_enc_bitrate))goto return_not_init;
			}
		
		}else if(global_enc_channel_mode == channel_mode_source){

			if(global_enc_bitrate_mode == bitrate_mode_variable)
			{
				if(vorbis_encode_init_vbr(&pestreams[id].vi, pestreams[id].cchannels, pestreams[id].cfrequency, ((float)global_enc_quality) / 10.0f))goto return_not_init;
			}else{
				if(vorbis_encode_init(&pestreams[id].vi, pestreams[id].cchannels, pestreams[id].cfrequency, global_enc_bitrate, global_enc_bitrate, global_enc_bitrate))goto return_not_init;
			}
		
		}else{ /* stereo */
			if(global_enc_bitrate_mode == bitrate_mode_variable)
			{
				if(vorbis_encode_init_vbr(&pestreams[id].vi, 2, pestreams[id].cfrequency, ((float)global_enc_quality) / 10.0f))goto return_not_init;
			}else{
				if(vorbis_encode_init(&pestreams[id].vi, 2, pestreams[id].cfrequency, global_enc_bitrate, global_enc_bitrate, global_enc_bitrate))goto return_not_init;
			}
		}
		
		vorbis_comment_init(&pestreams[id].vc);
		/* vorbis_comment_add_tag(&pestreams[id].vc, (char*)"ENCODER", "fennec"); */

		vorbis_analysis_init(&pestreams[id].vd, &pestreams[id].vi);
		vorbis_block_init(&pestreams[id].vd, &pestreams[id].vb);
	  
		ogg_stream_init(&pestreams[id].os, rand());

		vorbis_analysis_headerout(&pestreams[id].vd, &pestreams[id].vc, &header, &header_comm, &header_code);
		ogg_stream_packetin(&pestreams[id].os, &header); // automatically placed in its own page
		ogg_stream_packetin(&pestreams[id].os, &header_comm);
		ogg_stream_packetin(&pestreams[id].os, &header_code);

		pestreams[id].g_eos = 0;

		while(!pestreams[id].g_eos)
		{
			int result = ogg_stream_flush(&pestreams[id].os, &pestreams[id].og);
			if(result == 0)
				break;
			sys_file_write(pestreams[id].fhandle, pestreams[id].og.header, pestreams[id].og.header_len);
			sys_file_write(pestreams[id].fhandle, pestreams[id].og.body, pestreams[id].og.body_len);
		}

		pestreams[id].firstwrite = 0;
	}

	samplez = (pestreams[id].cchannels * (pestreams[id].cbitspersample / 8));
	vals = bsize / samplez;

	pestreams[id].outbuffer = vorbis_analysis_buffer(&pestreams[id].vd, vals);

	frames = bsize / (pestreams[id].cbitspersample / 8);


	if(global_enc_channel_mode == channel_mode_source)
	{
		for(i=0; i<frames;  i++) /* pc */
		{
			pestreams[id].outbuffer[i % pestreams[id].cchannels][i / pestreams[id].cchannels] = (float)sbuffer[i];
		}
	}else if(global_enc_channel_mode == channel_mode_mono){

		for(i=0; i<frames;  i+=pestreams[id].cchannels)
		{
			pestreams[id].outbuffer[0][i / pestreams[id].cchannels] = (float)sbuffer[i];
		}
	}else if(global_enc_channel_mode == channel_mode_stereo){

		for(i=0; i<frames;  i+=pestreams[id].cchannels)
		{
			pestreams[id].outbuffer[0][i / pestreams[id].cchannels] = (float)sbuffer[i];

			if(pestreams[id].cchannels >= 2) /* downmix */
				pestreams[id].outbuffer[1][i / pestreams[id].cchannels] = (float)sbuffer[i + 1];
			else /* simulate */
				pestreams[id].outbuffer[1][i / pestreams[id].cchannels] = (float)sbuffer[i];
		}
	}


	vorbis_analysis_wrote(&pestreams[id].vd, i / pestreams[id].cchannels);

    while(vorbis_analysis_blockout(&pestreams[id].vd, &pestreams[id].vb) == 1)
	{
		vorbis_analysis(&pestreams[id].vb, 0);
		vorbis_bitrate_addblock(&pestreams[id].vb);

		while(vorbis_bitrate_flushpacket(&pestreams[id].vd, &pestreams[id].op))
		{
			ogg_stream_packetin(&pestreams[id].os, &pestreams[id].op);

			while(!pestreams[id].g_eos)
			{
				int result = ogg_stream_pageout(&pestreams[id].os, &pestreams[id].og);
				if(result == 0)
					break;

				sys_file_write(pestreams[id].fhandle, pestreams[id].og.header, pestreams[id].og.header_len);
				sys_file_write(pestreams[id].fhandle, pestreams[id].og.body,   pestreams[id].og.body_len);

				if(ogg_page_eos(&pestreams[id].og))
					pestreams[id].g_eos = 1;					
			}
		}
	}

	return 1;

return_not_init:
	pestreams[id].initialized = 0;
	return 0;
}

int proc_settings_fileencoder(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int k, v = 0;

				fsettings.plugin_settings_setnum("ogg-vorbis", "bitrate-c", (int)SendDlgItemMessage(hwnd, IDC_BITRATE_C, CB_GETCURSEL, 0, 0));
				fsettings.plugin_settings_setnum("ogg-vorbis", "bitrate-v", (int)SendDlgItemMessage(hwnd, IDC_BITRATE_V, CB_GETCURSEL, 0, 0));
				fsettings.plugin_settings_setnum("ogg-vorbis", "channels",  (int)SendDlgItemMessage(hwnd, IDC_CHANNELS, CB_GETCURSEL, 0, 0));
				fsettings.plugin_settings_setnum("ogg-vorbis", "quality",   (int)SendDlgItemMessage(hwnd, IDC_SQUALITY, TBM_GETPOS, 0, 0));

				if(IsDlgButtonChecked(hwnd, IDC_BITRATE_SC) == BST_CHECKED)
				{
					int brates[14] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
					
					fsettings.plugin_settings_setnum("ogg-vorbis", "constant?", 1);

					global_enc_bitrate_mode = bitrate_mode_constant;
					k = (int)SendDlgItemMessage(hwnd, IDC_BITRATE_C, CB_GETCURSEL, 0, 0);

					if(k < 14)global_enc_bitrate = brates[k] * 1000;
				}else{
					int brates[14] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
					
					fsettings.plugin_settings_setnum("ogg-vorbis", "constant?", 0);

					global_enc_bitrate_mode = bitrate_mode_variable;
					k = (int)SendDlgItemMessage(hwnd, IDC_BITRATE_V, CB_GETCURSEL, 0, 0);

					if(k < 14)global_enc_bitrate = brates[k] * 1000;

					
				}
				
				k = (int)SendDlgItemMessage(hwnd, IDC_SQUALITY, TBM_GETPOS, 0, 0);
				if(k >= 0 && k <= 10)
					global_enc_quality = k;

				k = (int)SendDlgItemMessage(hwnd, IDC_CHANNELS, CB_GETCURSEL, 0, 0);

				switch(k)
				{
				case 0: global_enc_channel_mode = channel_mode_source; break;
				case 1: global_enc_channel_mode = channel_mode_stereo; break;
				case 2: global_enc_channel_mode = channel_mode_mono;   break;
				default:global_enc_channel_mode = channel_mode_source; break;
				}

			}
		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			int   v = 0;
			string d_channels[3]      = {uni("Same as the source file"), uni("Stereo"), uni("Mono")};
			string d_bitrates_c_a[14] = {uni("32 Kbps"), uni("40 Kbps"), uni("48 Kbps"), uni("56 Kbps"), uni("64 Kbps"), uni("80 Kbps"), uni("96 Kbps"), uni("112 Kbps"), uni("128 Kbps"), uni("160 Kbps"), uni("192 Kbps"), uni("224 Kbps"), uni("256 Kbps"), uni("320 Kbps")};
			string d_bitrates_v[7]    = {uni("80 Kbps"), uni("96 Kbps"), uni("128 Kbps"), uni("192 Kbps"), uni("256 Kbps"), uni("320 Kbps"), uni("450 Kbps")};

			int i;

			for(i=0; i<14; i++)
			{
				SendDlgItemMessage(hwnd, IDC_BITRATE_C, CB_INSERTSTRING, (WPARAM)i, (LPARAM)d_bitrates_c_a[i]);
			}

			for(i=0; i<7; i++)SendDlgItemMessage(hwnd, IDC_BITRATE_V, CB_INSERTSTRING, (WPARAM)i, (LPARAM)d_bitrates_v[i]);
			for(i=0; i<3; i++)SendDlgItemMessage(hwnd, IDC_CHANNELS, CB_INSERTSTRING, (WPARAM)i, (LPARAM)d_channels[i]);
		
			if(fsettings.plugin_settings_getnum("ogg-vorbis", "bitrate-c", &v, 0, 0))v = 8;
			SendDlgItemMessage(hwnd, IDC_BITRATE_C, CB_SETCURSEL, (WPARAM)v, 0);

			if(fsettings.plugin_settings_getnum("ogg-vorbis", "bitrate-v", &v, 0, 0))v = 3;
			SendDlgItemMessage(hwnd, IDC_BITRATE_V, CB_SETCURSEL, (WPARAM)v, 0);

			if(fsettings.plugin_settings_getnum("ogg-vorbis", "channels", &v, 0, 0))v = 0;
			SendDlgItemMessage(hwnd, IDC_CHANNELS , CB_SETCURSEL, (WPARAM)v, 0);

			if(fsettings.plugin_settings_getnum("ogg-vorbis", "quality", &v, 0, 0))v = 5;

			SendDlgItemMessage(hwnd, IDC_SQUALITY , TBM_SETRANGE, (WPARAM)1, MAKELONG(0, 10));
			SendDlgItemMessage(hwnd, IDC_SQUALITY , TBM_SETPOS, (WPARAM)1, (LPARAM)v);

			if(fsettings.plugin_settings_getnum("ogg-vorbis", "constant?", &v, 0, 0))v = 1;

			if(v)CheckRadioButton(hwnd, IDC_BITRATE_SC, IDC_BITRATE_SA, IDC_BITRATE_SC);
			else CheckRadioButton(hwnd, IDC_BITRATE_SC, IDC_BITRATE_SA, IDC_BITRATE_SV);
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
