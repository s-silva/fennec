/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
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

#include "fennec_main.h"
#include "fennec_global_environment.h"
#include "fennec_audio.h"




/* defines ------------------------------------------------------------------*/

#define timer_volume           10110
#define volume_active_nothing  0
#define volume_active_increase 1
#define volume_active_decrease 2




/* prototypes ---------------------------------------------------------------*/

void CALLBACK TimerProc_Volume(HWND hwnd, UINT uMsg, UINT_PTR idEvent, unsigned long dwTime);
int callback_seldrive(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);




/* data ---------------------------------------------------------------------*/

double volume_currentvolume_l;
double volume_currentvolume_r;
unsigned char volume_active = 0;





/* code ---------------------------------------------------------------------*/

int Menu_MainProc(long mi)
{

/*
menu_vol_100
menu_vol_75
menu_vol_50
menu_vol_25
menu_vol_mute

menu_vol_autodec
menu_vol_autoinc

menu_eject
menu_openfile
menu_addfile

menu_play
menu_pause
menu_stop
menu_next
menu_previous
		
menu_rewind
menu_fastforward
		
menu_about

menu_openfile
menu_addfile

menu_about
menu_about

menu_settings
menu_skins
menu_effects

menu_conversion
menu_ripping
menu_joining
menu_tageditor

menu_select_drive
menu_load_tracks
menu_eject
	
menu_sleep
menu_wake
menu_exit
*/


	switch(mi)
	{
	case menu_about:
		basewindows_show_about(1);
		break;

	case menu_help:
		{
			letter  fpath[v_sys_maxpath];

			str_cpy(fpath, uni("file:///"));
			str_cat(fpath, fennec_get_path(0, 0));
			str_cat(fpath, uni("/help/fennec help.html"));

			ShellExecute(window_main, 0, fpath, 0, 0, SW_SHOW);
		}
		break;

	case menu_vol_100:
		audio_setvolume(1.0f, 1.0f);
		break;

	case menu_vol_75:
		audio_setvolume(0.75f, 0.75f);
		break;

	case menu_vol_50:
		audio_setvolume(0.5f, 0.5f);
		break;

	case menu_vol_25:
		audio_setvolume(0.25f, 0.25f);
		break;

	case menu_vol_mute:
		audio_setvolume(0.0f, 0.0f);
		break;

	case menu_vol_autoinc:
		audio_getvolume(&volume_currentvolume_l, &volume_currentvolume_r);
		if(volume_active == volume_active_nothing) SetTimer(0, timer_volume, 20, (TIMERPROC)TimerProc_Volume);

		volume_active = volume_active_increase;
		break;

	case menu_vol_autodec:
		audio_getvolume(&volume_currentvolume_l, &volume_currentvolume_r);
		if(volume_active == volume_active_nothing) SetTimer(0, timer_volume, 20, (TIMERPROC)TimerProc_Volume);

		volume_active = volume_active_decrease;
		break;

	case menu_openfile:
		GlobalFunction(Function_OpenFileDialog);
		break;
	
	case menu_load_tracks:
		playback_loadtracks();
		break;

	case menu_select_drive:
		DialogBox(instance_fennec, MAKEINTRESOURCE(IDD_SELDRIVE), window_main, (DLGPROC)callback_seldrive);
		break;

	case menu_eject:
		if(settings.player.selected_drive)
		{
			if(audio_playlist_getsource(audio_playlist_getcurrentindex())[0] == settings.player.selected_drive)audio_stop();
			audio_input_drive_eject((char)settings.player.selected_drive);
		}
		break;

	case menu_play:
		GlobalFunction(Function_Play);
		break;

	case menu_pause:
		GlobalFunction(Function_Pause);
		break;

	case menu_stop:
		GlobalFunction(Function_Stop);
		break;

	case menu_previous:
		GlobalFunction(Function_Previous);
		break;

	case menu_next:
		GlobalFunction(Function_Next);
		break;

	case menu_rewind:
		GlobalFunction(Function_Rewind);
		break;

	case menu_fastforward:
		GlobalFunction(Function_Forward);
		break;

	case menu_addfile:
		GlobalFunction(Function_AddFileDialog);
		break;

	case menu_settings:
		settings_ui_show();
		break;

	case menu_skins:
		settings_ui_show_ex(fennec_v_settings_ui_panel_skins);
		break;

	case menu_effects:
		settings_ui_show_ex(fennec_v_settings_ui_panel_dsp);
		break;

	case menu_conversion:
		basewindows_show_conversion(0);
		break;

	case menu_ripping:
		basewindows_show_ripping(1);
		break;

	case menu_joining:
		BaseWindows_ShowJoining(1);
		break;

	case menu_tageditor:
		basewindows_show_tagging(1, audio_playlist_getsource(audio_playlist_getcurrentindex()));
		break;

	case menu_sleep:
		fennec_power_sleep();
		break;

	case menu_wake:
		fennec_power_wake();
		break;

	case menu_exit:
		fennec_power_set(fennec_power_mode_default);
		break;

	default:
		return 0;
	}

	fennec_refresh(fennec_v_refresh_force_not);
	return 1;
}


/* locals  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




void CALLBACK TimerProc_Volume(HWND hwnd, UINT uMsg, UINT_PTR idEvent, unsigned long dwTime)
{
	if(volume_active == volume_active_increase)
	{
		volume_currentvolume_l += 0.01f;
		volume_currentvolume_r += 0.01f;

		if(volume_currentvolume_l >= 1.0f) volume_currentvolume_l = 1.0f;
		if(volume_currentvolume_r >= 1.0f) volume_currentvolume_r = 1.0f;

		audio_setvolume(volume_currentvolume_l, volume_currentvolume_r);

		fennec_refresh(fennec_v_refresh_force_not);

		if(volume_currentvolume_l == 1.0f && volume_currentvolume_r == 1.0f)
		{
			volume_active = volume_active_nothing;
			KillTimer(hwnd, idEvent);
			return;
		}

	}else if(volume_active == volume_active_decrease){

		volume_currentvolume_l -= 0.01f;
		volume_currentvolume_r -= 0.01f;

		if(volume_currentvolume_l <= 0.0f) volume_currentvolume_l = 0.0f;
		if(volume_currentvolume_r <= 0.0f) volume_currentvolume_r = 0.0f;

		audio_setvolume(volume_currentvolume_l, volume_currentvolume_r);

		fennec_refresh(fennec_v_refresh_force_not);

		if(volume_currentvolume_l == 0.0f && volume_currentvolume_r == 0.0f)
		{
			volume_active = volume_active_nothing;
			KillTimer(hwnd, idEvent);
			return;
		}
	}
}


int callback_seldrive(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char tmpbuf[10];
				int csel = (int)SendDlgItemMessage(hwnd, IDC_DRIVES, CB_GETCURSEL, 0, 0);
				
				if(csel != -1)
				{
					SendDlgItemMessage(hwnd, IDC_DRIVES, CB_GETLBTEXT, csel, (LPARAM)tmpbuf);
					settings.player.selected_drive = tmpbuf[0];
				}else{
					settings.player.selected_drive = 0;
				}
			}
			EndDialog(hwnd, 1);
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int i;
			letter droot[]  = uni("X:\\");
			letter drname[] = uni("1:");

			for(i=0; i<26; i++)
			{
				droot[0] = (letter)(uni('A') + i);
				if(GetDriveType(droot) == DRIVE_CDROM)
				{
					drname[0] = (letter)(uni('A') + i);
					SendDlgItemMessage(hwnd, IDC_DRIVES, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)drname);
				}
			}

			if(drname[0] == uni('1'))
			{
				MessageBox(hwnd, uni("No CD drives found"), uni("Error"), MB_ICONEXCLAMATION);
				settings.player.selected_drive = 0;
				EndDialog(hwnd, 1);
			}
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
