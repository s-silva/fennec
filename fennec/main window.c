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


#include "fennec main.h"
#include "fennec audio.h"
#include "keyboard.h"
#include <zmouse.h>



/* defines ------------------------------------------------------------------*/

#define timer_scroll 1




/* declarations -------------------------------------------------------------*/

void CALLBACK callback_timer_scroll(HWND hwnd,UINT uMsg,UINT_PTR idEvent,unsigned long dwTime);




/* data ---------------------------------------------------------------------*/

HMENU             menu_main;
int	              keys_globalids[256];
letter            scroll_title[512];
char              scroll_last_mode;
struct skin_data  fennec_skin_data;
struct fennec     fennec;
WNDPROC           skinproc = 0;
NOTIFYICONDATA    notify_data;
int               appother_found = -1;
letter            appother_txt[512];




/* functions ----------------------------------------------------------------*/

void fennec_initialize(HWND hwnd)
{
	letter       tmpbuf[260];
	unsigned int i, kcount;
	long         wkey, wmsk;
	letter       tbuf[255],  tbuf2[32];

	/* set global variables */

	window_main    = hwnd;
	window_main_dc = GetDC(hwnd);

	/* general settings storage system for plug-ins */

	fennec_get_plugings_path(tmpbuf);
	str_cat(tmpbuf, uni("settings.txt"));
	plugin_settings_load(tmpbuf);

	reportx("plugins settings file path: %s", rt_stepping, tmpbuf);

	/* fill the fennec struct. */

	shared_functions_fill(v_shared_function_mask_audio_input  |
						  v_shared_function_mask_audio_output |
						  v_shared_function_mask_general      | 
						  v_shared_function_mask_simple       |
						  v_shared_function_mask_settings     |
						  v_shared_function_mask_media_library|
						  v_shared_function_mask_video, &fennec);

	/* register hotkeys */

	kcount = sizeof(settings.shortcuts.globalkeys) / sizeof(settings.shortcuts.globalkeys[0]);

	for(i=0; i<kcount; i++)
	{
		if(settings.shortcuts.globalkeys[i].kcomb == 0 && settings.shortcuts.globalkeys[i].kaction == keypurpose_null)break;

		wkey = fennec_convertkeyrev(settings.shortcuts.globalkeys[i].kcomb, &wmsk);

		str_cpy(tbuf, uni("fennec 7.1 hotkey: "));
		memset(tbuf2, 0, sizeof(tbuf2));
		str_itos(wkey, tbuf2, 16);
		str_cat(tbuf, tbuf2);
		memset(tbuf2, 0, sizeof(tbuf2));
		str_itos(wmsk, tbuf2, 16);
		str_cat(tbuf, tbuf2);

		keys_globalids[i] = GlobalAddAtom(tbuf);

		RegisterHotKey(hwnd, keys_globalids[i], wmsk, wkey);
	}

	keys_globalids[i] = 0;

	report("registered %u global shortcut keys", rt_stepping, i);

	/* initialize the audio interface */

	audio_playlist_initialize();

	audio_input_initialize();
	report("initialized audio input.", rt_stepping);
	
	audio_initialize();

	report("initialized audio output.", rt_stepping);
	

	/* restore last values */

	audio_setvolume(settings.player.volume[0], settings.player.volume[1]);


	/* load media list we've saved. */

	fennec_get_path(tmpbuf, sizeof(tmpbuf));
	str_cat(tmpbuf, uni("/playlist.txt"));

	reportx("loaded last playlist: %s", rt_stepping, tmpbuf);

	text_command_base(GetCommandLine());

	if(audio_playlist_getcount() == 0)
	{
		playlist_t_load_current(tmpbuf);
		audio_playlist_switch(0);
	}else{
		audio_playlist_switch(0);
		audio_play();
	}

	report("loaded last media", rt_stepping);

	/* load main menu */

	//menu_main = GetSubMenu(LoadMenu(instance_fennec, (LPCTSTR)IDR_POPUP), 0);

	/* <lang> */

	{
		HMENU  tsm_vol, tsm_play;

		menu_main = CreatePopupMenu();

		/* submenu - volume */

		tsm_vol = CreatePopupMenu();

		InsertMenu(tsm_vol, 0, MF_BYPOSITION | MF_ENABLED | MF_STRING, menu_vol_100, uni("100%"));
		AppendMenu(tsm_vol, MF_ENABLED | MF_STRING, menu_vol_75,    uni("75%") );
		AppendMenu(tsm_vol, MF_ENABLED | MF_STRING, menu_vol_50,    uni("50%") );
		AppendMenu(tsm_vol, MF_ENABLED | MF_STRING, menu_vol_25,    uni("25%") );
		AppendMenu(tsm_vol, MF_ENABLED | MF_STRING, menu_vol_mute,  text(oooo_menu_mute) );
		
		AppendMenu(tsm_vol, MF_SEPARATOR, 0, 0);

		AppendMenu(tsm_vol, MF_ENABLED | MF_STRING, menu_vol_autodec,  text(oooo_menu_auto_dec) );
		AppendMenu(tsm_vol, MF_ENABLED | MF_STRING, menu_vol_autoinc,  text(oooo_menu_auto_inc) );
		
		/* submenu - playback */

		tsm_play = CreatePopupMenu();

		InsertMenu(tsm_play, 0, MF_BYPOSITION | MF_ENABLED | MF_STRING, menu_eject,  text(oooo_menu_eject_load) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_openfile,                  text(oooo_menu_openfiles) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_addfile,                   text(oooo_menu_addfiles) );

		AppendMenu(tsm_play, MF_SEPARATOR, 0, 0);

		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_play,      text(oooo_play) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_pause,     text(oooo_pause) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_stop,      text(oooo_stop) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_next,      text(oooo_next) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_previous,  text(oooo_previous) );
		
		AppendMenu(tsm_play, MF_SEPARATOR, 0, 0);

		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_rewind,       text(oooo_menu_rewind) );
		AppendMenu(tsm_play, MF_ENABLED | MF_STRING, menu_fastforward,  text(oooo_menu_fastforward) );
		
		/* main menu */

		InsertMenu(menu_main, 0, MF_BYPOSITION | MF_ENABLED | MF_STRING, menu_about, text(oooo_menu_about) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_help, text(oooo_help) );

		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);
		
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_openfile, text(oooo_menu_openfiles) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_addfile,  text(oooo_menu_addfiles) );
		
		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);

		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_play, text(oooo_menu_play_pause) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_stop, text(oooo_stop) );

		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);

		AppendMenu(menu_main, MF_POPUP | MF_STRING | MF_ENABLED, (UINT_PTR)tsm_vol,  text(oooo_menu_volume) );
		AppendMenu(menu_main, MF_POPUP | MF_STRING | MF_ENABLED, (UINT_PTR)tsm_play,  text(oooo_menu_playback) );
	
		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);

		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_settings, text(oooo_menu_preferences) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_skins,    text(oooo_menu_select_skin) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_effects,  text(oooo_menu_effects) );

		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);

		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_conversion, text(oooo_menu_conversion) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_ripping,    text(oooo_menu_ripping) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_joining,    text(oooo_menu_joining) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_tageditor,  text(oooo_menu_tageditor) );
	
		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);

		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_select_drive,   text(oooo_menu_select_drive) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_load_tracks,    text(oooo_menu_load_tracks) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_eject,          text(oooo_menu_eject_load) );
	
		AppendMenu(menu_main, MF_SEPARATOR, 0, 0);

		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_sleep,   text(oooo_sleep) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_wake,    text(oooo_wake) );
		AppendMenu(menu_main, MF_ENABLED | MF_STRING, menu_exit,    text(oooo_exit) );
		
	}

	report("initialized main menu.", rt_stepping);

	/* </lang> */

	/* always on top? */

	if(settings.general.always_on_top)
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	/* skins stuff */

	if(settings.skins.selected[0] == 0) /* base skin */
	{
		/*SwitchToBaseSkin(1);*/

		if(settings.skins.selected[1] != 1)
		{
			settings_ui_show_ex(fennec_v_settings_ui_panel_skins);

			report("no skin has selected", rt_stepping);
		}else{
			/* settings.environment.main_window_state = setting_window_hidden; */
		}

	}else{

		int                      res;
		struct skin_data_return *rdata;
		RECT                     rct;
		letter                   skin_abs_path[v_sys_maxpath];

		fennec_skin_data.finstance = instance_fennec;
		fennec_skin_data.wnd       = hwnd;
		fennec_skin_data.wproc     = (WNDPROC)MainWndProc;
		fennec_skin_data.shared    = &fennec;

		fennec_get_abs_path(skin_abs_path, settings.skins.selected);

		reportx("initializing selected skin: %s", rt_stepping, skin_abs_path);

		res = skins_initialize(skin_abs_path, &fennec_skin_data);

		rdata = skins_getdata();
		
		if(rdata)
		{
			if(rdata->woptions)
			{
				SetWindowLong(hwnd, GWL_STYLE, rdata->woptions);
			}
			skinproc = rdata->callback;
		}else{
			skinproc = 0;
		}

		
		GetClientRect(hwnd, &rct);

		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE);
		SetWindowPos(hwnd, 0, 0, 0, rct.right, rct.bottom, SWP_NOMOVE);
		if(res < 0)settings_ui_show_ex(fennec_v_settings_ui_panel_skins);
		
		report("initialized selected skin.", rt_stepping);
	}

	/* initialize visualization */

	if(settings.visualizations.selected[0])
		visualizations_initialize(settings.visualizations.selected);

	report("initialized selected visualization", rt_stepping);

	if(settings.videooutput.selected[0])
		videoout_initialize(settings.videooutput.selected);

	report("initialized selected video output plugin", rt_stepping);

	/* initialize tips */
	
	tips_create();

	/* <notify icon> */

	notify_data.cbSize = sizeof(notify_data);
	notify_data.hWnd   = window_main;
	notify_data.uID    = 0;
	notify_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notify_data.hIcon  = LoadIcon(instance_fennec, (LPCTSTR)IDI_MAIN);
	
	str_cpy(notify_data.szTip, uni("Fennec Player"));
		
	notify_data.uCallbackMessage = WM_USER + 123;

	Shell_NotifyIcon(NIM_ADD, &notify_data);

	/* </notify icon> */
	
	/* all the other stuff */

	scroll_last_mode = settings.general.scroll_title_taskbar;
	
	SetTimer(0, timer_scroll, 150, (TIMERPROC)callback_timer_scroll);

	fennec_refresh(fennec_v_refresh_force_not);

	/* finally, add 'fennec path' to registry */

	{
		string  fpath = fennec_get_path(0, 0);
		HKEY    kf, pf;

		RegCreateKey(HKEY_LOCAL_MACHINE, uni("software\\fennec"), &kf);
		RegCreateKey(kf, uni("player"), &pf);

		RegSetValueEx(pf, uni("version"), 0, REG_SZ, (CONST BYTE*)fennec_u_version_string, (DWORD)str_size(fennec_u_version_string));
		RegSetValueEx(pf, uni("path"), 0, REG_SZ, (CONST BYTE*)fpath, (DWORD)str_size(fpath));

		RegCloseKey(kf);
		RegCloseKey(pf);
	}

	report("everything seems okay.", rt_stepping);

	video_initialize();


	return;
}

void main_refresh(void)
{
	struct fennec_audiotag  at;
	unsigned long           pid;
	letter                  ntitle[512];
	int                     da = 1;
	int                     pstate;
			
	scroll_title[0] = 0;


	pstate = audio_getplayerstate();

	if(pstate == audio_v_playerstate_notinit ||
	   pstate == audio_v_playerstate_null ||
	   pstate == audio_v_playerstate_init)
	{
		str_cpy(ntitle, uni("Fennec Player"));
		inform_other_programs(0);

	}else{

		pid = audio_input_tagread(audio_playlist_getsource(audio_playlist_getcurrentindex()), &at);
		
		if(pid == (unsigned long)-1)return;

		str_cpy(ntitle, uni("Fennec Player: "));
		memset(appother_txt, 0, sizeof(appother_txt));

		if(at.tag_artist.tsize)
		{
			str_cat(ntitle, at.tag_artist.tdata);
			str_cat(appother_txt, at.tag_artist.tdata);
			appother_found = 1;
		}else{
			da = 0;
			appother_found = 0;
		}

		if(at.tag_title.tsize)
		{
			if(da)str_cat(ntitle, uni(" - "));
			str_cat(ntitle, at.tag_title.tdata);

			if(da)str_cat(appother_txt, uni("\\0"));
			str_cat(appother_txt, at.tag_title.tdata);
			appother_found = 1;
		}else{
			da = 0;
			appother_found = 0;
		}

		if(!da)
		{
			str_cat(ntitle, uni("\n"));
			str_cat(ntitle, audio_playlist_getsource(audio_playlist_getcurrentindex()));
		}

		refresh_others();

		audio_input_tagread_known(pid, 0, &at);

	}
	
	str_ncpy(notify_data.szTip, ntitle, 64);

	Shell_NotifyIcon(NIM_MODIFY, &notify_data);
}


		
void refresh_others(void)
{
	letter                  dbuf[32];
	
	{
		int p;

		p = (int)(audio_getduration_ms() / 1000);


		memset(dbuf, 0, sizeof(dbuf));
		
		if(p / 60 <= 60)
		{
			_itow(p / 60, dbuf, 10);
			dbuf[str_len(dbuf)] = uni(':');

			if((p % 60) < 10)
			{	
				dbuf[str_len(dbuf)] = uni('0');
				_itow(p % 60, dbuf + str_len(dbuf), 10);
			}else{
				_itow(p % 60, dbuf + str_len(dbuf), 10);
			}

		}else{
			_itow(p / 3600, dbuf, 10);
			str_cat(dbuf, uni("h"));
		}
	}

	/*{
		int p;

		p = (int)(audio_getposition_ms() / 1000);


		memset(cbuf, 0, sizeof(cbuf));
		
		if(p / 60 <= 60)
		{
			_itow(p / 60, cbuf, 10);
			cbuf[str_len(cbuf)] = uni(':');

			if((p % 60) < 10)
			{	
				cbuf[str_len(cbuf)] = uni('0');
				_itow(p % 60, cbuf + str_len(cbuf), 10);
			}else{
				_itow(p % 60, cbuf + str_len(cbuf), 10);
			}

		}else{
			_itow(p / 3600, cbuf, 10);
			str_cat(cbuf, uni("h"));
		}
	}
	*/
	if(appother_found != 1)
	{
		_wsplitpath(audio_playlist_getsource(audio_playlist_getcurrentindex()), 0, 0, appother_txt, 0);
		str_cat(appother_txt, uni("\\0 "));
		//str_cat(appother_txt, cbuf);
		//str_cat(appother_txt, uni("/"));
		str_cat(appother_txt, dbuf);
		str_cat(appother_txt, uni(" (http://fennec.sf.net)\\0"));
	}else{
		str_cat(appother_txt, uni(" "));
		//str_cat(appother_txt, cbuf);
		//str_cat(appother_txt, uni("/"));
		str_cat(appother_txt, dbuf);
		str_cat(appother_txt, uni(" (http://fennec.sf.net)\\0"));
	}

	

	inform_other_programs(appother_txt);
	sys_sleep(0);
}


/* callback functions -------------------------------------------------------*/

void CALLBACK callback_timer_scroll(HWND hwnd, UINT uMsg, UINT_PTR idEvent, unsigned long dwTime)
{
	static int ienable_scrolling = 0;


	if(settings.general.scroll_title_taskbar)
	{
		scroll_last_mode = 1;

		if(!scroll_title[0])
		{
			struct fennec_audiotag at;
			unsigned long pid;
			int           pstate;

			pstate = audio_getplayerstate();

			if( pstate == audio_v_playerstate_notinit ||
				pstate == audio_v_playerstate_null ||
				pstate == audio_v_playerstate_init)
			{
				str_cpy(scroll_title, uni("Fennec Player"));

				ienable_scrolling = 0;

			}else{
				pid = audio_input_tagread(audio_playlist_getsource(audio_playlist_getcurrentindex()), &at);
				
				str_cpy(scroll_title, settings.formatting.scrolling_title);
				
				tags_translate(scroll_title, &at, audio_playlist_getsource(audio_playlist_getcurrentindex()));

				audio_input_tagread_known(pid, 0, &at);

				ienable_scrolling = 1;
			}

		}else{

			if(ienable_scrolling)
			{
				letter bkp[2];
				bkp[0] = scroll_title[0];
				bkp[1] = 0;

				str_cpy(scroll_title, scroll_title + 1);
				str_cat(scroll_title, bkp);
			}

			SetWindowText(window_main, scroll_title);
		}

	}else{

		if(scroll_last_mode) /* turned on */
		{
			SetWindowText(window_main, uni("Fennec Player 7.1"));
			scroll_last_mode = 0;
		}
	}
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_MOUSEWHEEL:
		if(LOWORD(wParam) == MK_SHIFT) /* seek */
		{
			double cpos;
			
			audio_getposition(&cpos);
			cpos += (double)(short)HIWORD(wParam) * 0.0001;
			if(cpos > 1.0)cpos = 1.0;
			else if(cpos < 0.0)cpos = 0.0;
			audio_setposition(cpos);

		}else if(LOWORD(wParam) == MK_CONTROL){ /* switch list */

			short v;

			v = (short)HIWORD(wParam);

			if(v > 0) audio_playlist_next();
			else      audio_playlist_previous();

		}else{ /* volume */
			double vl, vr, vm;

#			define c_volume_smoothness 2000.0

			audio_getvolume(&vl, &vr);

			vm = (vl + vr) / 2.0f;
			vm *= c_volume_smoothness;

			vm += (double)(short)HIWORD(wParam);
			
			if(vm < 0.0f)vm = 0.0f;
			if(vm > c_volume_smoothness)vm = c_volume_smoothness;
			
			vm /= c_volume_smoothness;
			
			audio_setvolume(vm, vm);

			fennec_refresh(fennec_v_refresh_force_not);
		}
		break;

	case WM_PAINT:
		if(!settings.skins.selected[0])
		{
			if(window_main_dc)
			{
				RECT          rct;
				HBRUSH        hbr, hobr;
				COLORREF      otc;
				int           tmode;
				const string  dtxt = uni("Fennec Player (No Skin)\n'F12' - Load default skin.\n'Esc' - Main menu.\n'F2' - Settings.");
				GetClientRect(window_main, &rct);

				hbr   = CreateSolidBrush(0x660000);
				hobr  = SelectObject(window_main_dc, hbr);
				otc   = GetTextColor(window_main_dc);
				tmode = GetBkMode(window_main_dc);

				SetTextColor(window_main_dc, 0x00dd00);
				SetBkMode(window_main_dc,    TRANSPARENT);

				Rectangle(window_main_dc, 0, 0, rct.right - 1, rct.bottom - 1);
				rct.left = rct.top = 2;
				DrawText(window_main_dc, dtxt, (int)str_len(dtxt), &rct, DT_CENTER);

				SetTextColor(window_main_dc, otc);
				SetBkMode(window_main_dc,    tmode);

				SelectObject(window_main_dc, hobr);
				DeleteObject(hbr);
			}
		}
		break;

	case WM_KEYDOWN:

		if(wParam == VK_F1) basewindows_show_about(1);

		if(wParam == VK_F12)
		{
			if(!settings.skins.selected[0])
			{
				letter   realpath[v_sys_maxpath];

				fennec_get_abs_path(realpath, uni("/skins/skin player") v_sys_lbrary_extension);
				skins_apply(realpath);
			}
		}

		if(wParam == VK_ESCAPE)
		{
			RECT  rct;
			POINT cpt;

			GetWindowRect(window_main, &rct);
			cpt.x = rct.left + ((rct.right - rct.left) / 2);
			cpt.y = rct.top  + ((rct.bottom - rct.top) / 2);

			TrackPopupMenu(menu_main, TPM_CENTERALIGN | TPM_VCENTERALIGN, cpt.x, cpt.y, 0, window_main, 0);
		}

		if(!settings.shortcuts.enable_local)
		{
			if(wParam == VK_F2) settings_ui_show();
			break;
		}
	
		{
			unsigned short a;
			a = fennec_convertkey((unsigned short*)&wParam);

			if(GetKeyState(VK_SHIFT)   & 0x8000) a |= fennec_key_shift;
			if(GetKeyState(VK_CONTROL) & 0x8000) a |= fennec_key_control;
			if(GetKeyState(VK_MENU)    & 0x8000) a |= fennec_key_alternative;
			
			kb_action(a, settings.shortcuts.localkeys, sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0]));
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCLOSE:
			if(fennec_power_set(fennec_power_mode_default) == 0)
				fennec_power_exit(); /* emergency exit */
			break;

		default:
			Menu_MainProc((long)wParam);
			break;
		}
		break;

	case (WM_USER + 123): /* notify icon */
		if(lParam == WM_RBUTTONUP)
		{
			POINT cpt;
			GetCursorPos(&cpt);

			SetForegroundWindow(hwnd);

			TrackPopupMenu(menu_main, TPM_LEFTALIGN | TPM_LEFTBUTTON, cpt.x, cpt.y, 0, window_main, 0);
		
			PostMessage(hwnd, WM_NULL, 0, 0);
		}else if(lParam == WM_LBUTTONUP){

			if(fennec_power_state == fennec_power_state_active)
				fennec_power_sleep();
			else
				fennec_power_wake();
		}
		break;

	case (WM_USER + 124): /* video test */
		 if(!settings.player.video_tested)
		{
			OSVERSIONINFO osvi;

			ZeroMemory(&osvi, sizeof(osvi));
			osvi.dwOSVersionInfoSize = sizeof(osvi);

			GetVersionEx(&osvi);

			if(osvi.dwMajorVersion > 5) /* Vista (& others maybe) has a DirectX video problem */
			{
				if(MessageBox(window_main, uni("Can you see the video properly?"), uni("Video Check"), MB_ICONQUESTION | MB_YESNO) != IDYES)
				{
					videoout_uninitialize(settings.videooutput.selected);
					sys_sleep(0);
					str_cpy(settings.videooutput.selected, uni("video out opengl.dll"));
					videoout_initialize(settings.videooutput.selected);
				}
				
				settings.player.video_tested = 1;
			}

		}
		break;

	case WM_DROPFILES:
		{
			unsigned int     fcount, i;
			letter           fpath[v_sys_maxpath];
			POINT            pt;
			HDROP            hd = (HDROP)wParam;
			HANDLE           hf;
			WIN32_FIND_DATA  fd;
			int              loadcommand = 0;
			
			fcount = DragQueryFile(hd, 0xFFFFFFFF, 0, 0);

			DragQueryPoint(hd, &pt);
			ClientToScreen(hwnd, &pt);

			if(WindowFromPoint(pt) == hwnd)
			{
				audio_playlist_clear();
				loadcommand = 1;
			}

			for(i=0; i<fcount; i++)
			{
				memset(fpath, 0, sizeof(fpath));
				DragQueryFile(hd, i, fpath, sizeof(fpath));

				hf = FindFirstFile(fpath, &fd);

				if(hf == INVALID_HANDLE_VALUE)continue;
				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					AddDirectory(fpath);
					continue;
				}

				FindClose(hf);

				/* add to playlist */
				if(!playlist_t_isplaylistfile(fpath))
				{
					audio_playlist_add(fpath, 0, 0);
				}else{
					playlist_t_load_current(fpath);
				}
			}

			if(loadcommand)
			{
				audio_playlist_switch(0);
				audio_play();
			}

			fennec_refresh(fennec_v_refresh_force_high);

			DragFinish(hd);
		}

		break;

	case WM_HOTKEY:
		{
			unsigned int i;
			for(i=0; i<sizeof(keys_globalids) / sizeof(keys_globalids[0]); i++)
			{
				if(wParam == (WPARAM)keys_globalids[i])
				{
					kb_action(settings.shortcuts.globalkeys[i].kcomb, (struct setting_key*) &settings.shortcuts.globalkeys, 256);
					break;
				}
			}
		}
		break;

	case WM_SYSCOMMAND:
		switch(wParam)
		{
		case SC_MINIMIZE:
			fennec_power_state |= fennec_power_state_minimized;
			fennec_power_state |= fennec_power_state_hidden;
			break;

		case SC_RESTORE:
			fennec_power_state &= ~fennec_power_state_minimized;
			fennec_power_state &= ~fennec_power_state_hidden;
			break;
		}
		break;

	case WM_COPYDATA:
		{
			COPYDATASTRUCT *cd = ((COPYDATASTRUCT*)lParam);

			switch(wParam)
			{
			case 1: /* command line */
				/* show the windows, if they were hidden */ fennec_power_wake();

				if(text_command_base((const string)((COPYDATASTRUCT*)lParam)->lpData))
				{
					audio_playlist_switch(0);
					audio_play();
				}
				break;

			case 2: /* add files */
				audio_playlist_add((const string)cd->lpData, 0, 0);
				fennec_refresh(fennec_v_refresh_force_high);
				return 1;

			case 3: /* clear playlist */
				audio_playlist_clear();
				fennec_refresh(fennec_v_refresh_force_high);
				return 1;

			case 4: /* switch into (int)indata */
				audio_playlist_switch((unsigned long)cd->dwData);
				audio_play();
				fennec_refresh(fennec_v_refresh_force_less);
				return 1;

			case 5: /* switch into list index (int)indata */
				audio_playlist_switch_list((unsigned long)cd->dwData);
				fennec_refresh(fennec_v_refresh_force_less);
				return 1;

			case 6: /* add directory */
				AddDirectory((const string)cd->lpData);
				fennec_refresh(fennec_v_refresh_force_high);
				return 1;

			case 7: /* start reporting */
				reporting_start();
				return 1;

			case 8: /* end reporting */
				reporting_end();
				return 1;
			}
		}
		break;

	default:
		switch(message)
		{
		case WM_ENDSESSION:
			fennec_power_set(fennec_power_mode_default);
			break;

		case WM_CREATE:
			fennec_initialize(hwnd);
			break;

		case WM_DESTROY:

			if(fennec_power_set(fennec_power_mode_default) == 0)
				fennec_power_exit(); /* emergency exit */

			break;
		}
		break;

	}

	if(skinproc)
	{
		if(skinproc(hwnd, message, wParam, lParam))
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}else{
			return DefWindowProc(hwnd, message, wParam, lParam);;
		}
	}
		
	return DefWindowProc(hwnd, message, wParam, lParam);
}



/*-----------------------------------------------------------------------------
 fennec.
-----------------------------------------------------------------------------*/
