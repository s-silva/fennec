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
#include "../../../include/fennec.h"
#include "data/systems/windows/resource.h"
#include "../../../include/system.h"

HINSTANCE     hInst;

int           callc plugin_dsp_initialize(void);
int           callc plugin_dsp_uninitialize(void);
unsigned long callc plugin_dsp_open(unsigned long id);
int           callc plugin_dsp_close(unsigned long id);
void*         callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr);
int           callc plugin_dsp_about(void *pdata);
int           callc plugin_dsp_settings(void *pdata);

double en_value = 1.0;

struct plugin_settings fsettings;

   float frequency, dB_boost;
   //filter parameters
   float xn1,xn2,yn1,yn2;
   float omega, sn, cs, a, shape, beta, b0, b1, b2, a0, a1, a2;


unsigned long callc fennec_initialize_dsp(struct general_dsp_data *gdn, string pname)
{
	if(gdn->ptype != fennec_plugintype_audiodsp)return fennec_input_invalidtype;

	pname[0] = 0;
	str_cpy(pname, uni("Stereo Enhancer"));

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

		if(fsettings.plugin_settings_getnum("dsp-stereo enhancer", "value", 0, 0, &en_value))en_value = 1.0;

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

int wrap16Bit(int data)
{
    if (data > 32767)
        data = 32767;
    else if (data < -32768)
        data = -32768;
    if (data < 0)
        data += 65536;
    return data;
}

void StereoEnhance16(short *ls, short *rs, double wc)
{
	double dleft, nls, nrs;
	double ms;
    int inls, inrs;  
	ms = ((double)(*ls + *rs)) / 2.0f;
	dleft = ((double)*ls) - ms;
	dleft = dleft * wc;

	nls = ((double)*ls) + dleft;
	nrs = ((double)*rs) - dleft;  

	inls = (int)nls;
	inrs = (int)nrs;

	*ls = (short)wrap16Bit(nls);
	*rs = (short)wrap16Bit(nrs);
}

void* callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr)
{
	double          cs, dl, lv, rv;
	fennec_sample  *din;
	unsigned int    i, c = (*bsize) / (bitspersample / 8);


	din = (fennec_sample*) (((char*)sbuffer) + apointer);

	for(i=0; i<c; i+=2)
	{
		lv = din[i];
		rv = din[i + 1];


		cs =  (lv + rv) / 2.0;
		dl =  lv - cs;
		dl *= en_value;


		lv = din[i]     + dl;
		rv = din[i + 1] - dl;

		if     (lv >  1.0)lv =  1.0;
		else if(lv < -1.0)lv = -1.0;

		if     (rv >  1.0)rv =  1.0;
		else if(rv < -1.0)rv = -1.0;
		

		din[i]      = lv;
		din[i +  1] = rv;
	}
	return sbuffer;
}

int callc plugin_dsp_about(void *pdata)
{
	MessageBox((HWND)pdata, "Stereo enhancer DSP for Fennec", "About", MB_ICONINFORMATION);
	return 1;
}

int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_HSCROLL:
		{
			int v = (int)SendDlgItemMessage(hwnd, IDC_VALUE, TBM_GETPOS, 0, 0);
			en_value = ((double)v) / 1000.0;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int v = (int)SendDlgItemMessage(hwnd, IDC_VALUE, TBM_GETPOS, 0, 0);

				en_value = ((double)v) / 1000.0;
				
				fsettings.plugin_settings_setfloat("dsp-stereo enhancer", "value", en_value);
			}
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDC_VALUE, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 1500));
		SendDlgItemMessage(hwnd, IDC_VALUE, TBM_SETPOS, (WPARAM)1, (LPARAM)(en_value * 1000.0));
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

int callc plugin_dsp_settings(void *pdata)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), (HWND)pdata, DialogProc);
	return 1;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
			hInst = hinstDLL;
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
