/**----------------------------------------------------------------------------

 Fennec Skin - Player 1
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

#include "skin.h"
#include "ids.h"
#include "zmouse.h"
#include <shlobj.h>



/* structs ------------------------------------------------------------------*/



/* prototypes ---------------------------------------------------------------*/

LRESULT CALLBACK callback_ml_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ml_draw_window(int rd);
void ml_cache_uninit(void);
void ml_cache_init(void);


#define  sel_toggle 4

#define  redraw_all       1
#define  redraw_default   0
#define  redraw_playlist  4
#define  redraw_playlist_scroll 5

#define  preview_panel_h      12
#define  preview_panel_w      125
#define  preview_panel_x      100
#define  preview_panel_y      2
#define  preview_panel_play_x 1
#define  preview_panel_play_y 1
#define  preview_panel_play_w 10
#define  preview_panel_play_h 10
#define  preview_panel_stop_x 11
#define  preview_panel_stop_y 1
#define  preview_panel_stop_w 10
#define  preview_panel_stop_h 10
#define  preview_panel_seek_x 24
#define  preview_panel_seek_y 1
#define  preview_panel_seek_w 100
#define  preview_panel_seek_h 10
#define  artist_field_item_height 50

#define  timer_id_i_search    1231

#define  txt_ml_add_files   uni("Please add files to the media library first.")


/* data ---------------------------------------------------------------------*/


int               ml_init = 0;
long              ml_pl_startid = 0;
long              ml_pl_startid_last;
unsigned long     cached_tags_count;
unsigned long     pl_last_cur = 0;
unsigned long     pl_cur_item = 0;
struct pl_cache  *cached_tags = 0;
int               mode_ml = 0;

unsigned long     ml_current_item = 0;
unsigned long     ml_last_dir    = media_library_dir_root;
unsigned long     ml_current_dir = media_library_dir_root;
int               ml_in_dir = 1;
int               ml_dir_changed = 1;
int               ml_last_dir_index = 0;

string            ml_current_cdir = 0;
letter            ml_cdir_text[512];


/* code ---------------------------------------------------------------------*/


/* cache - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * (re)initialize cache.
 */
void ml_cache_init(void)
{
	unsigned long i, j, k;
	unsigned long c, z;


	if(!mode_ml)
	{
		c = skin.shared->audio.output.playlist.getcount();
	}else{
		skin.shared->mlib.media_library_advanced_function(1, 0, 0); /* safe initialize */


		if(ml_in_dir)
		{
			z = 2;
			cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);

			for(i=0, k=0;;)
			{
				c = skin.shared->mlib.media_library_get_dir_names(k, ml_current_dir, &cached_tags[i].dname);
				if(!c) break;
				
				
				for(j=0; j<i; j++)
				{
					if(!str_icmp(cached_tags[j].dname, cached_tags[i].dname))
					{
						skin.shared->mlib.media_library_get_dir_names((unsigned long)-1, 0, (string*)cached_tags[i].dname); /* free */
						i--;
						break;
					}
				}

				

				i++;
				k++;
				if(i >= z)
				{
					z = i + 2;
					cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);
				}
			}

			cached_tags_count = i;

			for(i=0; i<cached_tags_count; i++)
			{
				cached_tags[i].pid   = (unsigned long)-1;
				cached_tags[i].sel   = 0;
				cached_tags[i].album_points[0]  = 0; /* like text strings, ends with (0) */
			}

			//if(skin_settings.ml_dir_sort_mode >= 2)
			//	list_sort_column(ml_current_dir, skin_settings.ml_dir_sort_mode);

			return;


		}else{

			if(ml_current_dir == media_library_dir_all)
			{
				c = (unsigned long)skin.shared->mlib.media_library_get_dir_files((unsigned long)-1, media_library_dir_all, 0, 0); /* get count */

			}else{

				uint32_t  litem = 0;

				z = 2;
				cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);

				for(i=0;;)
				{
					c = skin.shared->mlib.media_library_get_dir_files(i, ml_current_dir, ml_current_cdir, &litem);
					if(!c) break;

					skin.shared->mlib.media_library_translate(litem, &cached_tags[i].ft);
					litem++;
					
					i++;
		
					if(i >= z)
					{
						z = i + 2;
						cached_tags = (struct pl_cache*) realloc(cached_tags, sizeof(struct pl_cache) * z);
					}
				}

				cached_tags_count = i;

				for(i=0; i<cached_tags_count; i++)
				{
					cached_tags[i].pid   = 1;
					cached_tags[i].sel   = 0;
					cached_tags[i].dname = 0;
					cached_tags[i].album_points[0]  = 0;
				}
				return;
			}
		}
	}
	
	if(cached_tags)ml_cache_uninit();
	
	if(!c)
	{
		cached_tags = 0;
		return;
	}

	cached_tags = (struct pl_cache*) malloc(sizeof(struct pl_cache) * c);

	cached_tags_count = c;

	for(i=0; i<c; i++)
	{
		cached_tags[i].pid   = (unsigned long)-1;
		cached_tags[i].sel   = 0;
		cached_tags[i].dname = 0;
		cached_tags[i].album_points[0]  = 0;
	}

	if(pl_cur_item < cached_tags_count)
		cached_tags[pl_cur_item].sel = 1;
}

void ml_cache_exchange(unsigned long di, unsigned long si)
{
	struct pl_cache bpc;

	if(ml_in_dir && mode_ml && (ml_current_dir == media_library_dir_root)) return;

	if(!ml_in_dir && mode_ml)
	{
		skin.shared->mlib.media_library_advanced_function(3 /* set sorting mode */, 1, 0);
		skin.shared->mlib.media_library_advanced_function(2 /* exchange */, di, ULongToPtr(si));
	}

	memcpy(&bpc, &cached_tags[di], sizeof(struct pl_cache));
	memcpy(&cached_tags[di], &cached_tags[si], sizeof(struct pl_cache));
	memcpy(&cached_tags[si], &bpc, sizeof(struct pl_cache));
}


/*
 * clear selection.
 */
void ml_cache_clearsel(void)
{
	unsigned long i;

	if(!cached_tags)return;

	for(i=0; i<cached_tags_count; i++)
	{
		cached_tags[i].sel = 0;
	}
}


/*
 * is selected?
 */
int ml_cache_issel(unsigned long id)
{
	if(!cached_tags || id >= cached_tags_count)return (int)-1;
	return (int)cached_tags[id].sel;
}


/*
 * shift selection (up or down).
 */
int ml_cache_shiftsel(long s)
{
	long i;

	if(!cached_tags)return -1;

	if(s > 0)
	{
		if(cached_tags[cached_tags_count - 1].sel) return -2;

		for(i=(cached_tags_count-1); i>=0; i--)
		{
			if((unsigned long)(i + s) < cached_tags_count)
			{
				cached_tags[i + s].sel = cached_tags[i].sel;
				cached_tags[i].sel = 0;
			}

		}
	}else if(s < 0){

		if(cached_tags[0].sel) return -2;

		for(i=0; i<(long)cached_tags_count; i++)
		{
			if((i + s) >= 0)
			{
				cached_tags[i + s].sel = cached_tags[i].sel;
				cached_tags[i].sel = 0;
			}

		}
	}
	return 0;
}

unsigned long ml_cache_getcurrent_item(void)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getcurrentindex();
	else
		return skin.shared->audio.output.playlist.getcurrentindex();
}


unsigned long ml_cache_getnext_item(void)
{
	unsigned long cpid = ml_cache_getlistindex(ml_cache_getcurrent_item()) + 1;
	if(cpid >= cached_tags_count) return (unsigned long)-1;

	return ml_cache_getrealindex(cpid);
}

const string ml_cache_getpath(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getsource(itemid);
	else
		return cached_tags[itemid].ft.tag_filepath.tdata;
}

unsigned long ml_cache_getlistindex(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getlistindex(itemid);
	else
		return itemid;
}

unsigned long ml_cache_getrealindex(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.getrealindex(itemid);
	else
		return itemid;
}

int ml_cache_switch(unsigned long itemid)
{
	if(!mode_ml)
		return skin.shared->audio.output.playlist.switchindex(itemid);
	else
		return 0;
}

int ml_cache_switchlist(unsigned long itemid, int mplay)
{
	if(!mode_ml)
	{
		int rv;
		rv = skin.shared->audio.output.playlist.switch_list(itemid);
		if(mplay)skin.shared->audio.output.play();
		return rv;

	}else{

		if(!ml_in_dir)
		{
			unsigned long i;
			struct fennec_audiotag *at;

			ml_current_item = itemid;
			

			if(ml_dir_changed || (cached_tags_count != skin.shared->audio.output.playlist.getcount()) )
			{
				skin.shared->audio.output.playlist.clear();

				for(i=0; i<cached_tags_count; i++)
				{
					at = (struct fennec_audiotag*) ml_cache_get(i);
					skin.shared->audio.output.playlist.add((string)at->tag_filepath.tdata, 0, 0);
					//skin.shared->audio.output.playlist.add(0, 0, i);
				}
				ml_dir_changed = 0;
			}

			at = ml_cache_get(itemid);
			skin.shared->mlib.media_library_advanced_function(8 /* update info */, 0, (void*)(string)at->tag_filepath.tdata);

			skin.shared->audio.output.playlist.switch_list(itemid);
			if(mplay)skin.shared->audio.output.play();
		}else{
			
			ml_last_dir = ml_current_dir;

			ml_dir_changed = 1;

			if(ml_current_dir == media_library_dir_root)
			{	
				ml_current_dir = skin.shared->mlib.media_library_advanced_function(4 /* get child dir index */, itemid, 0);
			}else{
				if(itemid < cached_tags_count)
					str_cpy(ml_cdir_text, cached_tags[itemid].dname);
				ml_current_cdir = ml_cdir_text;

				ml_last_dir_index = ml_pl_startid;
				ml_pl_startid = 0;
			}

			ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
			
			ml_cache_uninit();
			ml_cache_init();
		}
		return 0;
	}
}

int ml_cache_add_to_playlist(unsigned long itemid)
{
	if(!mode_ml)
	{
		return skin.shared->audio.output.playlist.switch_list(itemid);
	}else{
		if(!ml_in_dir)
		{
			struct fennec_audiotag *at;
			at = (struct fennec_audiotag*) ml_cache_get(itemid);
			skin.shared->audio.output.playlist.add((string)at->tag_filepath.tdata, 0, 0);
		}
		return 0;
	}
}



/*
 * set selection.
 */
int ml_cache_setsel(unsigned long id, char v)
{
	if(!cached_tags || id >= cached_tags_count)return -1;
	if(v != sel_toggle)
		cached_tags[id].sel = v ? 1 : 0;
	else
		cached_tags[id].sel ^= 1;
	return 0;
}


/*
 * get tags from the cache.
 */
void *ml_cache_get(unsigned long id)
{
	if(!cached_tags || id >= cached_tags_count)return (void*)-1;

	if(cached_tags[id].pid != -1)
	{
		return (void*) &cached_tags[id].ft;
	}else{

		if(!mode_ml)
		{
			unsigned long mlid = (unsigned long) skin.shared->audio.output.playlist.getinfo(id, 0, 0, 0, 0, 0, 0);
			if(mlid == (unsigned long)-1)
			{
				cached_tags[id].pid = skin.shared->audio.input.tagread(skin.shared->audio.output.playlist.getsource(id), &cached_tags[id].ft);
			}else{
				skin.shared->mlib.media_library_translate(mlid, &cached_tags[id].ft);
				cached_tags[id].pid = 1;
			}

		}else{
			if(!ml_in_dir)
			{
				skin.shared->mlib.media_library_translate(id, &cached_tags[id].ft);
			}else{
				skin.shared->mlib.media_library_get_dir_names(id, ml_current_dir, &cached_tags[id].dname);
			}

			cached_tags[id].pid = 1;
		}

		return (void*) &cached_tags[id].ft;
	}
}


/*
 * free all the tags allocated.
 */
void ml_cache_freetags(void)
{
	unsigned long i;

	if(!cached_tags)return;

	for(i=0; i<cached_tags_count; i++)
	{
		if(!mode_ml)
		{
			if(cached_tags[i].pid != -1)
				skin.shared->audio.input.tagread_known(cached_tags[i].pid, 0, &cached_tags[i].ft);
		
			if(cached_tags[i].dname)
				skin.shared->mlib.media_library_get_dir_names((unsigned long)-1, 0, (string*)cached_tags[i].dname); /* free */
		}else{

		}

		cached_tags[i].pid = (unsigned long)-1;
	}
}


/*
 * uninitialization.
 */
void ml_cache_uninit(void)
{
	unsigned long i;

	if(!cached_tags)return;

	for(i=0; i<cached_tags_count; i++)
	{
		if(!mode_ml)
		{
			if(cached_tags[i].pid != -1)
				skin.shared->audio.input.tagread_known(cached_tags[i].pid, 0, &cached_tags[i].ft);
		}else{
			if(cached_tags[i].dname)
				skin.shared->mlib.media_library_get_dir_names((unsigned long)-1, 0, (string*)cached_tags[i].dname); /* free */
		}

		cached_tags[i].pid = (unsigned long)-1;
	}

	free(cached_tags);
	cached_tags = 0;
}


/*
 * remove selected media from the playlist.
 */
void ml_cache_removesel(void)
{
	unsigned long i;
	unsigned long j;

	if(!cached_tags)return;

	if(!mode_ml)
	{
		for(i=0, j=0; i<cached_tags_count; i++, j++)
		{
			if(cached_tags[i].sel)
			{
				skin.shared->audio.output.playlist.remove(j);
				j--;
			}
		}

		ml_cache_uninit();
		ml_cache_init();
	}
}

unsigned long ml_cache_getcount(void)
{
	return cached_tags_count;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

