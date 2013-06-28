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

#include <commctrl.h>
#include <errno.h>



/* prototypes ---------------------------------------------------------------*/

/* <dep> win */

BOOL CALLBACK callback_sleep_windows(HWND hwnd, LPARAM lParam);

/* </dep> */




/* data ---------------------------------------------------------------------*/

int fennec_power_state = fennec_power_state_active;
int power_sleep_firstcall;

/* <dep> win */

HWND                  hwnd_tip = 0;
extern NOTIFYICONDATA notify_data;

extern HWND window_conversion; /* 0 = closed */
extern HWND window_ripping;
extern HWND window_joining;


/* </dep> */




/* code ---------------------------------------------------------------------*/


/*
 * sleep (minimize to tray).
 */
void fennec_power_sleep(void)
{
	if(fennec_power_state == fennec_power_state_sleeping)return;
	power_sleep_firstcall = 1;

	report("sleeping", rt_stepping);

	/* <dep> win */

	EnumWindows((WNDENUMPROC)callback_sleep_windows, 0);

	/* </dep> */

	fennec_power_state = fennec_power_state_sleeping;
}


/*
 * wake (back to normal state).
 */
void fennec_power_wake(void)
{
	if(fennec_power_state == fennec_power_state_active)return;
	power_sleep_firstcall = 1;

	report("wake!", rt_stepping);

	/* <dep> win */

	if(IsIconic(window_main))
	{
		ShowWindow(window_main, SW_NORMAL);
	}else{
		EnumWindows((WNDENUMPROC)callback_sleep_windows, 1);
	}
	/* </dep> */

	fennec_power_state = fennec_power_state_active;
}


/*
 * power off (exit fennec).
 */
void fennec_power_exit(void)
{
	letter tmpbuf[v_sys_maxpath];
	unsigned int i = 0;

	report("start termination", rt_stepping);

	if(window_conversion)SendMessage(window_conversion, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
	Sleep(0);
	
	if(window_ripping)   SendMessage(window_ripping, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
	Sleep(0);

	if(window_joining)   SendMessage(window_joining, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
	Sleep(0);

	if(settings.visualizations.selected[0])
		visualizations_uninitialize();

	if(settings.videooutput.selected[0])
		videoout_uninitialize();

	encoder_uninitialize();

	if(settings.skins.selected[0] == 0)
	{
		/* no base skin */
	}else{
		skins_uninitialize();
	}

	audio_uninitialize();
	audio_input_uninitialize();
	audio_playlist_uninitialize();
	
	settings_uninitialize();

	fennec_get_plugings_path(tmpbuf);
	str_cat(tmpbuf, uni("settings.txt"));
	plugin_settings_save(tmpbuf);

	plugin_settings_save(tmpbuf);
	plugin_settings_unload();

	/* unregister global shortcut keys  */

	while(keys_globalids[i])
	{
		/* <dep> win */

		UnregisterHotKey(window_main, keys_globalids[i]);
		
		/* </dep> */

		i++;
	}

	/* <dep> win */

	Shell_NotifyIcon(NIM_DELETE, &notify_data);

	DestroyMenu(menu_main);
	DestroyWindow(hwnd_tip);

	lang_uninitialize();
	media_library_uninitialize();

	report("terminating process", rt_stepping);

	PostQuitMessage(0);
	ExitProcess(0);

	/* </dep> win */
	return;
}


/*
 * perform action.
 */
int fennec_power_set(unsigned long pmode)
{
	letter tmpbuf[v_sys_maxpath];

	switch(pmode)
	{
	case fennec_power_mode_default:
	case fennec_power_mode_save:

		/* save settings */

		settings_save();

		/* save settings of plug-ins */

		fennec_get_plugings_path(tmpbuf);
		str_cat(tmpbuf, uni("settings.txt"));
		plugin_settings_save(tmpbuf);

		/* save playlist */

		str_cpy(tmpbuf, fennec_get_path(0, 0));
		str_cat(tmpbuf, uni("/playlist.txt"));

		playlist_t_save_current(tmpbuf); /* save text playlist */

		/* exit! */

		fennec_power_exit();

		break;

	case fennec_power_mode_sleep:

		fennec_power_sleep();
		break;

	default:

		fennec_power_exit();
		break;
	}

	return 1;
}


/*
 * remove file name from file path
 * (up one level, i.e. "hdd/data/file.ext"
 * to "hdd/data"
 */
void get_file_path(string fpath)
{
	register unsigned int i = 0;
	register unsigned int j = 0;

	while(fpath[i])
	{
		if(fpath[i] == uni('/') || fpath[i] == uni('\\'))j = i;
		i++;
	}

	if(j)fpath[j] = 0;
}


/*
 * get plugins path.
 * i.e. "hdd/fennec/plugins/"
 */
void fennec_get_plugings_path(string mem)
{
	if(settings.plugins.input_path[0] == uni('\\') || settings.plugins.input_path[0] == uni('/'))
	{
		/* relative path */
		str_cpy(mem, fennec_get_path(0, 0));
		str_cat(mem, settings.plugins.input_path);
	}else{
		str_cpy(mem, settings.plugins.input_path);
	}
}


/*
 * get absolute path from a relative path.
 */
int fennec_get_abs_path(string apath, const string rpath)
{
	if((rpath[0] == uni('/') || rpath[0] == uni('\\')) &&
	   (rpath[1] != uni('/') && rpath[1] != uni('\\')))
	{
		str_cpy(apath, fennec_get_path(0, 0));
		str_cat(apath, rpath);
		return 1;
		
	}else{
		/* not a relative path */
		str_cpy(apath, rpath);
		return 0;
	}
}


/*
 * get relative path.
 */
int fennec_get_rel_path(string rpath, const string apath)
{
	string  fpath;
	size_t  fplen;
	size_t  fstart = 0;

	fpath = fennec_get_path(0, 0);
	fplen = str_len(fpath);

	if(str_incmp(fpath, apath, fplen) == 0)
		fstart = fplen;

	str_cpy(rpath, apath + fstart);

	return 1;
}


/*
 * get fennec path.
 * i.e. "hdd/fennec"
 */
string fennec_get_path(string out, size_t length)
{
	static   char    already_copied = 0;
	static   letter  buffer[v_sys_maxpath];
	register size_t  c;

	if(!out && already_copied)return buffer;
	if(!out)
	{
		c = (size_t)GetModuleFileName(0, buffer, (DWORD)sizeof(buffer));
		
		while(c)
		{
			if(buffer[c] == uni('/') || buffer[c] == uni('\\'))
			{
				buffer[c] = 0;
				break;
			}
			c--;
		}

		already_copied = 1;
		return buffer;
	}

	c = (size_t)GetModuleFileName(0, out, (DWORD)length);

	while(c)
	{
		if(out[c] == uni('/') || out[c] == uni('\\'))
		{
			out[c] = 0;
			break;
		}
		c--;
	}

	if(!already_copied)
	{
		str_mcpy(buffer, out, c + 1);
		already_copied = 1;
	}
	return buffer;
}


/*
 * conversion.
 */
int setting_priority_to_sys(int sp)
{
	switch(sp)
	{
	case setting_priority_idle:
	case setting_priority_lowest:      return v_sys_thread_idle;

	case setting_priority_low:         return v_sys_thread_low;        
	case setting_priority_below_normal: return v_sys_thread_belownormal;
	case setting_priority_normal:      return v_sys_thread_normal;
	case setting_priority_above_normal: return v_sys_thread_abovenormal;
	case setting_priority_high:        return v_sys_thread_high;

	case setting_priority_highest:
	case setting_priority_realtime:    return v_sys_thread_highest;
	}
	return v_sys_thread_normal;
}


/*
 * hsv to rgb conversion.
 */
void color_hsv_2_rgb_fullint(unsigned short h, unsigned char s, unsigned char v, unsigned char* r, unsigned char* g, unsigned char* b)
{
	unsigned char _a, _b, _c, pc;
	unsigned int osc;

	/* optimized edition (only fixed-point maths used). */

	pc  = (unsigned char)((int)h / 60);
	osc = (((h * 100) / 6)) - ((h / 60) * 1000);

	_a = (unsigned char)(((255 - s) * v) / 255);
	_b = (unsigned char)(((255 - ((s * osc) / 1000)) * v) / 255);
	_c = (unsigned char)(((255 - ((s * (1000 - osc)) / 1000)) * v) / 255);

	switch(pc)
	{
	case 0: *r =  v; *g = _c; *b = _a; break;
	case 1: *r = _b; *g =  v; *b = _a; break;
	case 2: *r = _a; *g =  v; *b = _c; break;
	case 3: *r = _a; *g = _b; *b =  v; break;
	case 4: *r = _c; *g = _a; *b =  v; break;
	case 5: *r =  v; *g = _a; *b = _b; break;
	}
}










/* not formatted --------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-----------------------------------------------------------------------------*/












letter  SinglePath_MemoryEx[0x8000]; /* memory for single path, 32K */
letter  SinglePath_Memory[0x8000];
string  LoadText_Memory = 0;
HGLOBAL LoadText_Handle;

int      tips_data;
TOOLINFO tips_tool;
UINT_PTR tips_timer;
UINT_PTR tips_dynamicpos;
int      tips_x, tips_y;

void str_replace(char* str, const char* dstr, const char* sstr);
UINT CALLBACK DlgHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
sys_thread_function_header(thread_dlgproc);





/*
 show file open/save dialog and return users selection.
*/

int dlg_opening = 0;
OPENFILENAME ofn;
HWND dlg_wnd = 0;

/*
 * Dialog hook
 */


UINT CALLBACK DlgHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
	case WM_INITDIALOG:
		dlg_wnd = GetParent(hdlg);
		SetWindowPos(dlg_wnd, 0, settings.environment.advanced_open_x, settings.environment.advanced_open_y, settings.environment.advanced_open_w, settings.environment.advanced_open_h, SWP_NOZORDER | SWP_NOSIZE);
		SetWindowLong(hdlg, DWL_MSGRESULT, 1);
		
		EnableWindow(GetDlgItem(GetParent(hdlg), 1), 0);
		EnableWindow(GetDlgItem(GetParent(hdlg), 2), 0);

		UpdateWindow(hdlg);

		SetWindowLong(hdlg, DWL_MSGRESULT, 1);
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_FASTCANCEL:
			dlg_wnd = GetParent(hdlg);
			ShowWindow(dlg_wnd, SW_HIDE);
			break;
		}
		return 1;

	case WM_SIZE:
		{
			RECT rct;
			RECT brct;
			HWND hwcb = GetDlgItem(hdlg, IDC_FASTCANCEL);
			GetClientRect(hwcb, &rct);
			GetClientRect(GetParent(hdlg), &brct);
			SetWindowPos(hwcb, 0, brct.right - rct.right - 12, brct.bottom - rct.bottom - 31, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		return 0;

	case WM_NOTIFY:
		{
			OFNOTIFY *oen;
			oen = (OFNOTIFY*)lParam;

			if(oen->hdr.code == CDN_FILEOK)
			{
				WINDOWPLACEMENT wpl;

				dlg_wnd = GetParent(hdlg);

				SinglePath_MemoryEx[0] = 0;
				SendMessage(dlg_wnd, CDM_GETFILEPATH, (WPARAM)sizeof(SinglePath_MemoryEx), (LPARAM)SinglePath_MemoryEx);
			
				 
				if(str_len(SinglePath_MemoryEx))
				{

					if(!IsDlgButtonChecked(hdlg, IDC_ADDTOPL))
					{
						audio_playlist_clear();
					}

					OpenFileListOld(SinglePath_MemoryEx);
					
					if(!IsDlgButtonChecked(hdlg, IDC_ADDTOPL))
					{
						audio_playlist_switch(0);

						if(settings.general.auto_play)
							audio_play();
					}

					fennec_refresh(fennec_v_refresh_force_not);
				}
				
				if(!IsDlgButtonChecked(hdlg, IDC_STAY))
				{
					ShowWindow(dlg_wnd, SW_HIDE);
				}

				GetWindowPlacement(dlg_wnd, &wpl);


				settings.environment.advanced_open_x = wpl.rcNormalPosition.left;
				settings.environment.advanced_open_y = wpl.rcNormalPosition.top;
				settings.environment.advanced_open_w = wpl.rcNormalPosition.right  - wpl.rcNormalPosition.left;
				settings.environment.advanced_open_h = wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top;

				SetWindowLong(hdlg, DWL_MSGRESULT, 1);
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

string fennec_file_dialog_ex(short issave, const string title, const string sfilter, const string initdir)
{
	letter idir[v_sys_maxpath];

	memset(&ofn, 0, sizeof(ofn));

	if(dlg_opening)
	{
		if(dlg_wnd)ShowWindow(dlg_wnd, SW_SHOW);
		return 0;
	}

	SinglePath_MemoryEx[0] = 0;

	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrTitle      = title;
	ofn.hwndOwner       = 0;
	ofn.lpstrFile       = SinglePath_MemoryEx;
	ofn.nMaxFile        = sizeof(SinglePath_MemoryEx);
	ofn.lpstrFilter     = sfilter;
	ofn.nFilterIndex    = 0;
	ofn.lpstrFileTitle  = 0;
	ofn.nMaxFileTitle   = 0;
	if(!initdir)
	{
		str_cpy(idir, settings.player.last_file);
		get_file_path(idir);

		ofn.lpstrInitialDir = idir;
	}else{
		ofn.lpstrInitialDir = 0;
	}
	ofn.lpfnHook        = (LPOFNHOOKPROC) DlgHook;
	ofn.lpTemplateName  = MAKEINTRESOURCE(IDD_OPEN);
	ofn.Flags           = OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLEHOOK;
	ofn.hInstance       = instance_fennec;

	if(!dlg_opening)
	{
		dlg_opening = 1;
		GetOpenFileName(&ofn);
		dlg_opening = 0;
	}
	return 0;
}

string fennec_file_dialog(short issave, const string title, const string sfilter, const string initdir)
{
	letter        idir[v_sys_maxpath];
	static OPENFILENAME  lofn;


	memset(&lofn, 0, sizeof(lofn));

	SinglePath_Memory[0] = 0;

	lofn.lStructSize       = sizeof(lofn);
	lofn.lpstrTitle        = title;
	lofn.hwndOwner         = GetFocus();
	lofn.lpstrFile         = SinglePath_Memory;
	lofn.nMaxFile          = sizeof(SinglePath_Memory) / sizeof(letter);
	lofn.lpstrFilter       = sfilter;


	if(!initdir)
	{
		str_cpy(idir, settings.player.last_file);
		get_file_path(idir);

		lofn.lpstrInitialDir = idir;
	}else{
		lofn.lpstrInitialDir = 0;
	}

	lofn.Flags           = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY;
	lofn.hInstance       = instance_fennec;

	if(issave)
	{
		GetSaveFileName(&lofn);
	}else{
		GetOpenFileName(&lofn);
	}
	return SinglePath_Memory;
}

/*
 Load text from resources.

 rtype : resource type, null to free allocated memory
 rname : resource name, null to free allocated memory

 return : pointer to data
*/

string fennec_loadtext(const string rtype, const string rname)
{
	HRSRC resfind;

	if(LoadText_Memory)
	{
		UnlockResource(LoadText_Handle);
		FreeLibrary(LoadText_Handle);	
		LoadText_Memory = 0;
	}
	if(!rname || !rtype)return 0;
	
	resfind = FindResource(instance_fennec, rname, rtype);
	LoadText_Handle = LoadResource(instance_fennec, resfind);
	LoadText_Memory = (string) LockResource(LoadText_Handle);

	return LoadText_Memory;
}


void AddDirectoryC(const string fpath)
{
	HANDLE           hf;
	WIN32_FIND_DATA  fd;
	unsigned long    ide = 0;
	letter           ext[128];
	letter           einfo[260];
	letter           dpath[v_sys_maxpath];
	letter           ipath[v_sys_maxpath];

	ext[0]   = 0;
	einfo[0] = 0;
	dpath[0] = 0;

	/* show wait window */

	while(audio_input_getextensionsinfo(ide, ext, einfo))
	{
		str_cpy(dpath, fpath);
		str_cat(dpath, uni("\\*."));
		str_cat(dpath, ext);

		hf = FindFirstFile(dpath, &fd);

		if(hf == INVALID_HANDLE_VALUE)goto extepoint;

		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		audio_playlist_add(ipath, 0, 0);

		basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);

		while(FindNextFile(hf, &fd))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			audio_playlist_add(ipath, 0, 0);

			basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);
		}

		FindClose(hf);

extepoint:
		ide++;
		ext[0]   = 0;
		dpath[0] = 0;
	}

	str_cpy(dpath, fpath);
	str_cat(dpath, uni("\\*"));

	hf = FindFirstFile(dpath, &fd);

	if(hf == INVALID_HANDLE_VALUE)return;

	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
	{
		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		AddDirectoryC(ipath);
	}

	while(FindNextFile(hf, &fd))
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			AddDirectoryC(ipath);
		}
	}

	FindClose(hf);
}

void AddDirectory(const string fpath)
{
	HANDLE           hf;
	WIN32_FIND_DATA  fd;
	unsigned long    ide = 0;
	letter           ext[128];
	letter           einfo[260];
	letter           dpath[v_sys_maxpath];
	letter           ipath[v_sys_maxpath];

	ext[0]   = 0;
	einfo[0] = 0;
	dpath[0] = 0;

	/* show wait window */

	basewindows_show_wait(window_main);
	basewindows_wait_function(wait_window_set_text, 0, uni("Please wait, searching for files..."));
	


	while(audio_input_getextensionsinfo(ide, ext, einfo))
	{
		str_cpy(dpath, fpath);
		str_cat(dpath, uni("\\*."));
		str_cat(dpath, ext);

		hf = FindFirstFile(dpath, &fd);

		if(hf == INVALID_HANDLE_VALUE)goto extepoint;

		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		audio_playlist_add(ipath, 0, 0);

		basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);
	

		while(FindNextFile(hf, &fd))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			audio_playlist_add(ipath, 0, 0);

			basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);
		}

		FindClose(hf);

extepoint:
		ide++;
		ext[0]   = 0;
		dpath[0] = 0;
	}

	str_cpy(dpath, fpath);
	str_cat(dpath, uni("\\*"));

	hf = FindFirstFile(dpath, &fd);

	if(hf == INVALID_HANDLE_VALUE)return;

	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
	{
		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		AddDirectoryC(ipath);
	}

	while(FindNextFile(hf, &fd))
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			AddDirectoryC(ipath);
		}
	}

	FindClose(hf);

	basewindows_wait_function(wait_window_end, 0, 0);
}

int OpenFileListOld(string fpaths)
{
	WIN32_FIND_DATA  fd;
	letter           spath[v_sys_maxpath];
	letter           apath[v_sys_maxpath];
	unsigned long    i = 0, j = 0, k, rs = 0;

	/* run till we find the first null */

	while(fpaths[i])
	{
		if(fpaths[i] == uni('\"'))
		{
			fpaths[i] = 0;
			rs = 1;
		}
		i++;
	}

	i = 0;

	while(fpaths[i])i++;
	i -= rs; /* no '\' if quotes found */

	str_mcpy(spath, fpaths, i);
	spath[i] = 0;

	j = i + 1;

	/* if the path is a directory, we'll have to add multiple files */

	
	if(j > 4)
	{
		if(FindFirstFile(spath, &fd) == INVALID_HANDLE_VALUE)return 0;
	}else{
		goto add_selected;
	}
	
	if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		/* just open the file */

		audio_playlist_add(spath, 0, 0);
		return 1;
	}else{

add_selected:

		/* oh, the hard way */

		for(;;)
		{

			str_mcpy(apath, spath, i);
			apath[i] = uni('\\');
			k = (unsigned long)str_len(fpaths + j + 1);
			if(!k)return 1;

			
			str_mcpy(apath + i + 1, fpaths + j + 1, k);
			apath[i + k + 1] = 0;
				
			j += k + 3;

			audio_playlist_add(apath, 0, 0);

		}
	}
}

int OpenFileList(const string fpaths)
{
	WIN32_FIND_DATA fd;
	letter          spath[v_sys_maxpath];
	letter          apath[v_sys_maxpath];
	unsigned long i = 0, j = 0, k;

	/* run till we find the first null */


	while(fpaths[i])i++;

	str_mcpy(spath, fpaths, i);
	spath[i] = 0;

	j = i + 1;

	/* if the path is a directory, we'll have to add multiple files */

	
	if(j > 4)
	{
		if(FindFirstFile(spath, &fd) == INVALID_HANDLE_VALUE)return 0;
	}else{
		goto add_selected;
	}
	
	if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		/* just open the file */

		audio_playlist_add(spath, 0, 0);
		str_cpy(settings.player.last_file, spath);
		return 1;
	}else{

add_selected:

		/* oh, the hard way */

		for(;;)
		{

			str_mcpy(apath, spath, i);
			apath[i] = uni('\\');
			k = (unsigned long)str_len(fpaths + j);
			if(!k)return 1;

			
			str_mcpy(apath + i + 1, fpaths + j, k);
			apath[i + k + 1] = 0;
				
			j += k + 1;

			audio_playlist_add(apath, 0, 0);
			str_cpy(settings.player.last_file, apath);

		}
	}
}

void CALLBACK tips_timer_proc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	/*if(GetForegroundWindow() != window_main)
	{
		DWORD  pid;
		
		GetWindowThreadProcessId(GetForegroundWindow(), &pid);

		if(pid != GetCurrentProcessId())
		{
			Beep(1000, 100);
			SendMessage(hwnd_tip, TTM_TRACKACTIVATE, (WPARAM)0, (LPARAM)&tips_tool);
		}
	}*/

	if(tips_data >= 1)
	{
		POINT pt;
	
		GetCursorPos(&pt);

		if(((pt.x <= tips_x + 2) && (pt.x >= tips_x - 2)) && 
		   ((pt.y <= tips_y + 2) && (pt.y >= tips_y - 2)))
		{
			SendMessage(hwnd_tip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELPARAM(pt.x + 16, pt.y + 16));
			SendMessage(hwnd_tip, TTM_TRACKACTIVATE, (WPARAM)1, (LPARAM)&tips_tool);	
			tips_data--;
		}else{
			tips_data = 0;
		}
	}else{
		SendMessage(hwnd_tip, TTM_TRACKACTIVATE, (WPARAM)0, (LPARAM)&tips_tool);
	}
}

int tips_create(void)
{
	INITCOMMONCONTROLSEX icex;

    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_BAR_CLASSES;

    if(!InitCommonControlsEx(&icex))
       return 0;
	   
    hwnd_tip = CreateWindow(TOOLTIPS_CLASS, uni(""),
                          WS_POPUP,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, (HMENU)NULL, instance_fennec,
                          NULL);

    tips_tool.cbSize    = sizeof(TOOLINFO);
    tips_tool.uFlags    = TTF_TRACK | TTF_ABSOLUTE;
    tips_tool.hwnd      = window_main;
    tips_tool.uId       = 423;
    tips_tool.hinst     = instance_fennec;
    tips_tool.lpszText  = uni("tip");
    tips_tool.rect.left = tips_tool.rect.top = tips_tool.rect.bottom = tips_tool.rect.right = 0; 

    if(!SendMessage(hwnd_tip, TTM_ADDTOOL, 0, (LPARAM)&tips_tool))
        return 0;
 
    SendMessage(hwnd_tip, TTM_TRACKACTIVATE, (WPARAM)0, (LPARAM)&tips_tool);

	tips_timer = SetTimer(0, 903245, 10, (TIMERPROC)tips_timer_proc);

	SendMessage(hwnd_tip, TTM_SETMAXTIPWIDTH, 0, 300);

	tips_data = 0;
	tips_dynamicpos = 1;
	return 1;
}


int tips_display(int x, int y, string msg)
{
	static string lmsg = 0;
	POINT pt;

	if(!msg)
	{
		lmsg = lmsg;
		SendMessage(hwnd_tip, TTM_TRACKACTIVATE, (WPARAM)0, (LPARAM)&tips_tool);
		return 1;
	}

	GetCursorPos(&pt);
	
	tips_x = pt.x;
	tips_y = pt.y;

	if(msg != lmsg)
	{		
		SendMessage(hwnd_tip, TTM_TRACKACTIVATE, (WPARAM)0, (LPARAM)&tips_tool);
		KillTimer(0, tips_timer);
	}else{
		return 1;
	}

	lmsg = msg;

    tips_tool.cbSize    = sizeof(TOOLINFO);
    tips_tool.uFlags    = TTF_TRACK | TTF_ABSOLUTE;
    tips_tool.hwnd      = window_main;
    tips_tool.uId       = 423;
    tips_tool.hinst     = instance_fennec;
    tips_tool.lpszText  = msg;
    tips_tool.rect.left = tips_tool.rect.top = tips_tool.rect.bottom = tips_tool.rect.right = 0; 

	SendMessage(hwnd_tip, TTM_UPDATETIPTEXT, 0, (LPARAM)&tips_tool);

	tips_data = 40;

	tips_timer = SetTimer(0, 903245, 1000, (TIMERPROC)tips_timer_proc);
	return 1;
}

BOOL CALLBACK callback_sleep_windows(HWND hwnd, LPARAM lParam)
{
	DWORD        pid;
	static HWND *wnds = 0;
	static int   wndc = 0;
	static int   wndz = 0;

	GetWindowThreadProcessId(hwnd, &pid);

	if(power_sleep_firstcall && (lParam == 0))
	{	
		if(wnds) sys_mem_free(wnds);
		wnds = 0;
		wndz = 0;
		wndc = 0;
		power_sleep_firstcall = 0;
	}
	
	if(pid == GetCurrentProcessId())
	{
		if(lParam == 0)
		{
		
			if(IsWindowVisible(hwnd))
			{
				ShowWindow(hwnd, SW_HIDE);
			}else{
				if(!wnds || !wndc)
				{
					wndz = 2;
					wndc = 0;
					wnds = (HWND*) sys_mem_alloc(wndz * sizeof(HWND));
				}else if(wndc >= wndz){
					wndz += 2;
					wnds = (HWND*) sys_mem_realloc(wnds, wndz * sizeof(HWND));
				}

				wnds[wndc] = hwnd;
				
				wndc++;
			}

		}else{ /* show */

			int i;

			for(i = 0;  i < wndc;  i++)
				if(hwnd == wnds[i])return 1;

			ShowWindow(hwnd, SW_SHOW);
		}
	}
	return 1;
}


int fennec_create_shortcut(const char *obj, const char *lnk, const char *dsc)
{
   
	return 1;
}

int local_reg_makepath(HKEY pkey, const string spath, const string svalue, const string sdata)
{
	HKEY    hksub = 0;
	size_t  i = 0;
	letter  tpath[v_sys_maxpath];
	int     epos = 0;

	str_cpy(tpath, spath);

	for(;;)
	{
		if(tpath[i] == uni('\\') || tpath[i] == uni('/') || tpath[i] == uni('\0'))
		{
			if(tpath[i] == uni('\0')) epos = 1;

			if(hksub) RegCloseKey(hksub);

			tpath[i] = uni('\0');
			RegCreateKey(pkey, tpath, &hksub);
			tpath[i] = uni('\\');

			if(epos == 1)break;
		}
		
		i++;
	}

	RegSetValueEx(hksub, svalue, 0, REG_SZ, (const BYTE*)sdata, (DWORD)(str_size(sdata) + sizeof(letter)));

	if(hksub) RegCloseKey(hksub);

	return 1;
}

int fennec_register_contextmenu(int regc)
{
	if(regc) /* register */
	{
		letter  sdllpath[v_sys_maxpath];

		/*
		model registry script
		---------------------

		HKCR - *\shellex\ContextMenuHandlers\fennec\ = {2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}
		HKCR - Directory\shellex\ContextMenuHandlers\fennec\ = {2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}

		HKCR - CLSID\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\                                = Fennec Player
		HKCR - CLSID\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\InprocServer32\                 = \fennec shell.dll
		HKCR - CLSID\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\InprocServer32\ThreadingModel   = Apartment
		HKCR - CLSID\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\ProgID\                         = fennec.1
		HKCR - CLSID\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\VersionIndependentProgID\       = fennec

		HKCR - fennec\CLSID = {2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}
		HKCR - fennec.1\CLSID = {2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}

		HKLM\Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\ = Fennec Player
		
		*/

		fennec_get_abs_path(sdllpath, uni("\\fennec shell.dll"));
		
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("*\\shellex\\ContextMenuHandlers\\fennec\\"), uni(""), uni("{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"));
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("Directory\\shellex\\ContextMenuHandlers\\fennec\\"), uni(""), uni("{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"));
	
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\"), uni(""), uni("Fennec Player"));
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\InprocServer32"), uni(""), sdllpath);
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\InprocServer32"), uni("ThreadingModel"), uni("Apartment"));
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\ProgID"), uni(""), uni("fennec.1"));
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\VersionIndependentProgID"), uni(""), uni("fennec"));
	
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("fennec\\CLSID"), uni(""), uni("{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"));
		local_reg_makepath(HKEY_CLASSES_ROOT, uni("fennec.1\\CLSID"), uni(""), uni("{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"));
	
		local_reg_makepath(HKEY_LOCAL_MACHINE, uni("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"), uni(""), uni("Fennec Player"));
	

	}else{ /* unregister */
		
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("*\\shellex\\ContextMenuHandlers\\fennec"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("Directory\\shellex\\ContextMenuHandlers\\fennec"));
		
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\InprocServer32"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\ProgID"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}\\VersionIndependentProgID"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("CLSID\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"));
		
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("fennec\\CLSID"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("fennec"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("fennec.1\\CLSID"));
		RegDeleteKey(HKEY_CLASSES_ROOT, uni("fennec.1"));

		RegDeleteKey(HKEY_LOCAL_MACHINE, uni("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved\\{2625DEE6-B52B-4b51-AC1B-3DCA7A50B080}"));
	} /* </register/unregister> */

	return 1;
}

int __stdcall fennec_control_function(unsigned long id, void *indata, void *outdata)
{
	switch(id)
	{
	case 1: /* add files */
		return audio_playlist_add((const string)indata, 0, 0);

	case 2: /* clear playlist */
		return audio_playlist_clear();

	case 3: /* switch into (int)indata */
		return audio_playlist_switch(PtrToUlong(indata));

	case 4: /* switch into list index (int)indata */
		return audio_playlist_switch_list(PtrToUlong(indata));

	case 5:  /* add to conversion/ripping/joining... */
	case 6:  /* clear conversion/ripping/joining... */
	case 7:  /* direct conversion/ripping/joining... */
	case 8:  /* format file name */
	case 9:  /* show tag editor */
	case 10: /* add to media library */
		return -1;
	}

	return -1;
}


int fennec_show_file_dialog(int id, int mode, string title, void *data)
{
	switch(id)
	{
	case file_dialog_openfiles:
	case file_dialog_addfiles:
		{
			unsigned int c, sw, switch_ret = 0;
			string       fname;

			if(id == file_dialog_openfiles)
				fname = basewindows_show_open(title ? title : uni("Open File(s)"), 0, mode == file_dialog_mode_experimental ? 1 : 0);
			else
				fname = basewindows_show_open(title ? title : uni("Add File(s) to Playlist"), 0, mode == file_dialog_mode_experimental ? 1 : 0);
			
			if(!fname) return -1; /* not selected */

			
			c = (unsigned int) str_len(fname);
			if(!c) return -1;  /* not selected */

			/* condition */
			if(id == file_dialog_openfiles)
				audio_playlist_clear();

			if(c >= 4)
			{
				int found = 0;

				if(playlist_t_isplaylistfile(fname))found = 1;

				if(found)
				{
					playlist_t_load_current(fname);
					fennec_refresh(fennec_v_refresh_force_high);
					switch_ret = audio_playlist_switch(0);
			
					sys_pass();

					/* condition */
					if(settings.general.auto_play && switch_ret)
						audio_play();
					break;  /* ok */
				}
			}
			
			sw = audio_playlist_getcount();
				
			OpenFileList(fname);

			/* condition */
			if(!sw || (id == file_dialog_openfiles) )
				switch_ret = audio_playlist_switch(0);
			

			sys_pass();

			/* condition */
			if(settings.general.auto_play && (id == file_dialog_openfiles) && switch_ret)
				audio_play();

			fennec_refresh(fennec_v_refresh_force_high);
		}
		break;

	}

	return 0;
}



int file_list_open_ex(const string fpaths, file_list_callback  fc)
{
	WIN32_FIND_DATA fd;
	letter          spath[v_sys_maxpath];
	letter          apath[v_sys_maxpath];
	unsigned long i = 0, j = 0, k;

	/* run till we find the first null */


	while(fpaths[i])i++;

	str_mcpy(spath, fpaths, i);
	spath[i] = 0;

	j = i + 1;

	/* if the path is a directory, we'll have to add multiple files */

	
	if(j > 4)
	{
		if(FindFirstFile(spath, &fd) == INVALID_HANDLE_VALUE)return 0;
	}else{
		goto add_selected;
	}
	
	if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		/* just open the file */

		fc(spath);
		return 1;
	}else{

add_selected:

		/* oh, the hard way */

		for(;;)
		{

			str_mcpy(apath, spath, i);
			apath[i] = uni('\\');
			k = (unsigned long)str_len(fpaths + j);
			if(!k)return 1;

			
			str_mcpy(apath + i + 1, fpaths + j, k);
			apath[i + k + 1] = 0;
				
			j += k + 1;

			fc(apath);

		}
	}
}


void file_list_open_dir_child(const string fpath, file_list_callback  fc)
{
	HANDLE           hf;
	WIN32_FIND_DATA  fd;
	unsigned long    ide = 0;
	letter           ext[128];
	letter           einfo[260];
	letter           dpath[v_sys_maxpath];
	letter           ipath[v_sys_maxpath];

	ext[0]   = 0;
	einfo[0] = 0;
	dpath[0] = 0;

	/* show wait window */

	while(audio_input_getextensionsinfo(ide, ext, einfo))
	{
		str_cpy(dpath, fpath);
		str_cat(dpath, uni("\\*."));
		str_cat(dpath, ext);

		hf = FindFirstFile(dpath, &fd);

		if(hf == INVALID_HANDLE_VALUE)goto extepoint;

		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		fc(ipath);
		basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);

		while(FindNextFile(hf, &fd))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			fc(ipath);
			basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);
		}

		FindClose(hf);

extepoint:
		ide++;
		ext[0]   = 0;
		dpath[0] = 0;
	}

	str_cpy(dpath, fpath);
	str_cat(dpath, uni("\\*"));

	hf = FindFirstFile(dpath, &fd);

	if(hf == INVALID_HANDLE_VALUE)return;

	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
	{
		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		file_list_open_dir_child(ipath, fc);
	}

	while(FindNextFile(hf, &fd))
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			file_list_open_dir_child(ipath, fc);
		}
	}

	FindClose(hf);
}

int file_list_open_dir(const string fpath, file_list_callback  fc)
{
	HANDLE           hf;
	WIN32_FIND_DATA  fd;
	unsigned long    ide = 0;
	letter           ext[128];
	letter           einfo[260];
	letter           dpath[v_sys_maxpath];
	letter           ipath[v_sys_maxpath];

	ext[0]   = 0;
	einfo[0] = 0;
	dpath[0] = 0;

	/* show wait window */

	basewindows_show_wait(window_main);
	basewindows_wait_function(wait_window_set_text, 0, uni("Please wait, searching for files..."));
	


	while(audio_input_getextensionsinfo(ide, ext, einfo))
	{
		str_cpy(dpath, fpath);
		str_cat(dpath, uni("\\*."));
		str_cat(dpath, ext);

		hf = FindFirstFile(dpath, &fd);

		if(hf == INVALID_HANDLE_VALUE)goto extepoint;

		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		fc(ipath);
		basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);
	

		while(FindNextFile(hf, &fd))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			fc(ipath);
			basewindows_wait_function(wait_window_set_detail, 0, (void*)ipath);
		}

		FindClose(hf);

extepoint:
		ide++;
		ext[0]   = 0;
		dpath[0] = 0;
	}

	str_cpy(dpath, fpath);
	str_cat(dpath, uni("\\*"));

	hf = FindFirstFile(dpath, &fd);

	if(hf == INVALID_HANDLE_VALUE)return 1;

	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
	{
		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		file_list_open_dir_child(ipath, fc);
	}

	while(FindNextFile(hf, &fd))
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			file_list_open_dir_child(ipath, fc);
		}
	}

	FindClose(hf);

	basewindows_wait_function(wait_window_end, 0, 0);

	return 1;
}



int path_make(const string pstr)
{
	register letter       bfs = 0;
	register unsigned int i = 0;
	letter                tempbuffer[1024];

	str_ncpy(tempbuffer, pstr, 1024);

	for(;;)
	{
		if(tempbuffer[i] == uni('\\') || tempbuffer[i] == uni('/') || !tempbuffer[i])
		{
			if(bfs) /* root */
			{
				bfs = tempbuffer[i];
				tempbuffer[i] = 0;

				if(str_mkdir(tempbuffer) == -1)
				{
					if(errno == ENOENT)return 0;
				}

				tempbuffer[i] = bfs;

			}else{
				bfs = 1;
			}

			if(!tempbuffer[i])break;
		}
		i++;
	}

	return (int)i + 1;
}


int path_make_co(string pstr)
{
	register letter       bfs = 0;
	register unsigned int i = 0;
	letter                tempbuffer[1024];
	unsigned int          len = (unsigned int)str_len(pstr);


	if((tempbuffer[len] != uni('\\')) && (tempbuffer[len] != uni('/')))
	{
		tempbuffer[len + 1] = uni('\\');
		tempbuffer[len + 2] = 0;
	}

	str_ncpy(tempbuffer, pstr, 1024);

	for(;;)
	{
		if(tempbuffer[i] == uni('\\') || tempbuffer[i] == uni('/') || !tempbuffer[i])
		{
			if(bfs) /* root */
			{
				bfs = tempbuffer[i];
				tempbuffer[i] = 0;

				if(str_mkdir(tempbuffer) == -1)
				{
					if(errno == ENOENT)return 0;
				}

				tempbuffer[i] = bfs;

			}else{
				bfs = 1;
			}

			if(!tempbuffer[i])break;
		}
		i++;
	}

	return (int)i + 1;
}

int skins_apply(const string rskin_path)
{
	if(skins_checkfile(rskin_path))
	{
		struct skin_data_return *rdata;
		int res;

		fennec_get_rel_path(settings.skins.selected, rskin_path);
		
		fennec_skin_data.finstance = instance_fennec;
		fennec_skin_data.wnd       = window_main;
		fennec_skin_data.wproc     = (WNDPROC)MainWndProc;
		fennec_skin_data.shared    = &fennec;

		res = skins_initialize(rskin_path, &fennec_skin_data);

		rdata = skins_getdata();
		
		if(rdata)
		{
			if(rdata->woptions)
			{
				SetWindowLong(window_main, GWL_STYLE, rdata->woptions);
				SetWindowPos(window_main, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			}
			skinproc = rdata->callback;
		}else{
			skinproc = 0;
		}
	}else{
		return -1;
	}

	return 0;
}

/*
 * load tracks from a CD/DVD.
 */
int playback_loadtracks(void)
{
	if(settings.player.selected_drive)
	{
		int     i, c = audio_input_drive_gettracks((char)settings.player.selected_drive);
		letter  buf[128];
		letter  buf2[32];
		letter  dvdvts[128];
		letter  vcddir[128];

		if(c > 0)
		{
			audio_playlist_clear();

			buf[0] = (letter)settings.player.selected_drive;
			str_cpy(buf + 1, uni(":\\Track 1.cda"));
			dvdvts[0] = (letter)settings.player.selected_drive;
			str_cpy(dvdvts + 1, uni(":\\VIDEO_TS"));
			vcddir[0] = (letter)settings.player.selected_drive;
			str_cpy(vcddir + 1, uni(":\\MPEGAV"));


			if(sys_fs_file_exist(buf)) /* probably an audio CD */
			{
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
			}else if(sys_fs_file_exist(dvdvts)){  /* probably a video DVD */

				buf[0] = (letter)settings.player.selected_drive;
				str_cpy(buf + 1, uni(":\\root.vob"));
				audio_load(buf);

			}else if(sys_fs_file_exist(vcddir)){  /* probably a VCD */

				str_cat(vcddir, uni("\\"));
				AddDirectory(vcddir);

			}else{  /* hmm... data CD/DVD? */
				
				buf[0] = (letter)settings.player.selected_drive;
				str_cpy(buf + 1, uni(":\\"));
				AddDirectory(buf);
			}
				
			fennec_refresh(fennec_v_refresh_force_high);
		}
	}
	return 0;
}


int inform_other_programs(string str)
{
	HWND           msnui = 0;
	COPYDATASTRUCT msndata;

	static letter  smtbuf[1024];
	letter          mtbuf[1024];

	if(!str)
	{
		str_cpy(mtbuf, uni("\\0Music\\00\\0{0} - {1}\\0\\0\\0\\0\\0"));
	}else{
		str_cpy(mtbuf, uni("\\0Music\\01\\0{0} - {1}\\0"));
		str_cat(mtbuf, str);
		str_cat(mtbuf, uni("\\0\\0\\0"));
	}

	//if(str_cmp(smtbuf, mtbuf) != 0)
	{
		str_cpy(smtbuf, mtbuf);

		msndata.dwData = 0x547;
		msndata.lpData = smtbuf;
		msndata.cbData = (str_len(smtbuf) * sizeof(letter)) + 2;

		msnui = FindWindowEx(0, msnui, uni("MsnMsgrUIManager"), 0);

		if(msnui)
		{
			SendMessage(msnui, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&msndata);
		}
	}

	return 0;
}


void fennec_check_updates(void)
{
#define updatesfile uni("http://fennec.sourceforge.net/updates.php");

	/* check if 7 days elapsed */

	letter   rvalue[128];
	DWORD    rsize = sizeof(rvalue), rtype = 0;
	HKEY     rkey;

	rvalue[0] = '\0';
	RegOpenKey(HKEY_LOCAL_MACHINE, uni("software\\fennec\\player"), &rkey);
	RegQueryValueEx(rkey, uni("lastupdate"), 0, &rtype, (LPBYTE)rvalue, &rsize);
	RegCloseKey(rkey);

	



}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
