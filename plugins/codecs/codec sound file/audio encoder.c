/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (Sound Files).
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
#include <commctrl.h>

HINSTANCE hInst;


unsigned int setting_type     = 1; /* wave */
unsigned int setting_bps      = 0; /* default */
unsigned int setting_channels = 0; /* default */
unsigned int setting_rate     = 0;
unsigned int setting_resample = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, unsigned long fdwReason, LPVOID lpReserved)
{
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
			hInst = hinstDLL;
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

const string get_ext(unsigned int i)
{
	switch(i)
	{
	case 0:  return uni("aif");
	case 1:	 return uni("wav");
	case 2:  return uni("au");
	case 3:	 return uni("caf");
	case 4:	 return uni("snd");
	case 5:	 return uni("svx");
	case 6:	 return uni("paf");
	case 7:	 return uni("fap");
	case 8:	 return uni("gsm");
	case 9:	 return uni("nist");
	case 10: return uni("ircam");
	case 11: return uni("sf");
	case 12: return uni("voc");
	case 13: return uni("w64");
	case 14: return uni("raw");
	case 15: return uni("mat4");
	case 16: return uni("mat5");
	case 17: return uni("mat");
	case 18: return uni("pvf");
	case 19: return uni("sds");
	case 20: return uni("sd2");
	case 21: return uni("vox");
	case 22: return uni("xi");
					
	}

	return uni("wav");
}

int get_sf_format(unsigned int i)
{
	switch(i)
	{
	case 0:  return SF_FORMAT_AIFF		                   ; /* Apple/SGI AIFF format (aif) */
	case 1:	 return SF_FORMAT_WAV		                   ; /* Microsoft WAV format (wav) */
	case 2:  return SF_FORMAT_AU		                   ; /* Sun/NeXT AU format (au) */
	case 3:	 return SF_FORMAT_CAF		                   ; /* Core Audio File format (caf) */
	case 4:	 return SF_FORMAT_AU		                   ; /* Sun/NeXT AU format (snd) */
	case 5:	 return SF_FORMAT_SVX		                   ; /* Amiga IFF / SVX8 / SV16 format (svx) */
	case 6:	 return SF_ENDIAN_BIG    | SF_FORMAT_PAF	   ; /* Ensoniq PARIS file format (paf) */
	case 7:	 return SF_ENDIAN_LITTLE | SF_FORMAT_PAF	   ; /* Ensoniq PARIS file format (fap) */
	case 8:	 return SF_FORMAT_WAV    | SF_FORMAT_GSM610    ; /* RAW PCM data (gsm) */
	case 9:	 return SF_FORMAT_NIST	                       ; /* Sphere NIST format (nist) */
	case 10: return SF_FORMAT_IRCAM	                       ; /* Berkeley/IRCAM/CARL (ircam) */
	case 11: return SF_FORMAT_IRCAM	                       ; /* Berkeley/IRCAM/CARL (sf) */
	case 12: return SF_FORMAT_VOC	                       ; /* VOC files (voc) */
	case 13: return SF_FORMAT_W64	                       ; /* Sonic Foundry's 64 bit RIFF/WAV (w64) */
	case 14: return SF_FORMAT_RAW	                       ; /* RAW PCM data (raw) */
	case 15: return SF_FORMAT_MAT4	                       ; /* Matlab (tm) V4.2 / GNU Octave 2.0 (mat4) */
	case 16: return SF_FORMAT_MAT5 	                       ; /* Matlab (tm) V5.0 / GNU Octave 2.1 (mat5) */
	case 17: return SF_FORMAT_MAT4 	                       ; /* Matlab (tm) V4.2 / GNU Octave 2.0 (mat) */
	case 18: return SF_FORMAT_PVF 	                       ; /* Portable Voice Format (pvf) */
	case 19: return SF_FORMAT_SDS 	                       ; /* Midi Sample Dump Standard (sds) */
	case 20: return SF_FORMAT_SD2 	                       ; /* Sound Designer 2 (sd2) */
	case 21: return SF_FORMAT_RAW    | SF_FORMAT_VOX_ADPCM ; /* RAW PCM data (vox) */
	case 22: return SF_FORMAT_XI 	                       ; /* Fasttracker 2 Extended Instrument (xi) */

	}
	return SF_FORMAT_WAV;
}

int encoder_setdefaults(unsigned long id)
{
	//setting_type     = 1; /* wave */
	//setting_bps      = 0; /* default */
	//setting_channels = 0; /* default */
	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	str_cat(pestreams[id].filepath, uni("."));
	str_cat(pestreams[id].filepath, get_ext(setting_type));
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
		src_delete(pestreams[id].src_state);
		sf_close(pestreams[id].sndfile);
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

	if(pestreams[id].firstwrite)
	{
		int fmt, subformat, nchan = pestreams[id].cchannels;
		int src_error;

		pestreams[id].sfileinfo.format     = get_sf_format(setting_type);

		if(setting_bps == 0) /* default */
		{
			switch(setting_type)
			{
			case 8:  break;
			case 21: break;
			default:
				pestreams[id].sfileinfo.format	|=  SF_FORMAT_PCM_16;
				break;
			}
		}else{
			switch(setting_type)
			{
			case 8:  break;
			case 21: break;
			default:
				if(setting_bps == 1) /* 8-bit unsigned */
					pestreams[id].sfileinfo.format	|=  SF_FORMAT_PCM_U8;
				else
					pestreams[id].sfileinfo.format	|=  SF_FORMAT_PCM_16;
				break;
			}
		}	

		fmt  = pestreams[id].sfileinfo.format & SF_FORMAT_TYPEMASK;
		subformat = pestreams[id].sfileinfo.format & SF_FORMAT_SUBMASK;

		if(setting_channels == 0)
		{
			pestreams[id].sfileinfo.channels = nchan;
		}else if(setting_channels == 1){ /* stereo */
			pestreams[id].sfileinfo.channels = 2;
		}else if(setting_channels == 2){
			pestreams[id].sfileinfo.channels = 1;
		}
		
		switch(fmt)
		{
		
		case SF_FORMAT_WAV :
		case SF_FORMAT_WAVEX :
				if (subformat == SF_FORMAT_GSM610)
					pestreams[id].sfileinfo.channels = 1;
				if (subformat == SF_FORMAT_G721_32)
					pestreams[id].sfileinfo.channels = 1;
				break;


		case SF_FORMAT_AIFF :
				if ((subformat == SF_FORMAT_DWVW_12 || subformat == SF_FORMAT_DWVW_16 || subformat == SF_FORMAT_DWVW_24))
					pestreams[id].sfileinfo.channels = 1;
				if (subformat == SF_FORMAT_GSM610)
					pestreams[id].sfileinfo.channels = 1;
				if (subformat == SF_FORMAT_IMA_ADPCM)
					pestreams[id].sfileinfo.channels = min(nchan, 2);
				break ;

		case SF_FORMAT_AU :
				if (subformat == SF_FORMAT_G721_32)
				if (subformat == SF_FORMAT_G723_24)
				if (subformat == SF_FORMAT_G723_40)
					pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_RAW :

				if ((subformat == SF_FORMAT_DWVW_12 || subformat == SF_FORMAT_DWVW_16 || subformat == SF_FORMAT_DWVW_24))
					pestreams[id].sfileinfo.channels = 1;
				if (subformat == SF_FORMAT_GSM610)
					pestreams[id].sfileinfo.channels = 1;
				if (subformat == SF_FORMAT_VOX_ADPCM)
					pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_SVX :
				pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_W64 :
				if ((subformat == SF_FORMAT_IMA_ADPCM || subformat == SF_FORMAT_MS_ADPCM))
					pestreams[id].sfileinfo.channels = min(nchan, 2);
				if (subformat == SF_FORMAT_GSM610)
					pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_XI :
				pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_HTK :
				pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_SDS :
				pestreams[id].sfileinfo.channels = 1;
				break ;

		case SF_FORMAT_AVR :
				pestreams[id].sfileinfo.channels = min(nchan, 2);
				break ;

		default:
			pestreams[id].sfileinfo.channels = nchan;
			break;
		
		}

		/* SRC sample rate conversion */

			pestreams[id].src_converter = SRC_SINC_BEST_QUALITY;

			pestreams[id].src_state = src_new(pestreams[id].src_converter, pestreams[id].sfileinfo.channels, &src_error);

			pestreams[id].src_data.end_of_input = 0 ; /* Set this later. */

			pestreams[id].src_data.src_ratio = (double)setting_rate / (double)pestreams[id].cfrequency;

			if(setting_rate > 500 && setting_rate < 400000)
				pestreams[id].set_rate = setting_resample;
			else
				pestreams[id].set_rate = 0;

			if(!pestreams[id].set_rate)pestreams[id].src_data.src_ratio = 1.0;

		/* </sample rate conversion> */
		
		if(pestreams[id].set_rate)
			pestreams[id].sfileinfo.samplerate = (int)(((double)pestreams[id].cfrequency) * pestreams[id].src_data.src_ratio);
		else
			pestreams[id].sfileinfo.samplerate = pestreams[id].cfrequency;

		{
			char  afname[v_sys_maxpath];
			BOOL  useddef = 1;

			WideCharToMultiByte(CP_ACP, 0, pestreams[id].filepath, -1, afname, sizeof(afname), "?", &useddef);

			pestreams[id].sndfile = sf_open(afname, SFM_WRITE, &pestreams[id].sfileinfo);
		}

		pestreams[id].firstwrite = 0;
	
	}


	if(pestreams[id].sfileinfo.channels == pestreams[id].cchannels)
	{
		if(pestreams[id].set_rate)
		{
			fennec_sample  *ibuffer = (fennec_sample*)rbuffer;
			float          *fibuffer, *fobuffer;
			unsigned long   i;
			int             fsize = bsize / (pestreams[id].cbitspersample / 8) / pestreams[id].cchannels;
			int             fosize = ((int)(((double)fsize) * pestreams[id].src_data.src_ratio)) + 1;

			fibuffer = sys_mem_alloc(fsize * pestreams[id].cchannels * sizeof(float));
			fobuffer = sys_mem_alloc(fosize * pestreams[id].cchannels * sizeof(float));

			for(i=0; i<(fsize * pestreams[id].cchannels); i++)
			{
				fibuffer[i] = (float)ibuffer[i];
			}

			pestreams[id].src_data.data_in       = fibuffer;
			pestreams[id].src_data.data_out      = fobuffer;
			pestreams[id].src_data.input_frames  = fsize;
			pestreams[id].src_data.output_frames = fosize;

			src_process(pestreams[id].src_state, &pestreams[id].src_data);

			for(i=0; i<(fosize / sizeof(float)); i++)
			{
				if     (fobuffer[i] >  1.0) fobuffer[i] =  1.0;
				else if(fobuffer[i] < -1.0) fobuffer[i] = -1.0;
			}

			sf_writef_float(pestreams[id].sndfile, fobuffer, pestreams[id].src_data.output_frames_gen);

			sys_mem_free(fibuffer);
			sys_mem_free(fobuffer);

		}else{
			sf_writef_double(pestreams[id].sndfile, rbuffer, bsize / sizeof(fennec_sample) / pestreams[id].sfileinfo.channels);
		}
	}else{

		fennec_sample *obuffer = 0;
		unsigned long  osize, frames = bsize / (pestreams[id].cbitspersample / 8) / pestreams[id].cchannels;;

		if(pestreams[id].sfileinfo.channels < (int)pestreams[id].cchannels)
		{
			/* downmix */

			fennec_sample  *ibuffer = (fennec_sample*)rbuffer;
			unsigned long   i, j, k;
			int             fsize = bsize / (pestreams[id].cbitspersample / 8) / pestreams[id].cchannels;

			osize = fsize * pestreams[id].sfileinfo.channels;

			obuffer = (fennec_sample*) sys_mem_alloc(osize * sizeof(fennec_sample));

			for(i=0, j=0; i<frames * pestreams[id].cchannels; i+=pestreams[id].cchannels, j+=pestreams[id].sfileinfo.channels)
			{
				for(k=0; k<(unsigned long)pestreams[id].sfileinfo.channels; k++)
				{
					obuffer[j + k] = ibuffer[i + k];
				}
			}
			
		}else{

			fennec_sample  *ibuffer = (fennec_sample*)rbuffer;
			unsigned long   i, j;
			int             fsize = bsize / (pestreams[id].cbitspersample / 8) / pestreams[id].cchannels;

			osize = fsize * pestreams[id].sfileinfo.channels;

			obuffer = (fennec_sample*) sys_mem_alloc(osize * sizeof(fennec_sample));

			for(i=0; i<fsize * pestreams[id].cchannels; i++)
			{
				for(j=0; j<(unsigned long)pestreams[id].sfileinfo.channels; j++)
				{
					obuffer[(i * pestreams[id].sfileinfo.channels) + j] = ibuffer[i];
				}
			}
		}

		if(pestreams[id].set_rate)
		{
			fennec_sample  *ibuffer = (fennec_sample*)obuffer;
			float          *fibuffer, *fobuffer;
			unsigned long   i;
			int             fsize = frames * pestreams[id].sfileinfo.channels;
			int             fosize = ((int)(((double)fsize) * pestreams[id].src_data.src_ratio));

			fibuffer = sys_mem_alloc(fsize * sizeof(float));
			fobuffer = sys_mem_alloc(fosize * sizeof(float) * pestreams[id].sfileinfo.channels);

			for(i=0; i<(unsigned long)fsize; i++)
			{
				fibuffer[i] = (float)ibuffer[i];
			}

			pestreams[id].src_data.data_in       = fibuffer;
			pestreams[id].src_data.data_out      = fobuffer;
			pestreams[id].src_data.input_frames  = frames;
			pestreams[id].src_data.output_frames = fosize;

			src_process(pestreams[id].src_state, &pestreams[id].src_data);

			for(i=0; i<(fosize / sizeof(float)); i++)
			{
				if     (fobuffer[i] >  1.0) fobuffer[i] =  1.0;
				else if(fobuffer[i] < -1.0) fobuffer[i] = -1.0;
			}

			sf_writef_float(pestreams[id].sndfile, fobuffer, pestreams[id].src_data.output_frames_gen);

			sys_mem_free(fibuffer);
			sys_mem_free(fobuffer);
		}else{
			sf_writef_double(pestreams[id].sndfile, obuffer, frames);
		}

		if(obuffer)
			sys_mem_free(obuffer);

	}
	return 1;
}

int proc_settings_fileencoder(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{

#define addit(h, a)(SendDlgItemMessage(hwnd, h, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)a))
#define addit_type(a)(SendDlgItemMessage(hwnd, IDC_TYPE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)a))
#define selit(h, n)(SendDlgItemMessage(hwnd, h, CB_SETCURSEL, (WPARAM)n, 0))
#define getsel(h)(SendDlgItemMessage(hwnd, h, CB_GETCURSEL, 0, 0))

	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			
		setting_type     = (unsigned int)getsel(IDC_TYPE);
		setting_bps      = (unsigned int)getsel(IDC_BPS);
		setting_channels = (unsigned int)getsel(IDC_CHAN);
		
		if(IsDlgButtonChecked(hwnd, check_resample) == BST_CHECKED)
			setting_resample = 1;
		else
			setting_resample = 0;

		{
			BOOL trans = 1;
			setting_rate = GetDlgItemInt(hwnd, combo_rate, &trans, 0);
		}

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
		addit_type(uni("Apple/SGI AIFF format (aif)"));
		addit_type(uni("Microsoft WAV format (wav)"));
		addit_type(uni("Sun/NeXT AU format (au)"));
		addit_type(uni("Core Audio File format (caf)"));
		addit_type(uni("Sun/NeXT AU format (snd)"));
		addit_type(uni("Amiga IFF / SVX8 / SV16 format (svx)"));
		addit_type(uni("Ensoniq PARIS file format (paf)"));
		addit_type(uni("Ensoniq PARIS file format (fap)"));
		addit_type(uni("RAW PCM data (gsm)"));
		addit_type(uni("Sphere NIST format (nist)"));
		addit_type(uni("Berkeley/IRCAM/CARL (ircam)"));
		addit_type(uni("Berkeley/IRCAM/CARL (sf)"));
		addit_type(uni("VOC files (voc)"));
		addit_type(uni("Sonic Foundry's 64 bit RIFF/WAV (w64)"));
		addit_type(uni("RAW PCM data (raw)"));
		addit_type(uni("Matlab (tm) V4.2 / GNU Octave 2.0 (mat4)"));
		addit_type(uni("Matlab (tm) V5.0 / GNU Octave 2.1 (mat5)"));
		addit_type(uni("Matlab (tm) V4.2 / GNU Octave 2.0 (mat)"));
		addit_type(uni("Portable Voice Format (pvf)"));
		addit_type(uni("Midi Sample Dump Standard (sds)"));
		addit_type(uni("Sound Designer 2 (sd2)"));
		addit_type(uni("RAW PCM data (vox)"));
		addit_type(uni("Fasttracker 2 Extended Instrument (xi)"));

		addit(IDC_CHAN, uni("Default"));
		addit(IDC_CHAN, uni("Stereo"));
		addit(IDC_CHAN, uni("Mono"));

		addit(IDC_BPS, uni("Default"));
		addit(IDC_BPS, uni("Unsigned 8 Bit"));
		addit(IDC_BPS, uni("Signed 16 Bit"));

		addit(combo_rate, uni("8000Hz"));
		addit(combo_rate, uni("11025Hz)"));
		addit(combo_rate, uni("22050Hz"));
		addit(combo_rate, uni("44100Hz"));
		addit(combo_rate, uni("48000Hz"));
		addit(combo_rate, uni("88200Hz"));
		addit(combo_rate, uni("96000Hz"));
		addit(combo_rate, uni("192000Hz"));
		
		selit(IDC_TYPE, setting_type);
		selit(IDC_BPS,  setting_bps);
		selit(IDC_CHAN, setting_channels);
	
		SetDlgItemInt(hwnd, combo_rate, setting_rate, 0);

		CheckDlgButton(hwnd, check_resample, setting_resample ? BST_CHECKED : BST_UNCHECKED);

		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
