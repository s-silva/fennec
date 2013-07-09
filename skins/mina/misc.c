#include "skin.h"
#include "ids.h"

#define sort_score_max  1.0e200
#define sort_score_min  0.0
#define sort_score_fall (1.0 / 65536.0)
#define letter_max      (1 << (sizeof(letter) * 8))

double list_sort_score(string sv)
{
	double         cscore = sort_score_min;
	double         fpos = (sort_score_max / 2.0);
	unsigned long  i = 0;

	while(sv[i])
	{
		cscore += ((double)str_clower(sv[i]) / (double)letter_max) * fpos;
		fpos   *= sort_score_fall;
		i++;
	}
	return cscore;
}


void list_sort_column(int cid, int smode)
{
	struct fennec_audiotag       *atag;
	struct fennec_audiotag_item  *ati;
	unsigned long  i, j, s, c = ml_cache_getcount();
	double        *smem;
	double         fv;
	
	smem = (double*) sys_mem_alloc(c * sizeof(double));
	if(!smem) return;

	if(smode < 2)
	{
		skin_settings.ml_sorted_column = cid;
		skin_settings.ml_sorted_mode   = smode;

		for(i=0; i<c; i++)
		{
			atag = ml_cache_get(i);

			switch(cid)
			{
			case header_tag_title          : ati = &atag->tag_title       ; break;
			case header_tag_album		   : ati = &atag->tag_album		  ; break;
			case header_tag_artist		   : ati = &atag->tag_artist      ; break;
			case header_tag_origartist	   : ati = &atag->tag_origartist  ; break;
			case header_tag_composer	   : ati = &atag->tag_composer	  ; break;
			case header_tag_lyricist	   : ati = &atag->tag_lyricist	  ; break;
			case header_tag_band		   : ati = &atag->tag_band		  ; break;
			case header_tag_copyright	   : ati = &atag->tag_copyright	  ; break;
			case header_tag_publish		   : ati = &atag->tag_publish	  ; break;
			case header_tag_encodedby      : ati = &atag->tag_encodedby   ; break;
			case header_tag_genre		   : ati = &atag->tag_genre		  ; break;
			case header_tag_year		   : ati = &atag->tag_year		  ; break;
			case header_tag_url			   : ati = &atag->tag_url		  ; break;
			case header_tag_offiartisturl  : ati = &atag->tag_offiartisturl; break;
			case header_tag_filepath	   : ati = &atag->tag_filepath	  ; break;
			case header_tag_filename	   : ati = &atag->tag_filename	  ; break;
			case header_tag_comments	   : ati = &atag->tag_comments	  ; break;
			case header_tag_lyric		   : ati = &atag->tag_lyric		  ; break;
			case header_tag_bpm            : ati = &atag->tag_bpm         ; break;
			case header_tag_tracknum	   : ati = &atag->tag_tracknum	  ; break;
			default: return;
			}

			if(ati->tdata && ati->tsize)

				if(cid == header_tag_tracknum || cid == header_tag_year)
				{

					smem[i] = str_stoi(ati->tdata);
					
				}else{

					smem[i] = list_sort_score(ati->tdata);
				}

			else
				smem[i] = sort_score_max;
		}

	}else{ /* directories */

		if(cid == media_library_dir_years)
		{
			for(i=0; i<c; i++)
			{
				if(cached_tags[i].dname)
					smem[i] = str_stoi(cached_tags[i].dname);
				else
					smem[i] = sort_score_max;
			}

		}else{
			for(i=0; i<c; i++)
			{
				if(cached_tags[i].dname)
					smem[i] = list_sort_score(cached_tags[i].dname);
				else
					smem[i] = sort_score_max;
			}
		}
	}

	/* second run */

	if(smode & 1) /* descending (1, 3) */
	{
		for(i=0; i<c; i++)
		{
			fv = 0;
			s  = (unsigned long)-1;

			for(j=0; j<c; j++)
			{
				if((smem[j] > 0.0) && (smem[j] > fv))
				{
					fv = smem[j];
					s  = j;
				}
			}

			if(s != (unsigned long)-1)
			{
				ml_cache_exchange(i, s);
				skin.shared->audio.output.playlist.move(i, s, 1);
				
				smem[s] = smem[i];
				smem[i] = -1.0;
			}
		}
	}else{
		for(i=0; i<c; i++)
		{
			fv = (sort_score_max * 2.0);
			s  = (unsigned long)-1;

			for(j=0; j<c; j++)
			{
				if((smem[j] > 0.0) && (smem[j] < fv))
				{
					fv = smem[j];
					s  = j;
				}
			}

			if(s != (unsigned long)-1)
			{
				ml_cache_exchange(i, s);
				skin.shared->audio.output.playlist.move(i, s, 1);
				
				smem[s] = smem[i];
				smem[i] = -1.0;
			}
		}
	}

	sys_mem_free(smem);
}




HMENU user_create_menu(int mid, int flags)
{
	HMENU  m = CreatePopupMenu(), c;

#	define addmenu(x, i, v)  (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_STRING, (i), (v)) )
#	define menugroup(x)      (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, 0) )
#	define addchild(x, h, v) (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)(h), (v)) )
#	define menubegin(x)      (x = CreatePopupMenu())
#	define menuend(x)        (DestroyMenu(x))

	switch(mid)
	{
	case menu_ml_options:
		
		addmenu(m, mid_repeat_list,   skin.shared->language_text[oooo_skins_repeat_list]);
		addmenu(m, mid_repeat_track,  skin.shared->language_text[oooo_skins_repeat_track]);
		addmenu(m, mid_autoswitching, skin.shared->language_text[oooo_skins_autoswitching]);
		menugroup(m);
		addmenu(m, mid_shuffle, skin.shared->language_text[oooo_skins_shuffle]);
		addmenu(m, mid_switch_sindex, skin.shared->language_text[oooo_skins_display_shuffle_index]);
		menugroup(m);
		
		menubegin(c);

		addmenu(c, midc_title     , skin.shared->language_text[oooo_tag_title]);
		addmenu(c, midc_album     , skin.shared->language_text[oooo_tag_album]);
		addmenu(c, midc_artist    , skin.shared->language_text[oooo_tag_artist]);
		addmenu(c, midc_oartist   , skin.shared->language_text[oooo_tag_origartist]);
		addmenu(c, midc_composer  , skin.shared->language_text[oooo_tag_composer]);
		addmenu(c, midc_lyricist  , skin.shared->language_text[oooo_tag_lyricist]);
		addmenu(c, midc_band      , skin.shared->language_text[oooo_tag_band]);
		addmenu(c, midc_copyright , skin.shared->language_text[oooo_tag_copyright]);
		addmenu(c, midc_publisher , skin.shared->language_text[oooo_tag_publish]);
		addmenu(c, midc_encodedby , skin.shared->language_text[oooo_tag_encodedby]);
		addmenu(c, midc_genre	  , skin.shared->language_text[oooo_tag_genre]);
		addmenu(c, midc_year	  , skin.shared->language_text[oooo_tag_year]);
		addmenu(c, midc_url       , skin.shared->language_text[oooo_tag_url]);
		addmenu(c, midc_ourl	  , skin.shared->language_text[oooo_tag_offiartisturl]);
		addmenu(c, midc_filepath  , skin.shared->language_text[oooo_tag_filepath]);
		addmenu(c, midc_filename  , skin.shared->language_text[oooo_tag_filename]);
		addmenu(c, midc_bpm		  , skin.shared->language_text[oooo_tag_bpm]);
		addmenu(c, midc_track	  , skin.shared->language_text[oooo_tag_tracknum]);
		addmenu(c, midc_index     , skin.shared->language_text[oooo_tag_index]);

		addchild(m, c, skin.shared->language_text[oooo_skins_columns]);

		menuend(c);

		addmenu(m, midc_font      , uni("Select Font"));
		addmenu(m, mid_settings     , skin.shared->language_text[oooo_skins_show_preferences]);

		break;

	case menu_ml_add:
		addmenu(m, mid_addfiles , skin.shared->language_text[oooo_skins_addfiles]);
		addmenu(m, mid_add_dir  , skin.shared->language_text[oooo_skins_addfolders]);
		menugroup(m);
		addmenu(m, mid_add_mlib , skin.shared->language_text[oooo_skins_addtoml]);
		break;

	case menu_ml_save:
		addmenu(m, mid_savepl, skin.shared->language_text[oooo_skins_save_playlist]);
		menugroup(m);
		addmenu(m, mid_import, skin.shared->language_text[oooo_skins_import_ml]);
		addmenu(m, mid_export, skin.shared->language_text[oooo_skins_export_ml]);
		break;

	case menu_ml_remove:
		addmenu(m, mid_removesel, skin.shared->language_text[oooo_skins_remove_sel]);
		addmenu(m, mid_removeall, skin.shared->language_text[oooo_skins_clear_playlist]);
		menugroup(m);
		addmenu(m, mid_removeml,  skin.shared->language_text[oooo_skins_clearml]);
		break;

	case menu_ml_sort:
		addmenu(m, mid_sort_filename, skin.shared->language_text[oooo_skins_sort_by_filename]);
		addmenu(m, mid_unsort,        skin.shared->language_text[oooo_skins_sort_unsort]);
		break;

	case menu_ml_popup:
		addmenu(m, mid_edittags,   skin.shared->language_text[oooo_skins_edit_tags]);
		addmenu(m, mid_removesel,  skin.shared->language_text[oooo_skins_remove_sel]);
		menugroup(m);
		addmenu(m, mid_preview,    uni("Preview"));
		addmenu(m, mid_preview_stop,    uni("Stop preview"));

		if(flags == 1) /* media library */
		{
			addmenu(m, mid_addtoplaylist,  skin.shared->language_text[oooo_skins_addtoplaylist]);
		}
		break;


	case menu_open:
		{
			letter exptext[256];

			str_cpy(exptext, skin.shared->language_text[oooo_skins_open]);
			str_cat(exptext, uni(" (experimental)"));

			addmenu(m, mid_addfiles,     skin.shared->language_text[oooo_skins_addfiles]);
			addmenu(m, mid_adddirs,      skin.shared->language_text[oooo_skins_addfolders]);
			menugroup(m);
			addmenu(m, mid_loadtracks,   skin.shared->language_text[oooo_menu_load_tracks]);
			menugroup(m);
			addmenu(m, mid_tray,         skin.shared->language_text[oooo_menu_eject_load]);
			addmenu(m, mid_seldrive,     skin.shared->language_text[oooo_menu_select_drive]);
			menugroup(m);
			addmenu(m, mid_experimental, exptext);
		}
		break;
	}

	return m;
}

int setwinpos_clip(HWND hwnd, HWND hwa, int x, int y, int w, int h, UINT flags)
{
	int   rv;
	int   rx = x, ry = y;
	RECT  wrect;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &wrect, 0);

	if(rx >= wrect.right) rx = wrect.right  - w;
	if(ry >= wrect.bottom)ry = wrect.bottom - h;
	if(rx + w <= 0)rx = 0;
	if(ry + h <= 0)ry = 0;

	rv = SetWindowPos(hwnd, hwa, rx, ry, w, h, flags);
	return rv;
}