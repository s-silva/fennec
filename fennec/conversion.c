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



/* <dep> win */

#include <shlobj.h>

/* </dep> */



#include "fennec main.h"
#include "fennec audio.h"
#include "fennec help.h"




/* defines ------------------------------------------------------------------*/

#define icon_state_to_be_converted  0
#define icon_state_converting       1
#define icon_state_done_fine        2
#define icon_state_failed           3




/* prototypes ---------------------------------------------------------------*/


static void conversion_list_addfile(string fname);
static void conversion_list_removefile(unsigned long id);
static void conversion_file_list_callback(const string fname);


/* <dep> win */

static int callback_conversion(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void set_win_pos(HWND hwnd, unsigned int id, int x, int y);
static void switch_ui(HWND hwnd, int mode);
static void set_win_size(HWND hwnd, int w, int h);
static unsigned long WINAPI conversion_thread(LPVOID lpParam);

/* </dep> */




/* structs ------------------------------------------------------------------*/

struct conv_file_data
{
	letter  path[v_sys_maxpath];
	int     nameoffset;
	char    selected;
	char    state;
	string  return_text;
};




/* data ---------------------------------------------------------------------*/

static struct conv_file_data  *conversion_list;
static unsigned long           conversion_list_count = 0;
static unsigned long           current_encoder = (unsigned long)-1;
static int                     is_preview_playing = 0;
static int                     is_thread_running = 0;
static int                     is_conversion_paused = 0;
static int                     conversion_cancel = 0;
static int                     conversion_safe_close = 0;
static unsigned long           info_count, info_pos, info_index, info_datarate;
static double                  info_tickspersec, info_ctic;

int                     return_texts[] = {oooo_transcoding_finished, oooo_transcoding_error_in_not_support, oooo_transcoding_error_cant_load_in, oooo_transcoding_error_out_of_mem, oooo_transcoding_error_cant_init_enc, oooo_transcoding_error_cant_make_out}; 

/* <dep> win */

extern      HWND window_conversion;

static HWND        hwnd_conversion;
static UINT_PTR    conversion_timer_id;
static HIMAGELIST  himl_list_icons;

/* </dep> */





/* functions ----------------------------------------------------------------*/

int basewindows_show_conversion(int wmodal)
{
	/* <dep> win */

	DialogBoxParam(instance_fennec, (LPCTSTR)dialog_conversion, wmodal ? window_main : 0, (DLGPROC)callback_conversion, 0);
	
	/* </dep> */

	return 0;
}




/* callbacks ----------------------------------------------------------------*/

/* <dep> win */

/*
 * callback function for user input window.
 */
static int callback_conversion(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{
	case WM_SIZING:
		{
			RECT  *rct = (RECT*)lParam;

			if(rct->right - rct->left < 572)rct->right = 572 + rct->left;
			if(rct->bottom - rct->top < 501)rct->bottom = 501 + rct->top;
		}
		return 1;

	case WM_SIZE:
		{
			set_win_size(hwnd, LOWORD(lParam), HIWORD(lParam));
		}
		break;

	
	case WM_TIMER:
		if(is_thread_running)
		{
			static int mppne = 0;

			if(mppne > 10)
			{
				letter buf[256];
				
				if(info_tickspersec <= 0.0000001)
					swprintf(buf, sizeof(buf)/sizeof(letter), text(oooo_transcoding_printf_speed_kibs), info_datarate);
				else
					swprintf(buf, sizeof(buf)/sizeof(letter), text(oooo_transcoding_printf_speed_kibs_srem), info_datarate, ((1.0 - info_ctic) / info_tickspersec));

				SetDlgItemText(hwnd, static_speed_data, buf);
				mppne = 0;

			}else{
				mppne++;
			}

			SendDlgItemMessage(hwnd, progress_file, PBM_SETPOS, (WPARAM)info_pos, 0);
			SendDlgItemMessage(hwnd, progress_full, PBM_SETPOS, (WPARAM)(((info_index - 1) * 1000) + info_pos), 0);
		}else{
			SendDlgItemMessage(hwnd, progress_file, PBM_SETPOS, 0, 0);
			SendDlgItemMessage(hwnd, progress_full, PBM_SETPOS, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if(wParam == list_conv)
		{
			LV_DISPINFO *lvit = (LV_DISPINFO *)lParam;

			switch(((LPNMHDR)lParam)->code)
			{
			case LVN_GETDISPINFO:
				switch(lvit->item.iSubItem)
				{
				case 0:
					if(conversion_list_count)
					{
						lvit->item.pszText = conversion_list[lvit->item.iItem].path + conversion_list[lvit->item.iItem].nameoffset;
						lvit->item.iImage = conversion_list[lvit->item.iItem].state;
					}
					break;

				case 1:
					if(conversion_list_count)
					{
						lvit->item.pszText = conversion_list[lvit->item.iItem].path;
						lvit->item.iImage = conversion_list[lvit->item.iItem].state;
					}
					break;

				case 2:
					if(conversion_list_count)
					{
						lvit->item.pszText = conversion_list[lvit->item.iItem].return_text;
						lvit->item.iImage = conversion_list[lvit->item.iItem].state;
					}
					break;
				}
				return 0;
			}
			break;
		}
		break;


	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK: case IDCANCEL:
		case button_ok:
		case button_close:
			if(is_thread_running)
			{
				conversion_cancel = 1;
				conversion_safe_close = 1;
			}else{
				DestroyWindow(hwnd);
			}	
			break;

		case button_selall:
		case button_unsel:
			{
				unsigned long   i, sst;
				HWND  hwlist = GetDlgItem(hwnd, list_conv);

				sst = (LOWORD(wParam) == button_selall) ? 1 : 0;

				for(i=0; i<conversion_list_count; i++)
				{
					conversion_list[i].selected = (char)sst;
					ListView_SetCheckState(hwlist, i, sst);
				}
			}
			break;

		case button_clear:
			SendDlgItemMessage(hwnd, list_conv, LVM_DELETEALLITEMS, 0, 0);
			conversion_list_count = 0;
			sys_mem_free(conversion_list);
			break;

		case button_del:
			{
				unsigned int   i;
				HWND  hwlist = GetDlgItem(hwnd, list_conv);

				for(i=0; i<conversion_list_count; i++)
				{
					if(ListView_GetItemState(hwlist, i, LVIS_SELECTED))
					{
						ListView_DeleteItem(hwlist, i);
						conversion_list_removefile(i);
						i--;
					}
				}
			}
			break;

		case combo_format:
			if(HIWORD(wParam) == CBN_SELENDOK)
			{
				unsigned long encnew;

				encnew = (unsigned long)SendDlgItemMessage(hwnd, combo_format, CB_GETCURSEL, 0, 0);

				if(encnew == current_encoder)break;
				if(current_encoder != -1)encoder_plugin_uninitialize(current_encoder);

				current_encoder = encnew;
				encoder_plugin_initialize(current_encoder);

				settings.conversion.last_encoder = current_encoder;
			}
			break;

		case button_plghelp:
			encoder_plugin_global_about(current_encoder, hwnd);
			break;

		case button_plgsettings:
			encoder_plugin_global_file_settings(current_encoder, hwnd);
			break;

		case button_naminghelp:
			MessageBox(hwnd, formattinginfo, text(oooo_transcoding_file_name_syntax), MB_ICONINFORMATION);
			break;

		case button_browseoutputdir:
			{
				letter       fpath[v_sys_maxpath];
				BROWSEINFO   bi;
				LPITEMIDLIST lpi;

				fpath[0] = 0;

				bi.hwndOwner      = hwnd;
				bi.lpszTitle      = text(oooo_transcoding_save_conv_files_to);
				bi.pszDisplayName = fpath;
				bi.lpfn           = 0;
				bi.iImage         = 0;
				bi.lParam         = 0;
				bi.pidlRoot       = 0;
				bi.ulFlags        = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
	
				lpi = SHBrowseForFolder(&bi);
				SHGetPathFromIDList(lpi, fpath);

				if(str_len(fpath))
				{
					SendDlgItemMessage(hwnd, text_outdir, WM_SETTEXT, 0, (LPARAM)fpath);
				}
			}
			break;

		case button_help:
			{
				letter  fpath[v_sys_maxpath];

				str_cpy(fpath, fennec_get_path(0, 0));
				str_cat(fpath, uni("/help/link/fennec help - conversion.html"));

				ShellExecute(hwnd, 0, fpath, 0, 0, SW_SHOW);
			}
			break;

		case button_addfiles:
			{
				string fname = basewindows_show_open(text(oooo_transcoding_add_files_to_conv), settings.conversion.last_path, 0);

				if(fname)
					if(str_len(fname))
						file_list_open_ex(fname, conversion_file_list_callback);
			}
			break;

		case button_tagging:
			{
				int i;

				i = ListView_GetSelectionMark(GetDlgItem(hwnd, list_conv));
				if(i >= 0 && i < (int)conversion_list_count)
					basewindows_show_tagging(1, conversion_list[i].path);
			}
			break;

		case button_adddir:
			{
				letter       fpath[v_sys_maxpath];
				BROWSEINFO   bi;
				LPITEMIDLIST lpi;

				fpath[0] = 0;

				bi.hwndOwner      = hwnd;
				bi.lpszTitle      = text(oooo_transcoding_search_dirs_for_conv);
				bi.pszDisplayName = fpath;
				bi.lpfn           = 0;
				bi.iImage         = 0;
				bi.lParam         = 0;
				bi.pidlRoot       = 0;
				bi.ulFlags        = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
	
				lpi = SHBrowseForFolder(&bi);
				SHGetPathFromIDList(lpi, fpath);

				if(str_len(fpath))
				{
					file_list_open_dir(fpath, conversion_file_list_callback);
				}
			}
			break;
		
		case button_stop:
			conversion_cancel = 1;
			switch_ui(hwnd, 0);
			SetDlgItemText(hwnd, button_start, text(oooo_transcoding_start) );
			break;

		case button_start:

			if(is_thread_running)
			{
				is_conversion_paused ^= 1;
				if(is_conversion_paused)
				{
					SetDlgItemText(hwnd, button_start, text(oooo_transcoding_resume) );
				}else{
					SetDlgItemText(hwnd, button_start, text(oooo_transcoding_pause));
				}

			}else{

				HANDLE         ht;
				unsigned long  tid = 0;

				conversion_cancel    = 0;
				is_thread_running    = 1;
				is_conversion_paused = 0;

				/* stop playing audio */

				if(IsDlgButtonChecked(hwnd, check_stop) == BST_CHECKED) audio_stop();

				switch_ui(hwnd, 1);

				ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)conversion_thread, 0, 0, &tid);
				SetThreadPriority(ht, setting_priority_to_sys(settings.general.threads_priority));

				/* no multithreading?... just call it out! */

				if(ht == INVALID_HANDLE_VALUE) conversion_thread(0);

			}
			break;

		case button_config:
			transcode_settings_show(hwnd, &settings.conversion.trans);
			break;

		case button_preview:
			if(is_preview_playing)
			{
				audio_stop();
				is_preview_playing = 0;
				SetDlgItemText(hwnd, button_preview, text(oooo_transcoding_preview));
			}else{
				int i;

				i = ListView_GetSelectionMark(GetDlgItem(hwnd, list_conv));

				if(i >= 0 && i < (int)conversion_list_count)
				{
					audio_load(conversion_list[i].path);
					audio_play();
					is_preview_playing = 1;
					SetDlgItemText(hwnd, button_preview, text(oooo_transcoding_stop));
				}
			}
			break;
		}
		break;

	case WM_DROPFILES:
		{
			unsigned int     fcount, i;
			letter           fpath[v_sys_maxpath];
			HDROP            hd = (HDROP)wParam;
			HANDLE           hf;
			WIN32_FIND_DATA  fd;
			HWND             hwlist;
			
			hwlist = GetDlgItem(hwnd, list_conv);

			fcount = DragQueryFile(hd, 0xFFFFFFFF, 0, 0);

			for(i=0; i<fcount; i++)
			{
				memset(fpath, 0, sizeof(fpath));
				DragQueryFile(hd, i, fpath, sizeof(fpath));

				hf = FindFirstFile(fpath, &fd);

				if(hf == INVALID_HANDLE_VALUE)continue;
				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					file_list_open_dir(fpath, conversion_file_list_callback);
					FindClose(hf);
					continue;
				}

				FindClose(hf);

				/* add to conversion list */

				conversion_file_list_callback(fpath);
			}

			DragFinish(hd);
		}
		break;

	
	case WM_INITDIALOG:

		hwnd_conversion   = hwnd;
		window_conversion = hwnd; /* for external stuff */
		conversion_safe_close = 0;

		/* <lang> */

		SetDlgItemText(hwnd, button_help,     text(oooo_help  ));
		SetDlgItemText(hwnd, button_close,    text(oooo_close ));
		
		SetDlgItemText(hwnd, frame_main,      text(oooo_transcoding_files              ));
		SetDlgItemText(hwnd, button_addfiles, text(oooo_transcoding_add_files          ));
		SetDlgItemText(hwnd, button_adddir,   text(oooo_transcoding_add_dirs           ));
		SetDlgItemText(hwnd, check_stop,      text(oooo_transcoding_stop_playback      ));
		SetDlgItemText(hwnd, button_preview,  text(oooo_transcoding_preview            ));
		SetDlgItemText(hwnd, button_tagging,  text(oooo_transcoding_edit_tags          ));
		SetDlgItemText(hwnd, button_config,   text(oooo_transcoding_config             ));
		SetDlgItemText(hwnd, button_start,    text(oooo_transcoding_start              ));
		SetDlgItemText(hwnd, button_stop,     text(oooo_transcoding_stop               ));
		SetDlgItemText(hwnd, button_selall,   text(oooo_transcoding_select_all         ));
		SetDlgItemText(hwnd, button_unsel,    text(oooo_transcoding_select_none        ));
		SetDlgItemText(hwnd, button_clear,    text(oooo_transcoding_clear              ));
		SetDlgItemText(hwnd, button_del,      text(oooo_transcoding_remove             ));
		SetDlgItemText(hwnd, frame_format,    text(oooo_transcoding_destination_format ));
		SetDlgItemText(hwnd, frame_output,    text(oooo_transcoding_output             ));
		SetDlgItemText(hwnd, static_outdir,      text(oooo_transcoding_location        ));
		SetDlgItemText(hwnd, static_naming,      text(oooo_transcoding_naming          ));
		SetDlgItemText(hwnd, static_state,       text(oooo_transcoding_format          ));
		SetDlgItemText(hwnd, button_plgsettings, text(oooo_transcoding_settings        ));
	

		/* </lang> */


		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(instance_fennec, (LPCTSTR)IDI_MAIN));
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(instance_fennec, (LPCTSTR)IDI_MAIN));

		/* user interface stuff */

		switch_ui(hwnd, 0);
		{
			if(settings.conversion.winplace.showCmd == (unsigned int)-1)
			{
				RECT rct;
				GetClientRect(hwnd, &rct);
				set_win_size(hwnd, rct.right, rct.bottom);
			}else{
				SetWindowPlacement(hwnd, &settings.conversion.winplace);
			}
		}

		/* adding headers to the list view */

		{
			LVCOLUMN lvc;
			RECT     rct;
			HWND     hwlist = GetDlgItem(hwnd, list_conv);
			HICON    cico;

			GetClientRect(hwlist, &rct);

			ListView_SetExtendedListViewStyle(hwlist, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			lvc.mask    = LVCF_TEXT | LVCF_WIDTH;

			lvc.cx      = 100;
			lvc.pszText = text(oooo_transcoding_list_head_conv_report);
			SendMessage(hwlist, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

			lvc.cx      = rct.right - 310;
			lvc.pszText = text(oooo_transcoding_list_head_file_loc);
			SendMessage(hwlist, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

			lvc.cx      = 200;
			lvc.pszText = text(oooo_transcoding_list_head_file_name);
			SendMessage(hwlist, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
	
			/* image list stuff */
			

			himl_list_icons = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32, 1, 1);
			
			cico = LoadIcon(instance_fennec, MAKEINTRESOURCE(icon_start));
			ImageList_AddIcon(himl_list_icons, cico);
			DestroyIcon(cico);

			cico = LoadIcon(instance_fennec, MAKEINTRESOURCE(icon_converting));
			ImageList_AddIcon(himl_list_icons, cico);
			DestroyIcon(cico);

			cico = LoadIcon(instance_fennec, MAKEINTRESOURCE(icon_finished));
			ImageList_AddIcon(himl_list_icons, cico);
			DestroyIcon(cico);

			cico = LoadIcon(instance_fennec, MAKEINTRESOURCE(icon_failed));
			ImageList_AddIcon(himl_list_icons, cico);
			DestroyIcon(cico);



			ListView_SetImageList(hwlist, himl_list_icons, LVSIL_SMALL); 
		}

		/* add encoders list */

		encoder_initialize();
		{
			unsigned int i = 0;
			string       ename;

			ename = encoder_getname(i);

			while(ename)
			{
				SendDlgItemMessage(hwnd, combo_format, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)ename);
				i++;
				ename = encoder_getname(i);
			}

			if(settings.joining.last_encoder < i)
			{
				SendDlgItemMessage(hwnd, combo_format, CB_SETCURSEL, (WPARAM)settings.conversion.last_encoder, 0);
				current_encoder = settings.conversion.last_encoder;
				encoder_plugin_initialize(current_encoder);
			}
		}

		/* load saved stuff */

		SetDlgItemText(hwnd, text_naming, settings.conversion.last_formatting);
		SetDlgItemText(hwnd, text_outdir, settings.conversion.last_path);
		CheckDlgButton(hwnd, check_stop,  settings.conversion.stop_playback == 1 ? BST_CHECKED : BST_UNCHECKED);
		
		SendDlgItemMessage(hwnd_conversion, progress_file, PBM_SETRANGE32, (WPARAM)(int)0, (LPARAM)1000);


		conversion_timer_id = SetTimer(hwnd, 400101, 30, 0);
		break;

	case WM_DESTROY:

		KillTimer(hwnd, conversion_timer_id);

		if(is_thread_running)
		{
			conversion_cancel = 1;
			Sleep(0);
		}

		encoder_uninitialize();

		GetDlgItemText(hwnd, text_naming, settings.conversion.last_formatting, sizeof(settings.conversion.last_formatting) / sizeof(letter));
		GetDlgItemText(hwnd, text_outdir, settings.conversion.last_path, sizeof(settings.conversion.last_path) / sizeof(letter));

		if(IsDlgButtonChecked(hwnd, check_stop) == BST_CHECKED)
			settings.conversion.stop_playback = 1;
		else
			settings.conversion.stop_playback = 0;
		

		if(conversion_list_count)sys_mem_free(conversion_list);
		conversion_list_count = 0;


		GetWindowPlacement(hwnd, &settings.conversion.winplace);

		ImageList_Destroy(himl_list_icons);

		window_conversion = 0;

		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}

/* </dep> */





/* locals -------------------------------------------------------------------*/



static void set_win_pos(HWND hwnd, unsigned int id, int x, int y)
{
	HWND ihwnd;
	RECT rct;

	ihwnd = GetDlgItem(hwnd, id);


	if(y >= 0)
	{
		GetClientRect(ihwnd, &rct);
		MoveWindow(ihwnd, x, y, rct.right - rct.left, rct.bottom - rct.top, 1);
	
	}else{

		if(y < 0)y = -y;

		SetWindowPos(ihwnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	}
}

static void set_win_size(HWND hwnd, int w, int h)
{
#define hdlu2pix(x) ((int)((float)x * 1.5f))
#define vdlu2pix(y) ((int)((float)y * 1.65f))
#define hpix2dlu(x) ((int)((float)x / 1.5f))
#define vpix2dlu(y) ((int)((float)y / 1.65f))
#define setwposx(id, x, y, w, h) (MoveWindow(GetDlgItem(hwnd, id), hdlu2pix((x)), vdlu2pix((y)), hdlu2pix((w)), (((h) > 0) ? vdlu2pix((h)) : (-(h))), 1))
#define setwpos(id, x, y) (set_win_pos(hwnd, id, hdlu2pix((x)), vdlu2pix((y))))

	int lpx, lpy;
	int dw = hpix2dlu(w), dh = vpix2dlu(h);
	
	setwposx(list_conv, 14, 20, dw - 98 - 14, dh - 111 - 20);
	setwposx(frame_main, 7, 7, dw - 14, dh - 88);
	
	lpx = dw - 98 + 7;
	lpy = dh - 107;

	setwpos(button_addfiles, lpx, 20);
	setwpos(button_adddir,   lpx, 37);
	setwpos(static_bar1,     lpx, -56);
	setwpos(check_stop,      lpx, 63);
	setwpos(button_preview,  lpx, 77);
	setwpos(button_tagging,  lpx, 94);
	setwpos(static_bar2,     lpx, -115);
	setwpos(button_config,   lpx, 122);
	setwpos(static_bar3,     lpx, -143);
	setwpos(button_start,    lpx, 151);
	setwpos(button_stop,     lpx, 170);
	setwpos(static_logo,     lpx, -188);

	setwpos(button_selall,   14, lpy);
	setwpos(button_unsel,    94, lpy);

	setwpos(button_clear,    lpx - 113, lpy);
	setwpos(button_del,      lpx - 60, lpy);

	setwpos(button_help,     7, dh - 7 - 10);
	setwpos(button_close,    dw - 57, dh - 7 - 10);
	
	setwposx(static_info,    71, dh - 17, dw - 142, 14);

	lpx = (dw / 2) + 14;

	setwposx(frame_format,    7, dh - 78, lpx - 14, 52);
	setwposx(frame_output,    lpx, dh - 78, dw - lpx - 7, 52);


	setwposx(static_bar4,     7, dh - 23, dw - 14, -2 /* in pixels */);

	setwpos(static_state,     14, dh - 65);
	setwpos(static_speed,     14, dh - 50);

	setwposx(combo_format,    14 + 57, dh - 65, lpx - 14 - 14 - 57, -1);

	setwposx(static_state_data, 14 + 57, dh - 65, lpx - 14 - 14 - 57, 14);
	setwposx(static_speed_data, 14 + 57, dh - 50, lpx - 14 - 14 - 57, 14);

	setwpos(button_plghelp,      lpx - 32, dh - 50);
	setwpos(button_plgsettings,  lpx - 32 - 62, dh - 50);

	setwpos(static_outdir,    lpx + 7, dh - 65);
	setwpos(static_naming,    lpx + 7, dh - 50);
	setwpos(button_browseoutputdir,  dw - 14 - 17, dh - 65);
	setwpos(button_naminghelp,       dw - 14 - 17, dh - 50);

	setwposx(text_outdir,    lpx + 49, dh - 65, lpx - 42 - 14 - 54, 12);
	setwposx(text_naming,    lpx + 49, dh - 50, lpx - 42 - 14 - 54, 12);

	setwposx(progress_full,    lpx + 49, dh - 65, lpx - 22 - 14 - 54, 12);
	setwposx(progress_file,    lpx + 49, dh - 50, lpx - 22 - 14 - 54, 12);

	Sleep(0);
}


static void switch_ui(HWND hwnd, int mode)
{
	static letter enc_name[260];
	int enbt;   

	if(mode == 1) /* converting */
	{
		enbt = 0;

		ShowWindow(GetDlgItem(hwnd, static_state_data), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, static_speed_data), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, combo_format), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, button_plghelp), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, button_plgsettings), SW_HIDE);

		ShowWindow(GetDlgItem(hwnd, progress_full), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, progress_file), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, text_outdir), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, text_naming), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, button_browseoutputdir), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, button_naminghelp), SW_HIDE);

		SetDlgItemText(hwnd, static_state, text(oooo_transcoding_details));
		SetDlgItemText(hwnd, static_outdir, text(oooo_transcoding_all_files));
		SetDlgItemText(hwnd, static_naming, text(oooo_transcoding_current_file));

		SetDlgItemText(hwnd, static_state_data, encoder_getname(current_encoder));

	}else{	 /* manage */

		enbt = 1;

		ShowWindow(GetDlgItem(hwnd, static_state_data), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, static_speed_data), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, combo_format), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, button_plghelp), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, button_plgsettings), SW_SHOW);

		ShowWindow(GetDlgItem(hwnd, progress_full), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, progress_file), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, text_outdir), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, text_naming), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, button_browseoutputdir), SW_SHOW);
		ShowWindow(GetDlgItem(hwnd, button_naminghelp), SW_SHOW);

		SetDlgItemText(hwnd, static_state, text(oooo_transcoding_format));
		SetDlgItemText(hwnd, static_outdir, text(oooo_transcoding_location));
		SetDlgItemText(hwnd, static_naming, text(oooo_transcoding_naming));
	}

	/* EnableWindow(GetDlgItem(hwnd, list_conv),       enbt); */
	EnableWindow(GetDlgItem(hwnd, button_selall),   enbt);
	EnableWindow(GetDlgItem(hwnd, button_unsel),    enbt);
	EnableWindow(GetDlgItem(hwnd, button_clear),    enbt);
	EnableWindow(GetDlgItem(hwnd, button_del),      enbt);
	EnableWindow(GetDlgItem(hwnd, button_clear),    enbt);
	EnableWindow(GetDlgItem(hwnd, button_config),   enbt);
	EnableWindow(GetDlgItem(hwnd, button_clear),    enbt);
	EnableWindow(GetDlgItem(hwnd, button_tagging),  enbt);
	EnableWindow(GetDlgItem(hwnd, button_preview),  enbt);
	EnableWindow(GetDlgItem(hwnd, button_clear),    enbt);
	EnableWindow(GetDlgItem(hwnd, check_stop),      enbt);
	EnableWindow(GetDlgItem(hwnd, button_adddir),   enbt);
	EnableWindow(GetDlgItem(hwnd, button_addfiles), enbt);

}


static void conversion_list_addfile(string fname)
{
	int    i;

	if(!conversion_list_count)
		conversion_list = (struct conv_file_data*) sys_mem_alloc((++conversion_list_count) * sizeof(struct conv_file_data));
	else
		conversion_list = (struct conv_file_data*) sys_mem_realloc(conversion_list, (++conversion_list_count) * sizeof(struct conv_file_data));

	if(!conversion_list) report("out of memory, conversion_list_addfile", rt_error);
	
	str_cpy(conversion_list[conversion_list_count - 1].path, fname);

	i = (int)str_len(fname);

	for(;i>0;i--)if(fname[i] == '\\' || fname[i] == '/')break;

	conversion_list[conversion_list_count - 1].nameoffset  = i + 1;
	conversion_list[conversion_list_count - 1].selected    = 0;
	conversion_list[conversion_list_count - 1].state       = icon_state_to_be_converted;
	conversion_list[conversion_list_count - 1].return_text = uni("");
}

static void conversion_list_removefile(unsigned long id)
{
	memmove((char*)conversion_list + (sizeof(struct conv_file_data) * id), (char*)conversion_list + (sizeof(struct conv_file_data) * (id + 1)), (sizeof(struct conv_file_data) * (conversion_list_count - id - 1)) );
	conversion_list_count--;
}

static void conversion_file_list_callback(const string fname)
{
	LVITEM           lvi;
	HWND             hwlist;
	
	hwlist = GetDlgItem(hwnd_conversion, list_conv);

	conversion_list_addfile(fname);
				
	/* list view */

	lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE; 
	lvi.state     = 0; 
	lvi.stateMask = 0;
	lvi.iImage    = I_IMAGECALLBACK;
	lvi.iItem     = conversion_list_count - 1;
	lvi.iSubItem  = 0;
	lvi.lParam    = 0;
	lvi.pszText   = LPSTR_TEXTCALLBACK;

	SendMessage(hwlist, LVM_INSERTITEM, 0, (LPARAM)&lvi);
	ListView_SetItemText(hwlist, conversion_list_count - 1, 1, LPSTR_TEXTCALLBACK);

	ListView_SetCheckState(hwlist, conversion_list_count - 1, 1);
}









/*-----------------------------------------------------------------------------
 *
 * Conversion process
 *
 *---------------------------------------------------------------------------*/


static void setposmsg_conversion(double mp)
{
	static unsigned long ltime = 0;
	static double lmp = 0.0;
	unsigned long ntime, tdiff;
	
	info_pos =  (unsigned long)(mp * 1000.0f);
	
	ntime = GetTickCount(); /* time in ms */

	if(ntime <= ltime || ltime == 0)
	{
		ltime = ntime;
		return;
	}


	tdiff = ntime - ltime;

	info_datarate = (settings.conversion.trans.general.buffersize * 1000) / tdiff; /* KiB's sec */
	
	if(mp > lmp && tdiff)
		info_tickspersec = ((mp - lmp) * 1000.0) / (double)tdiff;

	info_ctic = mp;

	ltime = ntime;
	lmp = mp;
}


static unsigned long WINAPI conversion_thread(LPVOID lpParam) 
{ 
	unsigned long           ri, i = 0, c, rc = 0;
	letter                  fpath[v_sys_maxpath];
	letter                  mpath[v_sys_maxpath];
	letter                  dtext[1024];
	letter                  buf[250];
	unsigned int            bsize;
	unsigned long           ipid, bi;
	struct fennec_audiotag  at;
	SYSTEMTIME              st;
	FILETIME                ft;
	int                     rets;
	HWND                    hwnd_listview;

	/* don't disturb */

	is_thread_running = 1;
	Sleep(1);


	/* we're converting, there's nothing to start again, so rename 'start' button to 'pause' */

	SetDlgItemText(hwnd_conversion, button_start, text(oooo_transcoding_pause));


	/* calculate file count that needs to be converted; if it's zero, question 'insane?' */
	
	hwnd_listview = GetDlgItem(hwnd_conversion, list_conv);

	c = (unsigned long)ListView_GetItemCount(hwnd_listview);

	for(i=0; i<c; i++)
		if(ListView_GetCheckState(hwnd_listview, i)) rc++; /* increase real (selections) count */

	/* oh man! you're stupid or what? */

	if(!rc) MessageBox(0, text(oooo_transcoding_error_msg_sel_files), text(oooo_transcoding_error_msgt_sel_files), MB_ICONINFORMATION);

	info_count = rc;

	if(!c || current_encoder == -1)
	{
		if(current_encoder == -1)
			MessageBox(0, text(oooo_transcoding_error_msg_enc_select), text(oooo_transcoding_error_msgt_enc_select), MB_ICONINFORMATION);

		goto point_ct_end1;
	}

	SendDlgItemMessage(hwnd_conversion, progress_full, PBM_SETRANGE32, (WPARAM)(int)0, (LPARAM)(int)(rc * 1000));

	ri = 0;

	for(i=0; i<c; i++)
	{
		if(!ListView_GetCheckState(hwnd_listview, i)) continue;
		if(!conversion_list[i].path) continue;
		if(!conversion_list[i].path[0]) continue;

		conversion_list[i].state = icon_state_converting;

		ListView_Update(hwnd_listview, i);


		str_cpy(fpath, conversion_list[i].path);

		ri++;
		info_index = ri;

		/* stop without doing anything, if the user canceled the operation */

		if(conversion_cancel) goto point_ct_end1;
		
		/* create path */

		GetDlgItemText(hwnd_conversion, text_outdir, mpath, sizeof(mpath));

		if(mpath[0])
		{
			int mplen = (int)str_len(mpath);
			if(!mplen) continue;

			mplen--;

			if( (mpath[mplen] != uni('\\')) && (mpath[mplen] != uni('/')) )
			{
				mpath[mplen + 1] = uni('\\');
				mpath[mplen + 2] = 0;
			}
		}else{
			continue;
		}

		if(!path_make_co(mpath))
		{
			MessageBox(0, text(oooo_transcoding_error_msgt_inv_path), text(oooo_transcoding_error_msgt_inv_path), MB_ICONINFORMATION);
			goto point_ct_end1;
		}


		/* get and fix buffersize */

		bsize = settings.conversion.trans.general.buffersize;

		if(bsize < 64 || bsize > 25600) bsize = 64;
		bsize *= 1024; /* KiB to bytes */

		/* set file details */

		str_cpy(dtext, text(oooo_transcoding_converting));
		str_cat(dtext, uni(": \""));
		str_cat(dtext, fpath);
		str_cat(dtext, uni("\""));

		SetDlgItemText(hwnd_conversion, static_info, dtext);


		/* generate file name from tags */

		memset(buf, 0, sizeof(buf));

		ipid = audio_input_tagread(fpath, &at);
		GetDlgItemText(hwnd_conversion, text_naming, buf, sizeof(buf));
		tags_translate(buf, &at, fpath);
		
		/* format generated file name */

		bi = 0;

		while(buf[bi])
		{
			if(buf[bi] == '\\'){ buf[bi] = '-';  goto point_cr_end; }
			if(buf[bi] == '/' ){ buf[bi] = '-';  goto point_cr_end; }
			if(buf[bi] == ':' ){ buf[bi] = '-';  goto point_cr_end; }
			if(buf[bi] == '*' ){ buf[bi] = '-';  goto point_cr_end; }
			if(buf[bi] == '?' ){ buf[bi] = '-';  goto point_cr_end; }
			if(buf[bi] == '\"'){ buf[bi] = '\''; goto point_cr_end; }
			if(buf[bi] == '<' ){ buf[bi] = '[';  goto point_cr_end; }
			if(buf[bi] == '>' ){ buf[bi] = ']';  goto point_cr_end; }
			if(buf[bi] == '|' ){ buf[bi] = '-';  goto point_cr_end; }
			
		point_cr_end:
			bi++;
		}

		if(!str_len(buf)) str_cpy(buf, uni("Unknown"));
		str_cat(mpath, buf);

		/* convert file */

		rets = audioconvert_convertfile_ex(fpath, mpath, current_encoder, bsize, &conversion_cancel, &is_conversion_paused, (audioconvert_file_pfunc) setposmsg_conversion, &settings.conversion.trans);

		if(!rets)
			conversion_list[i].state = icon_state_done_fine;
		else
			conversion_list[i].state = icon_state_failed;

		if(rets <= 0 && rets >= -5) conversion_list[i].return_text = text(return_texts[-rets]);

		ListView_Update(hwnd_listview, i);

		/* do tagging stuff */

		if(!rets)
		{
			if( settings.conversion.trans.tagging.e_title        == 1 &&
				settings.conversion.trans.tagging.e_artist       == 1 &&
				settings.conversion.trans.tagging.e_album        == 1 &&
				settings.conversion.trans.tagging.e_year         == 1 &&
				settings.conversion.trans.tagging.e_genre        == 1 &&
				settings.conversion.trans.tagging.e_comments     == 1 &&
				settings.conversion.trans.tagging.enable_tagging == 1)
			{
				struct fennec_audiotag_item  ti;

				memcpy(&ti, &at.tag_encodedby, sizeof(struct fennec_audiotag_item));

				at.tag_encodedby.tdata = fennec_u_player_version_text;
				at.tag_encodedby.tsize = (int)str_size(fennec_u_player_version_text);
				at.tag_encodedby.tmode = tag_memmode_static;

				audio_input_tagwrite(mpath, &at);

				memcpy(&at.tag_encodedby, &ti, sizeof(struct fennec_audiotag_item));

			}else if(settings.conversion.trans.tagging.enable_tagging == 1){
				
				unsigned int  bkp_title_size     = 0;
				string        bkp_title_data     = 0;
				unsigned int  bkp_album_size     = 0;
				string        bkp_album_data     = 0;
				unsigned int  bkp_artist_size    = 0;
				string        bkp_artist_data    = 0;
				unsigned int  bkp_comments_size  = 0;
				string        bkp_comments_data  = 0;
				unsigned int  bkp_genre_size     = 0;
				string        bkp_genre_data     = 0;
				unsigned int  bkp_year_size      = 0;
				string        bkp_year_data      = 0;
				char          bkp_year_idata     = 0;
				int           bkp_genre_idata    = 0;

				/* title */
				if(!settings.conversion.trans.tagging.e_title)
				{
					bkp_title_size = at.tag_title.tsize;
					bkp_title_data = at.tag_title.tdata;
					at.tag_title.tsize = (unsigned int)str_size(settings.conversion.trans.tagging.d_title) + sizeof(letter);
					at.tag_title.tdata = settings.conversion.trans.tagging.d_title;
				}

				/* album */
				if(!settings.conversion.trans.tagging.e_album)
				{
					bkp_album_size = at.tag_album.tsize;
					bkp_album_data = at.tag_album.tdata;
					at.tag_album.tsize = (unsigned int)str_size(settings.conversion.trans.tagging.d_album) + sizeof(letter);
					at.tag_album.tdata = settings.conversion.trans.tagging.d_album;
				}

				/* artist */
				if(!settings.conversion.trans.tagging.e_artist)
				{
					bkp_artist_size = at.tag_artist.tsize;
					bkp_artist_data = at.tag_artist.tdata;
					at.tag_artist.tsize = (unsigned int)str_size(settings.conversion.trans.tagging.d_artist) + sizeof(letter);
					at.tag_artist.tdata = settings.conversion.trans.tagging.d_artist;
				}

				/* year */
				if(!settings.conversion.trans.tagging.e_year)
				{
					bkp_year_size = at.tag_year.tsize;
					bkp_year_data = at.tag_year.tdata;
					bkp_year_idata = (char)at.tag_year.tdatai;
					at.tag_year.tsize = (unsigned int)str_size(settings.conversion.trans.tagging.d_year) + sizeof(letter);
					at.tag_year.tdata = settings.conversion.trans.tagging.d_year;
					at.tag_genre.tdatai = 0;
				}

				/* comments */
				if(!settings.conversion.trans.tagging.e_comments)
				{
					bkp_comments_size = at.tag_comments.tsize;
					bkp_comments_data = at.tag_comments.tdata;
					at.tag_comments.tsize = (unsigned int)str_size(settings.conversion.trans.tagging.d_comments) + sizeof(letter);
					at.tag_comments.tdata = settings.conversion.trans.tagging.d_comments;
				}

				/* genre */
				if(!settings.conversion.trans.tagging.e_genre)
				{
					bkp_genre_size = at.tag_genre.tsize;
					bkp_genre_data = at.tag_genre.tdata;
					bkp_genre_idata = at.tag_genre.tdatai;
					at.tag_genre.tsize = (unsigned int)str_size(settings.conversion.trans.tagging.d_genre) + sizeof(letter);
					at.tag_genre.tdata = settings.conversion.trans.tagging.d_genre;
					at.tag_genre.tdatai = 0;
				}

				audio_input_tagwrite(mpath, &at);

				/* title */
				if(!settings.conversion.trans.tagging.e_title)
				{
					at.tag_title.tsize = bkp_title_size;
					at.tag_title.tdata = bkp_title_data;
				}

				/* album */
				if(!settings.conversion.trans.tagging.e_album)
				{
					at.tag_album.tsize = bkp_album_size;
					at.tag_album.tdata = bkp_album_data;
				}

				/* artist */
				if(!settings.conversion.trans.tagging.e_artist)
				{
					at.tag_artist.tsize = bkp_artist_size;
					at.tag_artist.tdata = bkp_artist_data;
				}

				/* year */
				if(!settings.conversion.trans.tagging.e_year)
				{
					at.tag_year.tsize  = bkp_year_size ;
					at.tag_year.tdata  = bkp_year_data ;
					at.tag_year.tdatai = bkp_year_idata;
				}

				/* comments */
				if(!settings.conversion.trans.tagging.e_comments)
				{
					at.tag_comments.tsize = bkp_comments_size;
					at.tag_comments.tdata = bkp_comments_data;
				}

				/* genre */
				if(!settings.conversion.trans.tagging.e_genre)
				{
					at.tag_genre.tsize  = bkp_genre_size;
					at.tag_genre.tdata  = bkp_genre_data;
					at.tag_genre.tdatai = bkp_genre_idata;
				}

			}

			audio_input_tagread_known(ipid, 0, &at); /* free */

			/* set time */

			
			if(settings.conversion.trans.misc.date_and_time_mode == 0 /* original */)
			{
				HANDLE hfile;
				HANDLE hsfile;

				hsfile = CreateFile(fpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
				hfile  = CreateFile(mpath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
				
				if(hfile != INVALID_HANDLE_VALUE && hsfile != INVALID_HANDLE_VALUE)
				{
					GetFileTime(hsfile, 0, 0, &ft);
					SetFileTime(hfile , 0, 0, &ft);
				}
				
				if(hfile  != INVALID_HANDLE_VALUE)CloseHandle(hfile);
				if(hsfile != INVALID_HANDLE_VALUE)CloseHandle(hsfile);

			}else if(settings.conversion.trans.misc.date_and_time_mode == 2 /* custom/user, there's no need to set current time cuz the system will do it automatically if we're keeping our mouths shut ;-) */){
				HANDLE hfile;

				GetSystemTime(&st);

				if(settings.conversion.trans.misc.custom_year   >= 1601 && settings.conversion.trans.misc.custom_year   <= 30827) st.wYear   = (WORD)settings.conversion.trans.misc.custom_year;
				if(settings.conversion.trans.misc.custom_month  >= 1    && settings.conversion.trans.misc.custom_month  <= 12)    st.wMonth  = (WORD)settings.conversion.trans.misc.custom_month;
				if(settings.conversion.trans.misc.custom_date   >= 1    && settings.conversion.trans.misc.custom_date   <= 31)    st.wDay    = (WORD)settings.conversion.trans.misc.custom_date;
				if(settings.conversion.trans.misc.custom_hour   >= 0    && settings.conversion.trans.misc.custom_hour   <= 24)    st.wHour   = (WORD)settings.conversion.trans.misc.custom_hour;
				if(settings.conversion.trans.misc.custom_minute >= 1    && settings.conversion.trans.misc.custom_minute <= 60)    st.wMinute = (WORD)settings.conversion.trans.misc.custom_minute;

				st.wDayOfWeek    = 0;
				st.wMilliseconds = 1;
				st.wSecond       = 1;

				SystemTimeToFileTime(&st, &ft);
				

				hfile = CreateFile(mpath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
				
				if(hfile != INVALID_HANDLE_VALUE)
				{
					SetFileTime(hfile, 0, 0, &ft);

					CloseHandle(hfile);
				}
			}

		} /* if(!rets) - if conversion was okay */

		/* set progress marker */

		SendDlgItemMessage(hwnd_conversion, progress_full, PBM_SETPOS, (WPARAM)ri * 1000, 0);

	}

	SetDlgItemText(hwnd_conversion, static_info, text(oooo_transcoding_finished));

point_ct_end1:

	is_thread_running = 0;

	if(!conversion_safe_close)
	{
		switch_ui(hwnd_conversion, 0);
		SetDlgItemText(hwnd_conversion, button_start, text(oooo_transcoding_start));
	}else{
		SendMessage(hwnd_conversion, WM_DESTROY, 0, 0);
	}
    return 0; 
} 





/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
