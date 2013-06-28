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
#include <commctrl.h>
#include "keyboard.h"
#include "fennec audio.h"
#include "fennec help.h"
#include <shlobj.h>

/* structs ------------------------------------------------------------------*/

struct fennec_setting_panels
{
	string   title;
	string   description;
	short    i_title;
	short    i_description;
	DLGPROC  dproc;
	HWND     hwnd;
	int      dlgid;
	string   helpfilepath;
};

struct interface_settings
{
	struct
	{
		unsigned char multiple_instances;
		unsigned char auto_play;
		unsigned char scroll_title;
		unsigned char show_splash;
		unsigned char always_on_top;
		unsigned int  base_priority;
		unsigned int  threads_priority;
	}general;

	struct
	{
		letter        title_main[v_sys_maxpath];
		letter        title_scroll[v_sys_maxpath];
		letter        playlist_item[v_sys_maxpath];
	}formatting;

	struct
	{
		unsigned long   buffer_memory;
		unsigned long   output_device_id;
		unsigned long   bit_depth;
		unsigned long   bit_depth_float;
		unsigned long   bit_depth_signed;
		unsigned long   noise_reduction;
		unsigned long   perform_effects;
	}audio_output;
};

/* declarations -------------------------------------------------------------*/

void settings_ui_drawcaption(void);
void settings_ui_showpanel(unsigned int pid);

int settings_proc_main(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_general(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_files(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_skins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_formatting(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_shortcuts(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_internalout(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_internalin(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_inplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_outplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_dspplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_encoderplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_generalplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_visualplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_themes(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_language_packs(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int settings_proc_videoout(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);


void local_settag(struct fennec_audiotag_item *tagi, string tival);
void local_settings_to_interface(void);
void local_interface_to_settings(void);

/* data ---------------------------------------------------------------------*/

struct fennec_setting_panels fs_panels[] = 
{
	/* ** if you're changing this, don't forget to rewrite 'fennec_v_settings_ui_panel_xxxxx' defines (fennec main.h) */

	/* general */	{0, 0, oooo_settings_general,           oooo_settings_general_title,          (DLGPROC)settings_proc_general       , 0, IDD_SETTINGS_GENERAL       ,uni("/help/link/fennec help - preferences - general.html")},
	/* file a. */	{0, 0, oooo_settings_file_association,  oooo_settings_file_association_title, (DLGPROC)settings_proc_files         , 0, IDD_SETTINGS_FHANDLE       ,uni("/help/link/fennec help - preferences - file association.html")},
	/* skins   */	{0, 0, oooo_settings_skins,             oooo_settings_skins_title,            (DLGPROC)settings_proc_skins         , 0, IDD_SETTINGS_SKINS         ,uni("/help/link/fennec help - preferences - skins.html")},
	/* themes  */	{0, 0, oooo_settings_themes,            oooo_settings_themes_and_colors,      (DLGPROC)settings_proc_themes        , 0, dialog_settings_themes     ,uni("/help/link/fennec help - preferences - themes.html")},
	/* langua. */	{0, 0, oooo_settings_languages,         oooo_settings_languages_title,        (DLGPROC)settings_proc_language_packs, 0, dialog_settings_languages  ,uni("/help/link/fennec help - preferences - languages.html")},
	/* format. */	{0, 0, oooo_settings_formatting,        oooo_settings_formatting_title,       (DLGPROC)settings_proc_formatting    , 0, IDD_SETTINGS_FORMAT        ,uni("/help/link/fennec help - preferences - formatting.html")},
	/* shortc. */	{0, 0, oooo_settings_shortcutkeys,      oooo_settings_shortcutkeys_title,     (DLGPROC)settings_proc_shortcuts     , 0, IDD_SETTINGS_SHORTCUTS     ,uni("/help/link/fennec help - preferences - shortcut keys.html")},
	/* output. */	{0, 0, oooo_settings_internaloutput,    oooo_settings_internaloutput_title,   (DLGPROC)settings_proc_internalout   , 0, IDD_SETTINGS_IOUTPUT       ,uni("/help/link/fennec help - preferences - audio output.html")},
	/* out plg */	{0, 0, oooo_settings_outputplugins,     oooo_settings_outputplugins_title,    (DLGPROC)settings_proc_outplugins    , 0, IDD_SETTINGS_OUTPLUGINS    ,uni("/help/link/fennec help - preferences - output plugins.html")},
	/* decoder */	{0, 0, oooo_settings_inputplugins,      oooo_settings_inputplugins_title,     (DLGPROC)settings_proc_inplugins     , 0, IDD_SETTINGS_INPLUGINS     ,uni("/help/link/fennec help - preferences - decoders.html")},
	/* encoder */	{0, 0, oooo_settings_encoders,          oooo_settings_encoders_title,         (DLGPROC)settings_proc_encoderplugins, 0, IDD_SETTINGS_ENCODERS      ,uni("/help/link/fennec help - preferences - encoders.html")},
	/* effects */	{0, 0, oooo_settings_dsp,               oooo_settings_dsp_title,              (DLGPROC)settings_proc_dspplugins    , 0, IDD_SETTINGS_DSP           ,uni("/help/link/fennec help - preferences - dsp.html")},
	/* visualz */	{0, 0, oooo_settings_visualizations,    oooo_settings_visualizations_title,   (DLGPROC)settings_proc_visualplugins , 0, IDD_SETTINGS_VISUAL        ,uni("/help/link/fennec help - preferences - visualizations.html")},
	/* vidout  */	{uni("Video Output"), uni("Video Output Plugins"), 0, 0,   (DLGPROC)settings_proc_videoout , 0, dialog_settings_videoout        ,uni("/help/link/fennec help - preferences - video output.html")}

};
string keyactions[] = 
{
uni("Play"),
uni("Pause"),
uni("Stop"),
uni("Load"),
uni("Rewind"),
uni("Forward"),
uni("Previous"),
uni("Next"),
uni("Eject"),
uni("Select"),
uni("Switch Panel Main"),
uni("Switch Panel Color"),
uni("Switch Panel Visualization"),
uni("Switch Panel Equalizer"),
uni("Switch Panel Media Info"),
uni("Switch Panel Playlist"),
uni("Next Panel"),
uni("Previous Panel"),
uni("Exit"),
uni("Sleep"),
uni("Minimize"),
uni("Refresh"),
uni("Conversion"),
uni("Ripping"),
uni("Joining"),
uni("Visualization"),
uni("Playlist"),
uni("Volume Up"),
uni("Volume Down"),
uni("Volume Up (Automatic)"),
uni("Volume Down (Automatic)"),
uni("Volume Minimum"),
uni("Volume Maximum"),
uni("Add File"),
uni("Fast Load"),
uni("Fast Add File"),
uni("Preferences"),
uni("Keyboard Viewer"),
uni("Tag Editor"),
uni("Switch Playlist"),
uni("Playlist - Auto Switching"),
uni("Playlist - Shuffle"),
uni("Playlist - Information"),
uni("Playlist - Repeat All"),
uni("Playlist - Repeat Single"),
uni("Playlist - Insert"),
uni("Playlist - Insert Directory"),
uni("Playlist - Remove"),
uni("Switch Main Window"),
uni("")};

#ifndef PtrToInt
#    define PtrToInt(a)((int) a)
#endif

#define panels_count     (sizeof(fs_panels) / sizeof(fs_panels[0]))
#define headerbar_width  350
#define headerbar_height 23
#define headerbar_x      160
#define headerbar_y      12

#define msg_settings_update WM_USER + 21

HWND           hwnd_settings_main = 0;
HDC            memdc_headerbar = 0;
HBITMAP        bmp_headerbar;
unsigned long *mem_headerbar;
HDC            dc_main;
unsigned int   current_panel = 0;

long          *panel_fh_iconids = 0;

struct fennec_audiotag formatting_sample_tag;
struct interface_settings current_settings;

unsigned char  settings_active = 0;

HWND           list_localsk, currenthwnd;
HWND           list_globalsk;
/* functions ----------------------------------------------------------------*/

/*
 * create and show the settings window.
 */
int settings_ui_show_ex(unsigned int pid)
{
	unsigned int   i;
	TVINSERTSTRUCT tvis;
	TVITEM         tvi;
	COLORREF       c3sh,c3lg,c3fc;
	int            x,y,rval,gval,bval;
	COLORREF       cval;
	HTREEITEM      parent_general, parent_skins, parent_ui, parent_shortcuts, parent_audio, parent_plugins, parent_video;

	if(settings_active)
	{
		SetFocus(hwnd_settings_main);
		return 1;
	}

	/* <lang> */

	for(i=0; i<panels_count; i++)
	{
		if(!fs_panels[i].title)
			fs_panels[i].title       = text(fs_panels[i].i_title);
		if(!fs_panels[i].description)
			fs_panels[i].description = text(fs_panels[i].i_description);
	}

	/* </lang> */

	hwnd_settings_main = CreateDialog(instance_fennec, (LPCTSTR)IDD_OPTIONS, 0, (DLGPROC)settings_proc_main);
	ShowWindow(hwnd_settings_main, SW_SHOW);

	window_active_dialog = hwnd_settings_main;
	
	local_settings_to_interface();

	/* add tree view items */

	tvi.mask     = TVIF_TEXT | TVIF_PARAM;
	tvis.hParent = 0;	
   

	tvi.pszText  = text(oooo_settings_group_general);
	tvi.lParam   = (LPARAM)-1;
	tvis.item    = tvi;
	parent_general = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	
	tvi.pszText  = text(oooo_settings_group_skins);
	tvis.item    = tvi;
	parent_skins = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	
	tvi.pszText  = text(oooo_settings_group_userinterface);
	tvis.item    = tvi;
	parent_ui = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	
	tvi.pszText  = text(oooo_settings_group_shortcuts);
	tvis.item    = tvi;
	parent_shortcuts = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	
	tvi.pszText  = text(oooo_settings_group_audio);
	tvis.item    = tvi;
	parent_audio = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	
	tvi.pszText  = text(oooo_settings_group_plugins);
	tvis.item    = tvi;
	parent_plugins = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	
	tvi.pszText  = uni("Video");
	tvis.item    = tvi;
	parent_video = (HTREEITEM)SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	


	tvis.hParent = parent_general;	

	for(i=0; i<panels_count; i++)
	{
		switch(i)
		{
		case 0:case 1: tvis.hParent = parent_general; break;
		case 2:case 3: tvis.hParent = parent_skins; break;
		case 4:case 5: tvis.hParent = parent_ui; break;
		case 6: tvis.hParent = parent_shortcuts; break;
		case 7: tvis.hParent = parent_audio; break;
		case 8: case 9: case 10: case 11: case 12: tvis.hParent = parent_plugins; break;
		case 13:  tvis.hParent = parent_video; break;
		}

		tvi.pszText  = fs_panels[i].title;
		tvi.lParam   = i;
		tvis.item    = tvi;
		SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	}

	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_general);
	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_skins);
	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_ui);
	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_shortcuts);
	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_audio);
	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_plugins);
	SendDlgItemMessage(hwnd_settings_main, IDC_OPT_TABSEL, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)parent_video);

	c3sh = GetSysColor(COLOR_3DSHADOW);
	c3lg = GetSysColor(COLOR_3DLIGHT);
	c3fc = GetSysColor(COLOR_3DFACE);
	
	memdc_headerbar = CreateCompatibleDC(0);

	mem_headerbar = sys_mem_alloc(headerbar_width * headerbar_height * sizeof(unsigned long));

	for(x=0; x<headerbar_width; x++)
	{
		bval = (int)((int)GetRValue(c3sh) + ((float)x / (float)headerbar_width) * (GetRValue(c3fc) - GetRValue(c3sh)));
		gval = (int)((int)GetGValue(c3sh) + ((float)x / (float)headerbar_width) * (GetGValue(c3fc) - GetGValue(c3sh)));
		rval = (int)((int)GetBValue(c3sh) + ((float)x / (float)headerbar_width) * (GetBValue(c3fc) - GetBValue(c3sh)));

		cval = RGB(rval, gval, bval);

		for(y=0; y<headerbar_height; y++)
		{
			mem_headerbar[(y * headerbar_width) + x] = cval;	
		}
	}
	
	bmp_headerbar = CreateBitmap(headerbar_width, headerbar_height, 1, 32, mem_headerbar);
	SelectObject(memdc_headerbar, bmp_headerbar);
		
	settings_ui_showpanel(pid);

	settings_active = 1;
	return 1;
}

int settings_ui_show(void)
{
	return settings_ui_show_ex(fennec_v_settings_ui_panel_general);
}

/*
 * 
 */
int settings_ui_clean(void)
{
	if(memdc_headerbar)
	{
		sys_mem_free(mem_headerbar);
		DeleteObject(bmp_headerbar);
		DeleteDC(memdc_headerbar);
	}
	return 1;
}

/*
 * 
 */
void settings_ui_drawcaption(void)
{
	COLORREF c3sh,c3lg,c3fc;

	if(current_panel >= panels_count)return;

	c3sh = GetSysColor(COLOR_3DSHADOW);
	c3lg = GetSysColor(COLOR_3DLIGHT);
	c3fc = GetSysColor(COLOR_3DFACE);

	BitBlt(dc_main, headerbar_x, headerbar_y, headerbar_width, headerbar_height, memdc_headerbar, 0, 0, SRCCOPY);

	SetBkMode   (dc_main, TRANSPARENT);
	SetTextColor(dc_main, c3sh);
	TextOut     (dc_main, headerbar_x + 5, headerbar_y + 3, fs_panels[current_panel].description, (int)str_len(fs_panels[current_panel].description));
	SetTextColor(dc_main, c3lg);
	TextOut     (dc_main, headerbar_x + 5, headerbar_y + 3, fs_panels[current_panel].description, (int)str_len(fs_panels[current_panel].description));
	return;
}

/*
 * 
 */
void settings_ui_showpanel(unsigned int pid)
{
	if(pid >= panels_count)return;
	if(!fs_panels[pid].dlgid)return;

	if(current_panel < panels_count)
	{
		SendMessage(fs_panels[current_panel].hwnd, msg_settings_update, 1, 0);
		sys_pass();
		DestroyWindow(fs_panels[current_panel].hwnd);
	}

	fs_panels[pid].hwnd = CreateDialog(instance_fennec, (LPCTSTR)MAKEINTRESOURCE(fs_panels[pid].dlgid), hwnd_settings_main, (DLGPROC)fs_panels[pid].dproc);
	SetWindowPos(fs_panels[pid].hwnd, 0, 160, 50, 0, 0, SWP_NOSIZE);
	ShowWindow(fs_panels[pid].hwnd, SW_SHOW);

	current_panel = pid;
	settings_ui_drawcaption();
}



/* local functions ----------------------------------------------------------*/

/*
 * 
 */
int settings_proc_main(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(current_panel < panels_count)
			{
				SendMessage(fs_panels[current_panel].hwnd, msg_settings_update, 2, 0);
				sys_pass();
			}

			local_interface_to_settings();
			settings_save();

		case IDCANCEL:
			settings_active = 0;
			settings_ui_clean();
			current_panel = (unsigned int)-1;
			EndDialog(hwnd, 0);
			break;

		case IDC_OPT_APPLY:
			if(current_panel < panels_count)
			{
				SendMessage(fs_panels[current_panel].hwnd, msg_settings_update, 2, 0);
				sys_pass();
			}

			local_interface_to_settings();
			settings_save();
			break;

		case IDC_OPT_HELP:
			{
				letter  fpath[v_sys_maxpath];

				str_cpy(fpath, fennec_get_path(0, 0));

				if(current_panel != -1)
					str_cat(fpath, fs_panels[current_panel].helpfilepath);
				else
					str_cat(fpath, uni("help/link/fennec help - preferences window.html"));

				ShellExecute(hwnd, 0, fpath, 0, 0, SW_SHOW);
			}
			break;

		case IDC_OPT_DEFAULTS:
			settings_default();
			settings_save();
			settings_active = 0;
			settings_ui_clean();
			current_panel = (unsigned int)-1;
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_PAINT:
		settings_ui_drawcaption();
		break;

	case WM_NOTIFY :
		switch(((LPNMHDR)lParam)->idFrom)
		{
		case IDC_OPT_TABSEL:
			if(((LPNMHDR)lParam)->code == TVN_SELCHANGED)
			{
				unsigned int md = ((LPNMTREEVIEW)lParam)->action;
				if(md == TVC_BYMOUSE || md == TVC_BYKEYBOARD)
					settings_ui_showpanel((unsigned int)(((LPNMTREEVIEW)lParam)->itemNew.lParam));
			
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		dc_main = GetDC(hwnd);
		local_settings_to_interface();

		/* <lang> */

		SetDlgItemText(hwnd, IDC_OPT_HELP,     text(oooo_help));
		SetDlgItemText(hwnd, IDC_OPT_DEFAULTS, text(oooo_defaults));
		SetDlgItemText(hwnd, IDC_OPT_APPLY,    text(oooo_apply));
		SetDlgItemText(hwnd, IDOK,             text(oooo_ok));
		SetDlgItemText(hwnd, IDCANCEL,         text(oooo_cancel));

		/* </lang> */

		break;

	case WM_DESTROY:
		settings_active = 0;
		current_panel = (unsigned int)-1;
		EndDialog(hwnd,0);
		break;
	}

	return 0;
}

/*
 * 
 */
int settings_proc_general(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_G_BP_IDLE:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_BP_LOW:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_BP_NORMAL:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_BP_HIGH:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_BP_REALTIME:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_CHECKED  , 0);
			break;



		case IDC_G_TP_IDLE:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_TP_LOW:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_TP_NORMAL:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_TP_HIGH:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		case IDC_G_TP_REALTIME:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_CHECKED  , 0);
			break;
		}
		break;

	case WM_INITDIALOG:

		/* <lang> */

		SetDlgItemText(hwnd, IDC_G_AMINSTANCES,   text(oooo_settings_allow_multi_instances));
		SetDlgItemText(hwnd, IDC_G_AUTOPLAY,      text(oooo_settings_auto_play));
		SetDlgItemText(hwnd, IDC_G_SCROLLT,       text(oooo_settings_scroll_taskbar));
		SetDlgItemText(hwnd, IDC_G_AOT,           text(oooo_settings_always_on_top));
		SetDlgItemText(hwnd, static_base_prio,    text(oooo_settings_base_priority));
		SetDlgItemText(hwnd, static_threads_prio, text(oooo_settings_threads_priority));
		SetDlgItemText(hwnd, IDC_G_BP_IDLE,       text(oooo_settings_idle));
		SetDlgItemText(hwnd, IDC_G_BP_LOW,        text(oooo_settings_low));
		SetDlgItemText(hwnd, IDC_G_BP_NORMAL,     text(oooo_settings_normal));
		SetDlgItemText(hwnd, IDC_G_BP_HIGH,       text(oooo_settings_high));
		SetDlgItemText(hwnd, IDC_G_BP_REALTIME,   text(oooo_settings_realtime));
		SetDlgItemText(hwnd, IDC_G_TP_IDLE,       text(oooo_settings_idle));
		SetDlgItemText(hwnd, IDC_G_TP_LOW,        text(oooo_settings_low));
		SetDlgItemText(hwnd, IDC_G_TP_NORMAL,     text(oooo_settings_normal));
		SetDlgItemText(hwnd, IDC_G_TP_HIGH,       text(oooo_settings_high));
		SetDlgItemText(hwnd, IDC_G_TP_REALTIME,   text(oooo_settings_realtime));
		SetDlgItemText(hwnd, static_base_info,    text(oooo_settings_base_priority_info));
		SetDlgItemText(hwnd, static_threads_info, text(oooo_settings_threads_priority_info));

		/* </lang> */
		
		CheckDlgButton(hwnd, IDC_G_AMINSTANCES, current_settings.general.multiple_instances ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_G_AUTOPLAY   , current_settings.general.auto_play          ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_G_SCROLLT    , current_settings.general.scroll_title       ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_G_SPLASH     , current_settings.general.show_splash        ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_G_AOT        , current_settings.general.always_on_top      ? BST_CHECKED : BST_UNCHECKED);
		
		switch(current_settings.general.base_priority)
		{
		case setting_priority_idle:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case setting_priority_lowest:
		case setting_priority_low:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case setting_priority_below_normal:
		case setting_priority_normal:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);	
			break;

		case setting_priority_above_normal:
		case setting_priority_high:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);	
			break;

		case setting_priority_highest:
		case setting_priority_realtime:
			SendDlgItemMessage(hwnd, IDC_G_BP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_BP_REALTIME, BM_SETCHECK, BST_CHECKED  , 0);
			break;
		}

		switch(current_settings.general.threads_priority)
		{
		case setting_priority_idle:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case setting_priority_lowest:
		case setting_priority_low:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case setting_priority_below_normal:
		case setting_priority_normal:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);	
			break;

		case setting_priority_above_normal:
		case setting_priority_high:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_UNCHECKED, 0);	
			break;

		case setting_priority_highest:
		case setting_priority_realtime:
			SendDlgItemMessage(hwnd, IDC_G_TP_IDLE    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_LOW     , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_NORMAL  , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_HIGH    , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_G_TP_REALTIME, BM_SETCHECK, BST_CHECKED  , 0);
			break;
		}
		break;

	case msg_settings_update:

		current_settings.general.multiple_instances = (unsigned char)(IsDlgButtonChecked(hwnd, IDC_G_AMINSTANCES) == BST_CHECKED);
		current_settings.general.auto_play          = (unsigned char)(IsDlgButtonChecked(hwnd, IDC_G_AUTOPLAY   ) == BST_CHECKED);
		current_settings.general.scroll_title       = (unsigned char)(IsDlgButtonChecked(hwnd, IDC_G_SCROLLT    ) == BST_CHECKED);
		current_settings.general.show_splash        = (unsigned char)(IsDlgButtonChecked(hwnd, IDC_G_SPLASH     ) == BST_CHECKED);
		current_settings.general.always_on_top      = (unsigned char)(IsDlgButtonChecked(hwnd, IDC_G_AOT        ) == BST_CHECKED);
		
		if     (IsDlgButtonChecked(hwnd, IDC_G_BP_IDLE    ) == BST_CHECKED)current_settings.general.base_priority = setting_priority_idle;
		else if(IsDlgButtonChecked(hwnd, IDC_G_BP_LOW     ) == BST_CHECKED)current_settings.general.base_priority = setting_priority_low;
		else if(IsDlgButtonChecked(hwnd, IDC_G_BP_NORMAL  ) == BST_CHECKED)current_settings.general.base_priority = setting_priority_normal;
		else if(IsDlgButtonChecked(hwnd, IDC_G_BP_HIGH    ) == BST_CHECKED)current_settings.general.base_priority = setting_priority_high;
		else if(IsDlgButtonChecked(hwnd, IDC_G_BP_REALTIME) == BST_CHECKED)current_settings.general.base_priority = setting_priority_realtime;

		if     (IsDlgButtonChecked(hwnd, IDC_G_TP_IDLE    ) == BST_CHECKED)current_settings.general.threads_priority = setting_priority_idle;
		else if(IsDlgButtonChecked(hwnd, IDC_G_TP_LOW     ) == BST_CHECKED)current_settings.general.threads_priority = setting_priority_low;
		else if(IsDlgButtonChecked(hwnd, IDC_G_TP_NORMAL  ) == BST_CHECKED)current_settings.general.threads_priority = setting_priority_normal;
		else if(IsDlgButtonChecked(hwnd, IDC_G_TP_HIGH    ) == BST_CHECKED)current_settings.general.threads_priority = setting_priority_high;
		else if(IsDlgButtonChecked(hwnd, IDC_G_TP_REALTIME) == BST_CHECKED)current_settings.general.threads_priority = setting_priority_realtime;
			
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_files(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static letter bext[256][64], bdsc[256][128];
	switch(uMsg)
	{
	case WM_NOTIFY:
		switch(wParam)
		{
		case list_filetypes:

			switch(((LPNMHDR)lParam)->code)
			{
			case LVN_GETDISPINFO:
				{
				
				LV_DISPINFO *lvit = (LV_DISPINFO *)lParam;

				switch(lvit->item.iSubItem)
				{
				case 0:
					audio_input_getextensionsinfo(lvit->item.iItem, bext[lvit->item.iItem], bdsc[lvit->item.iItem]);
					lvit->item.pszText = bext[lvit->item.iItem];
					break;

				case 1:
					audio_input_getextensionsinfo(lvit->item.iItem, bext[lvit->item.iItem], bdsc[lvit->item.iItem]);
					lvit->item.pszText = bdsc[lvit->item.iItem];
					break;
				}
				break;

				}

			case LVN_ITEMCHANGED:
				{
					unsigned int    i;
					int             j;

					j = ListView_GetNextItem(GetDlgItem(hwnd, list_filetypes), -1, LVNI_SELECTED);

					if(j == -1)break;
					i = (unsigned int) j;

						
					{
						HICON hico;
						HWND  hidisplay;
						HDC   hiddc;
						int   id;
						int   micons;
						RECT  pr;
						int   sit;

						pr.left   = 0;
						pr.top    = 0;
						pr.right  = 48;
						pr.bottom = 48;

						sit = (int)SendDlgItemMessage(hwnd, list_filetypes, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
						
						if(sit != -1)
						{
							id = panel_fh_iconids[sit];
						}else{
							id = 0;
						}

						SendDlgItemMessage(hwnd, IDC_SICONID, TBM_SETPOS, 1, id);

						micons = PtrToInt(ExtractIcon(0, icon_library, (UINT)-1));
						if(id > micons - 1)id = micons - 1;

						hico = ExtractIcon(0, icon_library, id);
						hidisplay = GetDlgItem(hwnd, IDC_ICONDISPLAY);
						
						hiddc = GetDC(hidisplay);

						InvalidateRect(hidisplay, &pr, 1);
						SendMessage(hidisplay, WM_PAINT, 0, 0);

						DrawIcon(hiddc, 0, 0, hico);

						DestroyIcon(hico);
						ReleaseDC(hidisplay, hiddc);
					}
					
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
        switch (LOWORD(wParam)) 
        { 
		case IDC_SELALL:
			{
				HWND hwl = GetDlgItem(hwnd, list_filetypes);
				int i, c = ListView_GetItemCount(hwl);

				for(i=0; i<c; i++)
				{
					ListView_SetCheckState(hwl, i, 1);
				}
			}	
			break;

		case IDC_SELNONE:
			{
				HWND hwl = GetDlgItem(hwnd, list_filetypes);
				int i, c = ListView_GetItemCount(hwl);

				for(i=0; i<c; i++)
				{
					ListView_SetCheckState(hwl, i, 0);
				}
			}	
			break;

		case IDC_FHANDLESET:
			{
				letter icopath[1024];
				letter fennecpath[1024];
				letter ext[260];
				letter dsc[260];
				unsigned int i = 0;
				HWND hwl = GetDlgItem(hwnd, list_filetypes);

				memset(fennecpath, 0, sizeof(fennecpath));

				GetModuleFileName(instance_fennec, fennecpath, sizeof(fennecpath));
					
				while(audio_input_getextensionsinfo(i, ext, dsc))
				{
					if(ListView_GetCheckState(hwl, i))
					{
						str_cpy(icopath, fennec_get_path(0, 0));
						str_cat(icopath, uni("\\"));
						str_cat(icopath, icon_library);

						fileassociation_set(ext, uni("Play"), uni("&Play in Fennec Player"), icopath, fennecpath, dsc, panel_fh_iconids[i]);
					}else{
						fileassociation_restore(ext);
					}
					
					memset(ext, 0, sizeof(ext));
					memset(dsc, 0, sizeof(dsc));

					i++;
				}

				fennec_register_contextmenu(1);

				SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, 0, 0);
				
			}
			break;
		}
		break;

	case WM_HSCROLL:
		{
			HICON hico;
			HWND  hidisplay;
			HDC   hiddc;
			int   id;
			int   micons;
			RECT  pr;
			int   sit;

			pr.left   = 0;
			pr.top    = 0;
			pr.right  = 48;
			pr.bottom = 48;

			id = (int)SendDlgItemMessage(hwnd, IDC_SICONID, TBM_GETPOS, 0, 0);

			micons = PtrToInt(ExtractIcon(0, icon_library, (UINT)-1));
			if(id > micons - 1)id = micons - 1;

			sit = (int)SendDlgItemMessage(hwnd, list_filetypes, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
			if(sit != -1)
			{
				panel_fh_iconids[sit] = id;
			}

			hico = ExtractIcon(0, icon_library, id);
			hidisplay = GetDlgItem(hwnd, IDC_ICONDISPLAY);
			
			hiddc = GetDC(hidisplay);

			InvalidateRect(hidisplay, &pr, 1);
			SendMessage(hidisplay, WM_PAINT, 0, 0);

			DrawIcon(hiddc, 0, 0, hico);

			DestroyIcon(hico);
			ReleaseDC(hidisplay, hiddc);
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int i = 0;	
			unsigned int j;
			letter       ext[128];
			letter       dsc[256];
			int          micons;
			LVCOLUMN     lvc;
			LVITEM       lvi;
			HWND         hwl = GetDlgItem(hwnd, list_filetypes);

			ListView_SetExtendedListViewStyle(hwl, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
			ListView_SetUnicodeFormat(hwl, 1);

			/* <lang> */

			SetDlgItemText(hwnd, IDC_SELALL,        text(oooo_select_all));
			SetDlgItemText(hwnd, IDC_SELNONE,       text(oooo_select_none));
			SetDlgItemText(hwnd, IDC_FHANDLESET,    text(oooo_settings_set_association));
			SetDlgItemText(hwnd, static_icon_index, text(oooo_settings_icon_index));

			/* </lang> */

			lvc.mask    = LVCF_TEXT | LVCF_WIDTH;
			lvc.cx      = 200;
			lvc.pszText = text(oooo_settings_description);
			SendMessage(hwl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

			lvc.cx      = 120;
			lvc.pszText = text(oooo_settings_extension);
			SendMessage(hwl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

			micons = PtrToInt(ExtractIcon(0, icon_library, (UINT)-1));

			if(micons)
			{
				SendDlgItemMessage(hwnd, IDC_SICONID, TBM_SETRANGE, 1, MAKELONG(0, micons));
			}else{
				EnableWindow(GetDlgItem(hwnd, IDC_SICONID), 0);
			}

			lvi.iSubItem  = 0;
			lvi.pszText   = LPSTR_TEXTCALLBACK;
			lvi.mask      = LVIF_TEXT; 

			while(audio_input_getextensionsinfo(i, ext, dsc))
			{				
				lvi.iItem = i;

				SendMessage(hwl, LVM_INSERTITEM, 0, (LPARAM)&lvi);
				ListView_SetCheckState(hwl, i, fileassociation_selected(ext));

				memset(ext, 0, sizeof(ext));
				i++;
			}


			panel_fh_iconids = (long*) sys_mem_alloc(sizeof(long) * (i + 2));
			for(j=0; j<=i; j++)
			{
				audio_input_getextensionsinfo(j, ext, dsc);
				panel_fh_iconids[j] = fileassociation_geticonid(ext);
				memset(ext, 0, sizeof(ext));
			}
		}
		break;

	case WM_DESTROY:
		sys_mem_free(panel_fh_iconids);
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


int subskins_callback_function(string fname, string ftitle)
{
	/*if(ftitle)
	{
		SendDlgItemMessage(currenthwnd, list_subs, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)ftitle);
	}else{
		SendDlgItemMessage(currenthwnd, list_subs, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)fname);
	}*/
	SendDlgItemMessage(currenthwnd, list_subs, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)fname);
	return 1;
}

/*
 * 
 */
int settings_proc_skins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_SK_LIST:
			if(HIWORD(wParam) == LBN_DBLCLK)
				goto offset_apply_selected_skin;
			break;

		case list_subs:
			if(HIWORD(wParam) == LBN_DBLCLK)
			{
				letter fname[v_sys_maxpath];
				int    csel;

				csel = (int) SendDlgItemMessage(hwnd, list_subs, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, list_subs, LB_GETTEXT, (WPARAM)csel, (LPARAM)fname);
				skins_function_subskins_select(fname);
			}
			break;

		case button_restore:
			settings.environment.main_window_x = 0;
			settings.environment.main_window_y = 0;
			settings.environment.main_window_state = setting_window_normal;
			SetWindowPos(window_main, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			break;

		case button_switch:
			
			{
				letter fname[v_sys_maxpath];
				int    csel;

				csel = (int) SendDlgItemMessage(hwnd, list_subs, LB_GETCURSEL, 0, 0);
				
				if(csel >= 0)
				{
					SendDlgItemMessage(hwnd, list_subs, LB_GETTEXT, (WPARAM)csel, (LPARAM)fname);
					skins_function_subskins_select(fname);
				}
			}
			break;

		case IDC_SK_BASE:
			
offset_apply_base_skin:
			SendDlgItemMessage(hwnd, list_subs, LB_RESETCONTENT, 0, 0);
			settings.skins.selected[0] = 0;
			settings.skins.selected[1] = 1;
			skins_uninitialize();
			//settings_ui_show_ex(2);
			break;
		
		case IDC_SK_SWITCH:
			
offset_apply_selected_skin:

			{
				RECT    rct;
				int     csel, base_skin_free = 0;
				letter  rskin_path[v_sys_maxpath];
				letter  lskin_path[v_sys_maxpath];

				csel = (int) SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_GETCURSEL, 0, 0);
				
				if(csel == -1)break;

				if(csel == 0) goto offset_apply_base_skin; /* <default> */

				SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_GETTEXT, csel, (LPARAM)lskin_path);

				str_cpy(rskin_path, fennec_get_path(0, 0));
				str_cat(rskin_path, uni("/skins/"));
				str_cat(rskin_path, lskin_path);
				str_cat(rskin_path, v_sys_lbrary_extension);

				if(!settings.skins.selected[0])base_skin_free = 1;
				
				fennec_get_rel_path(settings.skins.selected, rskin_path);

				skins_apply(rskin_path);

				GetClientRect(hwnd, &rct);

				SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE);
				SetWindowPos(hwnd, 0, 0, 0, rct.right, rct.bottom, SWP_NOMOVE);

				currenthwnd = hwnd;
				SendDlgItemMessage(hwnd, list_subs, LB_RESETCONTENT, 0, 0);
				skins_function_subskins_get(subskins_callback_function);
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			t_sys_fs_find_handle shandle;
			letter  skins_path[v_sys_maxpath];
			letter  cskin_path[v_sys_maxpath];
			int  sfound;

			/* <lang> */

			SetDlgItemText(hwnd, static_list_title, text(oooo_settings_available_skins));
			SetDlgItemText(hwnd, button_restore,    text(oooo_settings_restore));
			SetDlgItemText(hwnd, IDC_SK_SWITCH,     text(oooo_settings_switch));

			/* </lang> */

			SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("<skinless>"));

			str_cpy(skins_path, fennec_get_path(0, 0));
			str_cat(skins_path, uni("/skins/*"));
			str_cat(skins_path, v_sys_lbrary_extension);

			sfound = sys_fs_find_start(skins_path, cskin_path, sizeof(cskin_path), &shandle);

			while(sfound)
			{
				cskin_path[str_len(cskin_path) - 4] = 0;
				SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)cskin_path);

				sfound = sys_fs_find_next(cskin_path, sizeof(cskin_path), shandle);
			}

			sys_fs_find_close(shandle);

			currenthwnd = hwnd;
			SendDlgItemMessage(hwnd, list_subs, LB_RESETCONTENT, 0, 0);
			skins_function_subskins_get(subskins_callback_function);

			if(settings.skins.selected[0])
			{
				int i, m;

				fennec_get_abs_path(skins_path, settings.skins.selected);

				_wsplitpath(skins_path, 0, 0, cskin_path, 0);
				str_cpy(skins_path, cskin_path);

				m = (int)SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_GETCOUNT, 0, 0);

				for(i=0; i<m; i++)
				{
					SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_GETTEXT, (WPARAM)i, (LPARAM)cskin_path);
					if(str_icmp(skins_path, cskin_path) == 0)
					{
						SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_SETCURSEL, (WPARAM)i, 0);
						break;
					}
				}
			}else{
				SendDlgItemMessage(hwnd, IDC_SK_LIST, LB_SETCURSEL, 0, 0);
			}
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_formatting(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_HELP1:
		case IDC_HELP2:
		case IDC_HELP3:
			MessageBox(hwnd, formattinginfo, uni("Formatting Syntax."), MB_ICONINFORMATION);
			break;

		case IDC_MTITLE:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				letter stxt[1024];

				stxt[0] = 0;
				GetDlgItemText(hwnd, IDC_MTITLE, stxt, 260);
				tags_translate(stxt, &formatting_sample_tag, formatting_sample_tag.tag_filepath.tdata);

				SetDlgItemText(hwnd, IDC_PR_MTITLE, stxt);
			}
			break;

		case IDC_STITLE:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				letter stxt[1024];

				stxt[0] = 0;
				GetDlgItemText(hwnd, IDC_STITLE, stxt, 260);
				tags_translate(stxt, &formatting_sample_tag, formatting_sample_tag.tag_filepath.tdata);

				SetDlgItemText(hwnd, IDC_PR_STITLE, stxt);
			}
			break;

		case IDC_PITEM:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				letter stxt[1024];

				stxt[0] = 0;
				GetDlgItemText(hwnd, IDC_PITEM, stxt, 260);
				tags_translate(stxt, &formatting_sample_tag, formatting_sample_tag.tag_filepath.tdata);

				SetDlgItemText(hwnd, IDC_PR_PITEM, stxt);
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		formatting_sample_tag.tid       = 0;
		formatting_sample_tag.treserved = 0;

		local_settag(&formatting_sample_tag.tag_title        , uni("Riot"));
		local_settag(&formatting_sample_tag.tag_album        , uni("One-X"));
		local_settag(&formatting_sample_tag.tag_artist       , uni("Three Days Grace"));
		local_settag(&formatting_sample_tag.tag_origartist   , uni("Three Days Grace"));
		local_settag(&formatting_sample_tag.tag_composer     , uni("Barry Stock/Three Days Grace"));
		local_settag(&formatting_sample_tag.tag_lyricist     , uni("Adam Gontier"));
		local_settag(&formatting_sample_tag.tag_band         , uni("Three Days Grace"));
		local_settag(&formatting_sample_tag.tag_copyright    , uni("2006 Three Days Grace"));
		local_settag(&formatting_sample_tag.tag_publish      , uni("Jive"));
		local_settag(&formatting_sample_tag.tag_encodedby    , uni("Fennec Player 1.2"));
		local_settag(&formatting_sample_tag.tag_genre        , uni("Hard Rock/Alt. Rock/Post-Grunge"));
		local_settag(&formatting_sample_tag.tag_year         , uni("2006"));
		local_settag(&formatting_sample_tag.tag_url          , uni("http://fennec.sf.net"));
		local_settag(&formatting_sample_tag.tag_offiartisturl, uni("http://www.threedaysgrace.com"));
		local_settag(&formatting_sample_tag.tag_filepath     , uni("X:\\Three Days Grace - Riot.ogg"));
		local_settag(&formatting_sample_tag.tag_filename     , uni("Three Days Grace - Riot.ogg"));
		local_settag(&formatting_sample_tag.tag_comments     , uni("Comments"));
		local_settag(&formatting_sample_tag.tag_lyric        , uni("Lyrics"));
		local_settag(&formatting_sample_tag.tag_bpm          , uni("140"));
		local_settag(&formatting_sample_tag.tag_tracknum     , uni("06"));

		SetDlgItemText(hwnd, IDC_MTITLE, current_settings.formatting.title_main);
		SetDlgItemText(hwnd, IDC_STITLE, current_settings.formatting.title_scroll);
		SetDlgItemText(hwnd, IDC_PITEM,  current_settings.formatting.playlist_item);

		/* <lang> */

		SetDlgItemText(hwnd, static_main_formatting,      text(oooo_settings_formatting));
		SetDlgItemText(hwnd, static_scroll_formatting,    text(oooo_settings_formatting));
		SetDlgItemText(hwnd, static_playlist_formatting,  text(oooo_settings_formatting));
		SetDlgItemText(hwnd, static_main_preview,         text(oooo_settings_preview));
		SetDlgItemText(hwnd, static_scroll_preview,       text(oooo_settings_preview));
		SetDlgItemText(hwnd, static_playlist_preview,     text(oooo_settings_preview));
		SetDlgItemText(hwnd, group_main,                  text(oooo_settings_title_main));
		SetDlgItemText(hwnd, group_scroll,                text(oooo_settings_title_scrolling));
		SetDlgItemText(hwnd, group_playlist,              text(oooo_settings_title_playlist));

		/* </lang> */
		break;

	case msg_settings_update:
		GetDlgItemText(hwnd, IDC_MTITLE, current_settings.formatting.title_main   , 260);
		GetDlgItemText(hwnd, IDC_STITLE, current_settings.formatting.title_scroll , 260);
		GetDlgItemText(hwnd, IDC_PITEM,  current_settings.formatting.playlist_item, 260);
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

string local_action_to_text(unsigned short ka, string buf)
{
	switch(ka)
	{
	case keypurpose_null                    : str_cpy(buf, uni("Nothing")); break;
	case keypurpose_play                    : str_cpy(buf, uni("Play")); break;
	case keypurpose_pause                   : str_cpy(buf, uni("Pause")); break;
	case keypurpose_stop                    : str_cpy(buf, uni("Stop")); break;
	case keypurpose_load                    : str_cpy(buf, uni("Load")); break;
	case keypurpose_rewind                  : str_cpy(buf, uni("Rewind")); break;
    case keypurpose_forward                 : str_cpy(buf, uni("Forward")); break;
    case keypurpose_previous                : str_cpy(buf, uni("Previous")); break;
    case keypurpose_next                    : str_cpy(buf, uni("Next")); break;
    case keypurpose_eject                   : str_cpy(buf, uni("Eject")); break;
    case keypurpose_select                  : str_cpy(buf, uni("Select")); break;
    case keypurpose_panelsw_main            : str_cpy(buf, uni("Switch Panel Main")); break;
    case keypurpose_panelsw_color           : str_cpy(buf, uni("Switch Panel Color")); break;
    case keypurpose_panelsw_visualization   : str_cpy(buf, uni("Switch Panel Visualization")); break;
    case keypurpose_panelsw_equalizer       : str_cpy(buf, uni("Switch Panel Equalizer")); break;
    case keypurpose_panelsw_mediainfo       : str_cpy(buf, uni("Switch Panel Media Info")); break;
    case keypurpose_panelsw_playlist        : str_cpy(buf, uni("Switch Panel Playlist")); break;
    case keypurpose_panelnext               : str_cpy(buf, uni("Next Panel")); break;
    case keypurpose_panelprevious           : str_cpy(buf, uni("Previous Panel")); break;
    case keypurpose_exit                    : str_cpy(buf, uni("Exit")); break;
    case keypurpose_sleep                   : str_cpy(buf, uni("Sleep")); break;
    case keypurpose_minimize                : str_cpy(buf, uni("Minimize")); break;
    case keypurpose_refresh                 : str_cpy(buf, uni("Refresh")); break;
    case keypurpose_conversion              : str_cpy(buf, uni("Conversion")); break;
    case keypurpose_ripping                 : str_cpy(buf, uni("Ripping")); break;
    case keypurpose_joining                 : str_cpy(buf, uni("Joining")); break;
    case keypurpose_visualization           : str_cpy(buf, uni("Visualization")); break;
    case keypurpose_playlist                : str_cpy(buf, uni("Playlist")); break;
    case keypurpose_volumeup                : str_cpy(buf, uni("Volume Up")); break;
    case keypurpose_volumedown              : str_cpy(buf, uni("Volume Down")); break;
    case keypurpose_volumeup_auto           : str_cpy(buf, uni("Volume Up (Automatic)")); break;
    case keypurpose_volumedown_auto         : str_cpy(buf, uni("Volume Down (Automatic)")); break;
    case keypurpose_volumemin               : str_cpy(buf, uni("Volume Minimum")); break;
    case keypurpose_volumemax               : str_cpy(buf, uni("Volume Maximum")); break;
    case keypurpose_addfile                 : str_cpy(buf, uni("Add File")); break;
    case keypurpose_fast_load               : str_cpy(buf, uni("Fast Load")); break;
    case keypurpose_fast_addfile            : str_cpy(buf, uni("Fast Add File")); break;
    case keypurpose_preferences             : str_cpy(buf, uni("Preferences")); break;
    case keypurpose_keyboardviewer          : str_cpy(buf, uni("Keyboard Viewer")); break;
    case keypurpose_currenttagging          : str_cpy(buf, uni("Tag Editor")); break;
    case keypurpose_switch_playlist         : str_cpy(buf, uni("Switch Playlist")); break;
    case keypurpose_playlist_autoswitching  : str_cpy(buf, uni("Playlist - Auto Switching")); break;
    case keypurpose_playlist_shuffle        : str_cpy(buf, uni("Playlist - Shuffle")); break;
    case keypurpose_playlist_information    : str_cpy(buf, uni("Playlist - Information")); break;
    case keypurpose_playlist_repeatall      : str_cpy(buf, uni("Playlist - Repeat All")); break;
    case keypurpose_playlist_repeatsingle   : str_cpy(buf, uni("Playlist - Repeat Single")); break;
    case keypurpose_playlist_insert         : str_cpy(buf, uni("Playlist - Insert")); break;
    case keypurpose_playlist_insertdir      : str_cpy(buf, uni("Playlist - Insert Directory")); break;
    case keypurpose_playlist_remove         : str_cpy(buf, uni("Playlist - Remove")); break;
    case keypurpose_switch_main             : str_cpy(buf, uni("Switch Main Window")); break;
	}

	return buf;
}

string local_keys_to_text(unsigned short kc, string buf)
{
	unsigned short bkey = kc & 	0xff;

	buf[0] = 0;

	if(kc & fennec_key_shift)      str_cat(buf, uni("Shift + "));
	if(kc & fennec_key_control)    str_cat(buf, uni("Ctrl + "));
	if(kc & fennec_key_alternative)str_cat(buf, uni("Alt + "));

	switch(bkey)
	{
	case fennec_key_function1:  str_cat(buf, uni("F1")); break;
	case fennec_key_function2:  str_cat(buf, uni("F2")); break;
	case fennec_key_function3:  str_cat(buf, uni("F3")); break;
	case fennec_key_function4:  str_cat(buf, uni("F4")); break;
	case fennec_key_function5:  str_cat(buf, uni("F5")); break;
	case fennec_key_function6:  str_cat(buf, uni("F6")); break;
	case fennec_key_function7:  str_cat(buf, uni("F7")); break;
	case fennec_key_function8:	str_cat(buf, uni("F8")); break;
	case fennec_key_function9:	str_cat(buf, uni("F9")); break;
	case fennec_key_function10:	str_cat(buf, uni("F10")); break;
	case fennec_key_function11:	str_cat(buf, uni("F11")); break;
	case fennec_key_function12:	str_cat(buf, uni("F12")); break;
	case fennec_key_function13: str_cat(buf, uni("F13")); break;
	case fennec_key_function14:	str_cat(buf, uni("F14")); break;
	case fennec_key_function15: str_cat(buf, uni("F15")); break;             

	case fennec_key_a:	str_cat(buf, uni("\'A\'")); break;
	case fennec_key_b:	str_cat(buf, uni("\'B\'")); break;
	case fennec_key_c:  str_cat(buf, uni("\'C\'")); break;
	case fennec_key_d:	str_cat(buf, uni("\'D\'")); break;
	case fennec_key_e:	str_cat(buf, uni("\'E\'")); break;
	case fennec_key_f:	str_cat(buf, uni("\'F\'")); break;
	case fennec_key_g:	str_cat(buf, uni("\'G\'")); break;
	case fennec_key_h:	str_cat(buf, uni("\'H\'")); break;
	case fennec_key_i:	str_cat(buf, uni("\'I\'")); break;
	case fennec_key_j:	str_cat(buf, uni("\'J\'")); break;
	case fennec_key_k:	str_cat(buf, uni("\'K\'")); break;
	case fennec_key_l:  str_cat(buf, uni("\'L\'")); break;
	case fennec_key_m:	str_cat(buf, uni("\'M\'")); break;
	case fennec_key_n:	str_cat(buf, uni("\'N\'")); break;
	case fennec_key_o:	str_cat(buf, uni("\'O\'")); break;
	case fennec_key_p:	str_cat(buf, uni("\'P\'")); break;
	case fennec_key_q:	str_cat(buf, uni("\'Q\'")); break;
	case fennec_key_r:	str_cat(buf, uni("\'R\'")); break;
	case fennec_key_s:	str_cat(buf, uni("\'S\'")); break;
	case fennec_key_t:	str_cat(buf, uni("\'T\'")); break;
	case fennec_key_u:  str_cat(buf, uni("\'U\'")); break;
	case fennec_key_v:  str_cat(buf, uni("\'V\'")); break;
	case fennec_key_w:	str_cat(buf, uni("\'W\'")); break;
	case fennec_key_x:	str_cat(buf, uni("\'X\'")); break;
	case fennec_key_y:	str_cat(buf, uni("\'Y\'")); break;
	case fennec_key_z:	str_cat(buf, uni("\'Z\'")); break;

	case fennec_key_0:	str_cat(buf, uni("\'0\'")); break;
	case fennec_key_1:	str_cat(buf, uni("\'1\'")); break;
	case fennec_key_2:  str_cat(buf, uni("\'2\'")); break;
	case fennec_key_3:	str_cat(buf, uni("\'3\'")); break;
	case fennec_key_4:	str_cat(buf, uni("\'4\'")); break;
	case fennec_key_5:	str_cat(buf, uni("\'5\'")); break;
	case fennec_key_6:	str_cat(buf, uni("\'6\'")); break;
	case fennec_key_7:	str_cat(buf, uni("\'7\'")); break;
	case fennec_key_8:	str_cat(buf, uni("\'8\'")); break;
	case fennec_key_9:	str_cat(buf, uni("\'9\'")); break;

	case fennec_key_return:              str_cat(buf, uni("Enter")); break;
	case fennec_key_escape:				 str_cat(buf, uni("Esc")); break;
	case fennec_key_tab:				 str_cat(buf, uni("Tab")); break;
	case fennec_key_graveaccent:		 str_cat(buf, uni("\'`\'")); break;
	case fennec_key_minus:				 str_cat(buf, uni("\'-\'")); break;
	case fennec_key_equals:				 str_cat(buf, uni("\'=\'")); break;
	case fennec_key_forwardslash:		 str_cat(buf, uni("\'/\'")); break;
	case fennec_key_backslash:			 str_cat(buf, uni("\'\\\'")); break;
	case fennec_key_squarebracket_open:	 str_cat(buf, uni("\'[\'")); break;
	case fennec_key_squarebracket_close: str_cat(buf, uni("\']\'")); break;
	case fennec_key_semicolon:           str_cat(buf, uni("\';\'")); break;
	case fennec_key_accent:				 str_cat(buf, uni("\'\'\'")); break;
	case fennec_key_comma:				 str_cat(buf, uni("\',\'")); break;
	case fennec_key_period:				 str_cat(buf, uni("\'.\'")); break;
	case fennec_key_space:				 str_cat(buf, uni("Space")); break;

	case fennec_key_printscreen:		 str_cat(buf, uni("Print Screen")); break;
	case fennec_key_scrolllock:			 str_cat(buf, uni("Scroll Lock")); break;
	case fennec_key_pause:				 str_cat(buf, uni("Pause")); break;
	case fennec_key_insert:				 str_cat(buf, uni("Insert")); break;
	case fennec_key_home:                str_cat(buf, uni("Home")); break;
	case fennec_key_end:				 str_cat(buf, uni("End")); break;
	case fennec_key_pageup:				 str_cat(buf, uni("Page Up")); break;
	case fennec_key_pagedown:			 str_cat(buf, uni("Page Down")); break;
	case fennec_key_delete:				 str_cat(buf, uni("Delete")); break;
	case fennec_key_backspace:			 str_cat(buf, uni("Backspace")); break;
	case fennec_key_up:					 str_cat(buf, uni("Up")); break;
	case fennec_key_down:				 str_cat(buf, uni("Down")); break;
	case fennec_key_left:				 str_cat(buf, uni("Left")); break;
	case fennec_key_right:				 str_cat(buf, uni("Right")); break;

	case fennec_key_num_0:               str_cat(buf, uni("Num. \'0\'")); break;
	case fennec_key_num_1:				 str_cat(buf, uni("Num. \'1\'")); break;
	case fennec_key_num_2:				 str_cat(buf, uni("Num. \'2\'")); break;
	case fennec_key_num_3:				 str_cat(buf, uni("Num. \'3\'")); break;
	case fennec_key_num_4:				 str_cat(buf, uni("Num. \'4\'")); break;
	case fennec_key_num_5:				 str_cat(buf, uni("Num. \'5\'")); break;
	case fennec_key_num_6:				 str_cat(buf, uni("Num. \'6\'")); break;
	case fennec_key_num_7:				 str_cat(buf, uni("Num. \'7\'")); break;
	case fennec_key_num_8:				 str_cat(buf, uni("Num. \'8\'")); break;
	case fennec_key_num_9:				 str_cat(buf, uni("Num. \'9\'")); break;
	case fennec_key_num_numlock:         str_cat(buf, uni("Numlock")); break;
	case fennec_key_num_divide:			 str_cat(buf, uni("Num. \'/\'")); break;
	case fennec_key_num_multiply:		 str_cat(buf, uni("Num. \'*\'")); break;
	case fennec_key_num_minus:			 str_cat(buf, uni("Num. \'-\'")); break;
	case fennec_key_num_plus:			 str_cat(buf, uni("Num. \'+\'")); break;
	case fennec_key_num_return:          str_cat(buf, uni("Num. Enter")); break;
	}

	return buf;
}

/*
 * 
 */
int settings_proc_local_addshortcut(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int i = 0;
			HWND hwc = GetDlgItem(hwnd, IDC_ACTIONS);
		
			while(keyactions[i][0])
			{
				SendMessage(hwc, CB_INSERTSTRING, i, (LPARAM)keyactions[i]);
				i++;
			}

			SendMessage(hwc, CB_SETCURSEL, 0, 0);

		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int i, cs, ci;
				unsigned short  fek = 0;
				unsigned long   shk = (unsigned long)SendDlgItemMessage(hwnd, IDC_KEYS, HKM_GETHOTKEY, 0, 0);
				unsigned short  lshk = LOBYTE(LOWORD(shk));
				unsigned short  hshk = HIBYTE(LOWORD(shk));
				LVITEM          lvi;

				if(shk)
				{
					fek = fennec_convertkey(&lshk);

					if(hshk & HOTKEYF_ALT)     fek |= fennec_key_alternative;
					if(hshk & HOTKEYF_CONTROL) fek |= fennec_key_control;
					if(hshk & HOTKEYF_SHIFT)   fek |= fennec_key_shift;
				}

				if(list_localsk)
				{
					for(i=0; i<(sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0])); i++)
					{
						if(settings.shortcuts.localkeys[i].kcomb == 0 && settings.shortcuts.localkeys[i].kaction == keypurpose_null)break;
					}

					cs = (int)SendDlgItemMessage(hwnd, IDC_ACTIONS, CB_GETCURSEL, 0, 0);
					if(cs != -1 && cs < (sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0])) - 1)
					{
						settings.shortcuts.localkeys[i].kcomb   = fek; 
						settings.shortcuts.localkeys[i].kaction = (unsigned short)(cs + 1); /* + 1 for keypurpose_null */

						settings.shortcuts.localkeys[i + 1].kcomb   = 0;
						settings.shortcuts.localkeys[i + 1].kaction = keypurpose_null;

						ci = ListView_GetItemCount(list_localsk);

						lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
						lvi.state     = 0; 
						lvi.stateMask = 0; 
						lvi.iItem    = ci;
						lvi.iSubItem = 0;
						lvi.lParam   = 0;
						lvi.pszText  = LPSTR_TEXTCALLBACK;

						SendMessage(list_localsk, LVM_INSERTITEM, 0, (LPARAM)&lvi);
						ListView_SetItemText(list_localsk, ci, 1, LPSTR_TEXTCALLBACK);
					}

				}else{
					for(i=0; i<(sizeof(settings.shortcuts.globalkeys) / sizeof(settings.shortcuts.globalkeys[0])); i++)
					{
						if(settings.shortcuts.globalkeys[i].kcomb == 0 && settings.shortcuts.globalkeys[i].kaction == keypurpose_null)break;
					}

					cs = (int)SendDlgItemMessage(hwnd, IDC_ACTIONS, CB_GETCURSEL, 0, 0);

					if(cs != -1 && cs < (sizeof(settings.shortcuts.globalkeys) / sizeof(settings.shortcuts.globalkeys[0])) - 1)
					{
						settings.shortcuts.globalkeys[i].kcomb   = fek; 
						settings.shortcuts.globalkeys[i].kaction = (unsigned short)(cs + 1); /* + 1 for keypurpose_null */

						settings.shortcuts.globalkeys[i + 1].kcomb   = 0;
						settings.shortcuts.globalkeys[i + 1].kaction = keypurpose_null;

						ci = ListView_GetItemCount(list_globalsk);

						lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
						lvi.state     = 0; 
						lvi.stateMask = 0; 
						lvi.iItem     = ci;
						lvi.iSubItem  = 0;
						lvi.lParam    = 0;
						lvi.pszText   = LPSTR_TEXTCALLBACK;

						SendMessage(list_globalsk, LVM_INSERTITEM, 0, (LPARAM)&lvi);
						ListView_SetItemText(list_globalsk, ci, 1, LPSTR_TEXTCALLBACK);
					}

				}

				
			}

		case IDCANCEL:
			EndDialog(hwnd, 0);
			return 0;
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_shortcuts(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static letter  lkeys[256][64];
	static letter  lactions[256][64];
	static letter  gkeys[256][64];
	static letter  gactions[256][64];
	LV_DISPINFO *lvit = (LV_DISPINFO *)lParam;

	switch(uMsg)
	{
	case WM_NOTIFY:
		{
			switch(wParam)
			{
			case IDC_SH_LKEYS:

				switch(((LPNMHDR)lParam)->code)
				{
				case LVN_GETDISPINFO:
					switch(lvit->item.iSubItem)
					{
					case 0:
						lvit->item.pszText = local_keys_to_text(settings.shortcuts.localkeys[lvit->item.iItem].kcomb, lkeys[lvit->item.iItem]);
						break;

					case 1:
						lvit->item.pszText = local_action_to_text(settings.shortcuts.localkeys[lvit->item.iItem].kaction, lactions[lvit->item.iItem]);
						break;
					}
					return 0;
				}
				break;
			case IDC_SH_GKEYS:

				switch(((LPNMHDR)lParam)->code)
				{
				case LVN_GETDISPINFO:
					switch(lvit->item.iSubItem)
					{
					case 0:
						lvit->item.pszText = local_keys_to_text(settings.shortcuts.globalkeys[lvit->item.iItem].kcomb, gkeys[lvit->item.iItem]);
						break;

					case 1:
						lvit->item.pszText = local_action_to_text(settings.shortcuts.globalkeys[lvit->item.iItem].kaction, gactions[lvit->item.iItem]);
						break;
					}
					return 0;
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_SH_LREM:
			{
				HWND hwc = GetDlgItem(hwnd, IDC_SH_LKEYS);
				int  hi  = ListView_GetNextItem(hwc, -1, LVNI_SELECTED);
				int  i;

				if(hi >= 0 && hi < sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0]))
				{
					ListView_DeleteItem(hwc, hi);

					for(i=hi; i<(sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0])) - 1; i++)
					{
						settings.shortcuts.localkeys[i].kaction = settings.shortcuts.localkeys[i + 1].kaction;
						settings.shortcuts.localkeys[i].kcomb   = settings.shortcuts.localkeys[i + 1].kcomb  ;

						if(settings.shortcuts.localkeys[i + 1].kcomb == 0 && settings.shortcuts.localkeys[i + 1].kaction == keypurpose_null)break;

					}
				}
			}
			break;

		case IDC_SH_GREM:
			{
				HWND hwc = GetDlgItem(hwnd, IDC_SH_GKEYS);
				int  hi  = ListView_GetNextItem(hwc, -1, LVNI_SELECTED);
				int  i;

				if(hi >= 0 && hi < sizeof(settings.shortcuts.globalkeys) / sizeof(settings.shortcuts.globalkeys[0]))
				{
					ListView_DeleteItem(hwc, hi);

					for(i=hi; i<(sizeof(settings.shortcuts.globalkeys) / sizeof(settings.shortcuts.globalkeys[0])) - 1; i++)
					{
						settings.shortcuts.globalkeys[i].kaction = settings.shortcuts.globalkeys[i + 1].kaction;
						settings.shortcuts.globalkeys[i].kcomb   = settings.shortcuts.globalkeys[i + 1].kcomb  ;

						if(settings.shortcuts.globalkeys[i + 1].kcomb == 0 && settings.shortcuts.globalkeys[i + 1].kaction == keypurpose_null)break;

					}
				}
			}
			break;

		case IDC_SH_GADDNEW:
			list_globalsk = GetDlgItem(hwnd, IDC_SH_GKEYS);
			list_localsk  = 0;
			DialogBox(instance_fennec, MAKEINTRESOURCE(IDD_ADDSHORTCUT), hwnd, (DLGPROC)settings_proc_local_addshortcut);
			break;

		case IDC_SH_LADDNEW:
			list_globalsk = 0;
			list_localsk  = GetDlgItem(hwnd, IDC_SH_LKEYS);
			DialogBox(instance_fennec, MAKEINTRESOURCE(IDD_ADDSHORTCUT), hwnd, (DLGPROC)settings_proc_local_addshortcut);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int i;
			LVCOLUMN lvc;
			LVITEM   lvi;
			HWND     hwc = GetDlgItem(hwnd, IDC_SH_LKEYS);
			HWND     hwg = GetDlgItem(hwnd, IDC_SH_GKEYS);

			ListView_SetExtendedListViewStyle(hwc, LVS_EX_FULLROWSELECT);
			ListView_SetExtendedListViewStyle(hwg, LVS_EX_FULLROWSELECT);

			ListView_SetUnicodeFormat(hwc, 1);
			ListView_SetUnicodeFormat(hwg, 1);

			/* <lang> */
			
			SetDlgItemText(hwnd, static_local,    text(oooo_settings_local_keys));
			SetDlgItemText(hwnd, static_global,   text(oooo_settings_global_keys));
			SetDlgItemText(hwnd, IDC_SH_LENABLE,  text(oooo_settings_enable));
			SetDlgItemText(hwnd, IDC_SH_GENABLE,  text(oooo_settings_enable));
			SetDlgItemText(hwnd, IDC_SH_LREM,     text(oooo_remove));
			SetDlgItemText(hwnd, IDC_SH_GREM,     text(oooo_remove));
			SetDlgItemText(hwnd, IDC_SH_LADDNEW,  text(oooo_settings_addnew));
			SetDlgItemText(hwnd, IDC_SH_GADDNEW,  text(oooo_settings_addnew));

			/* </lang> */

			if(settings.shortcuts.enable_local)CheckDlgButton(hwnd, IDC_SH_LENABLE, BST_CHECKED);
			else CheckDlgButton(hwnd, IDC_SH_LENABLE, BST_UNCHECKED);

			if(settings.shortcuts.enable_global)CheckDlgButton(hwnd, IDC_SH_GENABLE, BST_CHECKED);
			else CheckDlgButton(hwnd, IDC_SH_GENABLE, BST_UNCHECKED);

			lvc.mask    = LVCF_TEXT | LVCF_WIDTH;
			lvc.cx      = 170;
			lvc.pszText = text(oooo_settings_action);
			SendDlgItemMessage(hwnd, IDC_SH_LKEYS, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
			SendDlgItemMessage(hwnd, IDC_SH_GKEYS, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

			lvc.cx      = 150;
			lvc.pszText = text(oooo_settings_keys);
			SendDlgItemMessage(hwnd, IDC_SH_LKEYS, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
			SendDlgItemMessage(hwnd, IDC_SH_GKEYS, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
		
			// items. 

			lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
			lvi.state     = 0; 
			lvi.stateMask = 0; 

			for(i=0; i<sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0]); i++)
			{
				if(settings.shortcuts.localkeys[i].kcomb == 0 && settings.shortcuts.localkeys[i].kaction == keypurpose_null)break;

				lvi.iItem    = i;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				lvi.pszText  = LPSTR_TEXTCALLBACK;
				SendDlgItemMessage(hwnd, IDC_SH_LKEYS, LVM_INSERTITEM, 0, (LPARAM)&lvi);
				ListView_SetItemText(GetDlgItem(hwnd, IDC_SH_LKEYS), i, 1, LPSTR_TEXTCALLBACK);
				local_keys_to_text(settings.shortcuts.localkeys[i].kcomb, lkeys[i]);
				local_action_to_text(settings.shortcuts.localkeys[i].kaction, lactions[i]);
			}

			for(i=0; i<sizeof(settings.shortcuts.globalkeys) / sizeof(settings.shortcuts.globalkeys[0]); i++)
			{
				if(settings.shortcuts.globalkeys[i].kcomb == 0 && settings.shortcuts.globalkeys[i].kaction == keypurpose_null)break;

				lvi.iItem    = i;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				lvi.pszText  = LPSTR_TEXTCALLBACK;
				SendDlgItemMessage(hwnd, IDC_SH_GKEYS, LVM_INSERTITEM, 0, (LPARAM)&lvi);
				ListView_SetItemText(GetDlgItem(hwnd, IDC_SH_GKEYS), i, 1, LPSTR_TEXTCALLBACK);
				local_keys_to_text(settings.shortcuts.localkeys[i].kcomb, gkeys[i]);
				local_action_to_text(settings.shortcuts.localkeys[i].kaction, gactions[i]);
			}
		}
		break;

	case msg_settings_update:
		if(IsDlgButtonChecked(hwnd, IDC_SH_LENABLE) == BST_CHECKED)settings.shortcuts.enable_local  = 1;
		else settings.shortcuts.enable_local  = 0;

		if(IsDlgButtonChecked(hwnd, IDC_SH_GENABLE) == BST_CHECKED)settings.shortcuts.enable_global = 1;
		else settings.shortcuts.enable_global = 0;
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_internalout(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_HSCROLL:
		SetDlgItemInt(hwnd, IDC_IO_CBUFFERSIZE, (unsigned int)SendDlgItemMessage(hwnd, IDC_SBUFFERMS, TBM_GETPOS, 0, 0) * 100, 0);
		break;

	case WM_COMMAND:
		
		if(IsDlgButtonChecked(hwnd, check_noise) == BST_CHECKED)
			settings.audio_output.noise_reduction = 1;
		else
			settings.audio_output.noise_reduction = 0;

		if(IsDlgButtonChecked(hwnd, check_effects) == BST_CHECKED)
			settings.audio_output.perform_effects = 1;
		else
			settings.audio_output.perform_effects = 0;
		break;

	case WM_INITDIALOG:
		{
			unsigned int wc = waveOutGetNumDevs();
			unsigned int i;
			WAVEOUTCAPS  wcaps;

			/* <lang> */

			SetDlgItemText(hwnd, static_buffersize,  text(oooo_settings_buffersize_ms));
			SetDlgItemText(hwnd, static_cbuffersize, text(oooo_settings_current_buffersize));
			SetDlgItemText(hwnd, static_default,     text(oooo_settings_default_buffersize));
			SetDlgItemText(hwnd, static_info,        text(oooo_settings_internaloutput_info));

			SetDlgItemText(hwnd, check_noise,   text(oooo_audio_out_noise_reduction  ) );
			SetDlgItemText(hwnd, check_effects,	text(oooo_audio_out_other_effects    ) );
			SetDlgItemText(hwnd, static_res,	text(oooo_audio_out_resolution       ) );
			SetDlgItemText(hwnd, group_res,		text(oooo_audio_out_resolution_title ) );

			/* </lang> */

			SendDlgItemMessage(hwnd, IDC_IO_DEVICE, CB_ADDSTRING, 0, (LPARAM)text(oooo_settings_default_device));

			for(i=0; i<wc; i++)
			{
				waveOutGetDevCaps(i, &wcaps, sizeof(wcaps));
				SendDlgItemMessage(hwnd, IDC_IO_DEVICE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)wcaps.szPname);
			}

			/* resolutions (bet-depth), may not need translations */

			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("8-bit Unsigned"));
			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("8-bit Signed"));
			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("16-bit"));
			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("24-bit"));
			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("32-bit"));
			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("64-bit"));

			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("Floating Point 32-bit"));
			SendDlgItemMessage(hwnd, combo_res, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("Floating Point 64-bit"));

			SetDlgItemInt(hwnd, IDC_IO_CBUFFERSIZE, current_settings.audio_output.buffer_memory, 0);

			SendDlgItemMessage(hwnd, IDC_IO_DEVICE, CB_SETCURSEL, current_settings.audio_output.output_device_id, 0);
			SendDlgItemMessage(hwnd, IDC_SBUFFERMS, TBM_SETRANGE, 1, MAKELONG(1, 20));
			SendDlgItemMessage(hwnd, IDC_SBUFFERMS, TBM_SETPOS, 1, current_settings.audio_output.buffer_memory / 100);

			switch(settings.audio_output.bit_depth)
			{
			case 8:
				if(settings.audio_output.bit_depth_signed)
					SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 0, 0);
				else
					SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 1, 0);
				break;

			case 16: SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 2, 0); break;
			case 24: SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 3, 0); break;
			case 32:
				if(!settings.audio_output.bit_depth_float)
					SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 4, 0);
				else
					SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 6, 0);
				break;

			case 64:
				if(!settings.audio_output.bit_depth_float)
					SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 5, 0);
				else
					SendDlgItemMessage(hwnd, combo_res, CB_SETCURSEL, 7, 0);
				break;
			}

			if(settings.audio_output.noise_reduction)
				CheckDlgButton(hwnd, check_noise, BST_CHECKED);
			else
				CheckDlgButton(hwnd, check_noise, BST_UNCHECKED);

			if(settings.audio_output.perform_effects)
				CheckDlgButton(hwnd, check_effects, BST_CHECKED);
			else
				CheckDlgButton(hwnd, check_effects, BST_UNCHECKED);

		}
		break;
	
	case msg_settings_update:

		{
			int cbsel = (int)SendDlgItemMessage(hwnd, combo_res, CB_GETCURSEL, 0, 0);

			switch(cbsel)
			{
			case 0: /* 8s */
				settings.audio_output.bit_depth        = 8;
				settings.audio_output.bit_depth_float  = 0;
				settings.audio_output.bit_depth_signed = 1;
				break;
			case 1: /*8u */
				settings.audio_output.bit_depth        = 8;
				settings.audio_output.bit_depth_float  = 0;
				settings.audio_output.bit_depth_signed = 0;
				break;
			case 2: /* 16 */
				settings.audio_output.bit_depth        = 16;
				settings.audio_output.bit_depth_float  = 0;
				settings.audio_output.bit_depth_signed = 1;
				break;
			case 3: /* 24 */
				settings.audio_output.bit_depth        = 24;
				settings.audio_output.bit_depth_float  = 0;
				settings.audio_output.bit_depth_signed = 1;
				break;
			case 4: /* 32 */
				settings.audio_output.bit_depth        = 32;
				settings.audio_output.bit_depth_float  = 0;
				settings.audio_output.bit_depth_signed = 1;
				break;
			case 5: /* 64 */
				settings.audio_output.bit_depth        = 64;
				settings.audio_output.bit_depth_float  = 0;
				settings.audio_output.bit_depth_signed = 1;
				break;
			case 6: /* 32f */
				settings.audio_output.bit_depth        = 32;
				settings.audio_output.bit_depth_float  = 1;
				settings.audio_output.bit_depth_signed = 1;
				break;
			case 7: /* 64f */
				settings.audio_output.bit_depth        = 64;
				settings.audio_output.bit_depth_float  = 1;
				settings.audio_output.bit_depth_signed = 1;
				break;
			}
		}
		
		current_settings.audio_output.output_device_id   = (unsigned long)SendDlgItemMessage(hwnd, IDC_IO_DEVICE, CB_GETCURSEL, 0, 0);
		current_settings.audio_output.buffer_memory      = (unsigned long)SendDlgItemMessage(hwnd, IDC_SBUFFERMS, TBM_GETPOS, 0, 0) * 100;
		if(current_settings.audio_output.buffer_memory > 2000)current_settings.audio_output.buffer_memory = 2000;
		if(current_settings.audio_output.buffer_memory < 50)  current_settings.audio_output.buffer_memory = 50;

		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

/*
 * 
 */
int settings_proc_internalin(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_inplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_INPLG_REFRESH:
			{
				extern int basewindows_open_refresh;
				
				audio_stop(); /* [stop all] */
				settings.plugins.input_plugins_count = 0;
				internal_input_initialize();
				basewindows_open_refresh = 0;
			}
			goto inplugins_posinit;
		
		case IDC_ABOUT:
			{
				int k = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(k < (int)settings.plugins.input_plugins_count)
				{
					internal_input_plugin_initialize(k);
					internal_input_show_about(k, (void*)hwnd);
				}
			}
			break;

		case IDC_CONFIG:
			{
				int k = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(k < (int)settings.plugins.input_plugins_count)
				{
					internal_input_plugin_initialize(k);
					internal_input_show_settings(k, (void*)hwnd);
				}
			}
			break;
		}
		break;

	case WM_INITDIALOG:
	inplugins_posinit:
		{
			unsigned long         iplg_size = 0;
			struct internal_input_plugin iplg;
			unsigned int          i;
			unsigned int          mex = 0;
			SIZE                  pfext;
			HDC                   hldc;

			SendDlgItemMessage(hwnd,IDC_PLGLIST, LB_RESETCONTENT ,(WPARAM)mex,0);

			/* <lang> */

			SetDlgItemText(hwnd, static_title,       text(oooo_settings_available_decoders));
			SetDlgItemText(hwnd, IDC_ABOUT,          text(oooo_settings_about));
			SetDlgItemText(hwnd, IDC_CONFIG,         text(oooo_settings_configuration));
			SetDlgItemText(hwnd, IDC_INPLG_REFRESH,  text(oooo_settings_refresh_cache));

			/* </lang> */

			for(i=0; i<settings.plugins.input_plugins_count; i++)
			{
				memset(&iplg, 0, sizeof(iplg));

				settings_data_get(setting_id_input_plugin, i, &iplg, &iplg_size);

				SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_ADDSTRING, 0, (LPARAM)iplg.pluginname);

				hldc = GetDC(hwnd);
				SelectObject(hldc, GetStockObject(DEFAULT_GUI_FONT));
				GetTextExtentPoint(hldc, iplg.pluginname, (int)str_len(iplg.pluginname), &pfext);
				ReleaseDC(hwnd, hldc);
				mex = max(mex, (unsigned int)pfext.cx);
			}
			SendDlgItemMessage(hwnd,IDC_PLGLIST,LB_SETHORIZONTALEXTENT,(WPARAM)mex,0);
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_outplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_PLGLIST:
			if(HIWORD(wParam) == LBN_DBLCLK)
				goto offset_outplg_sel;
			break;

		case IDC_ABOUT:
			{		
				letter fname[v_sys_maxpath];
				letter vname[v_sys_maxpath];
				int    i, csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);

				if(csel > 0)
				{
					if(!settings.output_plugins.selected[0])
					{
						MessageBox(hwnd, uni("Select and apply the plug-in you need to configure first."), uni("Configuration"), MB_ICONINFORMATION);
						break;
					}

					fennec_get_plugings_path(fname);

					SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETTEXT, (WPARAM)csel, (LPARAM)vname);

					str_cat(fname, vname);
					str_cat(fname, v_sys_lbrary_extension);

					fennec_get_rel_path(vname, fname);

					i=0;
					while(vname[i]) { if(vname[i] == '\\') vname[i] = '/'; i++; }
					i=0;
					while(settings.output_plugins.selected[i]) { if(settings.output_plugins.selected[i] == '\\') settings.output_plugins.selected[i] = '/'; i++; }
					
					if(str_icmp(vname, settings.output_plugins.selected) == 0)
					{
						external_output_about(hwnd);
					}else{
						MessageBox(hwnd, uni("Select and apply the plug-in you need to view information first."), uni("Configuration"), MB_ICONINFORMATION);
					}

				}else if(csel == 0){

					if(settings.output_plugins.selected[0] == 0)
					{
						basewindows_show_about(0);
					}else{
						MessageBox(hwnd, uni("Select and apply the plug-in you need to view information first."), uni("Configuration"), MB_ICONINFORMATION);
					}
				}
			}
			break;

		case IDC_CONFIG:
			{		
				letter fname[v_sys_maxpath];
				letter vname[v_sys_maxpath];
				int    i, csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);

				if(csel > 0)
				{
					if(!settings.output_plugins.selected[0])
					{
						MessageBox(hwnd, uni("Select and apply the plug-in you need to view information first."), uni("Configuration"), MB_ICONINFORMATION);
						break;
					}

					fennec_get_plugings_path(fname);

					SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETTEXT, (WPARAM)csel, (LPARAM)vname);

					str_cat(fname, vname);
					str_cat(fname, v_sys_lbrary_extension);

					fennec_get_rel_path(vname, fname);

					i=0;
					while(vname[i]) { if(vname[i] == '\\') vname[i] = '/'; i++; }
					i=0;
					while(settings.output_plugins.selected[i]) { if(settings.output_plugins.selected[i] == '\\') settings.output_plugins.selected[i] = '/'; i++; }
					
					if(str_icmp(vname, settings.output_plugins.selected) == 0)
					{
						external_output_settings(hwnd);
					}else{
						MessageBox(hwnd, uni("Select and apply the plug-in you need to configure first."), uni("Configuration"), MB_ICONINFORMATION);
					}

				}else if(csel == 0){

					if(settings.output_plugins.selected[0] == 0)
					{
						settings_ui_showpanel(7);
					}else{
						MessageBox(hwnd, uni("Select and apply the plug-in you need to configure first."), uni("Configuration"), MB_ICONINFORMATION);
					}
				}
			}
			break;

		case IDC_SELECT:
offset_outplg_sel:

			{
				letter vname[v_sys_maxpath];
				letter fpath[v_sys_maxpath];
				int    csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);

				if(csel > 0)
				{
					audio_stop();
					audio_uninitialize();

					fennec_get_plugings_path(fpath);

					SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETTEXT, (WPARAM)csel, (LPARAM)vname);

					str_cat(fpath, vname);
					str_cat(fpath, v_sys_lbrary_extension);

					fennec_get_rel_path(settings.output_plugins.selected, fpath);
						
					audio_initialize();

					audio_playlist_switch(0);

				}else if(csel == 0){

					if(settings.output_plugins.selected[0])
					{
						audio_stop();
						audio_uninitialize();
						settings.output_plugins.selected[0] = 0;
						audio_initialize();

						audio_playlist_switch(0);
					}
				}
			
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			t_sys_fs_find_handle shandle;
			letter op_path[v_sys_maxpath];
			letter cop_path[v_sys_maxpath];
			letter fop_path[v_sys_maxpath];
			int    sfound;
			int    lbase, sv = 0, i = 1, spdlen;

			/* <lang> */

			SetDlgItemText(hwnd, static_title, text(oooo_settings_available_output_plugins));
			SetDlgItemText(hwnd, IDC_ABOUT,    text(oooo_settings_about));
			SetDlgItemText(hwnd, IDC_CONFIG,   text(oooo_settings_configuration));
			SetDlgItemText(hwnd, IDC_SELECT,   text(oooo_select));

			/* </lang> */

			SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, 0, (LPARAM)uni("<failsafe: internal wave output>"));

			fennec_get_plugings_path(op_path);

			spdlen = (int)str_len(op_path);

			if(settings.output_plugins.selected[0])
			{
				for(i=(int)str_len(settings.output_plugins.selected); i>0; i--)
					if(settings.output_plugins.selected[i] == '\\' || settings.output_plugins.selected[i] == '/') break;

				lbase = i;
			}else{
				lbase = 0;
			}

			str_cat(op_path, uni("*"));
			str_cat(op_path, v_sys_lbrary_extension);

			sfound = sys_fs_find_start(op_path, cop_path, sizeof(cop_path), &shandle);

			i = 0;

			while(sfound)
			{
				str_cpy(fop_path, op_path);
				fop_path[spdlen] = uni('\0');
				str_cat(fop_path, cop_path);

				if(!output_plugin_checkfile(fop_path))goto outplugin_ignore;

				sv = 0;

				if(settings.output_plugins.selected[0])
				{
					if(!str_cmp(cop_path, settings.output_plugins.selected + lbase + 1)) sv = 1;
				}

				cop_path[str_len(cop_path) - 4] = 0;
				SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)cop_path);

				if(sv)
					SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_SETCURSEL, i + 1, 0);

				i++;

outplugin_ignore:

				sfound = sys_fs_find_next(cop_path, sizeof(cop_path), shandle);
			}

			sys_fs_find_close(shandle);

			if(!settings.output_plugins.selected[0])
				SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_SETCURSEL, 0, 0);
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_dspplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static int cafsel = 0;

	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_PLGLIST: 
            switch(HIWORD(wParam)) 
            {
			case LBN_SELCHANGE: 
point_dsp_sellist:;		
				
				{
					int i, seld = 0, csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
					if(csel == -1)break;
					if(csel > 16)break;

					seld = 0;

					for(i=0; i<(int)settings.dsp_plugins.plugins_count; i++)
					{
						if(settings.dsp_plugins.plugins[i].index == csel)
							seld = 1;
					}

					if(seld)
					{
						CheckDlgButton(hwnd, IDC_DSP_USE, BST_CHECKED);	
					}else{
						CheckDlgButton(hwnd, IDC_DSP_USE, BST_UNCHECKED);
					}

				}
				break;
			}
			break;

		case IDC_DSP_USE:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				if(csel > 16)break;

				for(i=0; i<(int)settings.dsp_plugins.plugins_count; i++)
				{
					if(settings.dsp_plugins.plugins[i].index == csel || settings.dsp_plugins.plugins[i].index - 20 == csel)
					{
						csel = i;
						break;
					}
				}

				if(IsDlgButtonChecked(hwnd, IDC_DSP_USE) == BST_CHECKED)
				{
					if(settings.dsp_plugins.plugins[csel].index >= 20)
					{
						settings.dsp_plugins.plugins[csel].index -= 20;
						if(settings.dsp_plugins.plugins[csel].index < 0)settings.dsp_plugins.plugins[csel].index = 0;
					}
				}else{
					if(settings.dsp_plugins.plugins[csel].index < 20)
					{
						settings.dsp_plugins.plugins[csel].index += 20;
					}
				}
			}
			break;

		case IDC_DSP_ENABLEDSP:
			{
				if(IsDlgButtonChecked(hwnd, IDC_DSP_ENABLEDSP) == BST_CHECKED)
				{
					/*if(!settings.dsp_plugins.enable)
					{
						MessageBox(hwnd, "DSP plug-ins will start working after restarting Fennec", "Enable DSP", MB_ICONINFORMATION);
						settings.dsp_plugins.enablenextstartup = 1;
					}*/
					settings.dsp_plugins.enable = 1;
					dsp_initialize();

				}else{
					//settings.dsp_plugins.enablenextstartup = 0;
					settings.dsp_plugins.enable = 0;

				}
			}	
			break;

		case IDC_REFRESH:
			CheckDlgButton(hwnd, IDC_DSP_ENABLEDSP, BST_CHECKED);
			//settings.dsp_plugins.enable        = 1;


			settings.dsp_plugins.enable        = 0;
			settings.dsp_plugins.plugins_count = 0;

			dsp_uninitialize();
			dsp_initialize();

			settings.dsp_plugins.enable        = 1;

			goto point_dsp_init;



			//MessageBox(hwnd, "DSP plug-ins cache will be refreshed after restarting fennec", "DSP Cache Refresh", MB_ICONINFORMATION);
			break;

		case IDC_CONFIG:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				if(csel > 16)break;

				for(i=0; i<(int)settings.dsp_plugins.plugins_count; i++)
				{
					if(settings.dsp_plugins.plugins[i].index == csel || settings.dsp_plugins.plugins[i].index - 20 == csel)
					{
						csel = i;
						break;
					}
				}

				if(!dsp_showconfig(csel, (void*)hwnd))
				{
					MessageBox(hwnd, uni("Not initialized"), uni("Error"), MB_ICONQUESTION);
				}
			}
			break;

		case IDC_ABOUT:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				if(csel > 16)break;

				for(i=0; i<(int)settings.dsp_plugins.plugins_count; i++)
				{
					if(settings.dsp_plugins.plugins[i].index == csel || settings.dsp_plugins.plugins[i].index - 20 == csel)
					{
						csel = i;
						break;
					}
				}

				if(!dsp_showabout(csel, (void*)hwnd))
				{
					MessageBox(hwnd, uni("Not initialized"), uni("Error"), MB_ICONQUESTION);
				}
			}
			break;

		case IDC_DSP_UP:
			{
				int i, j = -1;
				int csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				if(csel < 1)break;

				for(i=0; i<(int)min(16, settings.dsp_plugins.plugins_count); i++)
				{
					if(settings.dsp_plugins.plugins[i].index == csel)
						j = i;

					if(settings.dsp_plugins.plugins[i].index - 20 == csel)
						j = i;
				}

				for(i=0; i<(int)min(16, settings.dsp_plugins.plugins_count); i++)
				{
					if(settings.dsp_plugins.plugins[i].index == (csel - 1))
						settings.dsp_plugins.plugins[i].index = (char)csel;

					if(settings.dsp_plugins.plugins[i].index - 20 == (csel - 1))
						settings.dsp_plugins.plugins[i].index = (char)(csel + 20);
				}

				if(j != -1)
				{
					if(settings.dsp_plugins.plugins[j].index >= 20)
					{
						settings.dsp_plugins.plugins[j].index = (char)(csel - 1 + 20);
					}else{
						settings.dsp_plugins.plugins[j].index = (char)(csel - 1);
					}
				}

				SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_RESETCONTENT, 0, 0); 

				cafsel = csel - 1;
				goto point_dsp_init;
				
			}
			break;

		case IDC_DSP_DOWN:
			{
				int i, j = -1;
				int csel = (int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				if(csel >= (int)settings.dsp_plugins.plugins_count - 1)break;

				for(i=0; i<(int)min(16, settings.dsp_plugins.plugins_count); i++)
				{
					if(settings.dsp_plugins.plugins[i].index == csel)
						j = i;

					if(settings.dsp_plugins.plugins[i].index - 20 == csel)
						j = i;
				}

				for(i=0; i<(int)min(16, settings.dsp_plugins.plugins_count); i++)
				{
					if(settings.dsp_plugins.plugins[i].index == (csel + 1))
						settings.dsp_plugins.plugins[i].index = (char)csel;

					if(settings.dsp_plugins.plugins[i].index - 20 == (csel + 1))
						settings.dsp_plugins.plugins[i].index = (char)(csel + 20);
				}

				if(j != -1)
				{
					if(settings.dsp_plugins.plugins[j].index >= 20)
					{
						settings.dsp_plugins.plugins[j].index = (char)(csel + 1 + 20);
					}else{
						settings.dsp_plugins.plugins[j].index = (char)(csel + 1);
					}
				}

				SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_RESETCONTENT, 0, 0); 

				cafsel = csel + 1;
				goto point_dsp_init;
				
			}
			break;
		}
		break;

	case WM_INITDIALOG:

		/* <lang> */

		SetDlgItemText(hwnd, static_title,      text(oooo_settings_available_dsp));
		SetDlgItemText(hwnd, IDC_ABOUT,         text(oooo_settings_about));
		SetDlgItemText(hwnd, IDC_CONFIG,        text(oooo_settings_configuration));
		SetDlgItemText(hwnd, IDC_REFRESH,       text(oooo_settings_refresh_cache));
		SetDlgItemText(hwnd, IDC_DSP_UP,        text(oooo_settings_up));
		SetDlgItemText(hwnd, IDC_DSP_DOWN,      text(oooo_settings_down));
		SetDlgItemText(hwnd, IDC_DSP_USE,       text(oooo_settings_use));
		SetDlgItemText(hwnd, IDC_DSP_ENABLEDSP, text(oooo_settings_enable_dsp));

		/* </lang> */

point_dsp_init:;
		{
			unsigned int i, j;

			SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_RESETCONTENT, 0, 0); 

			CheckDlgButton(hwnd, IDC_DSP_ENABLEDSP, settings.dsp_plugins.enable ? BST_CHECKED : BST_UNCHECKED);


			for(j=0; j<settings.dsp_plugins.plugins_count; j++)
			{
				for(i=0; i<settings.dsp_plugins.plugins_count; i++)
				{
					if((unsigned int)settings.dsp_plugins.plugins[i].index == j || (unsigned int)(settings.dsp_plugins.plugins[i].index - 20) == j)
					{
						if(settings.dsp_plugins.plugins[i].index < 16)
						{
							if(str_len(settings.dsp_plugins.plugins[i].name))
							{
								SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, settings.dsp_plugins.plugins[i].index, (LPARAM) settings.dsp_plugins.plugins[i].name);
							}else{
								SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, settings.dsp_plugins.plugins[i].index, (LPARAM) settings.dsp_plugins.plugins[i].path);
							}
						}else{
							if(str_len(settings.dsp_plugins.plugins[i].name))
							{
								SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, settings.dsp_plugins.plugins[i].index - 20, (LPARAM) settings.dsp_plugins.plugins[i].name);
							}else{
								SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, settings.dsp_plugins.plugins[i].index - 20, (LPARAM) settings.dsp_plugins.plugins[i].path);
							}
						}
					}
				}
			}

			SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_SETCURSEL, cafsel, 0);
			goto point_dsp_sellist;
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_encoderplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static int ei = 0;

	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ABOUT:
			{
				unsigned int k = (unsigned int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(k != LB_ERR)
				{
					encoder_plugin_initialize(k);
					encoder_plugin_global_about(k, (void*)hwnd);
				}
			}
			break;

		case IDC_CONFIG:
			{
				unsigned int k = (unsigned int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(k != LB_ERR)
				{
					encoder_plugin_initialize(k);
					encoder_plugin_global_file_settings(k, (void*)hwnd);
				}
			}
			break;

		case IDC_CONFIG_STREAM:
			{
				unsigned int k = (unsigned int)SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_GETCURSEL, 0, 0);
				if(k != LB_ERR)
				{
					encoder_plugin_initialize(k);
					encoder_plugin_global_encoder_settings(k, (void*)hwnd);
				}
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int ec;
			unsigned int i;

			/* <lang> */

			SetDlgItemText(hwnd, static_title,      text(oooo_settings_available_encoders));
			SetDlgItemText(hwnd, IDC_ABOUT,         text(oooo_settings_about));
			SetDlgItemText(hwnd, IDC_CONFIG,        text(oooo_settings_config_file));
			SetDlgItemText(hwnd, IDC_CONFIG_STREAM, text(oooo_settings_config_stream));

			/* </lang> */
			
			ei = encoder_initialize();
			ec = encoder_getencoderscount();

			for(i=0; i<ec; i++)
			{
				SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_ADDSTRING, 0, (LPARAM)encoder_getname(i));
			}
		}
		break;

	case WM_DESTROY:
		/* close only if the function above initialized it */
		if(ei)encoder_uninitialize();
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * 
 */
int settings_proc_generalplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

/*
 * 
 */
int settings_proc_visualplugins(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case list_plugins:
			if(HIWORD(wParam) == LBN_DBLCLK)
				goto offset_select_visualization;
			break;

		case button_about:
			visualizations_about((void*)hwnd);
			break;

		case button_config:
			visualizations_settings((void*)hwnd);
			break;

		case button_select:

offset_select_visualization:

			{
				letter vname[v_sys_maxpath];
				int    csel = (int)SendDlgItemMessage(hwnd, list_plugins, LB_GETCURSEL, 0, 0);

				if(csel > 0)
				{
					str_cpy(settings.visualizations.selected, uni("/visualizations/"));

					SendDlgItemMessage(hwnd, list_plugins, LB_GETTEXT, (WPARAM)csel, (LPARAM)vname);

					str_cat(settings.visualizations.selected, vname);
					str_cat(settings.visualizations.selected, v_sys_lbrary_extension);

					visualizations_initialize(settings.visualizations.selected);

				}else if(csel == 0){

					visualizations_uninitialize();
					settings.visualizations.selected[0] = 0;
				}
			
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			t_sys_fs_find_handle shandle;
			letter vis_path[v_sys_maxpath];
			letter cvis_path[v_sys_maxpath];
			int    sfound;
			int    lbase, sv = 0, i = 1;

			/* <lang> */

			SetDlgItemText(hwnd, static_title,  text(oooo_settings_available_visualizations));
			SetDlgItemText(hwnd, button_about,     text(oooo_settings_about));
			SetDlgItemText(hwnd, button_config,    text(oooo_settings_configuration));
			SetDlgItemText(hwnd, button_select,    text(oooo_select));

			/* </lang> */

			SendDlgItemMessage(hwnd, list_plugins, LB_INSERTSTRING, 0, (LPARAM)uni("<nothing>"));

			str_cpy(vis_path, fennec_get_path(0, 0));
			str_cat(vis_path, uni("/visualizations/*"));
			str_cat(vis_path, v_sys_lbrary_extension);
			
			lbase = (int)str_len(uni("/visualizations/"));

			sfound = sys_fs_find_start(vis_path, cvis_path, sizeof(cvis_path), &shandle);

			while(sfound)
			{
				if(settings.visualizations.selected[0])
				{
					if(!str_cmp(cvis_path, settings.visualizations.selected + lbase)) sv = 1;
					else sv = 0;
				}else{
					sv = 0;
				}

				cvis_path[str_len(cvis_path) - 4] = 0;
				SendDlgItemMessage(hwnd, list_plugins, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)cvis_path);

				if(sv)
					SendDlgItemMessage(hwnd, list_plugins, LB_SETCURSEL, i, 0);

				sfound = sys_fs_find_next(cvis_path, sizeof(cvis_path), shandle);
				
				i++;
			}

			sys_fs_find_close(shandle);

			if(settings.visualizations.selected[0] == 0)
				SendDlgItemMessage(hwnd, list_plugins, LB_SETCURSEL, 0, 0);
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

/*
 * 
 */
int settings_proc_videoout(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case list_plugins:
			if(HIWORD(wParam) == LBN_DBLCLK)
				goto offset_voutplg_sel;
			break;

		case button_config:
		case button_about:
			{		
				letter fname[v_sys_maxpath];
				letter vname[v_sys_maxpath];
				int    i, csel = (int)SendDlgItemMessage(hwnd, list_plugins, LB_GETCURSEL, 0, 0);

				if(!settings.videooutput.selected[0])
				{
					MessageBox(hwnd, uni("Select and apply the plug-in you need to view information first."), uni("Configuration/About"), MB_ICONINFORMATION);
					break;
				}

				fennec_get_plugings_path(fname);

				SendDlgItemMessage(hwnd, list_plugins, LB_GETTEXT, (WPARAM)csel, (LPARAM)vname);

				str_cat(fname, vname);
				str_cat(fname, v_sys_lbrary_extension);

				fennec_get_rel_path(vname, fname);

				i=0;
				while(vname[i]) { if(vname[i] == '\\') vname[i] = '/'; i++; }
				i=0;
				while(settings.videooutput.selected[i]) { if(settings.videooutput.selected[i] == '\\') settings.videooutput.selected[i] = '/'; i++; }
				
				if(str_icmp(vname, settings.videooutput.selected) == 0)
				{
					if(LOWORD(wParam) == button_about)
						videoout_about(hwnd);
					else
						videoout_settings(hwnd);
				}else{
					MessageBox(hwnd, uni("Select and apply the plug-in you need to view information first."), uni("Configuration/About"), MB_ICONINFORMATION);
				}
			}
			break;

		case button_select:
offset_voutplg_sel:

			{
				letter vname[v_sys_maxpath];
				letter fpath[v_sys_maxpath];
				int    csel = (int)SendDlgItemMessage(hwnd, list_plugins, LB_GETCURSEL, 0, 0);

				videoout_uninitialize();
				sys_sleep(0);

				SendDlgItemMessage(hwnd, list_plugins, LB_GETTEXT, (WPARAM)csel, (LPARAM)vname);

				str_cpy(fpath, vname);
				str_cat(fpath, v_sys_lbrary_extension);

				str_cpy(settings.videooutput.selected, fpath);

				videoout_initialize(settings.videooutput.selected);
		
			}
			break;
		}
		break;

			
	case WM_INITDIALOG:
		{
			t_sys_fs_find_handle shandle;
			letter op_path[v_sys_maxpath];
			letter cop_path[v_sys_maxpath];
			letter fop_path[v_sys_maxpath];
			int    sfound;
			int    lbase, sv = 0, i = 1, spdlen;

			/* <lang> */

			//SetDlgItemText(hwnd, static_title, text(oooo_settings_available_output_plugins));
			SetDlgItemText(hwnd, button_about,    text(oooo_settings_about));
			SetDlgItemText(hwnd, button_config,   text(oooo_settings_configuration));
			SetDlgItemText(hwnd, button_select,   text(oooo_select));

			/* </lang> */

			//SendDlgItemMessage(hwnd, IDC_PLGLIST, LB_INSERTSTRING, 0, (LPARAM)uni("<failsafe: internal wave output>"));

			fennec_get_plugings_path(op_path);

			spdlen = (int)str_len(op_path);

			if(settings.videooutput.selected[0])
			{
				for(i=(int)str_len(settings.videooutput.selected); i>0; i--)
					if(settings.videooutput.selected[i] == '\\' || settings.videooutput.selected[i] == '/') break;

				lbase = i;
			}else{
				lbase = 0;
			}

			str_cat(op_path, uni("*"));
			str_cat(op_path, v_sys_lbrary_extension);

			sfound = sys_fs_find_start(op_path, cop_path, sizeof(cop_path), &shandle);

			i = 0;

			while(sfound)
			{
				//str_cpy(fop_path, op_path);
				//fop_path[spdlen] = uni('\0');
				str_cpy(fop_path, cop_path);

				if(!videoout_checkfile(fop_path))goto voutplugin_ignore;

				sv = 0;

				if(settings.videooutput.selected[0])
				{
					if(!str_cmp(cop_path, settings.videooutput.selected)) sv = 1;
				}

				cop_path[str_len(cop_path) - 4] = 0;
				SendDlgItemMessage(hwnd, list_plugins, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)cop_path);

				if(sv)
					SendDlgItemMessage(hwnd, list_plugins, LB_SETCURSEL, i, 0);

				i++;

voutplugin_ignore:

				sfound = sys_fs_find_next(cop_path, sizeof(cop_path), shandle);
			}

			sys_fs_find_close(shandle);

		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

/*
 * themes.
 */
int settings_proc_themes(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			letter themename[v_sys_maxpath];
			int    i = 0, s, csel = -1;

			
			/* <lang> */

			SetDlgItemText(hwnd, static_title,      text(oooo_settings_available_themes));
			SetDlgItemText(hwnd, group_color,       text(oooo_settings_color_selection));
			SetDlgItemText(hwnd, static_hue,        text(oooo_settings_hue));
			SetDlgItemText(hwnd, static_sat,        text(oooo_settings_saturation));
			SetDlgItemText(hwnd, static_light,      text(oooo_settings_lightness));
			SetDlgItemText(hwnd, check_realtime,    text(oooo_settings_color_realtime));
			SetDlgItemText(hwnd, button_switch,     text(oooo_settings_switch));
			SetDlgItemText(hwnd, button_set_color,  text(oooo_settings_set_colors));
			SetDlgItemText(hwnd, button_reset,      text(oooo_settings_reset));

			/* </lang> */

			SendDlgItemMessage(hwnd, slider_hue,   TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(hwnd, slider_sat,   TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(hwnd, slider_light, TBM_SETRANGE, 0, MAKELONG(0, 100));
			
			SendDlgItemMessage(hwnd, slider_hue,   TBM_SETPOS, 1, skins_setcolor(-1 /* get */, 0, 0));
			SendDlgItemMessage(hwnd, slider_sat,   TBM_SETPOS, 1, skins_setcolor(-1 /* get */, 1, 0));
			SendDlgItemMessage(hwnd, slider_light, TBM_SETPOS, 1, skins_setcolor(-1 /* get */, 2, 0));

			while((s = skins_getthemes(i, themename)) != 0)
			{
				if(s == 2 /* current */)csel = i;
				SendDlgItemMessage(hwnd, list_main, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)themename);
				i++;
			}
			if(csel != -1)
				SendDlgItemMessage(hwnd, list_main, LB_GETCURSEL, (WPARAM)csel, 0);
		}
		break;

	case WM_HSCROLL:
		if(IsDlgButtonChecked(hwnd, check_realtime) == BST_CHECKED)
		{
			skins_setcolor((int)SendDlgItemMessage(hwnd, slider_hue, TBM_GETPOS, 0, 0),
				           (int)SendDlgItemMessage(hwnd, slider_sat, TBM_GETPOS, 0, 0),
				           (int)SendDlgItemMessage(hwnd, slider_light, TBM_GETPOS, 0, 0));

		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case button_set_color:
			skins_setcolor((int)SendDlgItemMessage(hwnd, slider_hue, TBM_GETPOS, 0, 0),
				           (int)SendDlgItemMessage(hwnd, slider_sat, TBM_GETPOS, 0, 0),
				           (int)SendDlgItemMessage(hwnd, slider_light, TBM_GETPOS, 0, 0));
			break;

		case button_reset:
			skins_setcolor(-2 /* reset */, 0, 0);
			break;

		case list_main:
			if(HIWORD(wParam) == LBN_DBLCLK)
			{
				int x = (int)SendDlgItemMessage(hwnd, list_main, LB_GETCURSEL, 0, 0);

				if(x != -1)
					skins_settheme(x);

				SendDlgItemMessage(hwnd, slider_hue,   TBM_SETPOS, 1, skins_setcolor(-1 /* get */, 0, 0));
				SendDlgItemMessage(hwnd, slider_sat,   TBM_SETPOS, 1, skins_setcolor(-1 /* get */, 1, 0));
				SendDlgItemMessage(hwnd, slider_light, TBM_SETPOS, 1, skins_setcolor(-1 /* get */, 2, 0));
			}
			break;

		case button_switch:
			{
				int x = (int)SendDlgItemMessage(hwnd, list_main, LB_GETCURSEL, 0, 0);

				if(x != -1)
					skins_settheme(x);
			}
			break;
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}


/*
 * language packs.
 */
int settings_proc_language_packs(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static string mem_files = 0;
	static int    m_count;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			letter fpath[v_sys_maxpath];
			letter spath[v_sys_maxpath];
			letter pname[v_sys_maxpath];
			letter tlang[v_sys_maxpath];
			letter ttitle[v_sys_maxpath];
			letter tauthor[v_sys_maxpath];
			letter tcomments[4096];
			letter selpack[v_sys_maxpath];

			int                   fnd, csel = 0;
			t_sys_fs_find_handle  fh;

			/* <lang> */

			SetDlgItemText(hwnd, static_title,           text(oooo_settings_available_language_packs));
			SetDlgItemText(hwnd, static_title_lang,      text(oooo_language));
			SetDlgItemText(hwnd, static_title_author,    text(oooo_author));
			SetDlgItemText(hwnd, static_title_comments,  text(oooo_comments));
			SetDlgItemText(hwnd, button_select,          text(oooo_select));

			/* </lang> */

			m_count = 0;

			fennec_get_abs_path(selpack, settings.language_packs.pack_path);

			fennec_get_abs_path(fpath, uni("\\packs\\*.txt"));

			fnd = sys_fs_find_start(fpath, pname, sizeof(fpath), &fh);

			while(fnd)
			{
				fennec_get_abs_path(spath, uni("\\packs\\"));
				str_cat(spath, pname);

				m_count++;
				mem_files = (string)sys_mem_realloc(mem_files, m_count * v_sys_maxpath * sizeof(letter));

				str_cpy(mem_files + ((m_count - 1) * v_sys_maxpath), spath);

				lang_check(spath, tlang, ttitle, tauthor, tcomments);

				SendDlgItemMessage(hwnd, list_main, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)tlang);

				if(str_icmp(selpack, spath) == 0)
				{
					csel = m_count - 1;
					
					SetDlgItemText(hwnd, static_language, ttitle);
					SetDlgItemText(hwnd, static_author, tauthor);
					SetDlgItemText(hwnd, text_comments, tcomments);
				}

				fnd = sys_fs_find_next(pname, sizeof(fpath), fh);
			}

			SendDlgItemMessage(hwnd, list_main, LB_SETCURSEL, csel, 0);

			sys_fs_find_close(fh);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case button_select:
			{
				int x = (int)SendDlgItemMessage(hwnd, list_main, LB_GETCURSEL, 0, 0);

				if(x != -1 && x < m_count)
				{
					lang_load(mem_files + (x * v_sys_maxpath));
					fennec_get_rel_path(settings.language_packs.pack_path, mem_files + (x * v_sys_maxpath));
				}
			}
			break;

		case list_main:
			if(HIWORD(wParam) == LBN_SELCHANGE)
			{
				int x = (int)SendDlgItemMessage(hwnd, list_main, LB_GETCURSEL, 0, 0);

				if(x != -1 && x < m_count)
				{
					letter tlang[v_sys_maxpath];
					letter ttitle[v_sys_maxpath];
					letter tauthor[v_sys_maxpath];
					letter tcomments[4096];

					lang_check(mem_files + (x * v_sys_maxpath), tlang, ttitle, tauthor, tcomments);

					SetDlgItemText(hwnd, static_language, ttitle);
					SetDlgItemText(hwnd, static_author, tauthor);
					SetDlgItemText(hwnd, text_comments, tcomments);
				}
			}else if(HIWORD(wParam) == LBN_DBLCLK){


				int x = (int)SendDlgItemMessage(hwnd, list_main, LB_GETCURSEL, 0, 0);

				if(x != -1 && x < m_count)
				{
					lang_load(mem_files + (x * v_sys_maxpath));
					fennec_get_rel_path(settings.language_packs.pack_path, mem_files + (x * v_sys_maxpath));
				}
			}
			break;
		}
		break;

	case WM_DESTROY:
		if(mem_files)
		{
			mem_files = 0;
			sys_mem_free(mem_files);
		}
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

/* local functions ----------------------------------------------------------*/


void local_settag(struct fennec_audiotag_item *tagi, string tival)
{
	if(!tagi)return;

	if(!tival)
	{
		tagi->tdata     = 0;
		tagi->tsize     = 0;
		tagi->tdatai    = 0;
		tagi->tmode     = tag_memmode_static;
		tagi->treserved = 0;
		return;
	}

	tagi->tdata     = tival;
	tagi->tsize     = (unsigned int)str_size(tival);
	tagi->tdatai    = 0;
	tagi->tmode     = tag_memmode_static;
	tagi->treserved = 0;
}

/*
 * get settings.
 */
void local_settings_to_interface(void)
{
	current_settings.general.multiple_instances = settings.general.allow_multi_instances;
	current_settings.general.auto_play          = settings.general.auto_play;
	current_settings.general.scroll_title       = settings.general.scroll_title_taskbar;
	current_settings.general.show_splash        = settings.general.show_splash;
	current_settings.general.always_on_top      = settings.general.always_on_top;
	current_settings.general.base_priority      = (unsigned int)settings.general.base_priority;
	current_settings.general.threads_priority   = (unsigned int)settings.general.threads_priority;

	str_ncpy(current_settings.formatting.title_main   , settings.formatting.main_title     , v_sys_maxpath);
	str_ncpy(current_settings.formatting.title_scroll , settings.formatting.scrolling_title, v_sys_maxpath);
	str_ncpy(current_settings.formatting.playlist_item, settings.formatting.playlist_item  , v_sys_maxpath);

	current_settings.audio_output.buffer_memory      = settings.audio_output.buffer_memory;
	current_settings.audio_output.output_device_id   = settings.audio_output.output_device_id;
}

/*
 * set settings.
 */
void local_interface_to_settings(void)
{
	settings.general.allow_multi_instances = current_settings.general.multiple_instances;
	settings.general.auto_play             = current_settings.general.auto_play;
	settings.general.scroll_title_taskbar  = current_settings.general.scroll_title;
	settings.general.show_splash           = current_settings.general.show_splash;
	settings.general.always_on_top         = current_settings.general.always_on_top;
	settings.general.base_priority         = (unsigned long)current_settings.general.base_priority;
	settings.general.threads_priority      = (unsigned long)current_settings.general.threads_priority;

	str_ncpy(settings.formatting.main_title     , current_settings.formatting.title_main   , v_sys_maxpath);
	str_ncpy(settings.formatting.scrolling_title, current_settings.formatting.title_scroll , v_sys_maxpath);
	str_ncpy(settings.formatting.playlist_item  , current_settings.formatting.playlist_item, v_sys_maxpath);

	settings.audio_output.buffer_memory    = current_settings.audio_output.buffer_memory;
	settings.audio_output.output_device_id = current_settings.audio_output.output_device_id;
		
	if(settings.general.always_on_top)SetWindowPos(window_main, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	else SetWindowPos(window_main, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

/*-----------------------------------------------------------------------------
 fennec, june 2007.
-----------------------------------------------------------------------------*/
