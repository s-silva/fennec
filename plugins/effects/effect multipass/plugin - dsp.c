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

double setting_highpass  = -1.0;
double setting_lowpass   = -1.0;
double setting_bandpass  = -1.0;
double setting_allpass   = -1.0;
double setting_notch     = -1.0;

double setting_log       = 0;



/* code ---------------------------------------------------------------------*/


unsigned long callc fennec_initialize_dsp(struct general_dsp_data *gdn, string pname)
{
	if(gdn->ptype != fennec_plugintype_audiodsp)return fennec_input_invalidtype;

	str_cpy(pname, uni("Multipass"));

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

		fsettings.plugin_settings_getnum("dsp-multipass", "highpass",  0, 0, &setting_highpass);
		fsettings.plugin_settings_getnum("dsp-multipass", "lowpass",   0, 0, &setting_lowpass);
		fsettings.plugin_settings_getnum("dsp-multipass", "bandpass",  0, 0, &setting_bandpass);
		fsettings.plugin_settings_getnum("dsp-multipass", "allpass",   0, 0, &setting_allpass);
		fsettings.plugin_settings_getnum("dsp-multipass", "log",       0, 0, &setting_log);


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
	MessageBox((HWND)pdata, "Multipass effect", "About", MB_ICONINFORMATION);
	return 1;
}


int callc plugin_dsp_settings(void *pdata)
{
	DialogBox(hinst, MAKEINTRESOURCE(IDD_SETTINGS), (HWND)pdata, (DLGPROC)DialogProc);
	return 1;
}


void user_changing(HWND hwnd)
{
	setting_highpass  = (double)(int)SendDlgItemMessage(hwnd, slider_highpass, TBM_GETPOS, 0, 0);
	setting_lowpass   = (double)(int)SendDlgItemMessage(hwnd, slider_lowpass,  TBM_GETPOS, 0, 0);
	setting_bandpass  = (double)(int)SendDlgItemMessage(hwnd, slider_bandpass, TBM_GETPOS, 0, 0);
	setting_allpass   = (double)(int)SendDlgItemMessage(hwnd, slider_allpass,  TBM_GETPOS, 0, 0);
	setting_notch     = (double)(int)SendDlgItemMessage(hwnd, slider_notch,    TBM_GETPOS, 0, 0);
	
	setting_highpass  /= 1000.0;
	setting_lowpass   /= 1000.0;
	setting_bandpass  /= 1000.0;
	setting_allpass   /= 1000.0;
	setting_notch     /= 1000.0;

	if(IsDlgButtonChecked(hwnd, check_highpass) != BST_CHECKED) setting_highpass = -fabs(setting_highpass);
	if(IsDlgButtonChecked(hwnd, check_lowpass)  != BST_CHECKED) setting_lowpass  = -fabs(setting_lowpass);
	if(IsDlgButtonChecked(hwnd, check_bandpass) != BST_CHECKED) setting_bandpass = -fabs(setting_bandpass);
	if(IsDlgButtonChecked(hwnd, check_allpass)  != BST_CHECKED) setting_allpass  = -fabs(setting_allpass);
	if(IsDlgButtonChecked(hwnd, check_notch)    != BST_CHECKED) setting_notch    = -fabs(setting_notch);


	fsettings.plugin_settings_setfloat("dsp-multipass", "highpass" ,setting_highpass  );
	fsettings.plugin_settings_setfloat("dsp-multipass", "lowpass"  ,setting_lowpass   );
	fsettings.plugin_settings_setfloat("dsp-multipass", "bandpass" ,setting_bandpass  );
	fsettings.plugin_settings_setfloat("dsp-multipass", "allpass"  ,setting_allpass   );
	fsettings.plugin_settings_setfloat("dsp-multipass", "notch"    ,setting_notch     );
	fsettings.plugin_settings_setfloat("dsp-multipass", "log"      ,setting_log       );

	effect_set(last_frequency, last_channels, effect_set_mode_low);
}

void setgui(double v, HWND hwnd, unsigned long mid, unsigned long sid)
{
	if(v < 0)
	{
		SendDlgItemMessage(hwnd, mid, TBM_SETPOS, (WPARAM)1, (LPARAM)(-v * 1000));
		CheckDlgButton(hwnd, sid, BST_UNCHECKED);
	}else{
		SendDlgItemMessage(hwnd, mid, TBM_SETPOS, (WPARAM)1, (LPARAM)(v * 1000));
		CheckDlgButton(hwnd, sid, BST_CHECKED);
	}
}

int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_HSCROLL:
		user_changing(hwnd);
		break;

	case WM_COMMAND:
		user_changing(hwnd);

		switch(LOWORD(wParam))
		{
		case IDOK:
			/* let it go down */

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, slider_highpass  , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 1000));
		SendDlgItemMessage(hwnd, slider_lowpass   , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 1000));
		SendDlgItemMessage(hwnd, slider_bandpass  , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 1000));
		SendDlgItemMessage(hwnd, slider_allpass   , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 1000));
		SendDlgItemMessage(hwnd, slider_notch     , TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 1000));


		setgui(setting_highpass , hwnd  , slider_highpass  , check_highpass);
		setgui(setting_lowpass  , hwnd  , slider_lowpass   , check_lowpass );
		setgui(setting_bandpass , hwnd  , slider_bandpass  , check_bandpass);
		setgui(setting_allpass  , hwnd  , slider_allpass   , check_allpass );
		setgui(setting_notch    , hwnd  , slider_notch     , check_notch   );

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
