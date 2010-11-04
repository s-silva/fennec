#include "main.h"

HINSTANCE dll_instance;

int WINAPI DllMain(HINSTANCE hinst, DWORD dwreason, void* dreserved)
{
    if(dwreason == DLL_PROCESS_ATTACH)
	{
		dll_instance = hinst;
    }

    return 1;
}

int encoder_setdefaults(unsigned long id)
{
	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	str_cat(pstreams[id].filepath, uni(".mp3"));
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
	if(pstreams[id].smode == fennec_encoder_file)
	{
		if(fcreatemode == fennec_encode_openmode_createnew)
		{
			pstreams[id].fhandle = sys_file_create(pstreams[id].filepath, v_sys_file_forwrite);
			if(pstreams[id].fhandle == v_error_sys_file_create)return 0;

		}else if(fcreatemode == fennec_encode_openmode_create){

			pstreams[id].fhandle = sys_file_createforcestream(pstreams[id].filepath, v_sys_file_forwrite);
			if(pstreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seteof(pstreams[id].fhandle);

		}else if(fcreatemode == fennec_encode_openmode_append){

			pstreams[id].fhandle = sys_file_createforce(pstreams[id].filepath, v_sys_file_forwrite);
			if(pstreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seek(pstreams[id].fhandle, sys_file_getsize(pstreams[id].fhandle));

		}else{

			/* unknown mode! */
			return 0;
		}
	}

	
			
	pstreams[id].outdata = 0;
	return 1;
}


int encoder_uninitialize(unsigned long id)
{

	return 1;
}

int encoder_set(unsigned long id)
{	


	return 1;
}

int encoder_write(unsigned long id, double* buffer, unsigned long bsize)
{
	float  *sbuffer[2];
	int     rsize;
	int     samplesize, scount, i; 

	samplesize = (pstreams[id].cbitspersample / 8) * pstreams[id].cchannels;
	scount = (bsize / samplesize);


	sbuffer[0] = (float*) sys_mem_alloc(scount * sizeof(float) * 2);
	sbuffer[1] = sbuffer[0] + scount;

	
	if(!pstreams[id].outdata)
		pstreams[id].outdata = (char*) sys_mem_alloc(bsize * 2);

	/* decode from PCM */

	if(pstreams[id].cchannels > 1)
	{
		for(i=0; i<scount; i++)
		{
			sbuffer[0][i] = (float) (buffer[ i * pstreams[id].cchannels     ] * 32767.0);
			sbuffer[1][i] = (float) (buffer[(i * pstreams[id].cchannels) + 1] * 32767.0);
		}

	}else{

		for(i=0; i<scount; i++)
			sbuffer[0][i] = sbuffer[1][i] = (float)(buffer[i] * 32767.0);
	}

	/* encode */



	rsize = 0;//lame_encode_buffer_float(pstreams[id].lameflags, sbuffer[0], sbuffer[1], bsize / samplesize, (unsigned char*)pstreams[id].outdata, bsize * 2);

	if(rsize > 0)
	{
		sys_file_write(pstreams[id].fhandle, pstreams[id].outdata, rsize);
	}


	/* free all */

	sys_mem_free(sbuffer[0]);

	return 1;
}

int proc_settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define getsel(c)(int)SendDlgItemMessage(hwnd, c, CB_GETCURSEL, 0, 0);
#define setsel(c, x)SendDlgItemMessage(hwnd, c, CB_SETCURSEL, (WPARAM)x, 0);

	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int csel;

				csel = getsel(combo_bitrate_1);
				if(csel >= 0)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg1.bitrate", csel);
				
				csel = getsel(combo_freq_1);
				if(csel >= 0)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg1.frequency", csel);
				
				csel = getsel(combo_bitrate_2);
				if(csel >= 0)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg2.bitrate", csel);
				
				csel = getsel(combo_freq_2);
				if(csel >= 0)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg2.frequency", csel);
				
				csel = getsel(combo_channels);
				if(csel >= 0)
					fsettings.plugin_settings_setnum("mp3-lame", "channelmode", csel);
			
				if(IsDlgButtonChecked(hwnd, check_vbr1) == BST_CHECKED)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg1.vbr?", 1);
				else
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg1.vbr?", 0);
				
				if(IsDlgButtonChecked(hwnd, check_vbr2) == BST_CHECKED)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg2.vbr?", 1);
				else
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg2.vbr?", 0);

				if(IsDlgButtonChecked(hwnd, option_1) == BST_CHECKED)
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg1?", 1);
				else
					fsettings.plugin_settings_setnum("mp3-lame", "mpeg1?", 0);

				csel = getsel(combo_quality);
				if(csel >= 0)
					fsettings.plugin_settings_setnum("mp3-lame", "quality", csel);

			}

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int i;
			int csel = 0;

			string bindex_mpeg1[] = {uni("32 Kbps"), uni("40 Kbps"),uni("48 Kbps"), uni("56 Kbps"), uni("64 Kbps"), uni("80 Kbps"), uni("96 Kbps"), uni("112 Kbps"), uni("128 Kbps"), uni("160 Kbps"), uni("192 Kbps"), uni("224 Kbps"), uni("256 Kbps"), uni("320 Kbps")};
			string findex_mpeg1[] = {uni("Default"), uni("32000 Hz"), uni("44100 Hz"), uni("48000 Hz")};
			string bindex_mpeg2[] = {uni("8 Kbps"), uni("16 Kbps"), uni("24 Kbps"), uni("32 Kbps"), uni("40 Kbps"), uni("48 Kbps"), uni("56 Kbps"), uni("64 Kbps"), uni("80 Kbps"), uni("96 Kbps"), uni("112 Kbps"), uni("128 Kbps"), uni("144 Kbps"), uni("160 Kbps")};
			string findex_mpeg2[] = {uni("Default"), uni("8000 Hz"), uni("11025 Hz"), uni("12000 Hz"), uni("16000 Hz"), uni("22050 Hz"), uni("24000 Hz")};
			string cindex[]       = {uni("Same as the source"), uni("Stereo"), uni("Joint Stereo"), uni("Forced Joint Stereo"), uni("Duel Channel"), uni("Mono")};
			string qindex[]       = {uni("Normal"), uni("10%"), uni("20%"), uni("30%"), uni("40%"), uni("50%"), uni("60%"), uni("70%"), uni("80%"), uni("90%"), uni("100%")};

			for(i=0; i<sizeof(bindex_mpeg1)/sizeof(string); i++)
			{
				SendDlgItemMessage(hwnd, combo_bitrate_1, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)bindex_mpeg1[i]);
			}
			
			for(i=0; i<sizeof(findex_mpeg1)/sizeof(string); i++)
			{
				SendDlgItemMessage(hwnd, combo_freq_1, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)findex_mpeg1[i]);
			}
			
			for(i=0; i<sizeof(bindex_mpeg2)/sizeof(string); i++)
			{
				SendDlgItemMessage(hwnd, combo_bitrate_2, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)bindex_mpeg2[i]);
			}
			
			for(i=0; i<sizeof(findex_mpeg2)/sizeof(string); i++)
			{
				SendDlgItemMessage(hwnd, combo_freq_2, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)findex_mpeg2[i]);
			}

			for(i=0; i<sizeof(cindex)/sizeof(string); i++)
			{
				SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)cindex[i]);
			}

			for(i=0; i<sizeof(qindex)/sizeof(string); i++)
			{
				SendDlgItemMessage(hwnd, combo_quality, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)qindex[i]);
			}

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg1.bitrate", &csel, 0, 0))csel = 8;
			setsel(combo_bitrate_1, csel);

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg1.frequency", &csel, 0, 0))csel = 0;
			setsel(combo_freq_1, csel);

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg2.bitrate", &csel, 0, 0))csel = 7;
			setsel(combo_bitrate_2, csel);

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg2.frequency", &csel, 0, 0))csel = 0;
			setsel(combo_freq_2, csel);

			if(fsettings.plugin_settings_getnum("mp3-lame", "channelmode", &csel, 0, 0))csel = 0;
			setsel(combo_channels, csel);

			if(fsettings.plugin_settings_getnum("mp3-lame", "quality", &csel, 0, 0))csel = 0;
			setsel(combo_quality, csel);

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg1.vbr?", &csel, 0, 0))csel = 0;

			if(csel)
				CheckDlgButton(hwnd, check_vbr1, BST_CHECKED);
			else
				CheckDlgButton(hwnd, check_vbr1, BST_UNCHECKED);

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg2.vbr?", &csel, 0, 0))csel = 0;

			if(csel)
				CheckDlgButton(hwnd, check_vbr2, BST_CHECKED);
			else
				CheckDlgButton(hwnd, check_vbr2, BST_UNCHECKED);

			if(fsettings.plugin_settings_getnum("mp3-lame", "mpeg1?", &csel, 0, 0)) csel = 0;

			if(csel)
			{
				CheckDlgButton(hwnd, option_1, BST_CHECKED);
				CheckDlgButton(hwnd, option_2, BST_UNCHECKED);
			}else{
				CheckDlgButton(hwnd, option_2, BST_CHECKED);
				CheckDlgButton(hwnd, option_1, BST_UNCHECKED);
			}
		}
		break;
	}
	return 0;
}