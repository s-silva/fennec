/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Stereo Enhancer).
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

#include <math.h>
#include <windows.h>
#include <commctrl.h>
#include "data/systems/windows/resource.h"

#include "effect.h"




/* prototypes ---------------------------------------------------------------*/

int           callc plugin_dsp_initialize(void);
int           callc plugin_dsp_uninitialize(void);
unsigned long callc plugin_dsp_open(unsigned long id);
int           callc plugin_dsp_close(unsigned long id);
void*         callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr);
int           callc plugin_dsp_about(void *pdata);
int           callc plugin_dsp_settings(void *pdata);

int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);




/* data ---------------------------------------------------------------------*/

HINSTANCE               hinst;
struct plugin_settings  fsettings;


unsigned long   last_frequency = 0;
unsigned long   last_channels  = 0;

unsigned int    state_processing = 0;

double setting_highpass  = 0.0;
double setting_lowpass   = 100.0;
double setting_wet       = 80.0;
double setting_dry       = 30.0;
double setting_diffusion = 50.0;
double setting_delay     = 50.0;
double setting_phase     = 0.0;



/* code ---------------------------------------------------------------------*/


unsigned long callc fennec_initialize_dsp(struct general_dsp_data *gdn, string pname)
{
	if(gdn->ptype != fennec_plugintype_audiodsp)return fennec_input_invalidtype;

	str_cpy(pname, uni("Reverb"));

	if(gdn->fiversion >= plugin_version)
	{
		gdn->initialize    = plugin_dsp_initialize;
		gdn->uninitialize  = plugin_dsp_uninitialize;

		gdn->open          = plugin_dsp_open;
		gdn->process       = plugin_dsp_process;
		gdn->close         = plugin_dsp_close;

		gdn->settings      = plugin_dsp_settings;
		gdn->about         = plugin_dsp_about;

		gdn->messageproc   = 0;

		memcpy(&fsettings, &gdn->fsettings, sizeof(struct plugin_settings));

		fsettings.plugin_settings_getnum("dsp-reverb", "highpass",   0, 0, &setting_highpass  );
		fsettings.plugin_settings_getnum("dsp-reverb", "lowpass",    0, 0, &setting_lowpass   );
		fsettings.plugin_settings_getnum("dsp-reverb", "wet",        0, 0, &setting_wet       );
		fsettings.plugin_settings_getnum("dsp-reverb", "dry",        0, 0, &setting_dry       );
		fsettings.plugin_settings_getnum("dsp-reverb", "diffusion",  0, 0, &setting_diffusion );
		fsettings.plugin_settings_getnum("dsp-reverb", "delay",      0, 0, &setting_delay     );
		fsettings.plugin_settings_getnum("dsp-reverb", "phase",      0, 0, &setting_phase     );
			


		if(gdn->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gdn->pversion = plugin_version;
			return fennec_input_unknownversion;
		}
	}else{
		/* version 1 is supported, what? zero? oh no your fault */
		return fennec_input_invalidversion;
	}
}

int callc plugin_dsp_initialize(void)
{
	effect_initialize();
	return 1;
}

int callc plugin_dsp_uninitialize(void)
{
	return 1;
}

unsigned long callc plugin_dsp_open(unsigned long id)
{

	return 1;
}

int callc plugin_dsp_close(unsigned long id)
{
	return 1;
}

void* callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr)
{
	fennec_sample  *din;
	unsigned int    i, c = (*bsize) / (bitspersample / 8);

	din = (fennec_sample*) (((char*)sbuffer) + apointer);

	if(last_frequency != freqency || last_channels != channels)
	{
		effect_set(freqency, channels, effect_set_mode_high);
		last_frequency = freqency;
		last_channels  = channels;
	}

	for(i=0; i<c; i++)
	{
		din[i] = effect_process(din[i], freqency, i % channels);
	}

	return sbuffer;
}


int callc plugin_dsp_about(void *pdata)
{
	MessageBox((HWND)pdata, "Reverb effect", "About", MB_ICONINFORMATION);
	return 1;
}


int callc plugin_dsp_settings(void *pdata)
{
	DialogBox(hinst, MAKEINTRESOURCE(IDD_SETTINGS), (HWND)pdata, (DLGPROC)DialogProc);
	return 1;
}


void user_changing(HWND hwnd)
{
	int v;

	v = (int)SendDlgItemMessage(hwnd, IDC_VALUE, TBM_GETPOS, 0, 0);
	setting_highpass  = ((double)v);

	v = (int)SendDlgItemMessage(hwnd, IDC_BAND, TBM_GETPOS, 0, 0);
	setting_lowpass   = ((double)v);

	v = (int)SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_GETPOS, 0, 0);
	setting_wet = ((double)v);

	v = (int)SendDlgItemMessage(hwnd, slider_damp, TBM_GETPOS, 0, 0);
	setting_dry = ((double)v);

	v = (int)SendDlgItemMessage(hwnd, slider_diffusion, TBM_GETPOS, 0, 0);
	setting_diffusion = ((double)v);

	v = (int)SendDlgItemMessage(hwnd, slider_delay, TBM_GETPOS, 0, 0);
	setting_delay = ((double)v);

	v = (int)SendDlgItemMessage(hwnd, slider_phase, TBM_GETPOS, 0, 0);
	setting_phase = ((double)v);


	fsettings.plugin_settings_setfloat("dsp-reverb", "highpass"    ,setting_highpass  );
	fsettings.plugin_settings_setfloat("dsp-reverb", "lowpass"     ,setting_lowpass   );
	fsettings.plugin_settings_setfloat("dsp-reverb", "wet"         ,setting_wet       );
	fsettings.plugin_settings_setfloat("dsp-reverb", "dry"         ,setting_dry       );
	fsettings.plugin_settings_setfloat("dsp-reverb", "diffusion"   ,setting_diffusion );
	fsettings.plugin_settings_setfloat("dsp-reverb", "delay"       ,setting_delay     );
	fsettings.plugin_settings_setfloat("dsp-reverb", "phase"       ,setting_phase     );

	effect_set(last_frequency, last_channels, effect_set_mode_low);
}



int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_HSCROLL:
		user_changing(hwnd);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			user_changing(hwnd);
			/* let it go down */

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDC_VALUE        , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, IDC_BAND         , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, IDC_PREAMP       , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, slider_damp      , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, slider_diffusion , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, slider_delay     , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, slider_phase     , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		
		
		SendDlgItemMessage(hwnd, IDC_VALUE        , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_highpass));
		SendDlgItemMessage(hwnd, IDC_BAND         , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_lowpass));
		SendDlgItemMessage(hwnd, IDC_PREAMP       , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_wet));
		SendDlgItemMessage(hwnd, slider_damp      , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_dry));
		SendDlgItemMessage(hwnd, slider_diffusion , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_diffusion));
		SendDlgItemMessage(hwnd, slider_delay     , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_delay));
		SendDlgItemMessage(hwnd, slider_phase     , TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_phase));

		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)hinst = hinstDLL;
	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
