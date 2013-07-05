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
#include "keyboard.h"
#include "fennec_audio.h"
#include <shlobj.h>

unsigned long kb_getpurpose(unsigned short fkey, struct setting_key *kcol, int kcount)
{
	int i;
	for(i=0; i<kcount; i++)
		if(kcol[i].kcomb == fkey)return kcol[i].kaction;
	return keypurpose_null;
}

unsigned long kb_action(unsigned short fkey, struct setting_key *kcol, int kcount)
{
	unsigned short kp = keypurpose_null;
	int   i;
	
	for(i=0; i<kcount; i++)
	{
		if(kcol[i].kcomb == fkey)
		{
			kp = kcol[i].kaction;
			break;
		}else if(kcol[i].kcomb == 0 && kcol[i].kaction == keypurpose_null){
			return 0;
		}
	}
	
	if(kp == keypurpose_null)return 0;

	switch(kp)
	{
	case keypurpose_play:
		audio_play();
		break;

	case keypurpose_pause:
		audio_pause();
		break;

	case keypurpose_stop:
		audio_stop();
		break;

	case keypurpose_load:
		GlobalFunction(Function_OpenFileDialog);
		break;
                  
	case keypurpose_rewind:
		GlobalFunction(Function_Rewind);
		break;

	case keypurpose_forward:
		GlobalFunction(Function_Forward);
		break;

	case keypurpose_previous:
		audio_playlist_previous();
		break;

	case keypurpose_next:
		audio_playlist_next();
		break;

	case keypurpose_eject:
		if(settings.player.selected_drive)
		{
			if(audio_playlist_getsource(audio_playlist_getcurrentindex())[0] == settings.player.selected_drive)audio_stop();
			audio_input_drive_eject((char)settings.player.selected_drive);
		}
		break;

	case keypurpose_select:
		if(settings.player.selected_drive)
		{
			int     i, c = audio_input_drive_gettracks((char)settings.player.selected_drive);
			letter  buf[128];
			letter  buf2[32];

			if(c > 0)
			{
				audio_playlist_clear();
				for(i=0; i<c; i++)
				{
					buf[0] = (letter)settings.player.selected_drive;
					str_cpy(buf + 1, uni(":\\Track"));
							
					memset(buf2, 0, sizeof(buf2));
					str_itos(i + 1, buf2, 10);
					str_cat(buf, buf2);
					str_cat(buf, uni(".cda"));

					audio_playlist_add(buf, 0, 0);
				}
				fennec_refresh(fennec_v_refresh_force_high);
			}
		}
		break;

	case keypurpose_panelsw_main:
		GlobalFunction(Function_MainPanel);
		break;

	case keypurpose_panelsw_color:
		GlobalFunction(Function_SelectThemePanel);
		break;

	case keypurpose_panelsw_visualization:
		//Display_ShowPanel(PanelID_Visualization);
		break;

	case keypurpose_panelsw_equalizer:
		GlobalFunction(Function_EqualizerPanel);
		break;

	case keypurpose_panelsw_mediainfo:
		break;

	case keypurpose_panelsw_playlist:
		break;

	case keypurpose_panelnext:
		//Display_ShowPanel_Next();
		break;

	case keypurpose_panelprevious:
		//Display_ShowPanel_Back();
		break;

	case keypurpose_exit:
		fennec_power_set(fennec_power_mode_default);
		break;

	case keypurpose_sleep:
		fennec_power_sleep();
		break;

	case keypurpose_minimize:
		ShowWindow(window_main,SW_MINIMIZE);
		break;

	case keypurpose_refresh:
		fennec_refresh(fennec_v_refresh_force_full);
		break;

	case keypurpose_conversion:
		basewindows_show_conversion(0);
		break;

	case keypurpose_ripping:
		basewindows_show_ripping(1);
		break;

	case keypurpose_joining:
		BaseWindows_ShowJoining(1);
		break;

	case keypurpose_visualization:

		break;

	case keypurpose_playlist:

		break;

	case keypurpose_volumeup:
		{
			double vl = 0.0f, vr = 0.0f;
			audio_getvolume(&vl, &vr);
			vl += 0.05;
			vr += 0.05;
			if(vl > 1.0f)vl = 1.0f;
			if(vr > 1.0f)vr = 1.0f;

			audio_setvolume(vl, vr);

			fennec_refresh(fennec_v_refresh_force_not);
		}
		break;

	case keypurpose_volumedown:
		{
			double vl = 0.0f, vr = 0.0f;
			audio_getvolume(&vl, &vr);
			vl -= 0.05;
			vr -= 0.05;
			if(vl < 0.0f)vl = 0.0f;
			if(vr < 0.0f)vr = 0.0f;

			audio_setvolume(vl, vr);

			fennec_refresh(fennec_v_refresh_force_not);
		}
		break;

	case keypurpose_volumeup_auto:
		Menu_MainProc(menu_vol_autoinc);
		break;

	case keypurpose_volumedown_auto:
		Menu_MainProc(menu_vol_autodec);
		break;

	case keypurpose_volumemin:
		audio_setvolume(0.0f, 0.0f);
		fennec_refresh(fennec_v_refresh_force_not);
		break;

	case keypurpose_volumemax:
		audio_setvolume(1.0f, 1.0f);
		fennec_refresh(fennec_v_refresh_force_not);
		break;

	case keypurpose_addfile:
		GlobalFunction(Function_AddFileDialog);
		break;

	case keypurpose_fast_load:
		break;

	case keypurpose_fast_addfile:
		break;

	case keypurpose_preferences:
		settings_ui_show();
		break;

	case keypurpose_keyboardviewer:
		break;

	case keypurpose_currenttagging:
		basewindows_show_tagging(0, audio_playlist_getsource(audio_playlist_getcurrentindex()));
		break;

	case keypurpose_switch_main:
		SetFocus(window_main);
		break;

	case keypurpose_switch_playlist:
		if(GetFocus() != window_main)
		{
			SetFocus(window_main);
		}else{
			HWND plwnd;

			skins_function_getdata(get_window_playlist, &plwnd, 0);
			
			if(plwnd)
			{
				SetFocus(plwnd);
				break;
			}
		}
		break;

	case keypurpose_playlist_autoswitching:
		settings.player.auto_switching ^= 1;
		break;

	case keypurpose_playlist_shuffle:
		if(settings.player.playlist_shuffle)
			audio_playlist_setshuffle(0, 1);
		else
			audio_playlist_setshuffle(1, 1);

		break;

	case keypurpose_playlist_information:
		/* [todo] wrong!! */
		basewindows_show_tagging(0, audio_playlist_getsource(audio_playlist_getcurrentindex()));
		break;

	case keypurpose_playlist_repeatall:
		settings.player.playlist_repeat_list ^= 1;
		break;

	case keypurpose_playlist_repeatsingle:
		settings.player.playlist_repeat_single ^= 1;
		break;

	case keypurpose_playlist_insert:
		GlobalFunction(Function_AddFileDialog);
		fennec_refresh(fennec_v_refresh_force_high);
		break;

	case keypurpose_playlist_insertdir:
		{
			letter        fpath[v_sys_maxpath];
			BROWSEINFO    bi;
			LPITEMIDLIST  lpi;

			fpath[0] = 0;

			bi.hwndOwner      = window_main;
			bi.lpszTitle      = uni("Add to playlist.");
			bi.pszDisplayName = fpath;
			bi.lpfn           = 0;
			bi.iImage         = 0;
			bi.lParam         = 0;
			bi.pidlRoot       = 0;
			bi.ulFlags        = BIF_RETURNONLYFSDIRS;

			lpi = SHBrowseForFolder(&bi);
			SHGetPathFromIDList(lpi, fpath);

			if(str_len(fpath))
			{
				AddDirectory(fpath);
			}
			fennec_refresh(fennec_v_refresh_force_high);
		}
		break;

	case keypurpose_playlist_remove:

		break;


	}
	
	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, june 2007.
-----------------------------------------------------------------------------*/