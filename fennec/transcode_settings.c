/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
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


#include "fennec_main.h"
#include "fennec_audio.h"
#include <commctrl.h>
#include <uxtheme.h>



/* prototypes ---------------------------------------------------------------*/

int callback_transcode_settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_transcode_settings_general(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_transcode_settings_eq(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_transcode_settings_tagging(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_transcode_settings_misc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);




/* data ---------------------------------------------------------------------*/

transcoder_settings  temp_trans_settings;
transcoder_settings *trans_settings;
TCITEM               window_ti[4];
int                  tabnames[4] = {oooo_trans_dlg_main_general, oooo_trans_dlg_main_equalizer, oooo_trans_dlg_main_tagging, oooo_trans_dlg_main_misc};
HWND                 last_tabpage;
int                  last_tabpageid = -1;
int                  ts_paneleq_lastsel = 0; /* universal */
int                  ts_paneleq_changed;


/* functions ----------------------------------------------------------------*/


/* <dep> win */

int transcode_settings_show(HWND hwndp, transcoder_settings *tsettings)
{
	int  ret;

	memcpy(&temp_trans_settings, tsettings, sizeof(transcoder_settings));
	trans_settings = &temp_trans_settings;

	last_tabpage   = 0;
	last_tabpageid = -1;

	ret = (int)DialogBox(instance_fennec, (LPCTSTR)dialog_transcode_config, hwndp, (DLGPROC)callback_transcode_settings);


	if(ret) memcpy(tsettings, &temp_trans_settings, sizeof(transcoder_settings));
	
	return ret;
}

/* </dep> */




/* <dep> win */

void dialog_tab_sel(HWND hwnd, int idt)
{
	HWND    tmain;
	HWND    hwnddlg = 0;
	RECT    rct;

	if(idt == last_tabpageid) return; /* same shit */

	DestroyWindow(last_tabpage);

	tmain = GetDlgItem(hwnd, tab_main);

	switch(idt)
	{
	case 0:
		hwnddlg = CreateDialog(instance_fennec, MAKEINTRESOURCE(dialog_transcode_general), tmain, (DLGPROC)callback_transcode_settings_general); 
		break;
	case 1:
		hwnddlg = CreateDialog(instance_fennec, MAKEINTRESOURCE(dialog_transcode_eq), tmain, (DLGPROC)callback_transcode_settings_eq); 
		break;
	case 2:
		hwnddlg = CreateDialog(instance_fennec, MAKEINTRESOURCE(dialog_transcode_tagging), tmain, (DLGPROC)callback_transcode_settings_tagging); 
		break;
	case 3:
		hwnddlg = CreateDialog(instance_fennec, MAKEINTRESOURCE(dialog_transcode_misc), tmain, (DLGPROC)callback_transcode_settings_misc); 
		break;
	}

	last_tabpage   = hwnddlg;
	last_tabpageid = idt;

	GetClientRect(hwnddlg, &rct);
	SendMessage(tmain, TCM_ADJUSTRECT, 0, (LPARAM) &rct);

	SetWindowPos(hwnddlg, HWND_TOP, rct.left + 2, rct.top + 2, 0, 0, SWP_NOSIZE);
	
	ShowWindow(hwnddlg, SW_SHOW);
	UpdateWindow(hwnddlg);
}


void dialog_tab_enabletex(HWND hwnd)
{
	typedef HRESULT (WINAPI *tetdt) (HWND, DWORD);

	HMODULE thememod;
	
	tetdt   Call_EnableThemeDialogTexture;

	thememod = LoadLibrary(uni("uxtheme.dll"));

	if(thememod)
	{
		Call_EnableThemeDialogTexture = (tetdt) GetProcAddress(thememod, "EnableThemeDialogTexture");
		if(Call_EnableThemeDialogTexture)
			Call_EnableThemeDialogTexture(hwnd, ETDT_USETABTEXTURE);
	}
}


/* </dep> */





/* callbacks ----------------------------------------------------------------*/

/* <dep> win */

/*
 * callback function.
 */
int callback_transcode_settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_NOTIFY:
		if( ((NMHDR*)lParam)->code == TCN_SELCHANGE)
		{
			int id = (int)SendDlgItemMessage(hwnd, tab_main, TCM_GETCURSEL, 0, 0);
			dialog_tab_sel(hwnd, id);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case button_ok:
			if(last_tabpage) DestroyWindow(last_tabpage);
			EndDialog(hwnd, 1);
			break;

		case IDCANCEL:
		case button_cancel:
			if(last_tabpage) DestroyWindow(last_tabpage);
			EndDialog(hwnd, 0);
			break;
		}
		break;
	
	case WM_INITDIALOG:

		window_active_dialog = hwnd; /* tab selection stuff */
		

		/* <lang> */

		SetWindowText(hwnd, text(oooo_trans_dlg_main_title));
		SetDlgItemText(hwnd, button_help,   text(oooo_help));
		SetDlgItemText(hwnd, button_cancel, text(oooo_cancel));
		SetDlgItemText(hwnd, button_ok,     text(oooo_ok));

		/* </lang> */

		{
			HWND  tmain;
			int   i;

			tmain = GetDlgItem(hwnd, tab_main);
			
			for(i=3; i>=0; i--)
			{
				window_ti[i].mask    = TCIF_TEXT;
				window_ti[i].pszText = text(tabnames[i]);
			
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


/*
 * callback function for general tab.
 */
int callback_transcode_settings_general(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case check_volenable:
			if(IsDlgButtonChecked(hwnd, check_volenable) == BST_CHECKED)
			{
				EnableWindow((HWND)GetDlgItem(hwnd, slider_vol), 1);
				if(SendDlgItemMessage(hwnd, slider_vol, TBM_GETPOS, 0, 0) == 10000)
				{
					/* enable gain */
					EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 1);
				}else{
					EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 0);
				}
			}else{
				EnableWindow((HWND)GetDlgItem(hwnd, slider_vol),  0);
				EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 0);
			}
			break;
		}
		break;
	
	case WM_HSCROLL:
		if(lParam == (LPARAM)GetDlgItem(hwnd, slider_vol))
		{
			if(SendDlgItemMessage(hwnd, slider_vol, TBM_GETPOS, 0, 0) == 10000)
			{
				/* enable gain */
				EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 1);
			}else{
				EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 0);
			}
		}
		break;
	
	case WM_INITDIALOG:
		dialog_tab_enabletex(hwnd);

		/* <lang> */

		SetDlgItemText(hwnd, group_panel,          text(oooo_trans_dlg_general_general_settings ));
		SetDlgItemText(hwnd, group_audiobuffer,    text(oooo_trans_dlg_general_audio_buffer     ));
		SetDlgItemText(hwnd, static_bufsize,       text(oooo_trans_dlg_general_buffersize       ));
		SetDlgItemText(hwnd, static_bufsize_note,  text(oooo_trans_dlg_general_buffersize_note  ));
		SetDlgItemText(hwnd, group_vol_ctrl,       text(oooo_trans_dlg_general_volume_control   ));
		SetDlgItemText(hwnd, static_vol,           text(oooo_trans_dlg_general_volume           ));
		SetDlgItemText(hwnd, static_gain,          text(oooo_trans_dlg_general_gain             ));
		SetDlgItemText(hwnd, check_volenable,      text(oooo_trans_dlg_general_apply_vol_ctrl   ));

		/* </lang> */

		SetDlgItemInt(hwnd, text_buffersize, (int)trans_settings->general.buffersize, 0);
		
		
		SendDlgItemMessage(hwnd, slider_vol,  TBM_SETRANGEMIN, 1, (LPARAM)0);
		SendDlgItemMessage(hwnd, slider_vol,  TBM_SETRANGEMAX, 1, (LPARAM)10000);
		SendDlgItemMessage(hwnd, slider_gain, TBM_SETRANGEMIN, 1, (LPARAM)0);
		SendDlgItemMessage(hwnd, slider_gain, TBM_SETRANGEMAX, 1, (LPARAM)10000);

		SendDlgItemMessage(hwnd, slider_vol,  TBM_SETPOS, 1, (LPARAM)(trans_settings->volume.vol  * 10000.0f));
		SendDlgItemMessage(hwnd, slider_gain, TBM_SETPOS, 1, (LPARAM)(trans_settings->volume.gain * 10000.0f));

		CheckDlgButton(hwnd, check_volenable, (trans_settings->volume.enable_vol == 1 ? BST_CHECKED : BST_UNCHECKED));
		
		if(trans_settings->volume.enable_vol)
		{
			if(trans_settings->volume.vol > 0.98)
			{
				EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 1);
			}else{
				EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 0);
			}
		}else{
			EnableWindow((HWND)GetDlgItem(hwnd, slider_vol),  0);
			EnableWindow((HWND)GetDlgItem(hwnd, slider_gain), 0);
		}

		break;

	case WM_DESTROY:
		{
			BOOL btr = 1;

			trans_settings->general.buffersize = GetDlgItemInt(hwnd, text_buffersize, &btr, 0);
			if(IsDlgButtonChecked(hwnd, check_volenable) == BST_CHECKED)
			{
				trans_settings->volume.vol  = ((double)SendDlgItemMessage(hwnd, slider_vol, TBM_GETPOS, 0, 0)) / 10000.0;
				trans_settings->volume.gain = ((double)SendDlgItemMessage(hwnd, slider_gain, TBM_GETPOS, 0, 0)) / 10000.0;
				trans_settings->volume.enable_vol = 1;
			}else{
				trans_settings->volume.vol  = 1.0;
				trans_settings->volume.gain = 0.0;
				trans_settings->volume.enable_vol = 0;
			}
		}
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}


/*
 * callback function for equalizer tab.
 */
int callback_transcode_settings_eq(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_VSCROLL:
		ts_paneleq_changed = 1;
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case button_reset:
			{
				unsigned long     cid, i, j;
				
				cid = (unsigned long)SendDlgItemMessage(hwnd, combo_channelmode, CB_GETCURSEL, 0, 0);

				SendDlgItemMessage(hwnd, slider_preamp, TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar1,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar2,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar3,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar4,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar5,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar6,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar7,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar8,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar9,   TBM_SETPOS, 1, (LPARAM)0);
				SendDlgItemMessage(hwnd, slider_bar10,  TBM_SETPOS, 1, (LPARAM)0);

				if(cid == 0 || cid == -1) /* universal */
				{
					for(i=0; i<16; i++) /* channel */
					{
						trans_settings->eq.eq.preamp[i] = 0.0;

						for(j=0; j<32; j++)
						{
							trans_settings->eq.eq.boost[i][j] = 0.0;
						}
					}
				}else{
					
					trans_settings->eq.eq.preamp[cid - 1] = 0.0;

					for(j=0; j<32; j++)
					{
						trans_settings->eq.eq.boost[cid - 1][j] = 0.0;
					}

				}

				ts_paneleq_changed = 0;
			}
			break;

		case combo_channelmode:
			if(HIWORD(wParam) == CBN_SELENDOK)
			{
				unsigned long     cid, i;

				/* save current settings */

				if(ts_paneleq_changed)
				{

					cid = ts_paneleq_lastsel;

					if(cid == 0 /* universal */)
					{
						trans_settings->eq.eq.preamp[0]   = ((float)SendDlgItemMessage(hwnd, slider_preamp, TBM_GETPOS, 0, 0)) / -1000.0f;

						trans_settings->eq.eq.boost[0][0] = ((float)SendDlgItemMessage(hwnd, slider_bar1,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][1] = ((float)SendDlgItemMessage(hwnd, slider_bar2,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][2] = ((float)SendDlgItemMessage(hwnd, slider_bar3,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][3] = ((float)SendDlgItemMessage(hwnd, slider_bar4,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][4] = ((float)SendDlgItemMessage(hwnd, slider_bar5,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][5] = ((float)SendDlgItemMessage(hwnd, slider_bar6,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][6] = ((float)SendDlgItemMessage(hwnd, slider_bar7,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][7] = ((float)SendDlgItemMessage(hwnd, slider_bar8,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][8] = ((float)SendDlgItemMessage(hwnd, slider_bar9,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[0][9] = ((float)SendDlgItemMessage(hwnd, slider_bar10,  TBM_GETPOS, 0, 0)) / -1000.0f;

						for(i=1 /* we just filled 0th, all others need a copy of it. */; i<16; i++)
						{
							trans_settings->eq.eq.preamp[i]   = trans_settings->eq.eq.preamp[0];
							
							trans_settings->eq.eq.boost[i][0] = trans_settings->eq.eq.boost[0][0];
							trans_settings->eq.eq.boost[i][1] = trans_settings->eq.eq.boost[0][1];
							trans_settings->eq.eq.boost[i][2] = trans_settings->eq.eq.boost[0][2];
							trans_settings->eq.eq.boost[i][3] = trans_settings->eq.eq.boost[0][3];
							trans_settings->eq.eq.boost[i][4] = trans_settings->eq.eq.boost[0][4];
							trans_settings->eq.eq.boost[i][5] = trans_settings->eq.eq.boost[0][5];
							trans_settings->eq.eq.boost[i][6] = trans_settings->eq.eq.boost[0][6];
							trans_settings->eq.eq.boost[i][7] = trans_settings->eq.eq.boost[0][7];
							trans_settings->eq.eq.boost[i][8] = trans_settings->eq.eq.boost[0][8];
							trans_settings->eq.eq.boost[i][9] = trans_settings->eq.eq.boost[0][9];
						}

					}else{
						cid--;
						trans_settings->eq.eq.preamp[cid]   = ((float)SendDlgItemMessage(hwnd, slider_preamp, TBM_GETPOS, 0, 0)) / -1000.0f;

						trans_settings->eq.eq.boost[cid][0] = ((float)SendDlgItemMessage(hwnd, slider_bar1,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][1] = ((float)SendDlgItemMessage(hwnd, slider_bar2,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][2] = ((float)SendDlgItemMessage(hwnd, slider_bar3,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][3] = ((float)SendDlgItemMessage(hwnd, slider_bar4,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][4] = ((float)SendDlgItemMessage(hwnd, slider_bar5,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][5] = ((float)SendDlgItemMessage(hwnd, slider_bar6,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][6] = ((float)SendDlgItemMessage(hwnd, slider_bar7,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][7] = ((float)SendDlgItemMessage(hwnd, slider_bar8,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][8] = ((float)SendDlgItemMessage(hwnd, slider_bar9,   TBM_GETPOS, 0, 0)) / -1000.0f;
						trans_settings->eq.eq.boost[cid][9] = ((float)SendDlgItemMessage(hwnd, slider_bar10,  TBM_GETPOS, 0, 0)) / -1000.0f;
					}

				}

				/* load new channel bands */
				
				cid = (unsigned long)SendDlgItemMessage(hwnd, combo_channelmode, CB_GETCURSEL, 0, 0);
				
				ts_paneleq_lastsel = cid;

				if(cid == 0 /* universal */) cid = 1;
				cid--;

				SendDlgItemMessage(hwnd, slider_preamp, TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.preamp[cid] * -1000.0f));

				SendDlgItemMessage(hwnd, slider_bar1,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][0] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar2,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][1] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar3,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][2] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar4,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][3] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar5,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][4] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar6,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][5] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar7,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][6] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar8,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][7] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar9,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][8] * -1000.0f));
				SendDlgItemMessage(hwnd, slider_bar10,  TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][9] * -1000.0f));

				ts_paneleq_changed = 0;
			}
			break;

		case combo_preset:
			{
				unsigned long     eqpid, szret = 0, cid;
				
				eqpid = (unsigned long)SendDlgItemMessage(hwnd, combo_preset, CB_GETCURSEL, 0, 0);
				
				cid = ts_paneleq_lastsel;
				if(cid == 0 /* universal */) cid = 1;
				cid--;

				if(eqpid <= settings.player.equalizer_presets)
				{
					settings_data_get(setting_id_equalizer_preset, eqpid, &trans_settings->eq.eq, &szret);
					if(szret)
					{
						SendDlgItemMessage(hwnd, slider_preamp, TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.preamp[cid] * -1000.0f));

						SendDlgItemMessage(hwnd, slider_bar1,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][0] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar2,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][1] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar3,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][2] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar4,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][3] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar5,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][4] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar6,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][5] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar7,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][6] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar8,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][7] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar9,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][8] * -1000.0f));
						SendDlgItemMessage(hwnd, slider_bar10,  TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[cid][9] * -1000.0f));
					}
				}

				ts_paneleq_changed = 0;
				
			}
			
			break;
		}
		break;
	
	case WM_INITDIALOG:
		dialog_tab_enabletex(hwnd);

		/* <lang> */

		SetDlgItemText(hwnd, group_mode,     text(oooo_trans_dlg_eq_mode            ));
		SetDlgItemText(hwnd, static_channel, text(oooo_trans_dlg_eq_channel         ));
		SetDlgItemText(hwnd, static_preset,  text(oooo_trans_dlg_eq_preset          ));
		SetDlgItemText(hwnd, button_reset,   text(oooo_trans_dlg_eq_reset           ));
		SetDlgItemText(hwnd, check_useeq,    text(oooo_trans_dlg_eq_use_eq          ));
		SetDlgItemText(hwnd, group_eqbands,  text(oooo_trans_dlg_eq_equalizer_bands ));
		SetDlgItemText(hwnd, static_preamp,  text(oooo_trans_dlg_eq_preamp          ));
		SetDlgItemText(hwnd, static_bands,   text(oooo_trans_dlg_eq_bands           ));

		/* </lang> */

		SendDlgItemMessage(hwnd, slider_preamp,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_preamp,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar1,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar1,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar2,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar2,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar3,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar3,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar4,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar4,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar5,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar5,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar6,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar6,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar7,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar7,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar8,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar8,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar9,    TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar9,    TBM_SETRANGEMAX, 1, (LPARAM)+12000);
		SendDlgItemMessage(hwnd, slider_bar10,   TBM_SETRANGEMIN, 1, (LPARAM)-12000);
		SendDlgItemMessage(hwnd, slider_bar10,   TBM_SETRANGEMAX, 1, (LPARAM)+12000);

		SendDlgItemMessage(hwnd, slider_preamp, TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.preamp[0] * -1000.0f));

		SendDlgItemMessage(hwnd, slider_bar1,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][0] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar2,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][1] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar3,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][2] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar4,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][3] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar5,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][4] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar6,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][5] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar7,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][6] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar8,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][7] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar9,   TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][8] * -1000.0f));
		SendDlgItemMessage(hwnd, slider_bar10,  TBM_SETPOS, 1, (LPARAM)(trans_settings->eq.eq.boost[0][9] * -1000.0f));

		ts_paneleq_lastsel = 0; /* universal */
		ts_paneleq_changed = 0;

		{
			equalizer_preset  eqp;
			unsigned long     i;
			unsigned long     dsz = 0;
			string            channelmodes[17] = {uni("Universal"), uni("Channel 1"), uni("Channel 2"), uni("Channel 3"), uni("Channel 4")
												, uni("Channel 5"), uni("Channel 6"), uni("Channel 7"), uni("Channel 8"), uni("Channel 9")
												, uni("Channel 10"), uni("Channel 11"), uni("Channel 12"), uni("Channel 13"), uni("Channel 14")
												, uni("Channel 15"), uni("Channel 16")};


			for(i=0; i<settings.player.equalizer_presets; i++)
			{
				memset(eqp.name, 0, 30);
				settings_data_get(setting_id_equalizer_preset, i, &eqp, &dsz);
				if(dsz)
				{
					SendDlgItemMessage(hwnd, combo_preset, CB_ADDSTRING, 0, (LPARAM)eqp.name);
				}
			}

			for(i=0; i<17; i++)
			{
				SendDlgItemMessage(hwnd, combo_channelmode, CB_ADDSTRING, 0, (LPARAM)channelmodes[i]);
			}

			if(trans_settings->eq.enable_eq)
				CheckDlgButton(hwnd, check_useeq, BST_CHECKED);
			else
				CheckDlgButton(hwnd, check_useeq, BST_UNCHECKED);


			SendDlgItemMessage(hwnd, combo_channelmode, CB_SETCURSEL, (WPARAM)0, 0);

		}
		break;

	case WM_DESTROY:
		{
			unsigned long     cid, i;

			/* save current settings */

			cid = ts_paneleq_lastsel;

			if(cid == 0 /* universal */)
			{
				trans_settings->eq.eq.preamp[0]   = ((float)SendDlgItemMessage(hwnd, slider_preamp, TBM_GETPOS, 0, 0)) / -1000.0f;

				trans_settings->eq.eq.boost[0][0] = ((float)SendDlgItemMessage(hwnd, slider_bar1,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][1] = ((float)SendDlgItemMessage(hwnd, slider_bar2,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][2] = ((float)SendDlgItemMessage(hwnd, slider_bar3,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][3] = ((float)SendDlgItemMessage(hwnd, slider_bar4,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][4] = ((float)SendDlgItemMessage(hwnd, slider_bar5,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][5] = ((float)SendDlgItemMessage(hwnd, slider_bar6,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][6] = ((float)SendDlgItemMessage(hwnd, slider_bar7,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][7] = ((float)SendDlgItemMessage(hwnd, slider_bar8,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][8] = ((float)SendDlgItemMessage(hwnd, slider_bar9,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[0][9] = ((float)SendDlgItemMessage(hwnd, slider_bar10,  TBM_GETPOS, 0, 0)) / -1000.0f;

				for(i=1 /* we just filled 0th, all others need a copy of it. */; i<16; i++)
				{
					trans_settings->eq.eq.preamp[i]   = trans_settings->eq.eq.preamp[0];
					
					trans_settings->eq.eq.boost[i][0] = trans_settings->eq.eq.boost[0][0];
					trans_settings->eq.eq.boost[i][1] = trans_settings->eq.eq.boost[0][1];
					trans_settings->eq.eq.boost[i][2] = trans_settings->eq.eq.boost[0][2];
					trans_settings->eq.eq.boost[i][3] = trans_settings->eq.eq.boost[0][3];
					trans_settings->eq.eq.boost[i][4] = trans_settings->eq.eq.boost[0][4];
					trans_settings->eq.eq.boost[i][5] = trans_settings->eq.eq.boost[0][5];
					trans_settings->eq.eq.boost[i][6] = trans_settings->eq.eq.boost[0][6];
					trans_settings->eq.eq.boost[i][7] = trans_settings->eq.eq.boost[0][7];
					trans_settings->eq.eq.boost[i][8] = trans_settings->eq.eq.boost[0][8];
					trans_settings->eq.eq.boost[i][9] = trans_settings->eq.eq.boost[0][9];
				}

			}else{
				cid--;
				trans_settings->eq.eq.preamp[cid]   = ((float)SendDlgItemMessage(hwnd, slider_preamp, TBM_GETPOS, 0, 0)) / -1000.0f;

				trans_settings->eq.eq.boost[cid][0] = ((float)SendDlgItemMessage(hwnd, slider_bar1,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][1] = ((float)SendDlgItemMessage(hwnd, slider_bar2,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][2] = ((float)SendDlgItemMessage(hwnd, slider_bar3,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][3] = ((float)SendDlgItemMessage(hwnd, slider_bar4,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][4] = ((float)SendDlgItemMessage(hwnd, slider_bar5,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][5] = ((float)SendDlgItemMessage(hwnd, slider_bar6,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][6] = ((float)SendDlgItemMessage(hwnd, slider_bar7,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][7] = ((float)SendDlgItemMessage(hwnd, slider_bar8,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][8] = ((float)SendDlgItemMessage(hwnd, slider_bar9,   TBM_GETPOS, 0, 0)) / -1000.0f;
				trans_settings->eq.eq.boost[cid][9] = ((float)SendDlgItemMessage(hwnd, slider_bar10,  TBM_GETPOS, 0, 0)) / -1000.0f;
			}

			trans_settings->eq.enable_eq = (IsDlgButtonChecked(hwnd, check_useeq) == BST_CHECKED) ? 1 : 0;
		}
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}



/*
 * callback function for tagging tab.
 */
int callback_transcode_settings_tagging(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		EnableWindow(GetDlgItem(hwnd, text_title),     IsDlgButtonChecked(hwnd, check_title   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_artist),    IsDlgButtonChecked(hwnd, check_artist  ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_album),     IsDlgButtonChecked(hwnd, check_album   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_year),      IsDlgButtonChecked(hwnd, check_year    ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, combo_genre),    IsDlgButtonChecked(hwnd, check_genre  ) == BST_CHECKED  || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_comments),  IsDlgButtonChecked(hwnd, check_comments) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		
		EnableWindow(GetDlgItem(hwnd, check_title   ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_artist  ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_album   ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_year    ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_genre  ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_comments), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);

		break;
	
	case WM_INITDIALOG:
		dialog_tab_enabletex(hwnd);
		
		/* <lang> */

		SetDlgItemText(hwnd, check_title,          text(oooo_trans_dlg_tags_title    ));
		SetDlgItemText(hwnd, check_album,          text(oooo_trans_dlg_tags_album    ));
		SetDlgItemText(hwnd, check_artist,         text(oooo_trans_dlg_tags_artist   ));
		SetDlgItemText(hwnd, check_year,           text(oooo_trans_dlg_tags_year     ));
		SetDlgItemText(hwnd, check_genre,          text(oooo_trans_dlg_tags_genre    ));
		SetDlgItemText(hwnd, check_comments,       text(oooo_trans_dlg_tags_comments ));
		SetDlgItemText(hwnd, check_copycommontags, text(oooo_trans_dlg_tags_copy_tags));

		/* </lang> */


		SendDlgItemMessage(hwnd, text_title,    WM_SETTEXT, 0, (LPARAM)trans_settings->tagging.d_title);
		SendDlgItemMessage(hwnd, text_artist,   WM_SETTEXT, 0, (LPARAM)trans_settings->tagging.d_artist);
		SendDlgItemMessage(hwnd, text_album,    WM_SETTEXT, 0, (LPARAM)trans_settings->tagging.d_album);
		SendDlgItemMessage(hwnd, text_year,     WM_SETTEXT, 0, (LPARAM)trans_settings->tagging.d_year);
		SendDlgItemMessage(hwnd, combo_genre,   WM_SETTEXT, 0, (LPARAM)trans_settings->tagging.d_genre);
		SendDlgItemMessage(hwnd, text_comments, WM_SETTEXT, 0, (LPARAM)trans_settings->tagging.d_comments);
		
		CheckDlgButton(hwnd, check_title,    trans_settings->tagging.e_title        ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, check_artist,   trans_settings->tagging.e_artist       ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, check_album,    trans_settings->tagging.e_album        ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, check_year,     trans_settings->tagging.e_year         ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, check_genre,    trans_settings->tagging.e_genre        ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, check_comments, trans_settings->tagging.e_comments     ? BST_CHECKED : BST_UNCHECKED);
		
		CheckDlgButton(hwnd, check_copycommontags,  trans_settings->tagging.enable_tagging ? BST_CHECKED : BST_UNCHECKED);
		
		EnableWindow(GetDlgItem(hwnd, text_title),     IsDlgButtonChecked(hwnd, check_title   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_artist),    IsDlgButtonChecked(hwnd, check_artist  ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_album),     IsDlgButtonChecked(hwnd, check_album   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_year),      IsDlgButtonChecked(hwnd, check_year    ) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, combo_genre),    IsDlgButtonChecked(hwnd, check_genre  ) == BST_CHECKED  || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, text_comments),  IsDlgButtonChecked(hwnd, check_comments) == BST_CHECKED || IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		
		EnableWindow(GetDlgItem(hwnd, check_title   ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_artist  ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_album   ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_year    ), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_genre  ),  IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, check_comments), IsDlgButtonChecked(hwnd, check_copycommontags) != BST_CHECKED ? 0 : 1);

		{
			unsigned int i;
			for(i=0; i<tag_genres_count; i++)
			{
				SendDlgItemMessage(hwnd, combo_genre, CB_ADDSTRING, 0, (LPARAM)tag_genres[i]);
			}
		}
		break;

	case WM_DESTROY:
		{
			trans_settings->tagging.enable_tagging = (IsDlgButtonChecked(hwnd, check_copycommontags) == BST_CHECKED) ? 1 : 0;

			if(trans_settings->tagging.enable_tagging)
			{
				trans_settings->tagging.e_title    = (IsDlgButtonChecked(hwnd, check_title)    == BST_CHECKED) ? 1 : 0;
				trans_settings->tagging.e_artist   = (IsDlgButtonChecked(hwnd, check_artist)   == BST_CHECKED) ? 1 : 0;
				trans_settings->tagging.e_album    = (IsDlgButtonChecked(hwnd, check_album)    == BST_CHECKED) ? 1 : 0;
				trans_settings->tagging.e_year     = (IsDlgButtonChecked(hwnd, check_year)     == BST_CHECKED) ? 1 : 0;
				trans_settings->tagging.e_genre    = (IsDlgButtonChecked(hwnd, check_genre)    == BST_CHECKED) ? 1 : 0;
				trans_settings->tagging.e_comments = (IsDlgButtonChecked(hwnd, check_comments) == BST_CHECKED) ? 1 : 0;
			}else{
				trans_settings->tagging.e_title    = 0;
				trans_settings->tagging.e_artist   = 0;
				trans_settings->tagging.e_album    = 0;
				trans_settings->tagging.e_year     = 0;
				trans_settings->tagging.e_genre    = 0;
				trans_settings->tagging.e_comments = 0;
			}

			GetDlgItemText(hwnd, text_title,    trans_settings->tagging.d_title,    sizeof(trans_settings->tagging.d_title   ) / sizeof(letter));
			GetDlgItemText(hwnd, text_artist,   trans_settings->tagging.d_artist,   sizeof(trans_settings->tagging.d_artist  ) / sizeof(letter));
			GetDlgItemText(hwnd, text_album,    trans_settings->tagging.d_album,    sizeof(trans_settings->tagging.d_album   ) / sizeof(letter));
			GetDlgItemText(hwnd, text_year,     trans_settings->tagging.d_year,     sizeof(trans_settings->tagging.d_year    ) / sizeof(letter));
			GetDlgItemText(hwnd, combo_genre,   trans_settings->tagging.d_genre,    sizeof(trans_settings->tagging.d_genre   ) / sizeof(letter));
			GetDlgItemText(hwnd, text_comments, trans_settings->tagging.d_comments, sizeof(trans_settings->tagging.d_comments) / sizeof(letter));
		}
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}



/*
 * callback function for misc tab.
 */
int callback_transcode_settings_misc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:

		break;
	
	case WM_INITDIALOG:
		dialog_tab_enabletex(hwnd);

		
		/* <lang> */

		SetDlgItemText(hwnd, group_datentime,   text(oooo_trans_dlg_misc_date_and_time ));
		SetDlgItemText(hwnd, radio_origdate,    text(oooo_trans_dlg_misc_use_original  ));
		SetDlgItemText(hwnd, radio_currentdate, text(oooo_trans_dlg_misc_use_current   ));
		SetDlgItemText(hwnd, radio_cusdate,     text(oooo_trans_dlg_misc_use_custom    ));
		SetDlgItemText(hwnd, static_year,       text(oooo_trans_dlg_misc_year          ));
		SetDlgItemText(hwnd, static_month,      text(oooo_trans_dlg_misc_month         ));
		SetDlgItemText(hwnd, static_date,       text(oooo_trans_dlg_misc_date          ));
		SetDlgItemText(hwnd, static_hour,       text(oooo_trans_dlg_misc_hour          ));
		SetDlgItemText(hwnd, static_minute,     text(oooo_trans_dlg_misc_minute        ));
		
		/* </lang> */

		if(trans_settings->misc.date_and_time_mode == 0) /* original */
		{
			SendDlgItemMessage(hwnd, radio_origdate   , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, radio_currentdate, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, radio_cusdate    , BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if(trans_settings->misc.date_and_time_mode == 1) /* current */
		{
			SendDlgItemMessage(hwnd, radio_origdate   , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, radio_currentdate, BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, radio_cusdate    , BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if(trans_settings->misc.date_and_time_mode == 2) /* custom */
		{
			SendDlgItemMessage(hwnd, radio_origdate   , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, radio_currentdate, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, radio_cusdate    , BM_SETCHECK, BST_CHECKED  , 0);
		}

		SetDlgItemInt(hwnd, text_year  , trans_settings->misc.custom_year  , 0);
		SetDlgItemInt(hwnd, text_month , trans_settings->misc.custom_month , 0);
		SetDlgItemInt(hwnd, text_date  , trans_settings->misc.custom_date  , 0);
		SetDlgItemInt(hwnd, text_hour  , trans_settings->misc.custom_hour  , 0);
		SetDlgItemInt(hwnd, text_minute, trans_settings->misc.custom_minute, 0);
		break;

	case WM_DESTROY:
		{
			trans_settings->misc.custom_year   = (short)GetDlgItemInt(hwnd, text_year  , 0, 0);
			trans_settings->misc.custom_month  = (unsigned char)GetDlgItemInt(hwnd, text_month , 0, 0);
			trans_settings->misc.custom_date   = (unsigned char)GetDlgItemInt(hwnd, text_date  , 0, 0);
			trans_settings->misc.custom_hour   = (unsigned char)GetDlgItemInt(hwnd, text_hour  , 0, 0);
			trans_settings->misc.custom_minute = (unsigned char)GetDlgItemInt(hwnd, text_minute, 0, 0);

			if(IsDlgButtonChecked(hwnd, radio_origdate)    == BST_CHECKED) trans_settings->misc.date_and_time_mode = 0;
			if(IsDlgButtonChecked(hwnd, radio_currentdate) == BST_CHECKED) trans_settings->misc.date_and_time_mode = 1;
			if(IsDlgButtonChecked(hwnd, radio_cusdate)     == BST_CHECKED) trans_settings->misc.date_and_time_mode = 2;
		}
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}
/* </dep> */





/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
