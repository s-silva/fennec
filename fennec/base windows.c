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

#include <shlobj.h>
#include <errno.h>
#include <direct.h>

#include "fennec main.h"
#include "fennec audio.h"
#include "fennec help.h"



/* structs ------------------------------------------------------------------*/

typedef struct rip_tag
{
	letter title   [256];
	letter album   [256];
	letter artist  [256];
	letter genre   [256];
	letter comments[256];

}rip_tag;


typedef struct join_data
{
	double spos;                  /* start position fraction */
	double epos;                  /* end position fraction */
}join_data;




/* defines ------------------------------------------------------------------*/

#define uinput_size       1024

#define misc_type_orig    1       /* misc dialog -> set source time */
#define misc_type_current 2       /* misc dialog -> set current time */
#define misc_type_user    3       /* misc dialog -> set custom time */

#define open_default      uni("Unknown")
#define open_all          uni("All Supported Formats")
#define open_playlist     uni("Playlist Files")
#define open_pexten(i)    ((i) == 0 ? uni("txt") : ((i) == 1 ? uni("m3u") : 0))



/* prototypes ---------------------------------------------------------------*/

void str_tag_to_win(string str);
void str_win_to_tag(string str);

/* <dep> win */

int callback_userinput(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_tagging  (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_license(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_about(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int callback_presets_dialog(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int callback_wait(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_audioformat(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callback_restore_settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int local_credits_drawtext(HWND hwnd);
void local_eqpresets_refresh(HWND lstwnd);
void local_eqpresets_setcurrent(HWND lstwnd);
void tagedit_apply2playlist(HWND hwnd, int tclear);

/* </dep> */




/* data ---------------------------------------------------------------------*/

letter  uinput_title[256];           /* window title of user input dialog */
letter  uinput_caption[256];         /* caption (prompt) of user input dialog */
letter  uinput_default[256];         /* default text of user input dialog */
letter  uinput_text[uinput_size];    /* memory for user input text */
int     uinput_wait;                 /* wait for user input */

letter  tageditor_file[v_sys_maxpath];  /* current file for tag editing */ 

int     basewindows_open_refresh = 0;   /* refresh filters? */

string  open_filter = 0;                /* filter text */


string  wait_text;
string  wait_detail;
int    *wait_cancel;
int     wait_end;

int     audioformat_samplerate;
int     audioformat_channels;
int     audioformat_bps;


	

/* <dep> win */

HWND tageditor_active = 0;
HWND window_wait;

HWND window_conversion = 0;
HWND window_ripping = 0;
HWND window_joining = 0;

/* </dep> */

struct fennec_audiotag  tagedit_copiedtag; /* cache memory for copied tags */





/* functions ----------------------------------------------------------------*/


/*
 * get user input.
 * title - window title.
 * cap   - caption.
 * dtxt  - default input text.
 */
string basewindows_getuserinput(const string title, const string cap, const string dtxt)
{
	memset(uinput_text, 0, uinput_size);

	str_cpy(uinput_title, title);
	str_cpy(uinput_caption, cap);
	str_cpy(uinput_default, dtxt);

	/* </dep> win */

	DialogBox(instance_fennec, (LPCTSTR)dialog_input, window_main, (DLGPROC)callback_userinput);

	/* <dep> */

	uinput_wait = 1;

	while(!uinput_wait)sys_sleep(0);

	return uinput_text;
}


/*
 * show progress (wait...) window.
 */
int basewindows_show_wait(void* wmodal)
{
	wait_cancel = 0;
	wait_text   = uni("Please Wait");
	wait_detail = uni("");
	wait_end    = 0;

	/* </dep> win */

	{
		window_wait = CreateDialog(instance_fennec, (LPCTSTR)dialog_wait, (HWND)wmodal, (DLGPROC)callback_wait);
		ShowWindow(window_wait, SW_SHOW);
		UpdateWindow(window_wait);
		sys_sleep(1);
	}

	/* <dep> */
	
	return 1;
}


/*
 * show restore settings window
 */
int basewindows_show_restore_settings(void* wmodal)
{
	/* </dep> win */

	return (int)DialogBox(instance_fennec, (LPCTSTR)dialog_startup_restore, window_main, (DLGPROC)callback_restore_settings);
	
	/* <dep> */
}


/*
 * show progress (wait...) window.
 */
int basewindows_wait_function(int id, int data, void *ptr)
{
	switch(id)
	{
	case wait_window_setcancel: /* int pointer, to be zeroed */
		wait_cancel = (int*)ptr;
		return 1;

	case wait_window_set_detail:
		wait_detail = (string)ptr;
		SendMessage(window_wait, WM_TIMER, 0, 0);
		sys_pass();
		return 1;

	case wait_window_set_text:
		wait_text = (string)ptr;
		SendMessage(window_wait, WM_TIMER, 0, 0);
		sys_pass();
		return 1;

	case wait_window_end:
		wait_end = 1;
		SendMessage(window_wait, WM_TIMER, 0, 0);
		return 1;
	}

	return 0;
}


/*
 * show equalizer presets.
 * return 1 - ok, otherwise - cancel.
 */
int basewindows_show_presets(void* wmodal)
{
	/* </dep> win */

	return (int)DialogBox(instance_fennec, (LPCTSTR)dialog_presets, (HWND)wmodal, (DLGPROC)callback_presets_dialog);

	/* <dep> */
}

/*
 * show license agreement window.
 * return 1 - accepted.
 */
int basewindows_show_license(void)
{
	/* </dep> win */

	return (int)DialogBox(instance_fennec, (LPCTSTR)dialog_gpl, window_main, (DLGPROC)callback_license);

	/* <dep> */
}

/*
 * show manual format selection dialog (for raw files and such).
 * return 1 - ok.
 */
int basewindows_selectformat(int *samplerate, int *channels, int *bps)
{
	int rv = 0;

	/* </dep> win */

	rv = (int)DialogBox(instance_fennec, (LPCTSTR)dialog_audioformat, window_main, (DLGPROC)callback_audioformat);

	/* <dep> */
	
	*samplerate = audioformat_samplerate;
	*channels   = audioformat_channels;
	*bps        = audioformat_bps;

	return rv;
}

/*
 * show tag-editor for a file.
 * tfile - file path.
 */
int basewindows_show_tagging(int wmodal, const string tfile)
{
	if(!tfile)return 0;

	/* <dep> win */

	if(!tageditor_active)
	{
		str_cpy(tageditor_file, tfile);
		DialogBox(instance_fennec, (LPCTSTR)dialog_tageditor, 0, (DLGPROC)callback_tagging);
	}else{
		str_cpy(tageditor_file, tfile);
		ShowWindow(tageditor_active, SW_SHOW);
		SendMessage(tageditor_active, WM_APP + 1, 0, 0);
	}

	/* </dep> */

	return 0;
}


/*
 * show about/credits window.
 */
int basewindows_show_about(int wmodal)
{
	/* <dep> win */

	DialogBox(instance_fennec, (LPCTSTR)dialog_about, wmodal ? window_main : 0, (DLGPROC)callback_about);
	
	/* </dep> */

	return 0;
}



/*
 * show add directory window.
 */
int basewindows_show_addfolder(void)
{
	/* <dep> win */


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

	/* </dep> */

	fennec_refresh(fennec_v_refresh_force_high);
	return 1;
}


/*
 * show 'open file' window.
 * cap  - dialog caption.
 * idir - initial directory.
 * dcus - experimental version?.
 */
string basewindows_show_open(const string cap, const string idir, int dcus)
{
	if(!basewindows_open_refresh) /* need to refill open_filter buffer */
	{
		letter  warn_text[] = uni("All Files (This version of Windows can't filter all the supported formats)");
		letter  ext[128];          /* extension */
		letter  dsc[256];          /* description */
		letter  ldsc[256];         /* last description */
		size_t  filter_size = 60;  /* (initial) size */
		size_t  i;                 /* current index of 'open_filter' */
		size_t  k;

		size_t rs;                 /* range start */
		size_t rz;                 /* range length */
		
		unsigned int j;

		int     closelast = 0;


		/* not allocated? */

 		if(!open_filter) open_filter = sys_mem_alloc((filter_size + 2) * sizeof(letter));

		/* set all supported formats */

		i = str_len(open_all);
		
		str_mcpy(open_filter, open_all, i);
		open_filter[i] = uni('\0');
		i++;

		/* open_filter: "All Formats\0" */

		/* add playlist extensions */

		j = 0;
		while(open_pexten(j))
		{
			k = str_len(open_pexten(j));

			str_cpy(open_filter + i, uni("*."));
			i += 2;

			str_mcpy(open_filter + i, open_pexten(j), k);
			i += k;

			str_cpy(open_filter + i, uni(";"));
			i += 1;

			j++;
		}

		/* add dynamic extensions (internal input) */

		j  = 0;
		rs = i;
		while(audio_input_getextensionsinfo((unsigned long)j, ext, dsc))
		{
			k = str_len(ext);

			/* reallocation? (8, 256 - random numbers > 3) */

			if(i + k + 8 > filter_size)
			{
				filter_size += (k + 256);
				open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
			}

			str_cpy(open_filter + i, uni("*."));
			i += 2;

			str_mcpy(open_filter + i, ext, k);
			i += k;

			str_cpy(open_filter + i, uni(";"));
			i += 1;

			if(j > 40)
			{
				/* <dep> win */

				OSVERSIONINFO ovi;

				ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

				GetVersionEx(&ovi);

				if(ovi.dwMajorVersion <= 5)/* windows 9x/nt/me/2000/xp */
				{	
					str_cpy(open_filter, warn_text);
					i = str_len(warn_text);
					open_filter[i] = uni('\0');
					i++;
					str_cpy(open_filter + i, uni("*.*"));
					i += str_len(uni("*.*"));
					break;
				}
				
				/* </dep> win */
				
			}

			j++;
		}

		/* next step */

		open_filter[i] = uni('\0');
		i++;

		/* add extensions with description */

		/* playlist files */

		/* reallocation? */

		k = str_len(open_playlist);
		if(i + k > filter_size)
		{
			filter_size += (k + 256);
			open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
		}

		str_mcpy(open_filter + i, open_playlist, k);
		i += k;

		open_filter[i] = uni('(');
		i++;

		/* add extensions */

		j = 0;
		while(playlist_t_get_extensions(j))
		{
			k = str_len(playlist_t_get_extensions(j));

			if(i + k + 8 > filter_size)
			{
				filter_size += (k + 256);
				open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
			}

			str_cpy(open_filter + i, uni("*."));
			i += 2;

			str_mcpy(open_filter + i, playlist_t_get_extensions(j), k);
			i += k;

			str_cpy(open_filter + i, uni("; "));
			i += 2;

			j++;
		}

		open_filter[i - 2] = uni(')');  /* over last ;[space] */
		open_filter[i - 1] = uni('\0');

		/* real filtering extensions */

		j = 0;
		while(playlist_t_get_extensions(j))
		{
			k = str_len(playlist_t_get_extensions(j));

			if(i + k + 8 > filter_size)
			{
				filter_size += (k + 256);
				open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
			}

			str_cpy(open_filter + i, uni("*."));
			i += 2;

			str_mcpy(open_filter + i, playlist_t_get_extensions(j), k);
			i += k;

			str_cpy(open_filter + i, uni(";"));
			i += 1;

			j++;
		}

		open_filter[i - 1] = uni('\0');

		/* dynamic extensions */

		j = 0;
		while(audio_input_getextensionsinfo((unsigned long)j, ext, dsc))
		{
			if(str_icmp(dsc, ldsc))
			{
				/* new list */

				if(j)
				{
point_filters_close_last:
					
					rz = i - rs - 1;

					if(i + rz + 8 > filter_size)
					{
						filter_size += (rz + 256);
						open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
					}

					open_filter[i - 1] = uni(')');
					open_filter[i] = uni('\0');
					i++;

					str_mcpy(open_filter + i, open_filter + rs, rz);

					i += rz;

					open_filter[i] = uni('\0');
					i++;
					
					if(closelast)
					{
						open_filter[i] = uni('\0');
						i++;
						goto point_filters_end;
					}
				}
				
				k = str_len(dsc);

				if(i + k + 8 > filter_size)
				{
					filter_size += (k + 256);
					open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
				}
				
				str_mcpy(open_filter + i, dsc, k);
				i += k;

				open_filter[i] = uni('(');
				i++;
				
				rs = i;
				
				str_cpy(ldsc, dsc);
			}

			k = str_len(ext);

			/* reallocation? (8, 256 - random numbers > 3) */

			if(i + k + 8 > filter_size)
			{
				filter_size += (k + 256);
				open_filter = sys_mem_realloc(open_filter, (filter_size + 2) * sizeof(letter));
			}

			str_cpy(open_filter + i, uni("*."));
			i += 2;

			str_mcpy(open_filter + i, ext, k);
			i += k;

			str_cpy(open_filter + i, uni(";"));
			i += 1;

			j++;
		}

		/* the last index is not closed */

		closelast = 1;
		goto point_filters_close_last;

point_filters_end:;

		basewindows_open_refresh = 1;
	}


	if(!dcus)
	{
		return fennec_file_dialog(0, cap, open_filter, idir);
	}else{
		
		return fennec_file_dialog_ex(0, cap, open_filter, idir);;
	}
	
}




/* callbacks ----------------------------------------------------------------*/

/* <dep> win */

/*
 * callback function for user input window.
 */
int callback_userinput(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case button_ok:
			GetDlgItemText(hwnd, input_user, uinput_text, uinput_size);
			uinput_wait = 0;
			EndDialog(hwnd, 0);
			break;

		case button_cancel:
			uinput_wait = 0;
			EndDialog(hwnd, 0);
			break;
		}
		break;
	
	case WM_INITDIALOG:
		SetWindowText (hwnd, uinput_title);
		SetDlgItemText(hwnd, text_caption, uinput_caption);
		SetDlgItemText(hwnd, input_user, uinput_default);
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}


/*
 * callback function for wait window.
 */
int callback_wait(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_TIMER:
		SetDlgItemText(hwnd, static_text, wait_text);
		SetDlgItemText(hwnd, static_progress, wait_detail);
		if(wait_end)
			EndDialog(hwnd, 0);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case button_cancel:
			if(wait_cancel) *wait_cancel = 0;
			sys_pass();
			EndDialog(hwnd, 0);
		}
		break;
	
	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}


/*
 * callback function for manual format selection window
 */
int callback_audioformat(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;

		case IDOK:
			{
				BOOL tr = 1;
				audioformat_samplerate = GetDlgItemInt(hwnd, combo_freq, &tr, 0);
				audioformat_channels   = GetDlgItemInt(hwnd, combo_channels, &tr, 0);
				audioformat_bps        = GetDlgItemInt(hwnd, combo_bps, &tr, 0);

				settings.player.last_raw_samplerate = audioformat_samplerate;
				settings.player.last_raw_channels   = audioformat_channels;
				settings.player.last_raw_bps	    = audioformat_bps;
			}
			EndDialog(hwnd, 1);
		}
		break;

	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, combo_freq, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("8000"));
		SendDlgItemMessage(hwnd, combo_freq, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("11025"));
		SendDlgItemMessage(hwnd, combo_freq, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("22050"));
		SendDlgItemMessage(hwnd, combo_freq, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("44100"));
		SendDlgItemMessage(hwnd, combo_freq, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("48000"));
		SendDlgItemMessage(hwnd, combo_freq, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("96000"));

		SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("1"));
		SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("2"));
		SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("3"));
		SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("4"));
		SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("6"));
		SendDlgItemMessage(hwnd, combo_channels, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("8"));

		SendDlgItemMessage(hwnd, combo_bps, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("8"));
		SendDlgItemMessage(hwnd, combo_bps, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("16"));
		SendDlgItemMessage(hwnd, combo_bps, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("32"));
		SendDlgItemMessage(hwnd, combo_bps, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)uni("64"));
		
		SetDlgItemInt(hwnd, combo_freq,     settings.player.last_raw_samplerate, 0);
		SetDlgItemInt(hwnd, combo_channels, settings.player.last_raw_channels, 0);
		SetDlgItemInt(hwnd, combo_bps,      settings.player.last_raw_bps, 0);
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}


/*
 * callback function for manual format selection window
 */
int callback_restore_settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case button_ignore:
			EndDialog(hwnd, 0);
			break;

		case button_restore:
			{
				int rmode = 0;

				if(IsDlgButtonChecked(hwnd, check_default)    == BST_CHECKED) rmode |= fennec_restore_default;
				if(IsDlgButtonChecked(hwnd, check_erase_main) == BST_CHECKED) rmode |= fennec_restore_r_main;
				if(IsDlgButtonChecked(hwnd, check_skinfiles)  == BST_CHECKED) rmode |= fennec_restore_r_skin;
				if(IsDlgButtonChecked(hwnd, check_plg)        == BST_CHECKED) rmode |= fennec_restore_r_plugin;
				if(IsDlgButtonChecked(hwnd, check_erase_media)== BST_CHECKED) rmode |= fennec_restore_r_media;
			
				EndDialog(hwnd, rmode);
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		CheckDlgButton(hwnd, check_default, BST_CHECKED);
		{
			HDC  dc;
			HWND siwnd;

			siwnd = GetDlgItem(hwnd, static_icon);
			dc = GetDC(siwnd);

			DrawIcon(dc, 0, 0, LoadIcon(0, IDI_EXCLAMATION));

			ReleaseDC(siwnd, dc);
		}
		break;

	case WM_PAINT:
		{
			HDC  dc;
			HWND siwnd;

			siwnd = GetDlgItem(hwnd, static_icon);
			dc = GetDC(siwnd);

			DrawIcon(dc, 0, 0, LoadIcon(0, IDI_EXCLAMATION));

			ReleaseDC(siwnd, dc);
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}

/*
 * callback function for presets dialog.
 */
int callback_presets_dialog(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		/* <lang> */

		SetWindowText(hwnd, text(oooo_eqpresets) );

		SetDlgItemText(hwnd, static_name,       text(oooo_eqpresets_name        ) );
		SetDlgItemText(hwnd, group_management,  text(oooo_eqpresets_management  ) );
		SetDlgItemText(hwnd, button_rename,     text(oooo_eqpresets_rename      ) );
		SetDlgItemText(hwnd, button_delete,     text(oooo_eqpresets_delete      ) );
		SetDlgItemText(hwnd, button_load,       text(oooo_eqpresets_load        ) );
		SetDlgItemText(hwnd, button_save,       text(oooo_eqpresets_save        ) );
		SetDlgItemText(hwnd, button_add,        text(oooo_eqpresets_addcurrent  ) );
		SetDlgItemText(hwnd, button_ok,         text(oooo_ok                    ) );
		SetDlgItemText(hwnd, button_cancel,     text(oooo_cancel                ) );


		/* </lang> */

		local_eqpresets_refresh(GetDlgItem(hwnd, list_presets));
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case list_presets:
			if(HIWORD(wParam) == LBN_DBLCLK)
			{
				local_eqpresets_setcurrent(GetDlgItem(hwnd, list_presets));
			}else if(HIWORD(wParam) == LBN_SELCHANGE){

				letter tname[128];

				SendDlgItemMessage(hwnd, list_presets, LB_GETTEXT, (WPARAM)(int) SendDlgItemMessage(hwnd, list_presets, LB_GETCURSEL, 0, 0), (LPARAM)tname);
				SendDlgItemMessage(hwnd, text_name, WM_SETTEXT, 0, (LPARAM)tname);

			}
			break;

		case button_rename:
			{
				unsigned long     nbr;
				equalizer_preset  eqp;
				unsigned int      i = (unsigned int)SendDlgItemMessage(hwnd, list_presets, LB_GETCURSEL, 0, 0);

				if(i < settings.player.equalizer_presets)
				{
					settings_data_get(setting_id_equalizer_preset, i, &eqp, &nbr);
					if(nbr != sizeof(eqp))break;

					SendDlgItemMessage(hwnd, text_name, WM_GETTEXT, sizeof(eqp.name), (LPARAM)eqp.name);
					eqp.name[sizeof(eqp.name) - 1] = uni('\0');
					settings_data_set(setting_id_equalizer_preset, i, &eqp);
				
					local_eqpresets_refresh(GetDlgItem(hwnd, list_presets));
				}
			}
			break;

		case button_add:
			{
				equalizer_preset  eqp;
				unsigned int      i, j;
				float             eqb[32];

				equalizer_get_bands(0, 10, eqb);

				for(i=0; i<16; i++)
				{
					equalizer_get_bands(0, 32, eqb);

					eqp.preamp[i] = equalizer_get_preamp(0);

					for(j=0; j<32; j++)
						eqp.boost[i][j] = eqb[j];
				}

				eqp.parametric = 0;

				SendDlgItemMessage(hwnd, text_name, WM_GETTEXT, sizeof(eqp.name), (LPARAM)eqp.name);
				eqp.name[sizeof(eqp.name) - 1] = uni('\0');
				
				settings_data_add(setting_id_equalizer_preset, &eqp, sizeof(eqp));
				settings.player.equalizer_presets++;

				local_eqpresets_refresh(GetDlgItem(hwnd, list_presets));
			}
			break;

		case button_ok:
			local_eqpresets_setcurrent(GetDlgItem(hwnd, list_presets));
			EndDialog(hwnd, 1);
			break;

		case button_save:
			{
				unsigned int i;
				letter       fpath[1024];
				OPENFILENAME lofn;
				HANDLE       hfile;
				unsigned long        nbr = 0;
				unsigned long        pcount;

				memset(&lofn, 0, sizeof(lofn));

				fpath[0] = 0;

				lofn.lStructSize     = sizeof(lofn);
				lofn.lpstrTitle      = uni("Save Equalizer Presets File");
				lofn.hwndOwner       = window_main;
				lofn.lpstrFile       = fpath;
				lofn.nMaxFile        = sizeof(fpath);
				lofn.lpstrFilter     = uni("Equalizer Preset (*.feq)\0*.feq");
				lofn.nFilterIndex    = 0;
				lofn.lpstrFileTitle  = 0;
				lofn.nMaxFileTitle   = 0;
				lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
				lofn.hInstance       = instance_fennec;

				GetSaveFileName(&lofn);

				if(str_len(fpath))
				{
					hfile = CreateFile(fpath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);

					if(hfile != INVALID_HANDLE_VALUE)
					{
						equalizer_preset  eqp;

						pcount = settings.player.equalizer_presets;

						WriteFile(hfile, &pcount, sizeof(unsigned long), &nbr, 0);

						for(i=0; i<settings.player.equalizer_presets; i++)
						{
							nbr = sizeof(equalizer_preset);
							settings_data_get(setting_id_equalizer_preset, i, &eqp, &nbr);
							WriteFile(hfile, &eqp, sizeof(equalizer_preset), &nbr, 0);
						}

						CloseHandle(hfile);
					}
				}
			}
			local_eqpresets_refresh(GetDlgItem(hwnd, list_presets));
			break;

		case button_load:
			{
				unsigned int i;
				letter       fpath[1024];
				OPENFILENAME lofn;
				HANDLE       hfile;
				unsigned long        nbr = 0;
				unsigned long        pcount;

				memset(&lofn, 0, sizeof(lofn));

				fpath[0] = 0;

				lofn.lStructSize     = sizeof(lofn);
				lofn.lpstrTitle      = uni("Load Equalizer Presets File");
				lofn.hwndOwner       = window_main;
				lofn.lpstrFile       = fpath;
				lofn.nMaxFile        = sizeof(fpath);
				lofn.lpstrFilter     = uni("Equalizer Preset (*.feq)\0*.feq");
				lofn.nFilterIndex    = 0;
				lofn.lpstrFileTitle  = 0;
				lofn.nMaxFileTitle   = 0;
				lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
				lofn.hInstance       = instance_fennec;
				lofn.lpstrInitialDir = fennec_get_path(0, 0);

				GetOpenFileName(&lofn);

				if(str_len(fpath))
				{
					hfile = CreateFile(fpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

					if(hfile != INVALID_HANDLE_VALUE)
					{
						int x;
						equalizer_preset  eqp;

						ReadFile(hfile, &pcount, sizeof(unsigned long), &nbr, 0);
						
						x = (int)settings.player.equalizer_presets;
						x += pcount;

						settings.player.equalizer_presets = (unsigned short)x;

						for(i=0; i<pcount; i++)
						{
							ReadFile(hfile, &eqp, sizeof(equalizer_preset), &nbr, 0);

							settings_data_add(setting_id_equalizer_preset, &eqp, sizeof(equalizer_preset));
							
						}
						CloseHandle(hfile);
					}
				}
			}
			local_eqpresets_refresh(GetDlgItem(hwnd, list_presets));
			break;

		case button_delete:
			{
				HWND lstwnd = GetDlgItem(hwnd, list_presets);
				int i = (int)SendMessage(lstwnd, LB_GETCURSEL, 0, 0);

				if(i == -1 || !settings.player.equalizer_presets) break;
			
				if(i < settings.player.equalizer_presets)
				{
					if(settings.player.equalizer_last_preset_id  > i)
						settings.player.equalizer_last_preset_id--;
					else if(settings.player.equalizer_last_preset_id == i)
						settings.player.equalizer_last_preset_id = 0;

					
					settings_data_remove(setting_id_equalizer_preset, i);
					settings.player.equalizer_presets--;
				}

				local_eqpresets_refresh(lstwnd);
			}
			break;

		case IDCANCEL:
		case button_cancel:
			EndDialog(hwnd, 0);
			break;
		}
		break;


	}
	return 0;
}


/*
 * callback function for license agreement window.
 */
int callback_license(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwnd, text_gpl, fennec_loadtext(uni("text"), uni("license")) + 2);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case button_accept:
			EndDialog(hwnd, 1);
			break;

		case button_decline:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}


/*
 * callback function for about window.
 */
int callback_about(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int          running_credits = 0, show_license = 0;
	static HBRUSH       hbr = 0;
	static HFONT        font_url;

	switch(msg)
	{
	case WM_TIMER:
		if(running_credits)
		{
			local_credits_drawtext(GetDlgItem(hwnd, panel_credits));
		}
		break;


	case WM_CTLCOLORSTATIC:
		if(lParam == (LPARAM)GetDlgItem(hwnd, url_homepage))
		{
			if(!hbr)hbr = GetSysColorBrush(COLOR_BTNFACE);
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0, 0, 255));
			return PtrToInt(hbr);
		}
		break;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;

		case button_credits:
			running_credits ^= 1;

			if(running_credits)
			{
				ShowWindow(GetDlgItem(hwnd, text_about), SW_HIDE);
			}else{
				RECT rct;

				GetClientRect(hwnd, &rct);

				local_credits_drawtext(0);

				InvalidateRect(hwnd, &rct, 1);
				
				ShowWindow(GetDlgItem(hwnd, text_about), SW_SHOW);
			}
			break;

		case button_license:
			
			show_license ^= 1;

			if(running_credits)
			{
				RECT rct;

				GetClientRect(hwnd, &rct);

				local_credits_drawtext(0);

				InvalidateRect(hwnd, &rct, 1);
				
				ShowWindow(GetDlgItem(hwnd, text_about), SW_SHOW);
				
				running_credits = 0;
			}

			SetDlgItemText(hwnd, text_about, fennec_loadtext(uni("text"), show_license ? uni("license") : uni("about")) + 1);
			fennec_loadtext(0, 0); /* free */
			break;

		case url_homepage:
			if(PtrToInt(ShellExecute(0, uni("open"), uni("http://fennec.sourceforge.net"), 0,  0, SW_SHOWNORMAL)) <= 32)
			{
				MessageBox(hwnd, uni("Visit:\nhttp://fennec.sourceforge.net"), uni("Fennec Home Page"), MB_ICONINFORMATION);
			}
			break;
		}
		break;

	case WM_INITDIALOG:

		SetDlgItemText(hwnd, IDOK, text(oooo_ok));
		SetDlgItemText(hwnd, button_license, text(oooo_license));
		SetDlgItemText(hwnd, button_credits, text(oooo_credits));
		SetDlgItemText(hwnd, s_version, text(oooo_version));
		SetDlgItemText(hwnd, s_version_info, text(oooo_version_information));
		SetDlgItemText(hwnd, s_version_details, text(oooo_fennec_version_details));
		SetDlgItemText(hwnd, url_homepage, text(oooo_fennec_homepage));

		SetDlgItemText(hwnd, static_about       , fennec_u_version_text_full);
		SetDlgItemText(hwnd, static_copyright   , uni("Copyright (c) 2006 - 2009 Chase <c-h@users.sf.net>"));
		SetDlgItemText(hwnd, static_version     , fennec_u_version_string);
		SetDlgItemText(hwnd, static_version_info, fennec_u_version_information);

		SetDlgItemText(hwnd, text_about, fennec_loadtext(uni("text"), uni("about")) + 1);

		fennec_loadtext(0, 0); /* free */

		SetFocus(GetDlgItem(hwnd, IDOK));

		{
			HDC sdc = GetDC(0);

			font_url = CreateFont(-MulDiv(8, GetDeviceCaps(sdc, LOGPIXELSY), 72),
									0, 0, 0, FW_NORMAL, 0, 1, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									DEFAULT_PITCH, uni("Ms Sans Serif"));
			ReleaseDC(hwnd, sdc);
		}

		SendDlgItemMessage(hwnd, url_homepage, WM_SETFONT, (WPARAM)font_url, 1);

		SetTimer(hwnd, 1, 20, 0);
		break;

	case WM_PAINT:
		
		break;

	case WM_DESTROY:
		DeleteObject(font_url);
		DeleteObject(hbr);
		KillTimer(hwnd, 1);
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}


/*
 * callback function for tag-editor.
 */
int callback_tagging(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			/* save settings */

			settings.tag_editing.checked_tags[0 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_TITLE)     == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[1 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_ALBUM)     == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[2 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_ARTIST)    == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[3 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_OARTIST)   == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[4 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_BAND)      == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[5 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_COMPOSER)  == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[6 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRICIST)  == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[7 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_COPYRIGHT) == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[8 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_ENCODER)   == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[9 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_GENRE)     == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[10] = IsDlgButtonChecked(hwnd, IDC_TE_CK_YEAR)      == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[11] = IsDlgButtonChecked(hwnd, IDC_TE_CK_OFAURL)    == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[12] = IsDlgButtonChecked(hwnd, IDC_TE_CK_URL)       == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[13] = IsDlgButtonChecked(hwnd, IDC_TE_CK_COMMENTS)  == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[14] = IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRIC)     == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[15] = IsDlgButtonChecked(hwnd, IDC_TE_CK_BPM)       == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[16] = IsDlgButtonChecked(hwnd, IDC_TE_CK_TRACKNUM)  == BST_CHECKED ? 1 : 0;
			settings.tag_editing.checked_tags[17] = IsDlgButtonChecked(hwnd, IDC_TE_CK_PUBLISHER) == BST_CHECKED ? 1 : 0;

			memset(settings.tag_editing.rename_formatting, 0, sizeof(settings.tag_editing.rename_formatting));
			GetDlgItemText(hwnd, IDC_TE_FORMATTING, settings.tag_editing.rename_formatting, sizeof(settings.tag_editing.rename_formatting));

			/* hide the window, cuz we don't wanna load
			   all the values next time. */

			ShowWindow(hwnd, SW_HIDE);
			break;

		case IDC_TE_FRHELP:
			MessageBox(hwnd, formattinginfo, uni("Destination File Name Formatting"), MB_ICONINFORMATION);
			break;

		case IDC_TE_NEXTFILE:
			audio_playlist_next();
			str_cpy(tageditor_file, audio_playlist_getsource(audio_playlist_getcurrentindex()));
			sys_pass();
			goto load_tag_agin;

		case IDC_TE_PASTE:

			/* paste selected tags */

			if(tagedit_copiedtag.tag_title.tsize           && IsDlgButtonChecked(hwnd, IDC_TE_CK_TITLE    ))SetDlgItemText(hwnd, IDC_TE_TITLE    , tagedit_copiedtag.tag_title.tdata           );
			if(tagedit_copiedtag.tag_album.tsize           && IsDlgButtonChecked(hwnd, IDC_TE_CK_ALBUM    ))SetDlgItemText(hwnd, IDC_TE_ALBUM    , tagedit_copiedtag.tag_album.tdata           );
			if(tagedit_copiedtag.tag_artist.tsize          && IsDlgButtonChecked(hwnd, IDC_TE_CK_ARTIST   ))SetDlgItemText(hwnd, IDC_TE_ARTIST   , tagedit_copiedtag.tag_artist.tdata          );
			if(tagedit_copiedtag.tag_origartist.tsize      && IsDlgButtonChecked(hwnd, IDC_TE_CK_OARTIST  ))SetDlgItemText(hwnd, IDC_TE_OARTIST  , tagedit_copiedtag.tag_origartist.tdata      );
			if(tagedit_copiedtag.tag_composer.tsize        && IsDlgButtonChecked(hwnd, IDC_TE_CK_COMPOSER ))SetDlgItemText(hwnd, IDC_TE_COMPOSER , tagedit_copiedtag.tag_composer.tdata        );
			if(tagedit_copiedtag.tag_lyricist.tsize        && IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRICIST ))SetDlgItemText(hwnd, IDC_TE_LYRICIST , tagedit_copiedtag.tag_lyricist.tdata        );
			if(tagedit_copiedtag.tag_band.tsize            && IsDlgButtonChecked(hwnd, IDC_TE_CK_BAND     ))SetDlgItemText(hwnd, IDC_TE_BAND     , tagedit_copiedtag.tag_band.tdata            );
			if(tagedit_copiedtag.tag_copyright.tsize       && IsDlgButtonChecked(hwnd, IDC_TE_CK_COPYRIGHT))SetDlgItemText(hwnd, IDC_TE_COPYRIGHT, tagedit_copiedtag.tag_copyright.tdata       );
			if(tagedit_copiedtag.tag_encodedby.tsize       && IsDlgButtonChecked(hwnd, IDC_TE_CK_ENCODER  ))SetDlgItemText(hwnd, IDC_TE_ENCODEDBY, tagedit_copiedtag.tag_encodedby.tdata       );
			if(tagedit_copiedtag.tag_genre.tsize           && IsDlgButtonChecked(hwnd, IDC_TE_CK_GENRE    ))SetDlgItemText(hwnd, IDC_TE_GENRE    , tagedit_copiedtag.tag_genre.tdata           );
			if(tagedit_copiedtag.tag_year.tsize            && IsDlgButtonChecked(hwnd, IDC_TE_CK_YEAR     ))SetDlgItemText(hwnd, IDC_TE_YEAR     , tagedit_copiedtag.tag_year.tdata            );
			if(tagedit_copiedtag.tag_url.tsize             && IsDlgButtonChecked(hwnd, IDC_TE_CK_URL      ))SetDlgItemText(hwnd, IDC_TE_URL      , tagedit_copiedtag.tag_url.tdata             );
			if(tagedit_copiedtag.tag_offiartisturl.tsize   && IsDlgButtonChecked(hwnd, IDC_TE_CK_OFAURL   ))SetDlgItemText(hwnd, IDC_TE_OFAURL   , tagedit_copiedtag.tag_offiartisturl.tdata   );
			if(tagedit_copiedtag.tag_comments.tsize        && IsDlgButtonChecked(hwnd, IDC_TE_CK_COMMENTS ))SetDlgItemText(hwnd, IDC_TE_COMMENTS , tagedit_copiedtag.tag_comments.tdata        );
			if(tagedit_copiedtag.tag_lyric.tsize           && IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRIC    ))SetDlgItemText(hwnd, IDC_TE_LYRICS   , tagedit_copiedtag.tag_lyric.tdata           );
			if(tagedit_copiedtag.tag_bpm.tsize             && IsDlgButtonChecked(hwnd, IDC_TE_CK_BPM      ))SetDlgItemText(hwnd, IDC_TE_BPM      , tagedit_copiedtag.tag_bpm.tdata             );
			if(tagedit_copiedtag.tag_tracknum.tsize        && IsDlgButtonChecked(hwnd, IDC_TE_CK_TRACKNUM ))SetDlgItemText(hwnd, IDC_TE_TRACKNUM , tagedit_copiedtag.tag_tracknum.tdata        );
			if(tagedit_copiedtag.tag_publish.tsize         && IsDlgButtonChecked(hwnd, IDC_TE_CK_PUBLISHER))SetDlgItemText(hwnd, IDC_TE_PUBLISHER, tagedit_copiedtag.tag_publish.tdata         );
			break;

		case IDC_TE_COPY:

			/* copy all the tags to memory */

			/* first, free existing data */

			if(tagedit_copiedtag.tag_title.tsize           )sys_mem_free(tagedit_copiedtag.tag_title.tdata           );
			if(tagedit_copiedtag.tag_album.tsize           )sys_mem_free(tagedit_copiedtag.tag_album.tdata           );
			if(tagedit_copiedtag.tag_artist.tsize          )sys_mem_free(tagedit_copiedtag.tag_artist.tdata          );
			if(tagedit_copiedtag.tag_origartist.tsize      )sys_mem_free(tagedit_copiedtag.tag_origartist.tdata      );
			if(tagedit_copiedtag.tag_composer.tsize        )sys_mem_free(tagedit_copiedtag.tag_composer.tdata        );
			if(tagedit_copiedtag.tag_lyricist.tsize        )sys_mem_free(tagedit_copiedtag.tag_lyricist.tdata        );
			if(tagedit_copiedtag.tag_band.tsize            )sys_mem_free(tagedit_copiedtag.tag_band.tdata            );
			if(tagedit_copiedtag.tag_copyright.tsize       )sys_mem_free(tagedit_copiedtag.tag_copyright.tdata       );
			if(tagedit_copiedtag.tag_publish.tsize         )sys_mem_free(tagedit_copiedtag.tag_publish.tdata         );
			if(tagedit_copiedtag.tag_encodedby.tsize       )sys_mem_free(tagedit_copiedtag.tag_encodedby.tdata       );
			if(tagedit_copiedtag.tag_genre.tsize           )sys_mem_free(tagedit_copiedtag.tag_genre.tdata           );
			if(tagedit_copiedtag.tag_year.tsize            )sys_mem_free(tagedit_copiedtag.tag_year.tdata            );
			if(tagedit_copiedtag.tag_url.tsize             )sys_mem_free(tagedit_copiedtag.tag_url.tdata             );
			if(tagedit_copiedtag.tag_offiartisturl.tsize   )sys_mem_free(tagedit_copiedtag.tag_offiartisturl.tdata   );
			if(tagedit_copiedtag.tag_filepath.tsize        )sys_mem_free(tagedit_copiedtag.tag_filepath.tdata        );
			if(tagedit_copiedtag.tag_filename.tsize        )sys_mem_free(tagedit_copiedtag.tag_filename.tdata        );
			if(tagedit_copiedtag.tag_comments.tsize        )sys_mem_free(tagedit_copiedtag.tag_comments.tdata        );
			if(tagedit_copiedtag.tag_lyric.tsize           )sys_mem_free(tagedit_copiedtag.tag_lyric.tdata           );
			if(tagedit_copiedtag.tag_bpm.tsize             )sys_mem_free(tagedit_copiedtag.tag_bpm.tdata             );
			if(tagedit_copiedtag.tag_tracknum.tsize        )sys_mem_free(tagedit_copiedtag.tag_tracknum.tdata        );

			tagedit_copiedtag.tag_album.tsize           = 0;
			tagedit_copiedtag.tag_title.tsize           = 0;
			tagedit_copiedtag.tag_album.tsize           = 0;
			tagedit_copiedtag.tag_artist.tsize          = 0;
			tagedit_copiedtag.tag_origartist.tsize      = 0;
			tagedit_copiedtag.tag_composer.tsize        = 0;
			tagedit_copiedtag.tag_lyricist.tsize        = 0;
			tagedit_copiedtag.tag_band.tsize            = 0;
			tagedit_copiedtag.tag_copyright.tsize       = 0;
			tagedit_copiedtag.tag_publish.tsize         = 0;
			tagedit_copiedtag.tag_encodedby.tsize       = 0;
			tagedit_copiedtag.tag_genre.tsize           = 0;
			tagedit_copiedtag.tag_year.tsize            = 0;
			tagedit_copiedtag.tag_url.tsize             = 0;
			tagedit_copiedtag.tag_offiartisturl.tsize   = 0;
			tagedit_copiedtag.tag_filepath.tsize        = 0;
			tagedit_copiedtag.tag_filename.tsize        = 0;
			tagedit_copiedtag.tag_comments.tsize        = 0;
			tagedit_copiedtag.tag_lyric.tsize           = 0;
			tagedit_copiedtag.tag_bpm.tsize             = 0;
			tagedit_copiedtag.tag_tracknum.tsize        = 0;

			/* simple loop to copy all the selected tags */

			{
				struct fennec_audiotag_item *ct;
				int    ditem, ckitem, itc = 0;

				/* start of the loop */

point_tag_copy:

				itc++;

				switch(itc)
				{
				case 1:  ct = &tagedit_copiedtag.tag_title         ; ditem = IDC_TE_TITLE     ; ckitem = IDC_TE_CK_TITLE     ; break;
				case 2:  ct = &tagedit_copiedtag.tag_album         ; ditem = IDC_TE_ALBUM     ; ckitem = IDC_TE_CK_ALBUM     ; break;
				case 3:  ct = &tagedit_copiedtag.tag_artist        ; ditem = IDC_TE_ARTIST    ; ckitem = IDC_TE_CK_ARTIST    ; break;
				case 4:  ct = &tagedit_copiedtag.tag_origartist    ; ditem = IDC_TE_OARTIST   ; ckitem = IDC_TE_CK_OARTIST   ; break;
				case 5:  ct = &tagedit_copiedtag.tag_composer      ; ditem = IDC_TE_COMPOSER  ; ckitem = IDC_TE_CK_COMPOSER  ; break;
				case 6:  ct = &tagedit_copiedtag.tag_lyricist      ; ditem = IDC_TE_LYRICIST  ; ckitem = IDC_TE_CK_LYRICIST  ; break;
				case 7:  ct = &tagedit_copiedtag.tag_band          ; ditem = IDC_TE_BAND      ; ckitem = IDC_TE_CK_BAND      ; break;
				case 8:  ct = &tagedit_copiedtag.tag_copyright     ; ditem = IDC_TE_COPYRIGHT ; ckitem = IDC_TE_CK_COPYRIGHT ; break;
				case 9:  ct = &tagedit_copiedtag.tag_encodedby     ; ditem = IDC_TE_ENCODEDBY ; ckitem = IDC_TE_CK_ENCODER   ; break;
				case 10: ct = &tagedit_copiedtag.tag_genre         ; ditem = IDC_TE_GENRE     ; ckitem = IDC_TE_CK_GENRE     ; break;
				case 11: ct = &tagedit_copiedtag.tag_year          ; ditem = IDC_TE_YEAR      ; ckitem = IDC_TE_CK_YEAR      ; break;
				case 12: ct = &tagedit_copiedtag.tag_url           ; ditem = IDC_TE_URL       ; ckitem = IDC_TE_CK_URL       ; break;
				case 13: ct = &tagedit_copiedtag.tag_offiartisturl ; ditem = IDC_TE_OFAURL    ; ckitem = IDC_TE_CK_OFAURL    ; break;
				case 14: ct = &tagedit_copiedtag.tag_comments      ; ditem = IDC_TE_COMMENTS  ; ckitem = IDC_TE_CK_COMMENTS  ; break;
				case 15: ct = &tagedit_copiedtag.tag_lyric         ; ditem = IDC_TE_LYRICS    ; ckitem = IDC_TE_CK_LYRIC     ; break;
				case 16: ct = &tagedit_copiedtag.tag_bpm           ; ditem = IDC_TE_BPM       ; ckitem = IDC_TE_CK_BPM       ; break;
				case 17: ct = &tagedit_copiedtag.tag_tracknum      ; ditem = IDC_TE_TRACKNUM  ; ckitem = IDC_TE_CK_TRACKNUM  ; break;
				case 18: ct = &tagedit_copiedtag.tag_publish       ; ditem = IDC_TE_PUBLISHER ; ckitem = IDC_TE_CK_PUBLISHER ; break;
				default: goto point_tag_copy_end; /* end of the loop */
				}

				/* cache current tag */

				if(IsDlgButtonChecked(hwnd, ckitem) == BST_CHECKED)
				{
					unsigned int tsz = (unsigned int)SendDlgItemMessage(hwnd, ditem, WM_GETTEXTLENGTH, 0, 0);
					if(tsz)
					{
						ct->tdata = sys_mem_alloc((tsz + 1) * sizeof(letter));
						GetDlgItemText(hwnd, ditem, ct->tdata, tsz + 1);
						ct->tsize = tsz;
					}
				}

				/* jump to begining */

				goto point_tag_copy;

				/* end of the loop */

point_tag_copy_end:;

			}

			break;

		case IDOK:

			/* apply (write) tags to destination file */

			{
				struct fennec_audiotag at;
				letter tbuf[v_sys_maxpath];
				int    i;

				/* store tags and respective controller ids */

				struct tprops
				{
					struct fennec_audiotag_item *ta;
					int cont;
				};
				
				struct tprops tp[21 + 1];
				
				tp[ 0].ta = &at.tag_title           ; tp[ 0].cont = IDC_TE_TITLE;
				tp[ 1].ta = &at.tag_album           ; tp[ 1].cont = IDC_TE_ALBUM;
				tp[ 2].ta = &at.tag_artist          ; tp[ 2].cont = IDC_TE_ARTIST;
				tp[ 3].ta = &at.tag_origartist      ; tp[ 3].cont = IDC_TE_OARTIST;
				tp[ 4].ta = &at.tag_composer        ; tp[ 4].cont = IDC_TE_COMPOSER;
				tp[ 5].ta = &at.tag_lyricist        ; tp[ 5].cont = IDC_TE_LYRICIST;
				tp[ 6].ta = &at.tag_band            ; tp[ 6].cont = IDC_TE_BAND;
				tp[ 7].ta = &at.tag_copyright       ; tp[ 7].cont = IDC_TE_COPYRIGHT;
				tp[ 8].ta = &at.tag_publish         ; tp[ 8].cont = IDC_TE_COPYRIGHT;
				tp[ 9].ta = &at.tag_encodedby       ; tp[ 9].cont = IDC_TE_ENCODEDBY;
				tp[10].ta = &at.tag_genre           ; tp[10].cont = IDC_TE_GENRE;
				tp[11].ta = &at.tag_year            ; tp[11].cont = IDC_TE_YEAR;
				tp[12].ta = &at.tag_url             ; tp[12].cont = IDC_TE_URL;
				tp[13].ta = &at.tag_offiartisturl   ; tp[13].cont = IDC_TE_OFAURL;
				tp[14].ta = &at.tag_filepath        ; tp[14].cont = IDC_TE_PATH;
				tp[15].ta = &at.tag_filename        ; tp[15].cont = IDC_TE_PATH;
				tp[16].ta = &at.tag_comments        ; tp[16].cont = IDC_TE_COMMENTS;
				tp[17].ta = &at.tag_lyric           ; tp[17].cont = IDC_TE_LYRICS;
				tp[18].ta = &at.tag_bpm             ; tp[18].cont = IDC_TE_BPM;
				tp[19].ta = &at.tag_tracknum        ; tp[19].cont = IDC_TE_TRACKNUM;
				tp[20].ta = &at.tag_publish         ; tp[20].cont = IDC_TE_PUBLISHER;

				/* save settings */

				settings.tag_editing.checked_tags[0 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_TITLE)     == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[1 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_ALBUM)     == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[2 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_ARTIST)    == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[3 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_OARTIST)   == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[4 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_BAND)      == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[5 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_COMPOSER)  == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[6 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRICIST)  == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[7 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_COPYRIGHT) == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[8 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_ENCODER)   == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[9 ] = IsDlgButtonChecked(hwnd, IDC_TE_CK_GENRE)     == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[10] = IsDlgButtonChecked(hwnd, IDC_TE_CK_YEAR)      == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[11] = IsDlgButtonChecked(hwnd, IDC_TE_CK_OFAURL)    == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[12] = IsDlgButtonChecked(hwnd, IDC_TE_CK_URL)       == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[13] = IsDlgButtonChecked(hwnd, IDC_TE_CK_COMMENTS)  == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[14] = IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRIC)     == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[15] = IsDlgButtonChecked(hwnd, IDC_TE_CK_BPM)       == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[16] = IsDlgButtonChecked(hwnd, IDC_TE_CK_TRACKNUM)  == BST_CHECKED ? 1 : 0;
				settings.tag_editing.checked_tags[17] = IsDlgButtonChecked(hwnd, IDC_TE_CK_PUBLISHER) == BST_CHECKED ? 1 : 0;

				memset(settings.tag_editing.rename_formatting, 0, sizeof(settings.tag_editing.rename_formatting));
				GetDlgItemText(hwnd, IDC_TE_FORMATTING, settings.tag_editing.rename_formatting, sizeof(settings.tag_editing.rename_formatting));

				/* copy tags to memory */

				for(i=0; i<21; i++)
				{
					tp[i].ta->tsize = (unsigned int)SendDlgItemMessage(hwnd, tp[i].cont, WM_GETTEXTLENGTH, 0, 0) * sizeof(letter);
					
					if(tp[i].ta->tsize)
					{
						tp[i].ta->tsize += sizeof(letter); /* terminating null */
						tp[i].ta->tdata = sys_mem_alloc(tp[i].ta->tsize);
						memset(tp[i].ta->tdata, 0, tp[i].ta->tsize);
						
						SendDlgItemMessage(hwnd, tp[i].cont, WM_GETTEXT, (WPARAM)(tp[i].ta->tsize / sizeof(letter)), (LPARAM)tp[i].ta->tdata);
						
						/* replace cr-lf combinations with 'newlines' */

						str_win_to_tag(tp[i].ta->tdata);

						tp[i].ta->tsize = (unsigned int)str_size(tp[i].ta->tdata);
						
						tp[i].ta->tmode = tag_memmode_dynamic;
					}
				}

				/* get file path (easy way :-D ) */

				memset(tbuf, 0, sizeof(tbuf));
				SendDlgItemMessage(hwnd, IDC_TE_PATH, WM_GETTEXT, (WPARAM)(sizeof(tbuf) / sizeof(letter)), (LPARAM)tbuf);

				/* finally, write tags!! (remember, we can't assure
				   that; as long as we're depending from external
				   plug-ins */

				audio_input_tagwrite(tbuf, &at);

				/* free'em all! */

				for(i=0; i<21; i++)
				{
					if(tp[i].ta->tsize)
					{
						sys_mem_free(tp[i].ta->tdata);
					}
				}
				
			}
			break;

		case button_clear:
			if(MessageBox(hwnd, uni("Are you sure?"), uni("Clear Tags"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				letter fpath[v_sys_maxpath];

				memset(fpath, 0, sizeof(fpath));

				GetDlgItemText(hwnd, IDC_TE_PATH, fpath, sizeof(fpath) / sizeof(letter));

				/* clear tag */

				audio_input_tagwrite(fpath, 0);

				goto load_tag_agin;
			}
			break;

		case button_advanced:
			{
				HMENU  tm;
				POINT  pt;
				int    rid;

				tm = CreatePopupMenu();
				InsertMenu(tm, 0, MF_BYPOSITION | MF_ENABLED | MF_STRING, 1, uni("Apply selected tags to all playlist items.") );
				AppendMenu(tm, MF_ENABLED | MF_STRING, 2, uni("Apply selected tags to all playlist items (after clearing, only Fennec-handled tags will remain).") );
				GetCursorPos(&pt);
				rid = TrackPopupMenu(tm, TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, 0);

				switch(rid)
				{
				case 1:
					tagedit_apply2playlist(hwnd, 0);
					break;

				case 2:
					tagedit_apply2playlist(hwnd, 1);
					break;

				default:
					break;
				}

				DestroyMenu(tm);
			}
			break;

		case IDC_TE_SETFNAME:

			/* set formatted file name */

			{
				struct fennec_audiotag  at;                  /* audio tag */
				letter                  tbuf[v_sys_maxpath]; /* file path */
				letter                  ftname[250];         /* file name formatting */
				letter                  newfpath[1024];      /* formatted path */
				int                     i;                   /* first person singular ;-p */

				/* store controller ids and tags so we can access
				   them at once */

				struct tprops
				{
					struct fennec_audiotag_item *ta;
					int cont;
				};
				
				struct tprops tp[16 + 1];
				
				tp[ 0].ta = &at.tag_title           ;  tp[ 0].cont = IDC_TE_TITLE;
				tp[ 1].ta = &at.tag_album           ;  tp[ 1].cont = IDC_TE_ALBUM;
				tp[ 2].ta = &at.tag_artist          ;  tp[ 2].cont = IDC_TE_ARTIST;
				tp[ 3].ta = &at.tag_origartist      ;  tp[ 3].cont = IDC_TE_OARTIST;
				tp[ 4].ta = &at.tag_composer        ;  tp[ 4].cont = IDC_TE_COMPOSER;
				tp[ 5].ta = &at.tag_lyricist        ;  tp[ 5].cont = IDC_TE_LYRICIST;
				tp[ 6].ta = &at.tag_band            ;  tp[ 6].cont = IDC_TE_BAND;
				tp[ 7].ta = &at.tag_copyright       ;  tp[ 7].cont = IDC_TE_COPYRIGHT;
				tp[ 8].ta = &at.tag_encodedby       ;  tp[ 8].cont = IDC_TE_ENCODEDBY;
				tp[ 9].ta = &at.tag_genre           ;  tp[ 9].cont = IDC_TE_GENRE;
				tp[10].ta = &at.tag_year            ;  tp[10].cont = IDC_TE_YEAR;
				tp[11].ta = &at.tag_url             ;  tp[11].cont = IDC_TE_URL;
				tp[12].ta = &at.tag_offiartisturl   ;  tp[12].cont = IDC_TE_OFAURL;
				tp[13].ta = &at.tag_bpm             ;  tp[13].cont = IDC_TE_BPM;
				tp[14].ta = &at.tag_tracknum        ;  tp[14].cont = IDC_TE_TRACKNUM;
				tp[15].ta = &at.tag_publish         ;  tp[15].cont = IDC_TE_PUBLISHER;

				at.tag_album.tsize           = 0;
				at.tag_title.tsize           = 0;
				at.tag_album.tsize           = 0;
				at.tag_artist.tsize          = 0;
				at.tag_origartist.tsize      = 0;
				at.tag_composer.tsize        = 0;
				at.tag_lyricist.tsize        = 0;
				at.tag_band.tsize            = 0;
				at.tag_copyright.tsize       = 0;
				at.tag_publish.tsize         = 0;
				at.tag_encodedby.tsize       = 0;
				at.tag_genre.tsize           = 0;
				at.tag_year.tsize            = 0;
				at.tag_url.tsize             = 0;
				at.tag_offiartisturl.tsize   = 0;
				at.tag_filepath.tsize        = 0;
				at.tag_filename.tsize        = 0;
				at.tag_comments.tsize        = 0;
				at.tag_lyric.tsize           = 0;
				at.tag_bpm.tsize             = 0;
				at.tag_tracknum.tsize        = 0;

				/* get currently edited tags */

				for(i=0; i<16; i++)
				{
					tp[i].ta->tsize = ((unsigned int)SendDlgItemMessage(hwnd, tp[i].cont, WM_GETTEXTLENGTH, 0, 0) + 1) * sizeof(letter);
					
					if(tp[i].ta->tsize)
					{
						tp[i].ta->tsize++; /* null terminated! */
					
						tp[i].ta->tmode = tag_memmode_dynamic;
						tp[i].ta->tdata = sys_mem_alloc(tp[i].ta->tsize);

						memset(tp[i].ta->tdata, 0, tp[i].ta->tsize);

						SendDlgItemMessage(hwnd, tp[i].cont, WM_GETTEXT, (WPARAM)(tp[i].ta->tsize / sizeof(letter)), (LPARAM)tp[i].ta->tdata);
					}
				}

				/* get current formatting */

				ftname[0] = 0;
				SendDlgItemMessage(hwnd, IDC_TE_FORMATTING, WM_GETTEXT, (WPARAM)(sizeof(ftname) / sizeof(letter)), (LPARAM)ftname);

				/* invalid formatting? */

				if(!ftname[0])goto point_file_naming_free_tags;

				/* apply formatting */

				tags_translate(ftname, &at, 0);

				/* get current file path */

				memset(tbuf, 0, sizeof(tbuf));
				SendDlgItemMessage(hwnd, IDC_TE_PATH, WM_GETTEXT, (WPARAM)(sizeof(tbuf) / sizeof(letter)), (LPARAM)tbuf);

				/* if we're gonna play with the *wrong*
				   currently playing file, just stop it! */

				if(str_icmp(tbuf, audio_playlist_getsource(audio_playlist_getcurrentindex())) == 0)
				{
					audio_stop();
					sys_pass();
				}

				/* set new file path */

				str_cpy(newfpath, tbuf);
				newfpath[(sizeof(newfpath) / sizeof(letter)) - 1] = 0; /* make sure it's null-terminated */
				i = (unsigned int)str_len(newfpath);

				/* erase file name */

				while(!(newfpath[i] == uni('\\') || newfpath[i] == uni('/') || !i) ){ i--; }
				if(i)i++;

				/* copy new file name */

				str_cpy(newfpath + i, ftname);

				/* copy its extension back */

				i = (unsigned int)str_len(tbuf);
				while(!(tbuf[i] == uni('.') || !i)){i--;}

				/* oh! extensions ain't good at uppercase
				   vote me!! *(:oD) */
				
				if(i)
				{
					str_lower(tbuf + i);
					str_cat(newfpath, tbuf + i);
				}

				/* rename it! */

				if(sys_file_rename(tbuf /* old */, newfpath /* new */))
				{
					MessageBox(hwnd, uni("Could not rename the file; try setting file attributes (Read-Only?)."), uni("Rename file."), MB_ICONEXCLAMATION);
				}else{
					/* user may do it again! */
					SetDlgItemText(hwnd, IDC_TE_PATH, newfpath);
				}

point_file_naming_free_tags:

				for(i=0; i<16; i++)
				{
					if(tp[i].ta->tsize)
					{
						sys_mem_free(tp[i].ta->tdata);
					}
				}
				
			}
		}
		break;

	/* initialization routine,
	   this routine may be called
	   at the actual initialization
	   or at hide/show or 'next'
	   routines */

	case WM_APP + 1:
	case WM_INITDIALOG:

		tageditor_active = hwnd;

		/* <lang> */

		SetWindowText(hwnd, text(oooo_tagedit) );

		SetDlgItemText(hwnd, static_title		, text(oooo_tagedit_title				) );
		SetDlgItemText(hwnd, static_album		, text(oooo_tagedit_album				) );
		SetDlgItemText(hwnd, static_artist		, text(oooo_tagedit_artist				) );
		SetDlgItemText(hwnd, static_oartist	    , text(oooo_tagedit_original_artist	    ) );
		SetDlgItemText(hwnd, static_band		, text(oooo_tagedit_band				) );
		SetDlgItemText(hwnd, static_composer	, text(oooo_tagedit_composer			) );
		SetDlgItemText(hwnd, static_lyricist	, text(oooo_tagedit_lyricist			) );
		SetDlgItemText(hwnd, static_copyright	, text(oooo_tagedit_copyright			) );
		SetDlgItemText(hwnd, static_encodedby	, text(oooo_tagedit_encodedby			) );
		SetDlgItemText(hwnd, static_genre		, text(oooo_tagedit_genre				) );
		SetDlgItemText(hwnd, static_year		, text(oooo_tagedit_year				) );
		SetDlgItemText(hwnd, static_oartisturl  , text(oooo_tagedit_official_artist_url ) );
		SetDlgItemText(hwnd, static_url			, text(oooo_tagedit_url				    ) );
		SetDlgItemText(hwnd, static_comments	, text(oooo_tagedit_comments			) );
		SetDlgItemText(hwnd, static_lyric 		, text(oooo_tagedit_lyrics				) );
		SetDlgItemText(hwnd, static_bpm			, text(oooo_tagedit_bpm				    ) );
		SetDlgItemText(hwnd, static_track		, text(oooo_tagedit_track				) );
	  /*SetDlgItemText(hwnd, static_image		, text(oooo_tagedit_image				) ); */
	  /*SetDlgItemText(hwnd, static_description	, text(oooo_tagedit_description		    ) ); */
		SetDlgItemText(hwnd, static_source		, text(oooo_tagedit_source				) );
		SetDlgItemText(hwnd, static_publisher	, text(oooo_tagedit_publisher			) );
		
		SetDlgItemText(hwnd, IDC_TE_NEXTFILE	, text(oooo_tagedit_next				) );
	  /*SetDlgItemText(hwnd, button_previous	, text(oooo_tagedit_previous			) ); */
	  /*SetDlgItemText(hwnd, button_stop        , text(oooo_tagedit_stop                ) ); */
		SetDlgItemText(hwnd, IDC_TE_SETFNAME	, text(oooo_tagedit_set_filename		) );
		SetDlgItemText(hwnd, IDC_TE_COPY		, text(oooo_tagedit_copy				) );
		SetDlgItemText(hwnd, IDC_TE_PASTE		, text(oooo_tagedit_paste				) );
		SetDlgItemText(hwnd, button_clear		, text(oooo_tagedit_clear				) );
		SetDlgItemText(hwnd, IDOK				, text(oooo_tagedit_apply				) );
		SetDlgItemText(hwnd, IDCANCEL			, text(oooo_tagedit_cancel				) );


		/* </lang> */

		/* make sure, all zero (only at initialization) */

		tagedit_copiedtag.tag_album.tsize           = 0;
		tagedit_copiedtag.tag_title.tsize           = 0;
		tagedit_copiedtag.tag_album.tsize           = 0;
		tagedit_copiedtag.tag_artist.tsize          = 0;
		tagedit_copiedtag.tag_origartist.tsize      = 0;
		tagedit_copiedtag.tag_composer.tsize        = 0;
		tagedit_copiedtag.tag_lyricist.tsize        = 0;
		tagedit_copiedtag.tag_band.tsize            = 0;
		tagedit_copiedtag.tag_copyright.tsize       = 0;
		tagedit_copiedtag.tag_publish.tsize         = 0;
		tagedit_copiedtag.tag_encodedby.tsize       = 0;
		tagedit_copiedtag.tag_genre.tsize           = 0;
		tagedit_copiedtag.tag_year.tsize            = 0;
		tagedit_copiedtag.tag_url.tsize             = 0;
		tagedit_copiedtag.tag_offiartisturl.tsize   = 0;
		tagedit_copiedtag.tag_filepath.tsize        = 0;
		tagedit_copiedtag.tag_filename.tsize        = 0;
		tagedit_copiedtag.tag_comments.tsize        = 0;
		tagedit_copiedtag.tag_lyric.tsize           = 0;
		tagedit_copiedtag.tag_bpm.tsize             = 0;
		tagedit_copiedtag.tag_tracknum.tsize        = 0;

load_tag_agin:
		{
			struct fennec_audiotag       at;
			struct fennec_audiotag_item *ta; /* do u think i'm swapping bytes around? ;-D */
			unsigned long                ipid;
			unsigned int                 i;  /* this tiny ass had to go this far!! :o) */
			letter                       buf[10];

			/* zero'em all (we don't eat deadbeefs or baadf00ds :-) (not even faces! oh!) ) */

			at.tag_album.tsize           = 0;
			at.tag_title.tsize           = 0;
			at.tag_album.tsize           = 0;
			at.tag_artist.tsize          = 0;
			at.tag_origartist.tsize      = 0;
			at.tag_composer.tsize        = 0;
			at.tag_lyricist.tsize        = 0;
			at.tag_band.tsize            = 0;
			at.tag_copyright.tsize       = 0;
			at.tag_publish.tsize         = 0;
			at.tag_encodedby.tsize       = 0;
			at.tag_genre.tsize           = 0;
			at.tag_year.tsize            = 0;
			at.tag_url.tsize             = 0;
			at.tag_offiartisturl.tsize   = 0;
			at.tag_filepath.tsize        = 0;
			at.tag_filename.tsize        = 0;
			at.tag_comments.tsize        = 0;
			at.tag_lyric.tsize           = 0;
			at.tag_bpm.tsize             = 0;
			at.tag_tracknum.tsize        = 0;

			/* restore last values (called settings :-p ) */

			SetDlgItemText(hwnd, IDC_TE_FORMATTING, settings.tag_editing.rename_formatting);
			
			if(settings.tag_editing.checked_tags[0 ]) CheckDlgButton(hwnd, IDC_TE_CK_TITLE,     BST_CHECKED);
			if(settings.tag_editing.checked_tags[1 ]) CheckDlgButton(hwnd, IDC_TE_CK_ALBUM,     BST_CHECKED);
			if(settings.tag_editing.checked_tags[2 ]) CheckDlgButton(hwnd, IDC_TE_CK_ARTIST,    BST_CHECKED);
			if(settings.tag_editing.checked_tags[3 ]) CheckDlgButton(hwnd, IDC_TE_CK_OARTIST,   BST_CHECKED);
			if(settings.tag_editing.checked_tags[4 ]) CheckDlgButton(hwnd, IDC_TE_CK_BAND,      BST_CHECKED);
			if(settings.tag_editing.checked_tags[5 ]) CheckDlgButton(hwnd, IDC_TE_CK_COMPOSER,  BST_CHECKED);
			if(settings.tag_editing.checked_tags[6 ]) CheckDlgButton(hwnd, IDC_TE_CK_LYRICIST,  BST_CHECKED);
			if(settings.tag_editing.checked_tags[7 ]) CheckDlgButton(hwnd, IDC_TE_CK_COPYRIGHT, BST_CHECKED);
			if(settings.tag_editing.checked_tags[8 ]) CheckDlgButton(hwnd, IDC_TE_CK_ENCODER,   BST_CHECKED);
			if(settings.tag_editing.checked_tags[9 ]) CheckDlgButton(hwnd, IDC_TE_CK_GENRE,     BST_CHECKED);
			if(settings.tag_editing.checked_tags[10]) CheckDlgButton(hwnd, IDC_TE_CK_YEAR,      BST_CHECKED);
			if(settings.tag_editing.checked_tags[11]) CheckDlgButton(hwnd, IDC_TE_CK_OFAURL,    BST_CHECKED);
			if(settings.tag_editing.checked_tags[12]) CheckDlgButton(hwnd, IDC_TE_CK_URL,       BST_CHECKED);
			if(settings.tag_editing.checked_tags[13]) CheckDlgButton(hwnd, IDC_TE_CK_COMMENTS,  BST_CHECKED);
			if(settings.tag_editing.checked_tags[14]) CheckDlgButton(hwnd, IDC_TE_CK_LYRIC,     BST_CHECKED);
			if(settings.tag_editing.checked_tags[15]) CheckDlgButton(hwnd, IDC_TE_CK_BPM,       BST_CHECKED);
			if(settings.tag_editing.checked_tags[16]) CheckDlgButton(hwnd, IDC_TE_CK_TRACKNUM,  BST_CHECKED);
			if(settings.tag_editing.checked_tags[17]) CheckDlgButton(hwnd, IDC_TE_CK_PUBLISHER, BST_CHECKED);

			SetDlgItemText(hwnd, IDC_TE_PATH, tageditor_file);
	
			/* the user may have a desire to know the name of associated decoder ! :-) */

			{
				struct internal_input_plugin iplg;
				unsigned long         iplg_size = 0;
				int                   pslen;
				unsigned long         pid = audio_input_selectinput(tageditor_file);

				if(pid == -1)
				{
					/* the result should be printed in english ;~) */
					SetDlgItemText(hwnd, IDC_TE_DECNAME, uni("Unknown."));
				}else{
					settings_data_get(setting_id_input_plugin, pid, &iplg, &iplg_size);
					pslen = (int)str_len(iplg.pluginname);

					SetDlgItemText(hwnd, IDC_TE_DECNAME, iplg.pluginname);
				}
			}

			/* which genre you prefer? rock/metal/punk/electronic/acoustic
			   or .... opera! or hum..... firefox? #(:-D) */

			if(!SendDlgItemMessage(hwnd, IDC_TE_GENRE, CB_GETCOUNT, 0, 0))
			{
				for(i=0; i<=tag_genres_count; i++)
				{
					SendDlgItemMessage(hwnd, IDC_TE_GENRE, CB_ADDSTRING, 0, (LPARAM)tag_genres[i]);
				}
			}

			/* load tags from media source */

			ipid = audio_input_tagread(tageditor_file, &at);

			ta = &at.tag_title;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_TITLE, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_TITLE, uni(""));

			ta = &at.tag_album;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_ALBUM, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_ALBUM, uni(""));

			ta = &at.tag_artist;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_ARTIST, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_ARTIST, uni(""));

			ta = &at.tag_origartist;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_OARTIST, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_OARTIST, uni(""));

			ta = &at.tag_composer;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_COMPOSER, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_COMPOSER, uni(""));

			ta = &at.tag_lyricist;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_LYRICIST, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_LYRICIST, uni(""));
			
			ta = &at.tag_copyright;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_COPYRIGHT, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_COPYRIGHT, uni(""));
			
			ta = &at.tag_encodedby;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_ENCODEDBY, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_ENCODEDBY, uni(""));
			
			ta = &at.tag_genre;
			if(ta->tdata && ta->tsize)
			{
				if(ta->tdata)
				{
					if(ta->tdata[0] == '(')
					{
						int j = 0, k;
						while(j <= 4 && isdigit(ta->tdata[j + 1])){ j++; }

						k = str_stoi(ta->tdata + 1);

						if(k <= tag_genres_count)
						{
							SendDlgItemMessage(hwnd, IDC_TE_GENRE, CB_SETCURSEL, (WPARAM)k, 0);
						}else{
							SetDlgItemText(hwnd, IDC_TE_GENRE, ta->tdata);
						}
					}else{
						SetDlgItemText(hwnd, IDC_TE_GENRE, ta->tdata);
					}
				}
				

			}else{
				SetDlgItemText(hwnd, IDC_TE_GENRE, uni(""));
			}
			
			ta = &at.tag_year;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_YEAR, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_YEAR, uni(""));
			
			ta = &at.tag_offiartisturl;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_OFAURL, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_OFAURL, uni(""));
			
			ta = &at.tag_url;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_URL, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_URL, uni(""));
			
			ta = &at.tag_comments;
			if(ta->tdata && ta->tsize)
			{
				string tmem = (string) sys_mem_alloc((ta->tsize + sizeof(letter) /* null */) * 2 /* line ending conv. */ * sizeof(letter) /* tsize == character count? */);
				str_cpy(tmem, ta->tdata);
				str_tag_to_win(tmem);
				SetDlgItemText(hwnd, IDC_TE_COMMENTS, tmem);
				sys_mem_free(tmem);
			}else{
				SetDlgItemText(hwnd, IDC_TE_COMMENTS, uni(""));
			}
			
			ta = &at.tag_lyric;
			if(ta->tdata && ta->tsize)
			{
				string tmem = (string) sys_mem_alloc((ta->tsize + sizeof(letter) /* null */) * 2 /* line ending conv. */ * sizeof(letter) /* tsize == character count? */);
				str_cpy(tmem, ta->tdata);
				str_tag_to_win(tmem);
				SetDlgItemText(hwnd, IDC_TE_LYRICS, tmem);
				sys_mem_free(tmem);
			}else{
				SetDlgItemText(hwnd, IDC_TE_LYRICS, uni(""));
			}

			ta = &at.tag_bpm;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_BPM, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_BPM, uni(""));
			
			ta = &at.tag_tracknum;
			if(ta->tdata && ta->tsize)
				if(ta->tdata[0] < 48)
					SetDlgItemText(hwnd, IDC_TE_TRACKNUM, str_itos(ta->tdata[0], buf, 10));
				else
					SetDlgItemText(hwnd, IDC_TE_TRACKNUM, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_TRACKNUM, uni(""));

			ta = &at.tag_band;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_BAND, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_BAND, uni(""));
			
			ta = &at.tag_publish;
			if(ta->tdata && ta->tsize)
				SetDlgItemText(hwnd, IDC_TE_PUBLISHER, ta->tdata);
			else
				SetDlgItemText(hwnd, IDC_TE_PUBLISHER, uni(""));


			audio_input_tagread_known(ipid, 0, &at); /* free! */

			/* all the tags have been added! relax */
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}




/* </dep> */




/* local functions ----------------------------------------------------------*/


/* local functions */


/*
 * replace 'newline' with cr-lf.
 */
void str_tag_to_win(string str)
{
	unsigned int i = 0;                /* str (string to be replaced) pointer */
	unsigned int flen;                 /* full (str) length */

	flen = (unsigned int)str_len(str);

	while(str[i])
	{
		if((str[i] == uni('\n')) || (str[i] == uni('\r')))
		{

			str[i] = uni('\r');
			str_mmov(str + i + 1, str + i, flen - i);
			str[i + 1] = '\n';
			flen++;
			str[flen] = 0;
			i++;
		}

		i++;
	}
}


/*
 * replace cr-lf with 'newline'.
 */
void str_win_to_tag(string str)
{
	unsigned int i = 0;                /* str (string to be replaced) pointer */
	unsigned int flen;                 /* full (str) length */

	flen = (unsigned int)str_len(str);

	while(str[i])
	{
		if(str[i] == uni('\r') && str[i + 1] == uni('\n'))
		{

			str[i] = uni('\r');
			str_mmov(str + i, str + i + 1, flen - i - 1);
			flen--;
			str[flen] = 0;
			i++;
		}

		i++;
	}
}


/* <dep> win */


/*
 * equalizer preset
 */


/*
 * refresh equalizer presets list.
 * lstwnd - list window.
 */
void local_eqpresets_refresh(HWND lstwnd)
{
	unsigned long nbr;
	int i;
	int m;
	equalizer_preset eqp;

	m = settings.player.equalizer_presets;

	SendMessage(lstwnd, LB_RESETCONTENT, 0, 0);

	for(i=0; i<m; i++)
	{
		settings_data_get(setting_id_equalizer_preset, i, &eqp, &nbr);

		if(nbr == sizeof(eqp))
			SendMessage(lstwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)eqp.name);
	}

	SendMessage(lstwnd, LB_SETCURSEL, (WPARAM)settings.player.equalizer_last_preset_id, 0);
}


/*
 * set current equalizer preset.
 * lstwnd - list window.
 */
void local_eqpresets_setcurrent(HWND lstwnd)
{
	unsigned int i = (int)SendMessage(lstwnd, LB_GETCURSEL, 0, 0);
			
	if(i < settings.player.equalizer_presets)
	{
		unsigned long    nbr;
		equalizer_preset eqp;

		settings_data_get(setting_id_equalizer_preset, i, &eqp, &nbr);
		
		if(nbr == sizeof(eqp))
		{
			int   ci, j;
			float eqb[32];


			for(ci=0; ci<16; ci++)
			{
				for(j=0; j<32; j++)
				{
					eqb[j] = eqp.boost[ci][j];
				}

				equalizer_set_bands(ci, 32, eqb);
				equalizer_set_preamp(ci, eqp.preamp[ci]);
			}

			str_cpy(settings.player.equalizer_last.name, eqp.name);


			settings.player.equalizer_last_preset_id = (unsigned short)i;
			fennec_refresh(fennec_v_refresh_force_less);
		}
	}
}



/*
 * apply selected tags to all playlist items
 */
void tagedit_apply2playlist(HWND hwnd, int tclear)
{
	struct fennec_audiotag at;
	struct fennec_audiotag readtag;
	letter tbuf[v_sys_maxpath];
	int    i, k, m, anychange;
	unsigned long  tagplgid;

	/* store tags and respective controller ids */

	struct tprops
	{
		struct fennec_audiotag_item *ta;
		int                          cont;
		int                          checked;
	};
	
	struct tprops tp[18 + 1];
	
	tp[ 0].ta = &at.tag_title           ; tp[ 0].cont = IDC_TE_TITLE     ; tp[ 0].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_TITLE    );
	tp[ 1].ta = &at.tag_album           ; tp[ 1].cont = IDC_TE_ALBUM     ; tp[ 1].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_ALBUM    );
	tp[ 2].ta = &at.tag_artist          ; tp[ 2].cont = IDC_TE_ARTIST    ; tp[ 2].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_ARTIST   );
	tp[ 3].ta = &at.tag_origartist      ; tp[ 3].cont = IDC_TE_OARTIST   ; tp[ 3].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_OARTIST  );
	tp[ 4].ta = &at.tag_composer        ; tp[ 4].cont = IDC_TE_COMPOSER  ; tp[ 4].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_COMPOSER );
	tp[ 5].ta = &at.tag_lyricist        ; tp[ 5].cont = IDC_TE_LYRICIST  ; tp[ 5].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRICIST );
	tp[ 6].ta = &at.tag_band            ; tp[ 6].cont = IDC_TE_BAND      ; tp[ 6].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_BAND     );
	tp[ 7].ta = &at.tag_copyright       ; tp[ 7].cont = IDC_TE_COPYRIGHT ; tp[ 7].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_COPYRIGHT);
	tp[ 8].ta = &at.tag_encodedby       ; tp[ 8].cont = IDC_TE_ENCODEDBY ; tp[ 8].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_ENCODER  );
	tp[ 9].ta = &at.tag_genre           ; tp[ 9].cont = IDC_TE_GENRE     ; tp[ 9].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_GENRE    );
	tp[10].ta = &at.tag_year            ; tp[10].cont = IDC_TE_YEAR      ; tp[10].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_YEAR     );
	tp[11].ta = &at.tag_url             ; tp[11].cont = IDC_TE_URL       ; tp[11].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_URL      );
	tp[12].ta = &at.tag_offiartisturl   ; tp[12].cont = IDC_TE_OFAURL    ; tp[12].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_OFAURL   );
	tp[13].ta = &at.tag_comments        ; tp[13].cont = IDC_TE_COMMENTS  ; tp[13].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_COMMENTS );
	tp[14].ta = &at.tag_lyric           ; tp[14].cont = IDC_TE_LYRICS    ; tp[14].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_LYRIC    );
	tp[15].ta = &at.tag_bpm             ; tp[15].cont = IDC_TE_BPM       ; tp[15].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_BPM      );
	tp[16].ta = &at.tag_tracknum        ; tp[16].cont = IDC_TE_TRACKNUM  ; tp[16].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_TRACKNUM );
	tp[17].ta = &at.tag_publish         ; tp[17].cont = IDC_TE_PUBLISHER ; tp[17].checked = IsDlgButtonChecked(hwnd, IDC_TE_CK_PUBLISHER);

	/* copy tags to memory */

	m = audio_playlist_getcount();

	audio_stop();

	for(k=0; k<m; k++)
	{
		str_cpy(tbuf, audio_playlist_getsource(k));

		tagplgid = audio_input_tagread(tbuf, &readtag);

		/* clear it out if needed */

		if(tclear)
		{
			audio_input_tagwrite(tbuf, 0);
		}
		
		/* leave it as a backup */

		memcpy(&at, &readtag, sizeof(struct fennec_audiotag));

		anychange = 0;
	
		for(i=0; i<18; i++)
		{
			if(tp[i].checked)
			{
				anychange = 1;
				
				tp[i].ta->tsize = (unsigned int)SendDlgItemMessage(hwnd, tp[i].cont, WM_GETTEXTLENGTH, 0, 0) * sizeof(letter);
				
				if(tp[i].ta->tsize)
				{
					tp[i].ta->tsize += sizeof(letter); /* terminating null */
					tp[i].ta->tdata = sys_mem_alloc(tp[i].ta->tsize);
					memset(tp[i].ta->tdata, 0, tp[i].ta->tsize);
					
					SendDlgItemMessage(hwnd, tp[i].cont, WM_GETTEXT, (WPARAM)(tp[i].ta->tsize / sizeof(letter)), (LPARAM)tp[i].ta->tdata);
					
					/* replace cr-lf combinations with 'newlines' */

					str_win_to_tag(tp[i].ta->tdata);

					tp[i].ta->tsize = (unsigned int)str_size(tp[i].ta->tdata);
					
					tp[i].ta->tmode = tag_memmode_dynamic;
				}
			}
		} /* <for each tag> */

		if(anychange)
		{
			audio_input_tagwrite(tbuf, &at);

			/* free'em all! */

			for(i=0; i<18; i++)
			{
				if(tp[i].ta->tsize && tp[i].checked)
				{
					sys_mem_free(tp[i].ta->tdata);
				}
			}
		}

		//audio_input_tagfree(tagplgid, &readtag);

	} /* </for each file> */
}


/*
 * draw custom text.
 */
int local_credits_drawtext(HWND hwnd)
{
	static int credits_init = 0, sy;

	static HDC      dc;
	static HDC      credits_memdc;
	static HBITMAP  credits_bmp;
	static HWND     credits_wnd;
	static HFONT    credits_font_n;
	static HFONT    credits_font_b;
	static HFONT    credits_font_i;
	static HFONT    credits_font_t;
	static RECT     rct;
	static string   credits_text;
	static HPEN     oldpen;
	static HBRUSH   oldbrush;
	static HPEN     hpen;
	static HBRUSH   hbrush;

	size_t  ln;
	string  lstr = credits_text;
	string  nstr = credits_text;
	int     y    = sy;

	sy--;

	if(!credits_init)
	{
		credits_wnd = hwnd;
		dc          = GetDC(hwnd);

		GetClientRect(hwnd, &rct);

		credits_memdc = CreateCompatibleDC(window_main_dc);
		credits_bmp   = CreateCompatibleBitmap(window_main_dc, rct.right, rct.bottom);
		SelectObject(credits_memdc, credits_bmp);

		credits_text = fennec_loadtext(uni("text"), uni("credits")) + 1;

		sy = rct.bottom / 2;


		hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE));
		hbrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

		oldpen = SelectObject(credits_memdc, hpen);
		oldbrush = SelectObject(credits_memdc, hbrush);

		SetBkMode(credits_memdc, TRANSPARENT);


		credits_font_n = CreateFont(-MulDiv(8, GetDeviceCaps(credits_memdc, LOGPIXELSY), 72),
									0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									DEFAULT_PITCH, uni("MS Sans Serif"));

		credits_font_b = CreateFont(-MulDiv(8, GetDeviceCaps(credits_memdc, LOGPIXELSY), 72),
									0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									DEFAULT_PITCH, uni("Arial"));

		credits_font_i = CreateFont(-MulDiv(8, GetDeviceCaps(credits_memdc, LOGPIXELSY), 72),
									0, 0, 0, FW_NORMAL, 1, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									DEFAULT_PITCH, uni("Arial"));
	    
		credits_font_t = CreateFont(-MulDiv(10, GetDeviceCaps(credits_memdc, LOGPIXELSY), 72),
									0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									DEFAULT_PITCH, uni("Arial"));

		credits_init = 1;

		return 0;

	}else if(!hwnd && credits_init){

		DeleteObject(credits_bmp);
		DeleteDC(credits_memdc);
		DeleteDC(dc);

		DeleteObject(credits_font_n);
		DeleteObject(credits_font_b);
		DeleteObject(credits_font_i);
		DeleteObject(credits_font_t);

		SelectObject(dc, oldpen);
		SelectObject(dc, oldbrush);
		
		DeleteObject(hpen);
		DeleteObject(hbrush);

		fennec_loadtext(0, 0);
		credits_init = 0;

		return 0;
	}


	/* draw text ---------------------------------------------*/

	Rectangle(credits_memdc, 0, 0, rct.right, rct.bottom);

	while(nstr)
	{
		nstr = str_chr(nstr, uni('\n'));

		if(nstr)nstr++;

		ln = (size_t)(nstr - lstr);


		if(lstr[0] == uni('<'))
		{
			if(lstr[1] == uni('b') && lstr[2] == uni('>'))
			{
				SelectObject(credits_memdc, credits_font_b);
				SetTextColor(credits_memdc, 0);
			}

			if(lstr[1] == uni('n') && lstr[2] == uni('>'))
			{
				SelectObject(credits_memdc, credits_font_n);
				SetTextColor(credits_memdc, 0);
			}

			if(lstr[1] == uni('i') && lstr[2] == uni('>'))
			{
				SelectObject(credits_memdc, credits_font_i);
				SetTextColor(credits_memdc, 0);
			}

			if(lstr[1] == uni('t') && lstr[2] == uni('>'))
			{
				SelectObject(credits_memdc, credits_font_t);
				SetTextColor(credits_memdc, 0x88);
			}
		}

		lstr += 4;
		ln   -= 4;

		TextOut(credits_memdc, 0, y, lstr, (int)(ln - 2));
		y += 20;

		lstr = nstr;
	}

	if(y <= 0)sy = rct.bottom;

	BitBlt(dc, 0, 0, rct.right, rct.bottom, credits_memdc, 0, 0, SRCCOPY);

	return 0;
}


/* </dep> */






/* trashed --------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-----------------------------------------------------------------------------*/

join_data *j_data;

/* declarations */

int AboutProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int OptionsProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int ConversionProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int JoiningProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int RippingProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int Conv_MiscProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int Conv_TaggingProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int Conv_VolumeProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int Conv_EqualizerProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int TaggingProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

/* variables */

HWND  BaseWindow_About = 0;   /* 'About' window handle */


unsigned long conv_listextent = 0;
float         conv_eqbands[22];
unsigned char conv_useeq = 0;
unsigned long conv_volume = 10000; /* 0 to 10000 */
unsigned long conv_gain   = 0; /* 0 to 10000 */
unsigned char conv_normalize = 0;
unsigned char conv_copytags  = 1;
unsigned char conv_tag_def_title     = 1;
unsigned char conv_tag_def_artist    = 1;
unsigned char conv_tag_def_album     = 1;
unsigned char conv_tag_def_year      = 1;
unsigned char conv_tag_def_genre     = 1;
unsigned char conv_tag_def_comments  = 1;

letter        conv_tag_title   [1024];
letter        conv_tag_artist  [1024];
letter        conv_tag_album   [1024];
letter        conv_tag_year    [5];
letter        conv_tag_genre   [1024];
letter        conv_tag_comments[1024];

unsigned long conv_tag_genreid;
unsigned long conv_selected_enc = (unsigned long)-1;

int           conv_proc_cancel = 0;
int           conv_proc_pause  = 0;
int           conv_pos  = 0;

HWND          hwnd_current;
int           conv_threadrunning = 0;

int           conv_misc_type   = misc_type_orig;
int           conv_misc_year   = 2007;
int           conv_misc_month  = 6;
int           conv_misc_date   = 19;
int           conv_misc_hour   = 7;
int           conv_misc_minute = 0;

int           ct_rc = 0;
int           ct_ci = 0;


HWND          conv_thread_sclose = 0;
HWND          conv_hwnd;

int           seldrive_msgok;
int           seldrive_driveid;
t_sys_library_handle rip_libhandle;
rip_tag      *rip_tags = 0;
int           rip_tracks = 0;

typedef int (__cdecl *cd_genfunc)(char driveid);
typedef int (__cdecl *cd_trckfunc)(char driveid, unsigned int trackid);

cd_genfunc    rip_cda_load           = 0;
cd_genfunc    rip_cda_check          = 0;
cd_genfunc    rip_cda_teject         = 0;
cd_genfunc    rip_cda_tload          = 0;
cd_genfunc    rip_cda_gettrackscount = 0;

int           join_lastsel = -1;

unsigned long WINAPI conv_thread( LPVOID lpParam ) ;

int           conversion_window_open = 0;
int           ripping_window_open    = 0;
int           joining_window_open    = 0;

/* ----------------------------------------------------------------------------
 global functions.
---------------------------------------------------------------------------- */



/*
 show 'Conversion' window.

 wmodal : display modal?
 
 return : window handle of 'Conversion'.
*/
/*HWND BaseWindows_ShowConversion(int wmodal)
{
	if(conversion_window_open)return 0;
	conversion_window_open = 1;

	DialogBox(instance_fennec, (LPCTSTR)IDD_CONVERSION, wmodal ? window_main : 0, (DLGPROC)ConversionProc);
	return (HWND)1;
}*/

/*
 *
 */
HWND BaseWindows_ShowRipping(int wmodal)
{
	if(ripping_window_open)return 0;
	ripping_window_open = 1;

	DialogBox(instance_fennec, (LPCTSTR)IDD_RIP, wmodal ? window_main : 0, (DLGPROC)RippingProc);
	return (HWND)1;
}

HWND BaseWindows_ShowJoining(int wmodal)
{
	if(joining_window_open)return 0;
	joining_window_open = 1;

	DialogBox(instance_fennec, (LPCTSTR)IDD_JOIN, wmodal ? window_main : 0, (DLGPROC)JoiningProc);
	return (HWND)1;
}

/* ----------------------------------------------------------------------------
 local functions.
---------------------------------------------------------------------------- */

int Conv_OpenFileList(HWND hwlist, const string fpaths)
{
	WIN32_FIND_DATA   fd;
	letter            spath[v_sys_maxpath];
	letter            apath[v_sys_maxpath];
	unsigned long     i = 0, j = 0, k;
	HDC               hldc;
	SIZE              pfext;
	int               lcount;

	/* run till we find the first null */


	while(fpaths[i])i++;

	str_mcpy(spath, fpaths, i);
	spath[i] = 0;

	j = i + 1;

	/* if the path is a directory, we'll have to add multiple files */

	hldc = GetDC(hwlist);

	if(j > 4)
	{
		if(FindFirstFile(spath, &fd) == INVALID_HANDLE_VALUE)return 0;
	}else{
		i--;
		goto add_selected;
	}
	
	

	if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		/* just open the file */

		SendMessage(hwlist, LB_ADDSTRING, 0, (LPARAM)spath);
		lcount = (int)SendMessage(hwlist, LB_GETCOUNT, 0, 0);
		SendMessage(hwlist, LB_SETSEL, 1, (LPARAM)lcount - 1);

		GetTextExtentPoint(hldc, spath, (int)str_len(spath), &pfext);

		if(conv_listextent < (unsigned long)pfext.cx)
		{
			conv_listextent = pfext.cx;
			SendMessage(hwlist, LB_SETHORIZONTALEXTENT, (WPARAM)conv_listextent, 0);
			SetFocus(hwlist);
		}

		ReleaseDC(hwlist, hldc);
		return 1;
	}else{
add_selected:
		/* oh, the hard way */

		for(;;)
		{

			str_mcpy(apath, spath, i);
			apath[i] = uni('\\');
			k = (unsigned long)str_len(fpaths + j);
			if(!k)
			{
				ReleaseDC(hwlist, hldc);
				return 1;
			}
			
			str_mcpy(apath + i + 1, fpaths + j, k);
			apath[i + k + 1] = 0;
				
			j += k + 1;

			SendMessage(hwlist, LB_ADDSTRING, 0, (LPARAM)apath);
			lcount = (int)SendMessage(hwlist, LB_GETCOUNT, 0, 0);
			SendMessage(hwlist, LB_SETSEL, 1, (LPARAM)lcount - 1);

			GetTextExtentPoint(hldc, apath, (int)str_len(apath), &pfext);

			if(conv_listextent < (unsigned long)pfext.cx)
			{
				conv_listextent = pfext.cx;
				SendMessage(hwlist, LB_SETHORIZONTALEXTENT, (WPARAM)conv_listextent, 0);
				SetFocus(hwlist);
			}

		}
	}
}

int Conv_EqualizerProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;

		case IDC_EQUALIZER_PRESETS:
			{
				equalizer_preset  eqp;
				unsigned long     eqpid, szret = 0;
				
				eqpid = (unsigned long)SendDlgItemMessage(hwnd, IDC_EQUALIZER_PRESETS, CB_GETCURSEL, 0, 0);

				if(eqpid <= settings.player.equalizer_presets)
				{
					settings_data_get(setting_id_equalizer_preset, eqpid, &eqp, &szret);
					if(szret)
					{
						SendDlgItemMessage(hwnd, IDC_PREAMP1, TBM_SETPOS, 1, (LPARAM)(eqp.preamp[0] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_PREAMP2, TBM_SETPOS, 1, (LPARAM)(eqp.preamp[1] * -1000.0f));

						SendDlgItemMessage(hwnd, IDC_LEFT1,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][0] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT2,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][1] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT3,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][2] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT4,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][3] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT5,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][4] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT6,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][5] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT7,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][6] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT8,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][7] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT9,   TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][8] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_LEFT10,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[0][9] * -1000.0f));

						SendDlgItemMessage(hwnd, IDC_RIGHT1,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][0] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT2,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][1] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT3,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][2] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT4,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][3] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT5,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][4] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT6,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][5] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT7,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][6] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT8,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][7] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT9,  TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][8] * -1000.0f));
						SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_SETPOS, 1, (LPARAM)(eqp.boost[1][9] * -1000.0f));
					}
				}
				break;

			case IDOK:
				settings.conversion.equalizer_bands.preamp[0] = ((float)SendDlgItemMessage(hwnd, IDC_PREAMP1, TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.preamp[1]  = ((float)SendDlgItemMessage(hwnd, IDC_PREAMP2, TBM_GETPOS, 0, 0)) / -1000.0f;

				settings.conversion.equalizer_bands.boost[0][0] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT1,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][1] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT2,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][2] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT3,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][3] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT4,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][4] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT5,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][5] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT6,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][6] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT7,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][7] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT8,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][8] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT9,   TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[0][9] = ((float)SendDlgItemMessage(hwnd, IDC_LEFT10,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][0] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT1,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][1] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT2,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][2] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT3,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][3] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT4,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][4] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT5,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][5] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT6,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][6] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT7,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][7] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT8,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][8] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT9,  TBM_GETPOS, 0, 0)) / -1000.0f;
				settings.conversion.equalizer_bands.boost[1][9] = ((float)SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_GETPOS, 0, 0)) / -1000.0f;

				if(IsDlgButtonChecked(hwnd, IDC_EQUALIZER_USE) == BST_CHECKED)
				{
					settings.conversion.use_equalizer = 1;
				}else{
					settings.conversion.use_equalizer = 0;
				}

				EndDialog(hwnd, 0);
				break;
			}
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			equalizer_preset  eqp;
			unsigned long     i;
			unsigned long     dsz = 0;


			/* <lang> */

			SetWindowText(hwnd, text(oooo_conv_config_eq) );

			SetDlgItemText(hwnd, static_preset,             text(oooo_conv_config_eq_preset         ) );
			SetDlgItemText(hwnd, static_preamp,             text(oooo_conv_config_eq_preamp         ) );
			SetDlgItemText(hwnd, static_left,               text(oooo_conv_config_eq_left_channel   ) );
			SetDlgItemText(hwnd, static_right,              text(oooo_conv_config_eq_right_channel  ) );
			SetDlgItemText(hwnd, IDC_EQUALIZER_USE,         text(oooo_conv_config_eq_use_equalizer  ) );
			SetDlgItemText(hwnd, IDC_EQUALIZER_SELECTBOTH,  text(oooo_conv_config_eq_universal      ) );
			SetDlgItemText(hwnd, IDOK,                      text(oooo_ok                            ) );
			SetDlgItemText(hwnd, IDCANCEL,                  text(oooo_cancel                        ) );

			/* </lang> */



			for(i=0; i<settings.player.equalizer_presets; i++)
			{
				memset(eqp.name, 0, 30);
				settings_data_get(setting_id_equalizer_preset, i, &eqp, &dsz);
				if(dsz)
				{
					SendDlgItemMessage(hwnd, IDC_EQUALIZER_PRESETS, CB_ADDSTRING, 0, (LPARAM)eqp.name);
				}
			}

			SendDlgItemMessage(hwnd, IDC_PREAMP1,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_PREAMP1,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_PREAMP2,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_PREAMP2,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);

			SendDlgItemMessage(hwnd, IDC_LEFT1,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT1,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT2,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT2,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT3,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT3,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT4,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT4,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT5,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT5,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT6,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT6,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT7,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT7,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT8,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT8,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT9,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT9,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_LEFT10, TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_LEFT10, TBM_SETRANGEMAX, 1, (LPARAM)+12000);

			SendDlgItemMessage(hwnd, IDC_RIGHT1,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT1,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT2,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT2,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT3,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT3,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT4,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT4,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT5,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT5,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT6,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT6,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT7,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT7,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT8,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT8,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT9,  TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT9,  TBM_SETRANGEMAX, 1, (LPARAM)+12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_SETRANGEMIN, 1, (LPARAM)-12000);
			SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_SETRANGEMAX, 1, (LPARAM)+12000);

			SendDlgItemMessage(hwnd, IDC_PREAMP1, TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.preamp[0] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_PREAMP2, TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.preamp[1] * -1000.0f));

			SendDlgItemMessage(hwnd, IDC_LEFT1,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][0] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT2,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][1] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT3,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][2] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT4,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][3] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT5,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][4] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT6,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][5] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT7,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][6] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT8,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][7] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT9,   TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][8] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_LEFT10,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[0][9] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT1,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][0] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT2,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][1] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT3,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][2] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT4,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][3] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT5,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][4] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT6,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][5] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT7,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][6] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT8,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][7] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT9,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][8] * -1000.0f));
			SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_SETPOS, 1, (LPARAM)(settings.conversion.equalizer_bands.boost[1][9] * -1000.0f));
		
			CheckDlgButton(hwnd, IDC_EQUALIZER_USE, (settings.conversion.use_equalizer == 1 ? BST_CHECKED : BST_UNCHECKED));
		}
		break;

	case WM_VSCROLL:
		if(IsDlgButtonChecked(hwnd, IDC_EQUALIZER_SELECTBOTH) == BST_CHECKED)
		{
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_PREAMP1))SendDlgItemMessage(hwnd, IDC_PREAMP2, TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_PREAMP1, TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_PREAMP2))SendDlgItemMessage(hwnd, IDC_PREAMP1, TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_PREAMP2, TBM_GETPOS, 0, 0));

			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT1  ))SendDlgItemMessage(hwnd, IDC_RIGHT1 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT1  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT2  ))SendDlgItemMessage(hwnd, IDC_RIGHT2 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT2  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT3  ))SendDlgItemMessage(hwnd, IDC_RIGHT3 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT3  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT4  ))SendDlgItemMessage(hwnd, IDC_RIGHT4 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT4  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT5  ))SendDlgItemMessage(hwnd, IDC_RIGHT5 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT5  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT6  ))SendDlgItemMessage(hwnd, IDC_RIGHT6 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT6  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT7  ))SendDlgItemMessage(hwnd, IDC_RIGHT7 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT7  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT8  ))SendDlgItemMessage(hwnd, IDC_RIGHT8 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT8  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT9  ))SendDlgItemMessage(hwnd, IDC_RIGHT9 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT9  , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_LEFT10 ))SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_LEFT10 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT1 ))SendDlgItemMessage(hwnd, IDC_LEFT1  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT1 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT2 ))SendDlgItemMessage(hwnd, IDC_LEFT2  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT2 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT3 ))SendDlgItemMessage(hwnd, IDC_LEFT3  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT3 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT4 ))SendDlgItemMessage(hwnd, IDC_LEFT4  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT4 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT5 ))SendDlgItemMessage(hwnd, IDC_LEFT5  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT5 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT6 ))SendDlgItemMessage(hwnd, IDC_LEFT6  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT6 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT7 ))SendDlgItemMessage(hwnd, IDC_LEFT7  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT7 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT8 ))SendDlgItemMessage(hwnd, IDC_LEFT8  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT8 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT9 ))SendDlgItemMessage(hwnd, IDC_LEFT9  , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT9 , TBM_GETPOS, 0, 0));
			if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_RIGHT10))SendDlgItemMessage(hwnd, IDC_LEFT10 , TBM_SETPOS, 1, (LPARAM)SendDlgItemMessage(hwnd, IDC_RIGHT10, TBM_GETPOS, 0, 0));
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

int Conv_VolumeProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			settings.conversion.volume      = (((double)(unsigned long)SendDlgItemMessage(hwnd, IDC_VOLUME_VOLUME,  TBM_GETPOS, 0, 0)) / 10000.0f);
			settings.conversion.volume_gain  = (((double)(unsigned long)SendDlgItemMessage(hwnd, IDC_VOLUME_GAIN,    TBM_GETPOS, 0, 0)) / 10000.0f);
			
			settings.conversion.volume_normalization = IsDlgButtonChecked(hwnd, IDC_VOLUME_NORM) == BST_CHECKED ? 1 : 0;

			EndDialog(hwnd, 0);
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:

		/* <lang> */

		SetWindowText(hwnd, text(oooo_conv_config_vol) );

		SetDlgItemText(hwnd, static_volume, text(oooo_conv_config_vol_volume) );
		SetDlgItemText(hwnd, static_gain,   text(oooo_conv_config_vol_gain  ) );
		//SetDlgItemText(hwnd, static_note,   text(oooo_conv_config_vol_info  ) );
		SetDlgItemText(hwnd, IDOK,          text(oooo_ok                    ) );
		SetDlgItemText(hwnd, IDCANCEL,      text(oooo_cancel                ) );

		/* </lang> */

		SendDlgItemMessage(hwnd, IDC_VOLUME_VOLUME,  TBM_SETRANGEMIN, 1, (LPARAM)0);
		SendDlgItemMessage(hwnd, IDC_VOLUME_VOLUME,  TBM_SETRANGEMAX, 1, (LPARAM)10000);
		SendDlgItemMessage(hwnd, IDC_VOLUME_GAIN,    TBM_SETRANGEMIN, 1, (LPARAM)0);
		SendDlgItemMessage(hwnd, IDC_VOLUME_GAIN,    TBM_SETRANGEMAX, 1, (LPARAM)10000);

		SendDlgItemMessage(hwnd, IDC_VOLUME_VOLUME,  TBM_SETPOS, 1, (LPARAM)(settings.conversion.volume * 10000.0f));
		SendDlgItemMessage(hwnd, IDC_VOLUME_GAIN,    TBM_SETPOS, 1, (LPARAM)(settings.conversion.volume_gain * 10000.0f));

		CheckDlgButton(hwnd, IDC_VOLUME_NORM, (settings.conversion.volume_normalization == 1 ? BST_CHECKED : BST_UNCHECKED));
		
		if(settings.conversion.volume > 0.98)
		{
			EnableWindow((HWND)GetDlgItem(hwnd, IDC_VOLUME_GAIN), 1);
		}else{
			EnableWindow((HWND)GetDlgItem(hwnd, IDC_VOLUME_GAIN), 0);
		}

		break;

	case WM_HSCROLL:
		if(lParam == (LPARAM)GetDlgItem(hwnd, IDC_VOLUME_VOLUME))
		{
			if(SendDlgItemMessage(hwnd, IDC_VOLUME_VOLUME, TBM_GETPOS, 0, 0) == 10000)
			{
				/* enable gain */
				EnableWindow((HWND)GetDlgItem(hwnd, IDC_VOLUME_GAIN), 1);
			}else{
				EnableWindow((HWND)GetDlgItem(hwnd, IDC_VOLUME_GAIN), 0);
			}
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

int Conv_TaggingProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			SendDlgItemMessage(hwnd, IDC_TAG_EDIT_TITLE,    WM_GETTEXT, sizeof(conv_tag_title   ), (LPARAM)conv_tag_title);
			SendDlgItemMessage(hwnd, IDC_TAG_EDIT_ARTIST,   WM_GETTEXT, sizeof(conv_tag_artist  ), (LPARAM)conv_tag_artist);
			SendDlgItemMessage(hwnd, IDC_TAG_EDIT_ALBUM,    WM_GETTEXT, sizeof(conv_tag_album   ), (LPARAM)conv_tag_album);
			SendDlgItemMessage(hwnd, IDC_TAG_EDIT_YEAR,     WM_GETTEXT, sizeof(conv_tag_year    ), (LPARAM)conv_tag_year);
			SendDlgItemMessage(hwnd, IDC_TAG_COMBO_GENRE,   WM_GETTEXT, sizeof(conv_tag_genre   ), (LPARAM)conv_tag_genre);
			SendDlgItemMessage(hwnd, IDC_TAG_EDIT_COMMENTS, WM_GETTEXT, sizeof(conv_tag_comments), (LPARAM)conv_tag_comments);
			
			conv_tag_def_title    = (IsDlgButtonChecked(hwnd, IDC_TAG_TITLE)    == BST_CHECKED ? 1 : 0);
			conv_tag_def_artist   = (IsDlgButtonChecked(hwnd, IDC_TAG_ARTIST)   == BST_CHECKED ? 1 : 0);
			conv_tag_def_album    = (IsDlgButtonChecked(hwnd, IDC_TAG_ALBUM)    == BST_CHECKED ? 1 : 0);
			conv_tag_def_year     = (IsDlgButtonChecked(hwnd, IDC_TAG_YEAR)     == BST_CHECKED ? 1 : 0);
			conv_tag_def_genre    = (IsDlgButtonChecked(hwnd, IDC_TAG_GENRE)    == BST_CHECKED ? 1 : 0);
			conv_tag_def_comments = (IsDlgButtonChecked(hwnd, IDC_TAG_COMMENTS) == BST_CHECKED ? 1 : 0);
			conv_copytags         = (IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG)  == BST_CHECKED ? 1 : 0);
			conv_tag_genreid      = (unsigned long)-1;

			{
				unsigned int i;
				for(i=0; i<tag_genres_count; i++)
				{
					if(!str_icmp(conv_tag_genre, tag_genres[i]))
					{
						conv_tag_genreid = i;
						break;
					}
				}
			}
			EndDialog(hwnd, 0);
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}

		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_TITLE),    IsDlgButtonChecked(hwnd, IDC_TAG_TITLE   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_ARTIST),   IsDlgButtonChecked(hwnd, IDC_TAG_ARTIST  ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_ALBUM),    IsDlgButtonChecked(hwnd, IDC_TAG_ALBUM   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_YEAR),     IsDlgButtonChecked(hwnd, IDC_TAG_YEAR    ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_COMBO_GENRE),   IsDlgButtonChecked(hwnd, IDC_TAG_GENRE   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_COMMENTS), IsDlgButtonChecked(hwnd, IDC_TAG_COMMENTS) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);

		EnableWindow(GetDlgItem(hwnd, IDC_TAG_TITLE),    IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_ARTIST),   IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_ALBUM),    IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_YEAR),     IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_GENRE),    IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_COMMENTS), IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);

		break;

	case WM_INITDIALOG:

		/* <lang> */

		SetWindowText(hwnd, text(oooo_conv_config_tag) );

		SetDlgItemText(hwnd, IDC_TAG_COPYTAG,   text(oooo_conv_config_tag_copy_info  ) );
		SetDlgItemText(hwnd, IDC_TAG_TITLE,     text(oooo_tagedit_title              ) );
		SetDlgItemText(hwnd, IDC_TAG_ARTIST,    text(oooo_tagedit_artist             ) );
		SetDlgItemText(hwnd, IDC_TAG_ALBUM,     text(oooo_tagedit_album              ) );
		SetDlgItemText(hwnd, IDC_TAG_GENRE,     text(oooo_tagedit_genre              ) );
		SetDlgItemText(hwnd, IDC_TAG_COMMENTS,  text(oooo_tagedit_comments           ) );
		SetDlgItemText(hwnd, IDC_TAG_YEAR,      text(oooo_tagedit_year               ) );
		SetDlgItemText(hwnd, group_tagsel,      text(oooo_conv_config_tag_sel        ) );

		SetDlgItemText(hwnd, IDOK,                      text(oooo_ok                            ) );
		SetDlgItemText(hwnd, IDCANCEL,                  text(oooo_cancel                        ) );

		/* </lang> */


		SendDlgItemMessage(hwnd, IDC_TAG_EDIT_TITLE,    WM_SETTEXT, 0, (LPARAM)conv_tag_title);
		SendDlgItemMessage(hwnd, IDC_TAG_EDIT_ARTIST,   WM_SETTEXT, 0, (LPARAM)conv_tag_artist);
		SendDlgItemMessage(hwnd, IDC_TAG_EDIT_ALBUM,    WM_SETTEXT, 0, (LPARAM)conv_tag_album);
		SendDlgItemMessage(hwnd, IDC_TAG_EDIT_YEAR,     WM_SETTEXT, 0, (LPARAM)conv_tag_year);
		SendDlgItemMessage(hwnd, IDC_TAG_COMBO_GENRE,   WM_SETTEXT, 0, (LPARAM)conv_tag_genre);
		SendDlgItemMessage(hwnd, IDC_TAG_EDIT_COMMENTS, WM_SETTEXT, 0, (LPARAM)conv_tag_comments);
		
		CheckDlgButton(hwnd, IDC_TAG_TITLE,    conv_tag_def_title    ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TAG_ARTIST,   conv_tag_def_artist   ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TAG_ALBUM,    conv_tag_def_album    ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TAG_YEAR,     conv_tag_def_year     ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TAG_GENRE,    conv_tag_def_genre    ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TAG_COMMENTS, conv_tag_def_comments ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TAG_COPYTAG,  conv_copytags         ? BST_CHECKED : BST_UNCHECKED);
		
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_TITLE),    IsDlgButtonChecked(hwnd, IDC_TAG_TITLE   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_ARTIST),   IsDlgButtonChecked(hwnd, IDC_TAG_ARTIST  ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_ALBUM),    IsDlgButtonChecked(hwnd, IDC_TAG_ALBUM   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_YEAR),     IsDlgButtonChecked(hwnd, IDC_TAG_YEAR    ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_COMBO_GENRE),   IsDlgButtonChecked(hwnd, IDC_TAG_GENRE   ) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_EDIT_COMMENTS), IsDlgButtonChecked(hwnd, IDC_TAG_COMMENTS) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_TITLE),    IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_ARTIST),   IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_ALBUM),    IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_YEAR),     IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_GENRE),    IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);
		EnableWindow(GetDlgItem(hwnd, IDC_TAG_COMMENTS), IsDlgButtonChecked(hwnd, IDC_TAG_COPYTAG) != BST_CHECKED ? 0 : 1);

		{
			unsigned int i;
			for(i=0; i<tag_genres_count; i++)
			{
				SendDlgItemMessage(hwnd, IDC_TAG_COMBO_GENRE, CB_ADDSTRING, 0, (LPARAM)tag_genres[i]);
			}
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

int Conv_MiscProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(IsDlgButtonChecked(hwnd, IDC_ORIG)    == BST_CHECKED)conv_misc_type = misc_type_orig;
			if(IsDlgButtonChecked(hwnd, IDC_CURRENT) == BST_CHECKED)conv_misc_type = misc_type_current;
			if(IsDlgButtonChecked(hwnd, IDC_CUSTOM)  == BST_CHECKED)
			{
				int tr = 1;

				conv_misc_type = misc_type_user;

				conv_misc_year   = GetDlgItemInt(hwnd, IDC_YEAR  , &tr, 0);
				conv_misc_month  = GetDlgItemInt(hwnd, IDC_MONTH , &tr, 0);
				conv_misc_date   = GetDlgItemInt(hwnd, IDC_DATE  , &tr, 0);
				conv_misc_hour   = GetDlgItemInt(hwnd, IDC_HOUR  , &tr, 0);
				conv_misc_minute = GetDlgItemInt(hwnd, IDC_MINUTE, &tr, 0);
			}


		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		
		/* <lang> */

		SetWindowText(hwnd, text(oooo_conv_config_misc) );

		SetDlgItemText(hwnd, group_datetime,   text(oooo_conv_config_misc_date_and_time  ) );
		SetDlgItemText(hwnd, static_year,      text(oooo_conv_config_misc_year           ) );
		SetDlgItemText(hwnd, static_month,     text(oooo_conv_config_misc_month          ) );
		SetDlgItemText(hwnd, static_date,      text(oooo_conv_config_misc_date           ) );
		SetDlgItemText(hwnd, static_hour,      text(oooo_conv_config_misc_hour           ) );
		SetDlgItemText(hwnd, static_minute,    text(oooo_conv_config_misc_minute         ) );
		SetDlgItemText(hwnd, IDC_ORIG,         text(oooo_conv_config_misc_use_original   ) );
		SetDlgItemText(hwnd, IDC_CURRENT,      text(oooo_conv_config_misc_use_current    ) );
		SetDlgItemText(hwnd, IDC_CUSTOM,       text(oooo_conv_config_misc_use_custom     ) );

		SetDlgItemText(hwnd, IDOK,              text(oooo_ok                         ) );
		SetDlgItemText(hwnd, IDCANCEL,          text(oooo_cancel                     ) );

		/* </lang> */


		if(conv_misc_type == misc_type_orig)
		{
			SendDlgItemMessage(hwnd, IDC_ORIG   , BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_CURRENT, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_CUSTOM , BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if(conv_misc_type == misc_type_current)
		{
			SendDlgItemMessage(hwnd, IDC_ORIG   , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_CURRENT, BM_SETCHECK, BST_CHECKED  , 0);
			SendDlgItemMessage(hwnd, IDC_CUSTOM , BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if(conv_misc_type == misc_type_user)
		{
			SendDlgItemMessage(hwnd, IDC_ORIG   , BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_CURRENT, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_CUSTOM , BM_SETCHECK, BST_CHECKED  , 0);
		}

		SetDlgItemInt(hwnd, IDC_YEAR  , conv_misc_year  , 0);
		SetDlgItemInt(hwnd, IDC_MONTH , conv_misc_month , 0);
		SetDlgItemInt(hwnd, IDC_DATE  , conv_misc_date  , 0);
		SetDlgItemInt(hwnd, IDC_HOUR  , conv_misc_hour  , 0);
		SetDlgItemInt(hwnd, IDC_MINUTE, conv_misc_minute, 0);
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}

	return 0;
}

void Conv_SetEnable(HWND hwnd, int enb)
{
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_LIST), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_SEL_ADD), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_SEL_ALL), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_SEL_NONE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_SEL_REMOVE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_EQ)  , enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_TAG) , enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_VOL) , enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_MISC), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILENAME), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILENAME_HELP), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILEPATH), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILEPATH_BROWSE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_ENCODER), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_ENCODER_ABOUT), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_ENCODER_CONFIG), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_BUFFERSIZE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOPPLAYBACK), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOP), enb ? 0 : 1);
}

int ConversionProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			{
				int tr = 0;
				int csel;
				unsigned int bv;


				conv_proc_cancel = 1;

				//while(conv_threadrunning)sys_pass();

				GetDlgItemText(hwnd, IDC_FILENAME, settings.conversion.last_formatting, sizeof(settings.conversion.last_formatting));
				GetDlgItemText(hwnd, IDC_FILEPATH, settings.conversion.last_path, sizeof(settings.conversion.last_path));
				bv = GetDlgItemInt(hwnd, IDC_CONVERSION_BUFFERSIZE, &tr, 0);

				settings.conversion.last_buffer_size = bv;

				csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0);

				if(csel >= 0)
					settings.conversion.last_encoder = csel;
		
				if(!conv_threadrunning)
				{
					encoder_uninitialize();
					goto point_WM_DESTROY;
				}else{
					conv_thread_sclose = hwnd;
				}
			}
			break;

		case IDC_CONVERSION_STOPPLAYBACK:

			if(IsDlgButtonChecked(hwnd, IDC_CONVERSION_STOPPLAYBACK) == BST_CHECKED)
				settings.conversion.stop_playback = 1;
			else
				settings.conversion.stop_playback = 0;

			break;

		case IDC_FILENAME_HELP:
			MessageBox(hwnd, formattinginfo, uni("File Name Syntax"), MB_ICONINFORMATION);
			break;

		case IDC_CONVERSION_SEL_ADD:
			{
				string fname = basewindows_show_open(uni("Add file to convert."), settings.conversion.last_path, 0);
		
				if(fname)
				{
					if(str_len(fname))
					{
						//str_cpy(settings.conversion.last_path, fname);
						Conv_OpenFileList(GetDlgItem(hwnd, IDC_CONVERSION_LIST), fname);
					}
				}
			}
			break;

		case IDC_CONVERSION_SEL_REMOVE:
			{
				unsigned long i, c = (unsigned long)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);

				for(i=0; i<c; i++)
				{
					if(SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))
					{
						SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_DELETESTRING, i, 0);
						i--; c--;
					}
				}
			}
			break;

		case IDC_CONVERSION_SEL_ALL:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SELITEMRANGE, 1, MAKELPARAM(0, SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0) - 1));	
			break;

		case IDC_CONVERSION_SEL_NONE:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SELITEMRANGE, 0, MAKELPARAM(0, SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0) - 1));	
			break;

		case IDC_FILEPATH_BROWSE:
			{
				letter       fpath[v_sys_maxpath];
				BROWSEINFO   bi;
				LPITEMIDLIST lpi;

				fpath[0] = 0;

				bi.hwndOwner      = hwnd;
				bi.lpszTitle      = uni("Save converted files to?.");
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
					SendDlgItemMessage(hwnd, IDC_FILEPATH, WM_SETTEXT, 0, (LPARAM)fpath);
				}
			}
			break;

		case IDC_CONVERSION_EQ:
			DialogBox(instance_fennec, (LPCTSTR)IDD_EQUALIZER, hwnd, (DLGPROC)Conv_EqualizerProc);
			break;

		case IDC_CONVERSION_VOL:
			DialogBox(instance_fennec, (LPCTSTR)IDD_VOLUME, hwnd, (DLGPROC)Conv_VolumeProc);
			break;

		case IDC_CONVERSION_TAG:
			DialogBox(instance_fennec, (LPCTSTR)IDD_TAGGING, hwnd, (DLGPROC)Conv_TaggingProc);
			break;

		case IDC_CONVERSION_MISC:
			DialogBox(instance_fennec, (LPCTSTR)IDD_MISC, hwnd, (DLGPROC)Conv_MiscProc);
			break;

		case IDC_SHOWHELP:
			{
				letter fpath[v_sys_maxpath];

				str_cpy(fpath, fennec_get_path(0, 0));
				str_cat(fpath, uni("/Help/Conversion Dialog.rtf"));

				ShellExecute(hwnd, 0, fpath, 0, 0, SW_SHOW);
			}
			break;

		case IDC_CONVERSION_ENCODER:
			if(HIWORD(wParam) == CBN_SELENDOK)
			{
				unsigned long encnew;

				encnew = (unsigned long)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0);

				if(encnew == conv_selected_enc)break;
				if(conv_selected_enc != -1)encoder_plugin_uninitialize(conv_selected_enc);

				conv_selected_enc = encnew;
				encoder_plugin_initialize(conv_selected_enc);
			}
			break;

		case IDC_CONVERSION_ENCODER_ABOUT:
			encoder_plugin_global_about((unsigned int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0), hwnd);
			break;

		case IDC_CONVERSION_ENCODER_CONFIG:
			encoder_plugin_global_file_settings((unsigned int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0), hwnd);
			break;

		case IDC_CONVERSION_STOP:
			conv_proc_cancel = 1;
			Conv_SetEnable(hwnd, 1);
			SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_start) );
			break;

		case IDC_CONVERSION_START:
			if(conv_threadrunning)
			{
				conv_proc_pause ^= 1;
				if(conv_proc_pause)
				{
					SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_resume) );
				}else{
					SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_pause));
				}

			}else{
				HANDLE ht;
				unsigned long  tid = 0;

				hwnd_current = hwnd;

				conv_proc_cancel = 0;

				conv_threadrunning = 1;

				if(settings.conversion.stop_playback)audio_stop();

				Conv_SetEnable(hwnd, 0);

				ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)conv_thread, 0, 0, &tid);
				SetThreadPriority(ht, setting_priority_to_sys(settings.general.threads_priority));

				if(ht == INVALID_HANDLE_VALUE)conv_thread(0);
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
			int              lcount;
			
			fcount = DragQueryFile(hd, 0xFFFFFFFF, 0, 0);

			for(i=0; i<fcount; i++)
			{
				memset(fpath, 0, sizeof(fpath));
				DragQueryFile(hd, i, fpath, sizeof(fpath));

				hf = FindFirstFile(fpath, &fd);

				if(hf == INVALID_HANDLE_VALUE)continue;
				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)continue;
				
				FindClose(hf);

				/* add to playlist */
				SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_ADDSTRING, 0, (LPARAM)fpath);
				lcount = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);
				SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SETSEL, 1, lcount - 1);
			}

			DragFinish(hd);
		}
		break;

	case WM_INITDIALOG:

		window_conversion = hwnd;
		
		/* <lang> */


		SetWindowText(hwnd, text(oooo_conversion) );

		SetDlgItemText(hwnd, static_inputfiles ,             text(oooo_conversion_inputfiles        ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_EQ,              text(oooo_conversion_equalizer			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_VOL,             text(oooo_conversion_volume			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_TAG,             text(oooo_conversion_tagging			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_MISC,            text(oooo_conversion_misc				) );
		SetDlgItemText(hwnd, group_settings,                 text(oooo_conversion_settings			) );
		SetDlgItemText(hwnd, static_buffersize,              text(oooo_conversion_buffer_size		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_STOPPLAYBACK,    text(oooo_conversion_stop_playback		) );
		SetDlgItemText(hwnd, group_enc_selection,            text(oooo_conversion_encoder_selection	) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ENCODER_CONFIG,  text(oooo_conversion_enc_configure		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ENCODER_ABOUT,   text(oooo_conversion_enc_about			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_START,           text(oooo_conversion_start				) );
		SetDlgItemText(hwnd, IDC_CONVERSION_STOP,            text(oooo_conversion_stop				) );
		SetDlgItemText(hwnd, static_location,                text(oooo_conversion_output_location	) );
		SetDlgItemText(hwnd, static_filenaming,              text(oooo_conversion_file_naming		) );
		SetDlgItemText(hwnd, static_currentfile,             text(oooo_conversion_current_file		) );
		SetDlgItemText(hwnd, static_convprocess,             text(oooo_conversion_conv_process		) );

		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_ADD,         text(oooo_add	        ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_REMOVE,      text(oooo_remove		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_ALL,         text(oooo_select_all	) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_NONE,        text(oooo_select_none	) );
		SetDlgItemText(hwnd, IDCANCEL,                       text(oooo_cancel	    ) );
		SetDlgItemText(hwnd, IDC_SHOWHELP,                   text(oooo_help		    ) );

		/* </lang> */

		conv_tag_title[0]    = 0;
		conv_tag_artist[0]   = 0;
		conv_tag_album[0]    = 0;
		conv_tag_year[0]     = 0;
		conv_tag_genre[0]    = 0;
		conv_tag_comments[0] = 0;

		conv_tag_def_title    = 1;
		conv_tag_def_artist   = 1;
		conv_tag_def_album    = 1;
		conv_tag_def_year     = 1;
		conv_tag_def_genre    = 1;
		conv_tag_def_comments = 1;
		conv_copytags         = 1;

		conv_selected_enc     = (unsigned long)-1;

		settings.player.playlist_shuffle = 1;

		conv_hwnd = hwnd;

		EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOP), 0);

		SetDlgItemText(hwnd, IDC_FILENAME, settings.conversion.last_formatting);
		SetDlgItemText(hwnd, IDC_FILEPATH, settings.conversion.last_path);
		SetDlgItemInt(hwnd, IDC_CONVERSION_BUFFERSIZE, settings.conversion.last_buffer_size, 0);

		CheckDlgButton(hwnd, IDC_CONVERSION_STOPPLAYBACK, settings.conversion.stop_playback ? BST_CHECKED : BST_UNCHECKED);
		
		encoder_initialize();
		{
			unsigned int i = 0;
			string       ename;

			ename = encoder_getname(i);
			while(ename)
			{
				SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)ename);
				i++;
				ename = encoder_getname(i);
			}

			if(settings.conversion.last_encoder < i)
			{
				SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_SETCURSEL, (WPARAM) settings.conversion.last_encoder, 0);
				conv_selected_enc = settings.conversion.last_encoder;
				encoder_plugin_initialize(conv_selected_enc);
			}
		}

		SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));

		SetTimer(hwnd, 400101, 30, 0);
		break;

	case WM_TIMER:
		if(conv_threadrunning)
		{
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETPOS, (WPARAM)conv_pos, 0);
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PRG, PBM_SETPOS, (WPARAM)(((ct_ci - 1) * 1000) + conv_pos), 0);
		}else{
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETPOS, 0, 0);
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PRG, PBM_SETPOS, 0, 0);
		}
		break;

	case WM_DESTROY:

point_WM_DESTROY:
		conversion_window_open = 0;
		KillTimer(hwnd, 400101);
		EndDialog(hwnd, 0);

		window_conversion = 0;
		break;
	}

	return 0;
}

void setposmsg(double mp)
{
	conv_pos =  (unsigned int)(mp * 1000.0f);
}

int makepathco(string pstr)
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

int makepath(const string pstr)
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

unsigned long WINAPI conv_thread( LPVOID lpParam ) 
{ 
	unsigned long ri, i = 0, c, rc = 0;
	letter  fpath[v_sys_maxpath];
	letter  mpath[v_sys_maxpath];
	letter  dtext[1024];
	letter  buf[250];
	unsigned int bsize;
	unsigned long ipid;
	struct fennec_audiotag at;
	SYSTEMTIME st;
	FILETIME   ft;
	int  tr = 0;

	conv_threadrunning = 1;

	Sleep(1);

	SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Pause."));

	c = (unsigned long)SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);
	for(i=0; i<c; i++)
	{
		if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))rc++;
	}

	if(!rc)MessageBox(0, uni("Select files first."), uni("No Files Selected"), MB_ICONINFORMATION);

	ct_rc = rc;

	if(!c || conv_selected_enc == -1)
	{
		if(conv_selected_enc == -1)
		{
			MessageBox(0, uni("Select an encoder first."), uni("No Encoder Selected"), MB_ICONINFORMATION);
		}

		SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
		conv_threadrunning = 0;
		return 0; 
	}

	SendDlgItemMessage(hwnd_current, IDC_CONVERSION_PRG, PBM_SETRANGE32, (WPARAM)(int)0, (LPARAM)(int)(rc * 1000));

	ri = 0;

	for(i=0; i<c; i++)
	{
		if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))
		{
			if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETTEXT, i, (LPARAM)fpath) == LB_ERR)
			{
				continue;
			}

			ri++;
			ct_ci = ri;

			/* conversion */

			if(conv_proc_cancel)
			{
				conv_threadrunning = 0;
				return 0;
			}

			GetDlgItemText(hwnd_current, IDC_FILEPATH, mpath, sizeof(mpath));
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

			bsize = GetDlgItemInt(hwnd_current, IDC_CONVERSION_BUFFERSIZE, &tr, 0);

			if(!makepathco(mpath))
			{
				MessageBox(0, uni("Invalid path"), uni("Output Directory Path"), MB_ICONINFORMATION);
				SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
				conv_threadrunning = 0;
				return 0; 
			}

			str_cpy(dtext, uni("Converting \""));
			str_cat(dtext, fpath);
			str_cat(dtext, uni("\""));

			SetDlgItemText(hwnd_current, IDC_DETAILS, dtext);

			if(!tr || bsize < 64 || bsize > 25600)
			{
				bsize = 64;
			}

			bsize *= 1024;

			memset(buf, 0, sizeof(buf));

			ipid = audio_input_tagread(fpath, &at);

			GetDlgItemText(hwnd_current, IDC_FILENAME, buf, sizeof(buf));

			tags_translate(buf, &at, fpath);
			
			{
				unsigned int bi = 0;

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
			}
			
			
			if(!str_len(buf))str_cpy(buf, uni("Unknown"));

			str_cat(mpath, buf);

			audioconvert_convertfile(fpath, mpath, conv_selected_enc, bsize, &conv_proc_cancel, &conv_proc_pause, (audioconvert_file_pfunc) setposmsg);

			SendDlgItemMessage(hwnd_current, IDC_CONVERSION_PRG, PBM_SETPOS, (WPARAM)ri * 1000, 0);

			if(conv_tag_def_title    == 1 &&
			   conv_tag_def_artist   == 1 &&
			   conv_tag_def_album    == 1 &&
			   conv_tag_def_year     == 1 &&
			   conv_tag_def_genre    == 1 &&
			   conv_tag_def_comments == 1 &&
			   conv_copytags         == 1)
			{
				struct fennec_audiotag_item  ti;

				memcpy(&ti, &at.tag_encodedby, sizeof(struct fennec_audiotag_item));

				at.tag_encodedby.tdata = fennec_u_player_version_text;
				at.tag_encodedby.tsize = (int)str_size(fennec_u_player_version_text);
				at.tag_encodedby.tmode = tag_memmode_static;

				audio_input_tagwrite(mpath, &at);

				memcpy(&at.tag_encodedby, &ti, sizeof(struct fennec_audiotag_item));

			}else if(conv_copytags == 1){
				
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
				if(!conv_tag_def_title)
				{
					bkp_title_size = at.tag_title.tsize;
					bkp_title_data = at.tag_title.tdata;
					at.tag_title.tsize = (unsigned int)str_len(conv_tag_title) + sizeof(letter);
					at.tag_title.tdata = conv_tag_title;
				}

				/* album */
				if(!conv_tag_def_album)
				{
					bkp_album_size = at.tag_album.tsize;
					bkp_album_data = at.tag_album.tdata;
					at.tag_album.tsize = (unsigned int)str_len(conv_tag_album) + sizeof(letter);
					at.tag_album.tdata = conv_tag_album;
				}

				/* artist */
				if(!conv_tag_def_artist)
				{
					bkp_artist_size = at.tag_artist.tsize;
					bkp_artist_data = at.tag_artist.tdata;
					at.tag_artist.tsize = (unsigned int)str_len(conv_tag_artist) + sizeof(letter);
					at.tag_artist.tdata = conv_tag_artist;
				}

				/* year */
				if(!conv_tag_def_year)
				{
					bkp_year_size = at.tag_year.tsize;
					bkp_year_data = at.tag_year.tdata;
					bkp_year_idata = (char)at.tag_year.tdatai;
					at.tag_year.tsize = (unsigned int)str_len(conv_tag_year) + sizeof(letter);
					at.tag_year.tdata = conv_tag_year;
					at.tag_genre.tdatai = 0;
				}

				/* comments */
				if(!conv_tag_def_comments)
				{
					bkp_comments_size = at.tag_comments.tsize;
					bkp_comments_data = at.tag_comments.tdata;
					at.tag_comments.tsize = (unsigned int)str_len(conv_tag_comments) + sizeof(letter);
					at.tag_comments.tdata = conv_tag_comments;
				}

				/* genre */
				if(!conv_tag_def_genre)
				{
					bkp_genre_size = at.tag_genre.tsize;
					bkp_genre_data = at.tag_genre.tdata;
					bkp_genre_idata = at.tag_genre.tdatai;
					at.tag_genre.tsize = (unsigned int)str_len(conv_tag_genre) + sizeof(letter);
					at.tag_genre.tdata = conv_tag_genre;
					at.tag_genre.tdatai = 0;
				}

				audio_input_tagwrite(mpath, &at);

				/* title */
				if(!conv_tag_def_title)
				{
					at.tag_title.tsize = bkp_title_size;
					at.tag_title.tdata = bkp_title_data;
				}

				/* album */
				if(!conv_tag_def_album)
				{
					at.tag_album.tsize = bkp_album_size;
					at.tag_album.tdata = bkp_album_data;
				}

				/* artist */
				if(!conv_tag_def_artist)
				{
					at.tag_artist.tsize = bkp_artist_size;
					at.tag_artist.tdata = bkp_artist_data;
				}

				/* year */
				if(!conv_tag_def_year)
				{
					at.tag_year.tsize  = bkp_year_size ;
					at.tag_year.tdata  = bkp_year_data ;
					at.tag_year.tdatai = bkp_year_idata;
				}

				/* comments */
				if(!conv_tag_def_comments)
				{
					at.tag_comments.tsize = bkp_comments_size;
					at.tag_comments.tdata = bkp_comments_data;
				}

				/* genre */
				if(!conv_tag_def_genre)
				{
					at.tag_genre.tsize  = bkp_genre_size;
					at.tag_genre.tdata  = bkp_genre_data;
					at.tag_genre.tdatai = bkp_genre_idata;
				}

			}

			audio_input_tagread_known(ipid, 0, &at); /* free */

			/* set time */

			
			if(conv_misc_type == misc_type_orig)
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

			}else if(conv_misc_type == misc_type_user){
				HANDLE hfile;

				GetSystemTime(&st);

				if(conv_misc_year   >= 1601 && conv_misc_year   <= 30827) st.wYear   = (WORD)conv_misc_year;
				if(conv_misc_month  >= 1    && conv_misc_month  <= 12)    st.wMonth  = (WORD)conv_misc_month;
				if(conv_misc_date   >= 1    && conv_misc_date   <= 31)    st.wDay    = (WORD)conv_misc_date;
				if(conv_misc_hour   >= 0    && conv_misc_hour   <= 24)    st.wHour   = (WORD)conv_misc_hour;
				if(conv_misc_minute >= 1    && conv_misc_minute <= 60)    st.wMinute = (WORD)conv_misc_minute;

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

			SetDlgItemText(hwnd_current, IDC_DETAILS, uni("Finished."));
			/* end of conversion */
			
		}
	}

	SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
	conv_threadrunning = 0;

	if(conv_thread_sclose)
	{
		//encoder_uninitialize();
		SendMessage(conv_thread_sclose, WM_DESTROY, 0, 0);
		conv_thread_sclose = 0;
	}else{

		Conv_SetEnable(conv_hwnd, 1);
	}
    return 0; 
} 

unsigned long WINAPI rip_thread( LPVOID lpParam ) 
{ 
	unsigned long            ri, i = 0, c, rc = 0;
	letter                   fpath[v_sys_maxpath];
	letter                   mpath[v_sys_maxpath];
	letter                   dtext[1024];
	letter                   buf[250];
	unsigned int             bsize;
	struct fennec_audiotag   at;
	SYSTEMTIME               st;
	FILETIME                 ft;
	int                      tr = 0;
	letter                   mbuf[32];

	conv_threadrunning = 1;

	Sleep(1);

	SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Pause."));

	c = (unsigned long)SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);
	for(i=0; i<c; i++)
	{
		if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))rc++;
	}

	if(!rc)MessageBox(0, uni("Select files first."), uni("No Files Selected"), MB_ICONINFORMATION);

	ct_rc = rc;

	if(!c || conv_selected_enc == -1)
	{
		if(conv_selected_enc == -1)
		{
			MessageBox(0, uni("Select an encoder first."), uni("No Encoder Selected"), MB_ICONINFORMATION);
		}

		SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
		conv_threadrunning = 0;
		return 0; 
	}

	SendDlgItemMessage(hwnd_current, IDC_CONVERSION_PRG, PBM_SETRANGE32, (WPARAM)(int)0, (LPARAM)(int)(rc * 1000));

	ri = 0;

	for(i=0; i<c; i++)
	{
		if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))
		{
			if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETTEXT, i, (LPARAM)fpath + 3) == LB_ERR)
			{
				continue;
			}
			
			memset(mbuf, 0, sizeof(mbuf));
			fpath[0] = (char)seldrive_driveid;
			fpath[1] = uni(':');
			fpath[2] = uni('\\');
			str_cpy(fpath + 3, uni("Track"));
			str_itos(i + 1, mbuf, 10);
			str_cat(fpath, mbuf);
			str_cat(fpath, uni(".cda"));

			ri++;
			ct_ci = ri;

			/* conversion */

			if(conv_proc_cancel)
			{
				conv_threadrunning = 0;
				return 0;
			}

			GetDlgItemText(hwnd_current, IDC_FILEPATH, mpath, sizeof(mpath));
			
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

			bsize = GetDlgItemInt(hwnd_current, IDC_CONVERSION_BUFFERSIZE, &tr, 0);

			if(!makepathco(mpath))
			{
				MessageBox(0, uni("Invalid path"), uni("Output Directory Path"), MB_ICONINFORMATION);
				SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
				conv_threadrunning = 0;
				return 0; 
			}

			str_cpy(dtext, uni("Converting \""));
			str_cat(dtext, fpath);
			str_cat(dtext, uni("\""));

			SetDlgItemText(hwnd_current, IDC_DETAILS, dtext);

			if(!tr || bsize < 64 || bsize > 25600)
			{
				bsize = 64;
			}

			bsize *= 1024;

			memset(buf, 0, sizeof(buf));

			{
				letter buf[16];
				

				at.tag_title.tsize         = 0;
				at.tag_album.tsize         = 0;
				at.tag_artist.tsize        = 0;
				at.tag_origartist.tsize    = 0;
				at.tag_composer.tsize      = 0;
				at.tag_lyricist.tsize      = 0;
				at.tag_band.tsize          = 0;
				at.tag_copyright.tsize     = 0;
				at.tag_publish.tsize       = 0;
				at.tag_encodedby.tsize     = 0;
				at.tag_genre.tsize         = 0;
				at.tag_year.tsize          = 0;
				at.tag_url.tsize           = 0;
				at.tag_offiartisturl.tsize = 0;
				at.tag_filepath.tsize      = 0;
				at.tag_filename.tsize      = 0;
				at.tag_comments.tsize      = 0;
				at.tag_lyric.tsize         = 0;
				at.tag_bpm.tsize           = 0;
				at.tag_tracknum.tsize      = 0;

				memset(buf, 0, sizeof(buf));
				str_itos(i + 1, buf, 10);
				str_cpy(mbuf, buf);
				memset(buf, 0, sizeof(buf));
				str_itos(c, buf, 10);
				str_cat(mbuf, uni("/"));
				str_cat(mbuf, buf);

				at.tag_title.tdata = rip_tags[i].title;
				at.tag_title.tsize = (unsigned int)str_len(rip_tags[i].title);
				at.tag_album.tdata = rip_tags[i].album;
				at.tag_album.tsize = (unsigned int)str_len(rip_tags[i].album);
				at.tag_artist.tdata = rip_tags[i].artist;
				at.tag_artist.tsize = (unsigned int)str_len(rip_tags[i].artist);
				at.tag_genre.tdata = rip_tags[i].genre;
				at.tag_genre.tsize = (unsigned int)str_len(rip_tags[i].genre);
				at.tag_comments.tdata = rip_tags[i].comments;
				at.tag_comments.tsize = (unsigned int)str_len(rip_tags[i].comments);
				at.tag_tracknum.tdata = mbuf;
				at.tag_tracknum.tsize = (unsigned int)str_len(mbuf);

				at.tag_encodedby.tdata = fennec_u_player_version_text;
				at.tag_encodedby.tsize = (int)str_size(fennec_u_player_version_text);
				at.tag_encodedby.tmode = tag_memmode_static;

				//audio_input_tagwrite(mpath, &at);
			}
			//ipid = audio_input_tagread(fpath, &at);

			GetDlgItemText(hwnd_current, IDC_FILENAME, buf, sizeof(buf));

			tags_translate(buf, &at, fpath);
			
			{
				unsigned int bi = 0;

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
			}
			
			
			if(!str_len(buf))str_cpy(buf, uni("Unknown"));

			str_cat(mpath, buf);

			audioconvert_convertfile(fpath, mpath, conv_selected_enc, bsize, &conv_proc_cancel, &conv_proc_pause, (audioconvert_file_pfunc) setposmsg);

			SendDlgItemMessage(hwnd_current, IDC_CONVERSION_PRG, PBM_SETPOS, (WPARAM)ri * 1000, 0);

			audio_input_tagwrite(mpath, &at);

			/* set time */

			
			if(conv_misc_type == misc_type_orig)
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

			}else if(conv_misc_type == misc_type_user){
				HANDLE hfile;

				GetSystemTime(&st);

				if(conv_misc_year   >= 1601 && conv_misc_year   <= 30827) st.wYear   = (WORD)conv_misc_year;
				if(conv_misc_month  >= 1    && conv_misc_month  <= 12)    st.wMonth  = (WORD)conv_misc_month;
				if(conv_misc_date   >= 1    && conv_misc_date   <= 31)    st.wDay    = (WORD)conv_misc_date;
				if(conv_misc_hour   >= 0    && conv_misc_hour   <= 24)    st.wHour   = (WORD)conv_misc_hour;
				if(conv_misc_minute >= 1    && conv_misc_minute <= 60)    st.wMinute = (WORD)conv_misc_minute;

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

			SetDlgItemText(hwnd_current, IDC_DETAILS, uni("Finished."));
			/* end of conversion */
			
		}
	}

	SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
	conv_threadrunning = 0;

	if(conv_thread_sclose)
	{
		encoder_uninitialize();
		SendMessage(conv_thread_sclose, WM_DESTROY, 0, 0);
		conv_thread_sclose = 0;
	}else{

		Conv_SetEnable(conv_hwnd, 1);
	}
    return 0; 
} 

int seldrive_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					seldrive_driveid = tmpbuf[0];
				}else{
					seldrive_driveid = 0;
				}
			}
			seldrive_msgok = 1;
			EndDialog(hwnd, 1);
			break;

		case IDCANCEL:
			seldrive_driveid = 0;
			seldrive_msgok = 1;
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:

		/* <lang> */

		SetWindowText(hwnd, text(oooo_ripping_select_drive) );

		SetDlgItemText(hwnd, static_drive,  text(oooo_ripping_drive ) );
		SetDlgItemText(hwnd, IDOK,          text(oooo_ok     ) );
		SetDlgItemText(hwnd, IDCANCEL,      text(oooo_cancel ) );

		/* </lang> */


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

			if(drname[0] == '1')
			{
				MessageBox(hwnd, uni("No CD drives found"), uni("Error"), MB_ICONEXCLAMATION);
				seldrive_driveid = 0;
				seldrive_msgok = 1;
				EndDialog(hwnd, 0);
			}
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}

int Rip_ShowSelDrive()
{
	int sbt;
	seldrive_msgok = 0;
	sbt = (int)DialogBox(instance_fennec, MAKEINTRESOURCE(IDD_SELDRIVE), window_main, (DLGPROC)seldrive_proc);
	while(!seldrive_msgok)sys_pass();

	return sbt;
}

void Rip_SetEnable(HWND hwnd, int enb)
{
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_LIST), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_SEL_NONE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_SEL_ALL), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_EQ)  , enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_VOL) , enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_MISC), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILENAME), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILENAME_HELP), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILEPATH), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_FILEPATH_BROWSE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_ENCODER), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_ABOUT), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_ENCODER_CONFIG), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_BUFFERSIZE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOPPLAYBACK), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_TAG_TITLE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_TAG_ARTIST), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_TAG_ALBUM), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_TAG_GENRE), enb ? 1 : 0);
	EnableWindow(GetDlgItem(hwnd, IDC_TAG_COMMENTS), enb ? 1 : 0);
	//EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_START), enb ? 0 : 1);
}

int RippingProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CONVERSION_LIST: 
            switch(HIWORD(wParam)) 
            {
			case LBN_SELCHANGE: 
				{
					letter  buf2[10];
					letter  buf[32];
					int csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
					
					if(csel == -1)break;
					if(!rip_tags)break;

					str_cpy(buf, uni("Tag: Track "));
					memset(buf2, 0, sizeof(buf2));
					str_itos(csel + 1, buf2, 10);
					str_cat(buf, buf2);

					SetDlgItemText(hwnd, IDC_TAGS_FRAME  , buf);
					SetDlgItemText(hwnd, IDC_TAG_TITLE   , rip_tags[csel].title);
					SetDlgItemText(hwnd, IDC_TAG_ALBUM   , rip_tags[csel].album);
					SetDlgItemText(hwnd, IDC_TAG_ARTIST  , rip_tags[csel].artist);
					SetDlgItemText(hwnd, IDC_TAG_GENRE   , rip_tags[csel].genre);
					SetDlgItemText(hwnd, IDC_TAG_COMMENTS, rip_tags[csel].comments);
				}
				break;
			}
			break;

		case IDC_APP_TITLE:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				for(i=0; i<rip_tracks; i++)
				{
					if(i == csel)continue;
					str_cpy(rip_tags[i].title, rip_tags[csel].title);
				}
			}
			break;

		case IDC_APP_ALBUM:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				for(i=0; i<rip_tracks; i++)
				{
					if(i == csel)continue;
					str_cpy(rip_tags[i].album, rip_tags[csel].album);
				}
			}
			break;

		case IDC_APP_ARTIST:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				for(i=0; i<rip_tracks; i++)
				{
					if(i == csel)continue;
					str_cpy(rip_tags[i].artist, rip_tags[csel].artist);
				}
			}
			break;

		case IDC_APP_GENRE:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				for(i=0; i<rip_tracks; i++)
				{
					if(i == csel)continue;
					str_cpy(rip_tags[i].genre, rip_tags[csel].genre);
				}
			}
			break;

		case IDC_APP_COMMENTS:
			{
				int i, csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				for(i=0; i<rip_tracks; i++)
				{
					if(i == csel)continue;
					str_cpy(rip_tags[i].comments, rip_tags[csel].comments);
				}
			}
			break;

		case IDC_TAG_TITLE:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				int csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				GetDlgItemText(hwnd, IDC_TAG_TITLE, rip_tags[csel].title, 256);
			}
			break;

		case IDC_TAG_ALBUM:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				int csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				GetDlgItemText(hwnd, IDC_TAG_ALBUM, rip_tags[csel].album, 256);
			}
			break;

		case IDC_TAG_ARTIST:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				int csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				GetDlgItemText(hwnd, IDC_TAG_ARTIST, rip_tags[csel].artist, 256);
			}
			break;

		case IDC_TAG_GENRE:
			if(HIWORD(wParam) == CBN_EDITCHANGE)
			{
				int csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				GetDlgItemText(hwnd, IDC_TAG_GENRE, rip_tags[csel].genre, 256);
			}
			break;

		case IDC_TAG_COMMENTS:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				int csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);
				if(csel == -1)break;
				GetDlgItemText(hwnd, IDC_TAG_COMMENTS, rip_tags[csel].comments, 256);
			}
			break;

		case IDOK:
		case IDCANCEL:
			{
				int tr = 0;
				int csel;
				unsigned int bv;

				conv_proc_cancel = 1;

				//while(conv_threadrunning)sys_pass();

				GetDlgItemText(hwnd, IDC_FILENAME, settings.ripping.last_formatting, sizeof(settings.ripping.last_formatting));
				GetDlgItemText(hwnd, IDC_FILEPATH, settings.ripping.last_path, sizeof(settings.ripping.last_path));
				bv = GetDlgItemInt(hwnd, IDC_CONVERSION_BUFFERSIZE, &tr, 0);

				settings.ripping.last_buffer_size = bv;
	
				csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0);

				if(csel >= 0)
					settings.ripping.last_encoder = csel;
		
				if(!conv_threadrunning)
				{
					encoder_uninitialize();
					goto point_WM_DESTROY;
				}else{
					conv_thread_sclose = hwnd;
				}
			}
			break;

		
		case IDC_CONVERSION_STOPPLAYBACK:

			if(IsDlgButtonChecked(hwnd, IDC_CONVERSION_STOPPLAYBACK) == BST_CHECKED)
				settings.ripping.stop_playback = 1;
			else
				settings.ripping.stop_playback = 0;

			break;

		case IDC_FILENAME_HELP:
			MessageBox(hwnd, formattinginfo, uni("File Name Syntax"), MB_ICONINFORMATION);
			break;

		case IDC_CONVERSION_SEL_ADD:
			{
				string fname = basewindows_show_open(uni("Add file to convert."), settings.ripping.last_path, 0);
				if(fname)
				{
					if(str_len(fname))
					{
						// str_cpy(settings.ripping.last_path, fname); /* last_load? */
						Conv_OpenFileList(GetDlgItem(hwnd, IDC_CONVERSION_LIST), fname);
					}
				}
			}
			break;

		case IDC_CONVERSION_SEL_REMOVE:
			{
				unsigned long i, c = (unsigned long)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);

				for(i=0; i<c; i++)
				{
					if(SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))
					{
						SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_DELETESTRING, i, 0);
						i--; c--;
					}
				}
			}
			break;

		case IDC_CONVERSION_SEL_ALL:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SELITEMRANGE, 1, MAKELPARAM(0, SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0) - 1));	
			break;

		case IDC_CONVERSION_SEL_NONE:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SELITEMRANGE, 0, MAKELPARAM(0, SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0) - 1));	
			break;

		case IDC_FILEPATH_BROWSE:
			{
				letter        fpath[v_sys_maxpath];
				BROWSEINFO    bi;
				LPITEMIDLIST  lpi;

				fpath[0] = 0;

				bi.hwndOwner      = hwnd;
				bi.lpszTitle      = uni("Save converted files to?.");
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
					SendDlgItemMessage(hwnd, IDC_FILEPATH, WM_SETTEXT, 0, (LPARAM)fpath);
				}
			}
			break;

		case IDC_CONVERSION_EQ:
			DialogBox(instance_fennec, (LPCTSTR)IDD_EQUALIZER, hwnd, (DLGPROC)Conv_EqualizerProc);
			break;

		case IDC_CONVERSION_VOL:
			DialogBox(instance_fennec, (LPCTSTR)IDD_VOLUME, hwnd, (DLGPROC)Conv_VolumeProc);
			break;

		case IDC_CONVERSION_TAG:
			DialogBox(instance_fennec, (LPCTSTR)IDD_TAGGING, hwnd, (DLGPROC)Conv_TaggingProc);
			break;

		case IDC_CONVERSION_MISC:
			DialogBox(instance_fennec, (LPCTSTR)IDD_MISC, hwnd, (DLGPROC)Conv_MiscProc);
			break;

		case IDC_SHOWHELP:
			{
				letter fpath[v_sys_maxpath];

				str_cpy(fpath, fennec_get_path(0, 0));
				str_cat(fpath, uni("/Help/Ripping Dialog.rtf"));

				ShellExecute(hwnd, 0, fpath, 0, 0, SW_SHOW);
			}
			break;

		case IDC_CONVERSION_ENCODER:
			if(HIWORD(wParam) == CBN_SELENDOK)
			{
				unsigned long encnew;

				encnew = (unsigned long)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0);

				if(encnew == conv_selected_enc)break;
				if(conv_selected_enc != -1)encoder_plugin_uninitialize(conv_selected_enc);

				conv_selected_enc = encnew;
				encoder_plugin_initialize(conv_selected_enc);
			}
			break;

		case IDC_CONVERSION_ABOUT:
			encoder_plugin_global_about((unsigned int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0), hwnd);
			break;

		case IDC_CONVERSION_ENCODER_CONFIG:
			encoder_plugin_global_file_settings((unsigned int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0), hwnd);
			break;

		case IDC_CONVERSION_STOP:
			conv_proc_cancel = 1;
			Conv_SetEnable(hwnd, 1);
			SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_start) );
			break;

		case IDC_CONVERSION_START:
			if(conv_threadrunning)
			{
				conv_proc_pause ^= 1;
				if(conv_proc_pause)
				{
					SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_resume) );
				}else{
					SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_pause) );
				}

			}else{
				HANDLE ht;
				unsigned long  tid = 0;

				hwnd_current = hwnd;

				conv_proc_cancel = 0;

				conv_threadrunning = 1;

				if(settings.ripping.stop_playback)audio_stop();

				Rip_SetEnable(hwnd, 0);

				ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)rip_thread, 0, 0, &tid);
				SetThreadPriority(ht, setting_priority_to_sys(settings.general.threads_priority));

				if(ht == INVALID_HANDLE_VALUE)conv_thread(0);
			}
			break;

		}
		break;

	case WM_INITDIALOG:

		window_ripping = hwnd;

		/* <lang> */

		SetWindowText(hwnd, text(oooo_ripping) );

		SetDlgItemText(hwnd, static_inputfiles ,             text(oooo_conversion_inputfiles        ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_EQ,              text(oooo_conversion_equalizer			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_VOL,             text(oooo_conversion_volume			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_TAG,             text(oooo_conversion_tagging			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_MISC,            text(oooo_conversion_misc				) );
		SetDlgItemText(hwnd, group_settings,                 text(oooo_conversion_settings			) );
		SetDlgItemText(hwnd, static_buffersize,              text(oooo_conversion_buffer_size		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_STOPPLAYBACK,    text(oooo_conversion_stop_playback		) );
		SetDlgItemText(hwnd, group_enc_selection,            text(oooo_conversion_encoder_selection	) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ENCODER_CONFIG,  text(oooo_conversion_enc_configure		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ABOUT,           text(oooo_conversion_enc_about			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_START,           text(oooo_conversion_start				) );
		SetDlgItemText(hwnd, IDC_CONVERSION_STOP,            text(oooo_conversion_stop				) );
		SetDlgItemText(hwnd, static_location,                text(oooo_conversion_output_location	) );
		SetDlgItemText(hwnd, static_filenaming,              text(oooo_conversion_file_naming		) );
		SetDlgItemText(hwnd, static_currentfile,             text(oooo_conversion_current_track		) );
		SetDlgItemText(hwnd, static_convprocess,             text(oooo_conversion_rip_process		) );

		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_ALL,         text(oooo_select_all	) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_NONE,        text(oooo_select_none	) );
		SetDlgItemText(hwnd, IDCANCEL,                       text(oooo_cancel	    ) );
		SetDlgItemText(hwnd, IDC_SHOWHELP,                   text(oooo_help		    ) );

		SetDlgItemText(hwnd, static_title,                   text(oooo_tagedit_title	) );
		SetDlgItemText(hwnd, static_artist,                  text(oooo_tagedit_artist	) );
		SetDlgItemText(hwnd, static_album,                   text(oooo_tagedit_album    ) );
		SetDlgItemText(hwnd, static_genre,                   text(oooo_tagedit_genre    ) );
		SetDlgItemText(hwnd, static_comments,                text(oooo_tagedit_comments	) );
		SetDlgItemText(hwnd, IDC_TAGS_FRAME,                 text(oooo_ripping_tags	    ) );

		/* </lang> */

		conv_tag_title[0]    = 0;
		conv_tag_artist[0]   = 0;
		conv_tag_album[0]    = 0;
		conv_tag_year[0]     = 0;
		conv_tag_genre[0]    = 0;
		conv_tag_comments[0] = 0;

		conv_tag_def_title    = 1;
		conv_tag_def_artist   = 1;
		conv_tag_def_album    = 1;
		conv_tag_def_year     = 1;
		conv_tag_def_genre    = 1;
		conv_tag_def_comments = 1;
		conv_copytags         = 1;

		conv_selected_enc = (unsigned long)-1;

		if(!Rip_ShowSelDrive())
		{
			EndDialog(hwnd, 0);
			break;
		}

		if(!seldrive_driveid)
		{
			EndDialog(hwnd, 0);
			break;
		}

		settings.player.playlist_shuffle = 1;

		conv_hwnd = hwnd;

		EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOP), 0);

		SetDlgItemText(hwnd, IDC_FILENAME, settings.ripping.last_formatting);
		SetDlgItemText(hwnd, IDC_FILEPATH, settings.ripping.last_path);
		SetDlgItemInt(hwnd, IDC_CONVERSION_BUFFERSIZE, settings.ripping.last_buffer_size, 0);

		CheckDlgButton(hwnd, IDC_CONVERSION_STOPPLAYBACK, settings.ripping.stop_playback ? BST_CHECKED : BST_UNCHECKED);
		
		encoder_initialize();
		{
			unsigned int i = 0;
			string       ename;

			ename = encoder_getname(i);
			while(ename)
			{
				SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)ename);
				i++;
				ename = encoder_getname(i);
			}

			if(settings.ripping.last_encoder < i)
			{
				SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_SETCURSEL, (WPARAM)settings.ripping.last_encoder, 0);
				conv_selected_enc = settings.ripping.last_encoder;
				encoder_plugin_initialize(conv_selected_enc);
			}
		}

		SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
		
		{
			letter buf[32];

			buf[0] = (letter)seldrive_driveid;
			buf[1] = 0;
			str_cat(buf, uni("Track1.cda"));

			rip_libhandle = audio_input_gethandle(buf);
			if(rip_libhandle == (t_sys_library_handle)-1)
			{
				MessageBox(hwnd, uni("Couldn't find any CD audio input/decoder plug-in."), uni("Ripping: Error"), MB_ICONEXCLAMATION);
				EndDialog(hwnd, 0);
			}
		}

		rip_cda_load           = (cd_genfunc) sys_library_getaddress(rip_libhandle, "audiocd_load");
		rip_cda_check          = (cd_genfunc) sys_library_getaddress(rip_libhandle, "audiocd_check");
		rip_cda_teject         = (cd_genfunc) sys_library_getaddress(rip_libhandle, "audiocd_tray_eject");
		rip_cda_tload          = (cd_genfunc) sys_library_getaddress(rip_libhandle, "audiocd_tray_load");
		rip_cda_gettrackscount = (cd_genfunc) sys_library_getaddress(rip_libhandle, "audiocd_gettrackscount");

		SetTimer(hwnd, 400102, 30, 0);
		SetTimer(hwnd, 400103, 200, 0);

		if(rip_cda_load)rip_cda_load((char)seldrive_driveid);
		else break;

		if(rip_cda_gettrackscount)
		{
			letter  iabuf[10];
			letter  buf[32];
			
			int i, c = rip_cda_gettrackscount((char)seldrive_driveid);
			rip_tags = sys_mem_alloc(sizeof(rip_tag) * c);
			rip_tracks = c;

			for(i=0; i<c; i++)
			{
				
				str_cpy(buf, uni("Track "));
				memset(iabuf, 0, sizeof(iabuf));
				str_itos(i + 1, iabuf, 10);
				str_cat(buf, iabuf);
				
				str_cpy(rip_tags[i].title,   buf);
				str_cpy(rip_tags[i].album,   uni("Unknown"));
				str_cpy(rip_tags[i].artist,  uni("Unknown"));
				str_cpy(rip_tags[i].genre,   uni("Unknown"));
				str_cpy(rip_tags[i].comments, uni(""));

				SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)buf);
			}

			
		}

		{
			unsigned int i;
			for(i=0; i<tag_genres_count; i++)
			{
				SendDlgItemMessage(hwnd, IDC_TAG_GENRE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)tag_genres[i]);
			}
		}
		break;

	case WM_TIMER:
		switch(wParam)
		{
		case 400102:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETPOS, (WPARAM)conv_pos, 0);
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PRG, PBM_SETPOS, (WPARAM)(((ct_ci - 1) * 1000) + conv_pos), 0);
			break;

		case 400103:
			if(rip_cda_check)
			{
				if(rip_cda_check((char)seldrive_driveid))
				{
					if(!conv_threadrunning)
					{
						Rip_SetEnable(hwnd, 1);
					
						EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_START), 1);
						EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOP ), 1);
					}
				}else{
					if(!conv_threadrunning)
					{
						Rip_SetEnable(hwnd, 0);
					
						EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_START), 0);
						//EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOP ), 0);
					}
				}
			}else{
				if(!conv_threadrunning)
				{
					Rip_SetEnable(hwnd, 0);
				
					EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_START), 0);
					//EnableWindow(GetDlgItem(hwnd, IDC_CONVERSION_STOP ), 0);
				}
			}
		
			break;
		}
		break;

	case WM_DESTROY:
point_WM_DESTROY:
		if(rip_tags);
			sys_mem_free(rip_tags);
		rip_tags = 0;
		ripping_window_open = 0;
		KillTimer(hwnd, 400102);
		KillTimer(hwnd, 400103);
		EndDialog(hwnd, 0);

		window_ripping = 0;
		break;
	}

	return 0;
}

unsigned long WINAPI joining_thread(LPVOID lpParam)
{ 
	unsigned long  ri, i = 0, c, rc = 0;
	letter         fpath[v_sys_maxpath];
	letter         mpath[v_sys_maxpath];
	letter         dtext[1024];
	unsigned int   bsize;
	int            tr = 0;
	double         spos, epos;
	int            jhandle = 0;

	conv_threadrunning = 1;

	Sleep(1);

	SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Pause."));

	c = (unsigned long)SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);
	for(i=0; i<c; i++)
	{
		if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))rc++;
	}

	if(!rc)MessageBox(0, uni("Select files first."), uni("No Files Selected"), MB_ICONINFORMATION);

	ct_rc = rc;

	if(!c || conv_selected_enc == -1)
	{
		if(conv_selected_enc == -1)
		{
			MessageBox(0, uni("Select an encoder first."), uni("No Encoder Selected"), MB_ICONINFORMATION);
		}

		SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
		conv_threadrunning = 0;
		return 0; 
	}

	SendDlgItemMessage(hwnd_current, IDC_CONVERSION_PRG, PBM_SETRANGE32, (WPARAM)(int)0, (LPARAM)(int)(rc * 1000));

	ri = 0;

	for(i=0; i<c; i++)
	{
		if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))
		{
			if(SendDlgItemMessage(hwnd_current, IDC_CONVERSION_LIST, LB_GETTEXT, i, (LPARAM)fpath) == LB_ERR)
			{
				continue;
			}

			ri++;
			ct_ci = ri;

			/* conversion */

			if(conv_proc_cancel)
			{
				conv_threadrunning = 0;
				return 0;
			}

			GetDlgItemText(hwnd_current, IDC_FILEPATH, mpath, sizeof(mpath));
			bsize = GetDlgItemInt(hwnd_current, IDC_CONVERSION_BUFFERSIZE, &tr, 0);

			/*if(!makepathco(mpath))
			{
				MessageBox(0, "Invalid path", "Output Directory Path", MB_ICONINFORMATION);
				SetDlgItemText(hwnd_current, IDC_CONVERSION_START, "Start.");
				conv_threadrunning = 0;
				return 0; 
			}*/

			str_cpy(dtext, uni("Converting \""));
			str_cat(dtext, fpath);
			str_cat(dtext, uni("\""));

			SetDlgItemText(hwnd_current, IDC_DETAILS, dtext);

			if(!tr || bsize < 64 || bsize > 25600)
			{
				bsize = 64;
			}

			bsize *= 1024;

			/*audioconvert_convertfile(fpath, mpath, conv_selected_enc, bsize, &conv_proc_cancel, &conv_proc_pause, (audioconvert_file_pfunc) setposmsg);*/

			spos = 0.0;
			epos = 1.0; /* get range from an array */

			if(!jhandle)
			{
				jhandle = audiojoining_start(fpath, mpath, conv_selected_enc, bsize, &conv_proc_cancel, &conv_proc_pause, spos, epos, (audioconvert_file_pfunc) setposmsg);
			}else{
				audiojoining_push(jhandle, fpath, spos, epos);
			}

			SendDlgItemMessage(hwnd_current, IDC_CONVERSION_PRG, PBM_SETPOS, (WPARAM)ri * 1000, 0);

			SetDlgItemText(hwnd_current, IDC_DETAILS, uni("Finished."));
			/* end of conversion */
			
		}
	}

	
	if(jhandle)
	{
		audiojoining_end(jhandle);
	}
	

	SetDlgItemText(hwnd_current, IDC_CONVERSION_START, uni("Start."));
	conv_threadrunning = 0;

	if(conv_thread_sclose)
	{
		//encoder_uninitialize();
		SendMessage(conv_thread_sclose, WM_DESTROY, 0, 0);
		conv_thread_sclose = 0;
	}else{

		Conv_SetEnable(conv_hwnd, 1);
	}
    return 0; 
} 

int JoiningProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{	
	
	case WM_HSCROLL:
		if((LOWORD(wParam) == SB_THUMBPOSITION) || (LOWORD(wParam) == SB_ENDSCROLL))
		{
			if((HWND)lParam == GetDlgItem(hwnd, IDC_RANGE_LOWER))
			{
				audio_setposition((double)(SendDlgItemMessage(hwnd, IDC_RANGE_LOWER, TBM_GETPOS, 0, 0)) / 10000.0);
				break;
			}

			if((HWND)lParam == GetDlgItem(hwnd, IDC_RANGE_HIGHER))
			{
				audio_setposition((double)(SendDlgItemMessage(hwnd, IDC_RANGE_HIGHER, TBM_GETPOS, 0, 0)) / 10000.0);
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			{
				int tr = 0;
				int csel;
				unsigned int bv;

				//GetDlgItemText(hwnd, IDC_FILENAME, settings.Joining.LastFormatting, sizeof(settings.Joining.LastFormatting));
				GetDlgItemText(hwnd, IDC_FILEPATH, settings.joining.last_path, sizeof(settings.joining.last_path));

				bv = GetDlgItemInt(hwnd, IDC_CONVERSION_BUFFERSIZE, &tr, 0);

				settings.joining.last_buffer_size = bv;

				if(IsDlgButtonChecked(hwnd, IDC_CONVERSION_STOPPLAYBACK) == BST_CHECKED)
				{
					settings.joining.stop_playback = 1;
				}else{
					settings.joining.stop_playback = 0;
				}

				csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0);
				if(csel >= 0)
					settings.joining.last_encoder = csel;
				
				encoder_uninitialize();
				KillTimer(hwnd, 400101);
				EndDialog(hwnd, 0);

				window_joining = 0;
			}
			break;

		case IDC_CONVERSION_STOPPLAYBACK:

			if(IsDlgButtonChecked(hwnd, IDC_CONVERSION_STOPPLAYBACK) == BST_CHECKED)
				settings.joining.stop_playback = 1;
			else
				settings.joining.stop_playback = 0;

			break;

		case IDC_CONVERSION_SEL_ADD:
			{
				int lcount = (int) SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);
				string fname = basewindows_show_open(uni("Add file to convert."), settings.joining.last_load, 0);
				if(fname)
				{
					if(str_len(fname))
					{
						str_cpy(settings.joining.last_load, fname);
						Conv_OpenFileList(GetDlgItem(hwnd, IDC_CONVERSION_LIST), fname);
					}
				}

				if(lcount < 0)lcount = 1;
				j_data = (join_data*) sys_mem_realloc(j_data, sizeof(join_data) * lcount);
			}
			break;

		case IDC_BROWSE:
			{
				letter        fpath[v_sys_maxpath];
				OPENFILENAME  lofn;

				memset(&lofn, 0, sizeof(lofn));

				fpath[0] = 0;

				lofn.lStructSize     = sizeof(lofn);
				lofn.lpstrTitle      = uni("Save Joined File");
				lofn.hwndOwner       = window_main;
				lofn.lpstrFile       = fpath;
				lofn.nMaxFile        = sizeof(fpath);
				lofn.lpstrFilter     = uni("All Files (*.*)\0*.*");
				lofn.nFilterIndex    = 0;
				lofn.lpstrFileTitle  = 0;
				lofn.nMaxFileTitle   = 0;
				lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
				lofn.hInstance       = instance_fennec;

				GetSaveFileName(&lofn);

				if(str_len(fpath))
					SetDlgItemText(hwnd, IDC_FILEPATH, fpath);

			}
			break;

		case IDC_CONVERSION_SEL_REMOVE:
			{
				unsigned long i, c = (unsigned long)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0);

				for(i=0; i<c; i++)
				{
					if(SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETSEL, i, 0))
					{
						SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_DELETESTRING, i, 0);
						i--; c--;
					}
				}
			}
			break;

		case IDC_CONVERSION_SEL_ALL:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SELITEMRANGE, 1, MAKELPARAM(0, SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0) - 1));	
			break;

		case IDC_CONVERSION_SEL_NONE:
			SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_SELITEMRANGE, 0, MAKELPARAM(0, SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCOUNT, 0, 0) - 1));	
			break;

		case IDC_CONVERSION_EQ:
			DialogBox(instance_fennec, (LPCTSTR)IDD_EQUALIZER, hwnd, (DLGPROC)Conv_EqualizerProc);
			break;

		case IDC_CONVERSION_VOL:
			DialogBox(instance_fennec, (LPCTSTR)IDD_VOLUME, hwnd, (DLGPROC)Conv_VolumeProc);
			break;

		case IDC_SHOWHELP:
			{
				letter  fpath[v_sys_maxpath];

				str_cpy(fpath, fennec_get_path(0, 0));
				str_cat(fpath, uni("/help/link/fennec help - joining.html"));

				ShellExecute(hwnd, 0, fpath, 0, 0, SW_SHOW);
			}
			break;

		case IDC_CONVERSION_APLAY:
			{
				int  csel = (int)SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETCURSEL, 0, 0);

				if(csel >= 0)
				{
					if(csel != join_lastsel)
					{
						letter tbuffer[v_sys_maxpath];

						SendDlgItemMessage(hwnd, IDC_CONVERSION_LIST, LB_GETTEXT, (WPARAM)csel, (LPARAM)tbuffer);
						audio_load(tbuffer);
						join_lastsel = csel;

						audio_setposition((double)(SendDlgItemMessage(hwnd, IDC_RANGE_LOWER, TBM_GETPOS, 0, 0)) / 10000.0);
					}
					audio_play();
				}
			}
			break;

		case IDC_CONVERSION_APAUSE:
			audio_pause();
			break;

		case IDC_CONVERSION_ASTOP:
			audio_stop();
			break;

		case IDC_CONVERSION_ENCODER:
			if(HIWORD(wParam) == CBN_SELENDOK)
			{
				unsigned long encnew;

				encnew = (unsigned long)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0);

				if(encnew == conv_selected_enc)break;
				if(conv_selected_enc != -1)encoder_plugin_uninitialize(conv_selected_enc);

				conv_selected_enc = encnew;
				encoder_plugin_initialize(conv_selected_enc);
			}
			break;

		case IDC_CONVERSION_ENCODER_ABOUT:
			encoder_plugin_global_about((unsigned int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0), hwnd);
			break;

		case IDC_CONVERSION_ENCODER_CONFIG:
			encoder_plugin_global_file_settings((unsigned int)SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_GETCURSEL, 0, 0), hwnd);
			break;

		case IDC_CONVERSION_STOP:
			conv_proc_cancel = 1;
			SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_start) );
			break;

		case IDC_CONVERSION_START:
			if(conv_threadrunning)
			{
				conv_proc_pause ^= 1;
				if(conv_proc_pause)
				{
					SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_resume) );
				}else{
					SetDlgItemText(hwnd, IDC_CONVERSION_START, text(oooo_conversion_pause) );
				}

			}else{
				HANDLE ht;
				unsigned long  tid = 0;

				hwnd_current = hwnd;

				conv_proc_cancel = 0;

				conv_threadrunning = 1;

				ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)joining_thread, 0, 0, &tid);

				if(ht == INVALID_HANDLE_VALUE)joining_thread(0);
				else sys_thread_setpriority(ht, setting_priority_to_sys(settings.general.threads_priority));

			}
			break;

		}
		break;

	case WM_INITDIALOG:

		window_joining = hwnd;

		/* <lang> */


		SetWindowText(hwnd, text(oooo_joining) );

		SetDlgItemText(hwnd, static_inputfiles ,             text(oooo_conversion_inputfiles        ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_EQ,              text(oooo_conversion_equalizer			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_VOL,             text(oooo_conversion_volume			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_TAG,             text(oooo_conversion_tagging			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_MISC,            text(oooo_conversion_misc				) );
		SetDlgItemText(hwnd, group_settings,                 text(oooo_conversion_settings			) );
		SetDlgItemText(hwnd, static_buffersize,              text(oooo_conversion_buffer_size		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_STOPPLAYBACK,    text(oooo_conversion_stop_playback		) );
		SetDlgItemText(hwnd, group_enc_selection,            text(oooo_conversion_encoder_selection	) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ENCODER_CONFIG,  text(oooo_conversion_enc_configure		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ENCODER_ABOUT,   text(oooo_conversion_enc_about			) );
		SetDlgItemText(hwnd, IDC_CONVERSION_START,           text(oooo_conversion_start				) );
		SetDlgItemText(hwnd, IDC_CONVERSION_STOP,            text(oooo_conversion_stop				) );
		SetDlgItemText(hwnd, static_outfile,                 text(oooo_conversion_output_file	    ) );
		SetDlgItemText(hwnd, static_currentfile,             text(oooo_conversion_current_file		) );
		SetDlgItemText(hwnd, static_convprocess,             text(oooo_conversion_join_process		) );

		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_ADD,         text(oooo_add	        ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_REMOVE,      text(oooo_remove		) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_ALL,         text(oooo_select_all	) );
		SetDlgItemText(hwnd, IDC_CONVERSION_SEL_NONE,        text(oooo_select_none	) );
		SetDlgItemText(hwnd, IDCANCEL,                       text(oooo_cancel	    ) );
		SetDlgItemText(hwnd, IDC_SHOWHELP,                   text(oooo_help		    ) );

		SetDlgItemText(hwnd, IDC_CONVERSION_APLAY,           text(oooo_play	 ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_APAUSE,          text(oooo_pause ) );
		SetDlgItemText(hwnd, IDC_CONVERSION_ASTOP,           text(oooo_stop	 ) );

		/* </lang> */


		conv_tag_title[0]    = 0;
		conv_tag_artist[0]   = 0;
		conv_tag_album[0]    = 0;
		conv_tag_year[0]     = 0;
		conv_tag_genre[0]    = 0;
		conv_tag_comments[0] = 0;

		conv_selected_enc     = (unsigned long)-1;

		//if(!j_data)
		//	j_data = (join_data*) sys_mem_alloc(sizeof(join_data));

		join_lastsel = -1;

		SendDlgItemMessage(hwnd, IDC_RANGE_LOWER,  TBM_SETRANGE, 1, MAKELONG(0, 10000));
		SendDlgItemMessage(hwnd, IDC_RANGE_HIGHER, TBM_SETRANGE, 1, MAKELONG(0, 10000));

		SetDlgItemText(hwnd, IDC_FILEPATH, settings.joining.last_path);
		SetDlgItemInt(hwnd, IDC_CONVERSION_BUFFERSIZE, settings.joining.last_buffer_size, 0);
		CheckDlgButton(hwnd, IDC_CONVERSION_STOPPLAYBACK, settings.joining.stop_playback ? BST_CHECKED : BST_UNCHECKED);

		encoder_initialize();
		{
			unsigned int i = 0;
			string       ename;

			ename = encoder_getname(i);
			while(ename)
			{
				SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)ename);
				i++;
				ename = encoder_getname(i);
			}

			if(settings.joining.last_encoder < i)
			{
				SendDlgItemMessage(hwnd, IDC_CONVERSION_ENCODER, CB_SETCURSEL, (WPARAM)settings.joining.last_encoder, 0);
				conv_selected_enc = settings.joining.last_encoder;
				encoder_plugin_initialize(conv_selected_enc);
			}
		}

		SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));

		SetTimer(hwnd, 400101, 30, 0);
		break;

	case WM_TIMER:
		{
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PS, PBM_SETPOS, (WPARAM)conv_pos, 0);
			SendDlgItemMessage(hwnd, IDC_CONVERSION_PRG, PBM_SETPOS, (WPARAM)(((ct_ci - 1) * 1000) + conv_pos), 0);
		}
		break;

	case WM_DESTROY:
		//if(j_data)
		//{
		//	sys_mem_free(j_data);
		//	j_data = 0;
		//}


		joining_window_open = 0;
		KillTimer(hwnd, 400101);
		EndDialog(hwnd, 0);

		window_joining = 0;
		break;
	}

	return 0;
}
/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/
