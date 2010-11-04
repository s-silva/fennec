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


double setting_band   = 1.0;  /* band (in Hz) */
double setting_boost  = 1.0;  /* boost (in dB) */
double setting_preamp = 1.0;  /* preamp (coefficient) */
double setting_shape  = 1.0;  /* IIR shape */
int    setting_normal = 1;

unsigned long   last_frequency = 0;
unsigned long   last_channels  = 0;

unsigned int    state_processing = 0;




/* code ---------------------------------------------------------------------*/


unsigned long callc fennec_initialize_dsp(struct general_dsp_data *gdn, string pname)
{
	if(gdn->ptype != fennec_plugintype_audiodsp)return fennec_input_invalidtype;

	str_cpy(pname, uni("Extra Bass"));

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

		if(fsettings.plugin_settings_getnum("dsp-extra bass", "gain", 0, 0,   &setting_boost ))setting_boost = 1.0;
		if(fsettings.plugin_settings_getnum("dsp-extra bass", "band", 0, 0,   &setting_band  ))setting_band = 120.0;
		if(fsettings.plugin_settings_getnum("dsp-extra bass", "preamp", 0, 0, &setting_preamp))setting_preamp = 1.0; /* no changes */
		if(fsettings.plugin_settings_getnum("dsp-extra bass", "shape", 0, 0,  &setting_shape ))setting_shape = 1.0;
		if(fsettings.plugin_settings_getnum("dsp-extra bass", "normalize", &setting_normal, 0, 0 ))setting_normal = 1;   /* normalization: on */

		if(gdn->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gdn->pversion = plugin_version;
			return fennec_input_unknownversion;
		}
	}else{;
		/* version 1 is supported, what? zero? oh no your fault */
		return fennec_input_invalidversion;
	}
}

int callc plugin_dsp_initialize(void)
{
	return 1;
}

int callc plugin_dsp_uninitialize(void)
{
	return 1;
}

unsigned long callc plugin_dsp_open(unsigned long id)
{
	if(setting_normal)
		setting_preamp = 1.0;
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
	MessageBox((HWND)pdata, "Extra bass effect", "About", MB_ICONINFORMATION);
	return 1;
}


int callc plugin_dsp_settings(void *pdata)
{
	DialogBox(hinst, MAKEINTRESOURCE(IDD_SETTINGS), (HWND)pdata, (DLGPROC)DialogProc);
	return 1;
}


int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_HSCROLL:
		{
			int v;

			v = (int)SendDlgItemMessage(hwnd, IDC_VALUE, TBM_GETPOS, 0, 0);
			setting_boost  = ((double)v);

			v = (int)SendDlgItemMessage(hwnd, IDC_BAND, TBM_GETPOS, 0, 0);
			setting_band   = ((double)v);

			v = (int)SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_GETPOS, 0, 0);
			setting_preamp = ((double)v) / 100.0;

			v = (int)SendDlgItemMessage(hwnd, IDC_SHAPE, TBM_GETPOS, 0, 0);
			setting_shape = ((double)v) / 100.0;
			

			effect_set(last_frequency, last_channels, effect_set_mode_low);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_NORMAL:
			if(IsDlgButtonChecked(hwnd, IDC_NORMAL) == BST_CHECKED)
				setting_normal = 1;
			else
				setting_normal = 0;
			break;

		case IDOK:
			{
				int v;

				v = (int)SendDlgItemMessage(hwnd, IDC_VALUE, TBM_GETPOS, 0, 0);
				setting_boost  = ((double)v);

				v = (int)SendDlgItemMessage(hwnd, IDC_BAND, TBM_GETPOS, 0, 0);
				setting_band   = ((double)v);

				v = (int)SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_GETPOS, 0, 0);
				setting_preamp = ((double)v) / 100.0;

				v = (int)SendDlgItemMessage(hwnd, IDC_SHAPE, TBM_GETPOS, 0, 0);
				setting_shape = ((double)v) / 100.0;
				
				fsettings.plugin_settings_setfloat("dsp-extra bass", "gain",   setting_boost);
				fsettings.plugin_settings_setfloat("dsp-extra bass", "band",   setting_band);
				fsettings.plugin_settings_setfloat("dsp-extra bass", "preamp", setting_preamp);
				fsettings.plugin_settings_setfloat("dsp-extra bass", "shape",  setting_shape);
			
				fsettings.plugin_settings_setnum("dsp-extra bass", "normalize", setting_normal);
			}
			/* let it go down */

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_TIMER:
		if(setting_normal)
		{
			SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_SETPOS, (WPARAM)1, (LPARAM)(setting_preamp * 100.0));
		}
		break;

	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDC_VALUE,  TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 20));
		SendDlgItemMessage(hwnd, IDC_VALUE,  TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_boost));
		SendDlgItemMessage(hwnd, IDC_BAND,   TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(20, 350));
		SendDlgItemMessage(hwnd, IDC_BAND,   TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_band));
		SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(100, 2000));
		SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_preamp * 100.0));
		SendDlgItemMessage(hwnd, IDC_SHAPE,  TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(100, 500));
		SendDlgItemMessage(hwnd, IDC_SHAPE,  TBM_SETPOS,   (WPARAM)1, (LPARAM)(setting_shape * 100.0));
		if(setting_normal)
			CheckDlgButton(hwnd, IDC_NORMAL, BST_CHECKED);
		else
			CheckDlgButton(hwnd, IDC_NORMAL, BST_UNCHECKED);

		SetTimer(hwnd, 12345, 30, 0);
		break;

	case WM_DESTROY:
		KillTimer(hwnd, 12345);
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
