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

#include "Fennec Main.h"

#define CMWINDOWCLASS uni("fennec 7.1 player 1.10000 main")

HWND   mainhwnd;
HINSTANCE hInst;

HWND window_active_dialog = 0;

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	MSG      msg;
	BOOL     mret;
	WNDCLASS wndclass;
	letter   rvalue[128];
	DWORD    rsize = sizeof(rvalue), rtype = 0;
	HKEY     rkey;

	reporting_start();


	rvalue[0] = '\0';
	
	RegOpenKey(HKEY_LOCAL_MACHINE, uni("software\\fennec\\player"), &rkey);
	RegQueryValueEx(rkey, uni("version"), 0, &rtype, (LPBYTE)rvalue, &rsize);
	RegCloseKey(rkey);

	if(!rsize || str_icmp((const string)rvalue, fennec_u_version_string))
	{
		/* if(!basewindows_show_license())return 0; */

		fennec_register_contextmenu(1);
	}

	report("loading settings", rt_stepping);

	if(GetKeyState(VK_F8) & 0x8000)
	{
		if(MessageBox(0, uni("Reset settings?"), uni("Reset - Fennec Player"), MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			settings_default();
		}else{
			settings_load();
		}
	
	}else{

		settings_load();
	}

	report("checking instances (allow multiple): %d", rt_stepping, settings.general.allow_multi_instances);

	/* sign in */

	CreateMutex(0, 0, uni("fennec 7.1 player 1.0 - startup instances check (00 00 00 01)"));
	
	/* don't add any function here */

	if(!settings.general.allow_multi_instances)
	{
		/* get information about the mutex */

		if(GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED)
		{
			HWND owindow = FindWindow(CMWINDOWCLASS, 0);
			if(owindow)
			{
				COPYDATASTRUCT cd;

				cd.cbData = (unsigned long)str_size(GetCommandLine());
				cd.lpData = GetCommandLine();

				if(cd.cbData)
					SendMessage(owindow, WM_COPYDATA, 1, (LPARAM)&cd);
			}

			report("termination after sending data", rt_stepping);
			return 0;
		}else{
			settings_verify();
		}

	}else{

		/* by the way, one clone is currently working, so the fail flag must be set */
		
		if(GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED)
		{
			/* just ignore it */
		}else{
			/* oh, this is the first instance, so there should be a fail exit last time */
			settings_verify();
		}
	}


		

	report("loading language pack", rt_stepping);
	
	if(lang_load(settings.language_packs.pack_path) < 0)
	{
		str_cpy(settings.language_packs.pack_path, uni("\\packs\\english.txt"));
		lang_load(settings.language_packs.pack_path);
	}

	switch(settings.general.base_priority)
	{
	case setting_priority_idle: 
	case setting_priority_lowest:
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
		break;

	case setting_priority_low:
	case setting_priority_below_normal:
	case setting_priority_normal:
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		break;

	case setting_priority_above_normal:
	case setting_priority_high:
	case setting_priority_highest:
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		break;

	case setting_priority_realtime:
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		break;
	}

	instance_fennec = hInstance;

	hInst = hInstance;

	report("registering main window", rt_stepping);

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = MainWndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (hInstance,(LPCTSTR) IDI_MAIN) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = CMWINDOWCLASS ;

	RegisterClass(&wndclass);

	mainhwnd = CreateWindowEx(WS_EX_ACCEPTFILES, CMWINDOWCLASS, uni("Fennec Player 7.1"), WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MINIMIZEBOX,settings.environment.main_window_x, settings.environment.main_window_y,400,100,NULL,NULL,hInstance,NULL) ;
		
	if(settings.environment.main_window_state == setting_window_minimized)
	{	
		ShowWindow(mainhwnd,SW_MINIMIZE);

	}else if(settings.environment.main_window_state == setting_window_hidden){
		ShowWindow(mainhwnd,SW_HIDE);

	}else{
		ShowWindow(mainhwnd,SW_NORMAL);
	}
		
	UpdateWindow(mainhwnd);

	report("created main window", rt_stepping);

    while((mret = GetMessage(&msg, 0, 0, 0)) != 0)
	{
		if(mret != -1 && window_active_dialog)
		{
			if(!IsWindow(window_active_dialog) || !IsDialogMessage(window_active_dialog, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}else{

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/
