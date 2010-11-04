/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Multichannel).
 Copyright (C) 2009 Chase <c-h@users.sf.net>

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
#include <uxtheme.h>
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
int CALLBACK General_DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);




/* data ---------------------------------------------------------------------*/

HINSTANCE               hinst;
struct plugin_settings  fsettings;
HWND                    window_general;
TCITEM                  window_ti[4];
char                   *tabnames[4] = {"General", "Emulation", "Reverberation", "Effects"};
HWND                    last_tabpage;
int                     last_tabpageid = -1;

unsigned long   last_frequency = 0;
unsigned long   last_channels  = 0;

unsigned int    state_processing = 0;

const double    delay_ms = 70;

double          volumes[max_channels];

struct s_lowpass
{
	double a[3], b[3];
	double x[3], y[3];
}lowpass;

const double pi = 3.1415926535897932384626433832795;

/* code ---------------------------------------------------------------------*/


unsigned long callc fennec_initialize_dsp(struct general_dsp_data *gdn, string pname)
{
	if(gdn->ptype != fennec_plugintype_audiodsp)return fennec_input_invalidtype;

	str_cpy(pname, uni("Multichannel"));

	if(gdn->fiversion >= plugin_version)
	{
		gdn->initialize    = plugin_dsp_initialize;
		gdn->uninitialize  = plugin_dsp_uninitialize;

		gdn->open          = plugin_dsp_open;
		gdn->process       = plugin_dsp_process;
		gdn->close         = plugin_dsp_close;

		gdn->settings      = plugin_dsp_settings;
		gdn->about         = plugin_dsp_about;

		gdn->messageproc   = messageproc;

		memcpy(&fsettings, &gdn->fsettings, sizeof(struct plugin_settings));

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
	int i = 0;

	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.front_left",   0, 0, &speakers.volumes.front_left  )) speakers.volumes.front_left   = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.front_right",  0, 0, &speakers.volumes.front_right )) speakers.volumes.front_right  = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.front_center", 0, 0, &speakers.volumes.front_center)) speakers.volumes.front_center = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.subwoofer",    0, 0, &speakers.volumes.subwoofer   )) speakers.volumes.subwoofer    = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.rear_left",    0, 0, &speakers.volumes.rear_left   )) speakers.volumes.rear_left    = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.rear_right",   0, 0, &speakers.volumes.rear_right  )) speakers.volumes.rear_right   = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.side_left",    0, 0, &speakers.volumes.side_left   )) speakers.volumes.side_left    = 1.0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.volumes.side_right",   0, 0, &speakers.volumes.side_right  )) speakers.volumes.side_right   = 1.0;

	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.delay.front_delay_ms", &speakers.delay.front_delay_ms, 0, 0)) speakers.delay.front_delay_ms = 0;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.delay.rear_delay_ms",  &speakers.delay.rear_delay_ms,  0, 0)) speakers.delay.rear_delay_ms  = 100;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.delay.side_delay_ms",  &speakers.delay.side_delay_ms,  0, 0)) speakers.delay.side_delay_ms  = 0;

	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.front_left",   &speakers.mapping.front_left  , 0, 0)) speakers.mapping.front_left   = channel_front_left   ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.front_right",  &speakers.mapping.front_right , 0, 0)) speakers.mapping.front_right  = channel_front_right  ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.front_center", &speakers.mapping.front_center, 0, 0)) speakers.mapping.front_center = channel_front_center ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.subwoofer",    &speakers.mapping.subwoofer   , 0, 0)) speakers.mapping.subwoofer    = channel_subwoofer    ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.rear_left",    &speakers.mapping.rear_left   , 0, 0)) speakers.mapping.rear_left    = channel_rear_left    ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.rear_right",   &speakers.mapping.rear_right  , 0, 0)) speakers.mapping.rear_right   = channel_rear_right   ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.side_left",    &speakers.mapping.side_left   , 0, 0)) speakers.mapping.side_left    = channel_side_left    ;
	if(fsettings.plugin_settings_getnum("dsp-multichannel", "speakers.mapping.side_right",   &speakers.mapping.side_right  , 0, 0)) speakers.mapping.side_right   = channel_side_right   ;

	speakers.delay.point_front_left  = 0;
	speakers.delay.point_front_right = 0;
	speakers.delay.point_rear_left   = 0;
	speakers.delay.point_rear_right  = 0;
	speakers.delay.point_side_left   = 0;
	speakers.delay.point_side_right  = 0;

	speakers.count = 6;
	return 1;
}

int callc plugin_dsp_uninitialize(void)
{
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.front_left",   speakers.volumes.front_left  );
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.front_right",  speakers.volumes.front_right );
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.front_center", speakers.volumes.front_center);
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.subwoofer",    speakers.volumes.subwoofer   );
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.rear_left",    speakers.volumes.rear_left   );
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.rear_right",   speakers.volumes.rear_right  );
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.side_left",    speakers.volumes.side_left   );
	fsettings.plugin_settings_setfloat("dsp-multichannel", "speakers.volumes.side_right",   speakers.volumes.side_right  );

	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.delay.front_delay_ms",      speakers.delay.front_delay_ms );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.delay.rear_delay_ms",       speakers.delay.rear_delay_ms  );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.delay.side_delay_ms",       speakers.delay.side_delay_ms  );
	
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.front_left",        speakers.mapping.front_left  );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.front_right",       speakers.mapping.front_right );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.front_center",      speakers.mapping.front_center);
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.subwoofer",         speakers.mapping.subwoofer   );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.rear_left",         speakers.mapping.rear_left   );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.rear_right",        speakers.mapping.rear_right  );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.side_left",         speakers.mapping.side_left   );
	fsettings.plugin_settings_setnum("dsp-multichannel", "speakers.mapping.side_right",        speakers.mapping.side_right  );

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

void lowpass_init(double cutoff, double frequency)
{
	double w0    = (atan(1) * 2.0) * cutoff / frequency;
	double Q     = 0.7;
	double vcos  = cos(w0);
	double alpha = sin(w0) / (2*Q);


	lowpass.b[0] = (1 - vcos) / 2;
	lowpass.b[1] =  1 - vcos;
	lowpass.b[2] = (1 - vcos) / 2;
	lowpass.a[0] =  1 + alpha;
	lowpass.a[1] = -2 * vcos;
	lowpass.a[2] =  1 - alpha;


	lowpass.x[0] = lowpass.x[1] = 0;
	lowpass.y[0] = lowpass.y[1] = 0;
}

double lowpass_process(double x)
{
	double y;

	y =   (lowpass.b[0]/lowpass.a[0]) * x
		+ (lowpass.b[1]/lowpass.a[0]) * lowpass.x[0]
		+ (lowpass.b[2]/lowpass.a[0]) * lowpass.x[1]
		- (lowpass.a[1]/lowpass.a[0]) * lowpass.y[0]
		- (lowpass.a[2]/lowpass.a[0]) * lowpass.y[1];

	lowpass.x[1] = lowpass.x[0];
	lowpass.x[0] = x;

	lowpass.y[1] = lowpass.y[0];
	lowpass.y[0] = y;

	return y;
}

double delay_l(double s, double srate)
{
	static int    i = -1;
	static double sb[10000];
	double        r;

	if(i<0)
	{
		memset(sb, 0, sizeof(sb));
		i = 0;
	}

	r = sb[i];
	sb[i] = s;

	if(++i >= (srate / 1000) * delay_ms)i=0;

	return r;
}

double delay_r(double s, double srate)
{
	static int    i = -1;
	static double sb[10000];
	double        r;

	if(i<0)
	{
		memset(sb, 0, sizeof(sb));
		i = 0;
	}

	r = sb[i];
	sb[i] = s;

	if(++i >= (srate / 1000) * delay_ms)i=0;

	return r;
}

double msin(double v)
{
	if(v > pi || v < 0) return 0;
	else return sin(v);
}

double mcos(double v)
{
	if(v > pi || v < 0) return 0;
	else return cos(v);
}


void* callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr)
{
	fennec_sample  *din, bl, br, apl, apr;
	unsigned int    i, c = (*bsize) / (bitspersample / 8), cx, j;
	static   double t;
	unsigned long   sz;

	fennec_sample *bkp = malloc((*bsize) + 1);
	memcpy(bkp, sbuffer, *bsize);

	sz = ((*bsize) * speakers.count) / channels;
	
	if(avbsize < sz)
		sbuffer = fr(sbuffer, sz);

	din = (fennec_sample*) (((char*)sbuffer) + apointer);
	cx  = sz / (bitspersample / 8);

	speakers.sampling_rate = freqency;

	if(last_frequency != freqency)
	{
		/* initialize effects that depend on sampling rate */
		last_frequency = freqency;
	}

	/* ----------------------------- DSP ------------------------------ */
	
	for(i=0, j=0; i<cx; j+=channels)
	{
		bl = bkp[j];
		br = bkp[j + 1];

		sp_process(din + i, bkp + j, channels);

		i += speakers.count;
	}

	/* ---------------------------------------------------------------- */

	*bsize = sz;

	free(bkp);
	return sbuffer;
}


int callc plugin_dsp_about(void *pdata)
{
	MessageBox((HWND)pdata, "Multichannel effect", "About", MB_ICONINFORMATION);
	return 1;
}


int callc plugin_dsp_settings(void *pdata)
{
	DialogBox(hinst, MAKEINTRESOURCE(IDD_MAIN), (HWND)pdata, (DLGPROC)DialogProc);
	return 1;
}

void draw_current_setup(HDC dc, RECT *rct)
{

}


void dialog_tab_sel(HWND hwnd, int idt)
{
	HWND    tmain;
	HWND    hwnddlg;
	RECT    rct;

	if(idt == last_tabpageid) return; /* same shit */

	EndDialog(last_tabpage, 0);

	tmain = GetDlgItem(hwnd, IDC_TMAIN);

	switch(idt)
	{
	case 0:
		hwnddlg = CreateDialog(hinst, MAKEINTRESOURCE(dialog_general), tmain, (DLGPROC)General_DialogProc); 
		break;
	case 1:
		hwnddlg = CreateDialog(hinst, MAKEINTRESOURCE(dialog_emulation), tmain, (DLGPROC)General_DialogProc); 
		break;
	case 2:
		hwnddlg = CreateDialog(hinst, MAKEINTRESOURCE(dialog_reverb), tmain, (DLGPROC)General_DialogProc); 
		break;
	case 3:
		hwnddlg = CreateDialog(hinst, MAKEINTRESOURCE(dialog_effects), tmain, (DLGPROC)General_DialogProc); 
		break;
	}

	last_tabpage   = hwnddlg;
	last_tabpageid = idt;

	GetClientRect(hwnddlg, &rct);
	SendMessage(tmain, TCM_ADJUSTRECT, 0, (LPARAM) &rct);

	SetWindowPos(hwnddlg, HWND_TOP, rct.left, rct.top, 0, 0, SWP_NOSIZE);
	
	ShowWindow(hwnddlg, SW_SHOW);
	UpdateWindow(hwnddlg);
}


int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_NOTIFY:
		if( ((NMHDR*)lParam)->code == TCN_SELCHANGE)
		{
			int id = (int)SendDlgItemMessage(hwnd, IDC_TMAIN, TCM_GETCURSEL, 0, 0);
			dialog_tab_sel(hwnd, id);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			HWND  tmain;
			int   i;

			tmain = GetDlgItem(hwnd, IDC_TMAIN);
			
			for(i=3; i>=0; i--)
			{
				window_ti[i].mask    = TCIF_TEXT;
				window_ti[i].pszText = tabnames[i];
			
				SendMessage(tmain, TCM_INSERTITEM, 0, (LPARAM)&window_ti[i]);
			}

			SendMessage(tmain, TCM_SETCURSEL, 0, 0);
		}
		dialog_tab_sel(hwnd, 0);
		break;

	case WM_DESTROY:
		if(last_tabpage) EndDialog(last_tabpage, 0);
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}


void tab_general_init(HWND hwnd)
{
	int   i, j;
	UINT  cid = 0;
	char *source_channel_names[] = {"(Front) Left", "(Front) Right", "Center", "LFE/Subwoofer", "Rear Left", "Rear Right", "Side Left", "Side Right"};
	char *output_modes[] = {"Mono", "Stereo", "5.1 Channels", "7.1 Channels"};

	for(i=0; i<8; i++)
	{
		switch(i)
		{
		case 0: cid = combo_sc1; break;
		case 1: cid = combo_sc2; break;
		case 2: cid = combo_sc3; break;
		case 3: cid = combo_sc4; break;
		case 4: cid = combo_sc5; break;
		case 5: cid = combo_sc6; break;
		case 6: cid = combo_sc7; break;
		case 7: cid = combo_sc8; break;
		}

		for(j=0; j<8; j++)
		{
			SendDlgItemMessage(hwnd, cid, CB_INSERTSTRING, (WPARAM)-1, (LPARAM) source_channel_names[j]);
		}
	}

	for(i=0; i<4; i++)
	{
		SendDlgItemMessage(hwnd, combo_outmodes, CB_INSERTSTRING, (WPARAM)-1, (LPARAM) output_modes[i]);
	}

	SendDlgItemMessage(hwnd, sld_vol1, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol2, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol3, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol4, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol5, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol6, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol7, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, sld_vol8, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));
	SendDlgItemMessage(hwnd, slider_delay, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 100));

	SendDlgItemMessage(hwnd, sld_vol1, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.front_left   * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol2, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.front_right  * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol3, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.front_center * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol4, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.subwoofer    * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol5, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.rear_left    * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol6, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.rear_right   * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol7, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.side_left    * 1000.0) );
	SendDlgItemMessage(hwnd, sld_vol8, TBM_SETPOS, 1, (LPARAM) (speakers.volumes.side_right   * 1000.0) );

	SendDlgItemMessage(hwnd, slider_delay, TBM_SETPOS, 1, (LPARAM) speakers.delay.rear_delay_ms);

	SendDlgItemMessage(hwnd, combo_sc1, CB_SETCURSEL, (WPARAM)speakers.mapping.front_left  , 0);
	SendDlgItemMessage(hwnd, combo_sc2, CB_SETCURSEL, (WPARAM)speakers.mapping.front_right , 0);
	SendDlgItemMessage(hwnd, combo_sc3, CB_SETCURSEL, (WPARAM)speakers.mapping.front_center, 0);
	SendDlgItemMessage(hwnd, combo_sc4, CB_SETCURSEL, (WPARAM)speakers.mapping.subwoofer   , 0);
	SendDlgItemMessage(hwnd, combo_sc5, CB_SETCURSEL, (WPARAM)speakers.mapping.rear_left   , 0);
	SendDlgItemMessage(hwnd, combo_sc6, CB_SETCURSEL, (WPARAM)speakers.mapping.rear_right  , 0);
	SendDlgItemMessage(hwnd, combo_sc7, CB_SETCURSEL, (WPARAM)speakers.mapping.side_left   , 0);
	SendDlgItemMessage(hwnd, combo_sc8, CB_SETCURSEL, (WPARAM)speakers.mapping.side_right  , 0);
}


int CALLBACK General_DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		speakers.mapping.front_left   = (int)SendDlgItemMessage(hwnd, combo_sc1, CB_GETCURSEL, 0, 0);
		speakers.mapping.front_right  = (int)SendDlgItemMessage(hwnd, combo_sc2, CB_GETCURSEL, 0, 0);
		speakers.mapping.front_center = (int)SendDlgItemMessage(hwnd, combo_sc3, CB_GETCURSEL, 0, 0);
		speakers.mapping.subwoofer    = (int)SendDlgItemMessage(hwnd, combo_sc4, CB_GETCURSEL, 0, 0);
		speakers.mapping.rear_left    = (int)SendDlgItemMessage(hwnd, combo_sc5, CB_GETCURSEL, 0, 0);
		speakers.mapping.rear_right   = (int)SendDlgItemMessage(hwnd, combo_sc6, CB_GETCURSEL, 0, 0);
		speakers.mapping.side_left    = (int)SendDlgItemMessage(hwnd, combo_sc7, CB_GETCURSEL, 0, 0);
		speakers.mapping.side_right   = (int)SendDlgItemMessage(hwnd, combo_sc8, CB_GETCURSEL, 0, 0);
		break;

	case WM_HSCROLL:
		speakers.volumes.front_left   = (double)SendDlgItemMessage(hwnd, sld_vol1, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.front_right  = (double)SendDlgItemMessage(hwnd, sld_vol2, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.front_center = (double)SendDlgItemMessage(hwnd, sld_vol3, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.subwoofer    = (double)SendDlgItemMessage(hwnd, sld_vol4, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.rear_left    = (double)SendDlgItemMessage(hwnd, sld_vol5, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.rear_right   = (double)SendDlgItemMessage(hwnd, sld_vol6, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.side_left    = (double)SendDlgItemMessage(hwnd, sld_vol7, TBM_GETPOS, 0, 0) / 1000.0;
		speakers.volumes.side_right   = (double)SendDlgItemMessage(hwnd, sld_vol8, TBM_GETPOS, 0, 0) / 1000.0;
		
		speakers.delay.rear_delay_ms  = (int)SendDlgItemMessage(hwnd, slider_delay, TBM_GETPOS, 0, 0);

		break;

	case WM_INITDIALOG:
		{
			typedef HRESULT (WINAPI *tetdt) (HWND, DWORD);

			HMODULE thememod;
			
			tetdt   Call_EnableThemeDialogTexture;

			thememod = LoadLibrary("uxtheme.dll");

			if(thememod)
			{
				Call_EnableThemeDialogTexture = (tetdt) GetProcAddress(thememod, "EnableThemeDialogTexture");
				if(Call_EnableThemeDialogTexture)
					Call_EnableThemeDialogTexture(hwnd, ETDT_USETABTEXTURE);
			}
		}

		tab_general_init(hwnd);
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
 eof.
-----------------------------------------------------------------------------*/
