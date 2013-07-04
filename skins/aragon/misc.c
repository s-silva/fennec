#include "skin.h"
#include "skin settings.h"

#include "ids.h"

#define sort_score_max  1.0e200
#define sort_score_min  0.0
#define sort_score_fall (1.0 / 65536.0)
#define letter_max      (1 << (sizeof(letter) * 8))

extern int   enable_transparency;
extern int   window_transparency_amount;



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
		//skin_settings.ml_sorted_column = cid;
		//skin_settings.ml_sorted_mode   = smode;

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






void misc_time_to_string(int seconds, string buf)
{
	int     p = seconds;
	letter  pbuf[32], tbuf[32];

	memset(pbuf, 0, sizeof(pbuf));
	memset(tbuf, 0, sizeof(tbuf));
	

	{
		int x;

		memset(pbuf, 0, sizeof(pbuf));
		
		if(p / 60 < 60)
		{
			_itow(p / 60, pbuf, 10);
			pbuf[str_len(pbuf)] = uni(':');

			if((p % 60) < 10)
			{	
				pbuf[str_len(pbuf)] = uni('0');
				_itow(p % 60, pbuf + str_len(pbuf), 10);
			}else{
				_itow(p % 60, pbuf + str_len(pbuf), 10);
			}

		}else{

			x = p / 3600;
			_itow(x, pbuf, 10);
			str_cat(pbuf, uni(":"));

			x = (p - (x * 3600)) / 60;


			if(x < 10)
			{
				_itow(x, tbuf, 10);
				str_cat(pbuf, uni("0"));
				str_cat(pbuf, tbuf);
				str_cat(pbuf, uni(":"));
			}else{
				_itow(x, tbuf, 10);
				str_cat(pbuf, tbuf);
				str_cat(pbuf, uni(":"));
			}

			x = p % 60;

			if(x < 10)
			{
				_itow(x, tbuf, 10);
				str_cat(pbuf, uni("0"));
				str_cat(pbuf, tbuf);
			}else{
				_itow(x, tbuf, 10);
				str_cat(pbuf, tbuf);
			}
		}
	}

	str_cpy(buf, pbuf);
}


HMENU user_create_menu(int mid, int flags)
{
	HMENU  m = CreatePopupMenu(), c;

#	define addmenu(x, i, v)  (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_STRING, (i), (v)) )
#	define menugroup(x)      (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, 0) )
#	define addchild(x, h, v) (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)(h), (v)) )
#	define menubegin(x)      (x = CreatePopupMenu())
#	define checkmenuitem(x, i, c)(CheckMenuItem((x), (i), (c ? MFS_CHECKED : MFS_UNCHECKED)))
#	define menuend(x)        (DestroyMenu(x))

	switch(mid)
	{
	case menu_ml_columns:
		
		/*
		addmenu(m, mid_repeat_list,   skin.shared->language_text[oooo_skins_repeat_list]);
		addmenu(m, mid_repeat_track,  skin.shared->language_text[oooo_skins_repeat_track]);
		addmenu(m, mid_autoswitching, skin.shared->language_text[oooo_skins_autoswitching]);
		menugroup(m);
		addmenu(m, mid_shuffle, skin.shared->language_text[oooo_skins_shuffle]);
		addmenu(m, mid_switch_sindex, skin.shared->language_text[oooo_skins_display_shuffle_index]);
		menugroup(m);
		
		menubegin(c); */

		addmenu(m, midc_title     , skin.shared->language_text[oooo_tag_title]);
		addmenu(m, midc_album     , skin.shared->language_text[oooo_tag_album]);
		addmenu(m, midc_artist    , skin.shared->language_text[oooo_tag_artist]);
		addmenu(m, midc_oartist   , skin.shared->language_text[oooo_tag_origartist]);
		addmenu(m, midc_composer  , skin.shared->language_text[oooo_tag_composer]);
		addmenu(m, midc_lyricist  , skin.shared->language_text[oooo_tag_lyricist]);
		addmenu(m, midc_band      , skin.shared->language_text[oooo_tag_band]);
		addmenu(m, midc_copyright , skin.shared->language_text[oooo_tag_copyright]);
		addmenu(m, midc_publisher , skin.shared->language_text[oooo_tag_publish]);
		addmenu(m, midc_encodedby , skin.shared->language_text[oooo_tag_encodedby]);
		addmenu(m, midc_genre	  , skin.shared->language_text[oooo_tag_genre]);
		addmenu(m, midc_year	  , skin.shared->language_text[oooo_tag_year]);
		addmenu(m, midc_url       , skin.shared->language_text[oooo_tag_url]);
		addmenu(m, midc_ourl	  , skin.shared->language_text[oooo_tag_offiartisturl]);
		addmenu(m, midc_filepath  , skin.shared->language_text[oooo_tag_filepath]);
		addmenu(m, midc_filename  , skin.shared->language_text[oooo_tag_filename]);
		addmenu(m, midc_bpm		  , skin.shared->language_text[oooo_tag_bpm]);
		addmenu(m, midc_track	  , skin.shared->language_text[oooo_tag_tracknum]);
		addmenu(m, midc_index     , skin.shared->language_text[oooo_tag_index]);

		//addchild(m, c, skin.shared->language_text[oooo_skins_columns]);

		//menuend(c);

		//addmenu(m, midc_font      , uni("Select Font"));
		//addmenu(m, mid_settings     , skin.shared->language_text[oooo_skins_show_preferences]);

		break;

	case menu_ml_add:
		addmenu(m, mid_addfiles , skin.shared->language_text[oooo_skins_addfiles]);
		addmenu(m, mid_add_dir  , skin.shared->language_text[oooo_skins_addfolders]);
		menugroup(m);
		addmenu(m, mid_add_mlib , skin.shared->language_text[oooo_skins_addtoml]);
		menugroup(m);
		addmenu(m, mid_removeml,  skin.shared->language_text[oooo_skins_clearml]);
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
		addmenu(m, mid_removeall, skin.shared->language_text[oooo_skins_clear_playlist]);
		menugroup(m);
		addmenu(m, mid_preview,    uni("Preview"));
		addmenu(m, mid_preview_stop,    uni("Stop preview"));


		if(flags == 1) /* media library */
		{
			addmenu(m, mid_addtoplaylist,  skin.shared->language_text[oooo_skins_addtoplaylist]);
		}
		break;

	case menu_ml_settings:
		menubegin(c);
		addmenu(c, mid_settings_wallpaper_sel, uni("Select Wallpaper"));
		menugroup(c);
		addmenu(c, mid_settings_wallpaper_def, uni("Default"));
		addmenu(c, mid_settings_wallpaper_ld, uni("Default Low Definition"));
		addchild(m, c, uni("Wallpaper"));
		menuend(c);

		menubegin(c);
		addmenu(c, mid_settings_display_big, uni("Large"));
		addmenu(c, mid_settings_display_small, uni("Small"));

		if(settings_data.playlist.display_mode == playlist_display_small)
			checkmenuitem(c, mid_settings_display_small, 1);
		else
			checkmenuitem(c, mid_settings_display_big, 1);

		addchild(m, c, uni("Display"));


		menuend(c);


		menubegin(c);
		addmenu(c, mid_settings_transparency_no, uni("No Transparency"));
		addmenu(c, mid_settings_transparency_10, uni("10%"));
		addmenu(c, mid_settings_transparency_20, uni("20%"));
		addmenu(c, mid_settings_transparency_30, uni("30%"));
		addmenu(c, mid_settings_transparency_40, uni("40%"));
		addmenu(c, mid_settings_transparency_50, uni("50%"));
		addmenu(c, mid_settings_transparency_60, uni("60%"));
		addmenu(c, mid_settings_transparency_70, uni("70%"));

		if(enable_transparency)
		{
			switch(window_transparency_amount)
			{
			case 10:   checkmenuitem(c, mid_settings_transparency_10, 1); break;
			case 20:   checkmenuitem(c, mid_settings_transparency_20, 1); break;
			case 30:   checkmenuitem(c, mid_settings_transparency_30, 1); break;
			case 40:   checkmenuitem(c, mid_settings_transparency_40, 1); break;
			case 50:   checkmenuitem(c, mid_settings_transparency_50, 1); break;
			case 60:   checkmenuitem(c, mid_settings_transparency_60, 1); break;
			case 70:   checkmenuitem(c, mid_settings_transparency_70, 1); break;
			}
		}else{
			checkmenuitem(c, mid_settings_transparency_no, 1); 
		}

		addchild(m, c, uni("Transparency"));
		menuend(c);

		menubegin(c);
		addmenu(c, mid_settings_vis_showvideo,   uni("Show Video When Available"));

		if(settings_data.vis.video_when_available)
			checkmenuitem(c, mid_settings_vis_showvideo, 1);

		addchild(m, c, uni("Visualization"));
		menuend(c);

		menubegin(c);
		addmenu(c, mid_settings_covers_download,   uni("Automatically Download Albums Covers and Photos"));
		addmenu(c, mid_settings_covers_albums,   uni("Enable Album Art"));
		addmenu(c, mid_settings_covers_photos,   uni("Enable Artists\' Photos"));

		if(settings_data.advanced.auto_download_covers)
			checkmenuitem(c, mid_settings_covers_download, 1);

		if(settings_data.advanced.enable_album_art)
			checkmenuitem(c, mid_settings_covers_albums, 1);

		if(settings_data.advanced.enable_artist_photo)
			checkmenuitem(c, mid_settings_covers_photos, 1);

		addchild(m, c, uni("Album Covers and Photos"));
		menuend(c);
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


	case menu_ml_repeat:
		addmenu(m, mid_repeat_list,   skin.shared->language_text[oooo_skins_repeat_list]);
		addmenu(m, mid_repeat_track,  skin.shared->language_text[oooo_skins_repeat_track]);
		break;

	case menu_ml_dv_popup:
		addmenu(m, mid_dv_play,   uni("Play Now"));
		addmenu(m, mid_dv_add,  skin.shared->language_text[oooo_skins_addtoplaylist]);
		break;
	}

	return m;
}


void draw_imagebox(HDC ddc, HDC sdc, int w, int h, struct coord *tl, struct coord *tr, struct coord *bl, struct coord *br, struct coord *t, struct coord *b, struct coord *l, struct coord *r)
{
	int   i, c;

	/* drawing stuff */

	//	drawrect(hdc_vid, coords.window_vid.crop_ml.w, coords.window_vid.crop_tm.h, rct.right - coords.window_vid.crop_ml.w - coords.window_vid.crop_mr.w, rct.bottom - coords.window_vid.crop_tm.h - coords.window_vid.crop_bm.h, 0);

	/* top */

	c = (w - tl->w - tr->w);

	for(i=0; i<c; i+=t->w)
	{
		BitBlt(ddc, tl->w + i, 0, t->w, t->h, sdc, t->sx_n, t->sy_n, SRCCOPY);
	}

	/* bottom */

	c = (w - bl->w - br->w);

	for(i=0; i<c; i+=b->w)
	{
		BitBlt(ddc, bl->w + i, h - b->h, b->w, b->h, sdc, b->sx_n, b->sy_n, SRCCOPY);
	}

	/* left */

	c = (h - tl->h - bl->h);

	for(i=0; i<c; i+=l->h)
	{
		BitBlt(ddc, 0, tl->h + i, l->w, l->h, sdc, l->sx_n, l->sy_n, SRCCOPY);
	}

	/* right */

	c = (h - tl->h - bl->h);

	for(i=0; i<c; i+=r->h)
	{
		BitBlt(ddc, w - r->w, tl->h + i, r->w, r->h, sdc, r->sx_n, r->sy_n, SRCCOPY);
	}


	/* finalize */

	BitBlt(ddc, 0, 0, tl->w, tl->h, sdc, tl->sx_n, tl->sy_n, SRCCOPY);
	BitBlt(ddc, w - tr->w, 0, tr->w, tr->h, sdc, tr->sx_n, tr->sy_n, SRCCOPY);
	BitBlt(ddc, 0, h - bl->h, bl->w, bl->h, sdc, bl->sx_n, bl->sy_n, SRCCOPY);
	BitBlt(ddc, w - br->w, h - br->h, br->w, br->h, sdc, br->sx_n, br->sy_n, SRCCOPY);
}
