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




/* structures ---------------------------------------------------------------*/

typedef struct _playlist_item
{
    unsigned long rindex;               /* real index in shuffle mode */

	/* takes time in directory additin */
    /* letter        spath[v_sys_maxpath];  source path (file/stream name) */
	
	string        spath;
	unsigned long medialibid;           /* id in media library, if a standalone
										   file, this will be -1 */

    unsigned long imarked;              /* marked? (played...) */
}playlist_item;


/* defines ------------------------------------------------------------------*/

/* update: made the value to 200 cuz decreased the playlist item size */
#define internalout_playlist_memoryaddition 200 /* resize memory by? */

#define getrand(max) ( rand() % (int)((max) + 1) )




/* data ---------------------------------------------------------------------*/

unsigned long  playlist_count;               /* playlist items count */
unsigned long  playlist_size;                /* items count can be stored in the allocated memory block */
unsigned long  playlist_currentindex;        /* current index (real) */
unsigned long  playlist_shufflefirst;

playlist_item *playlist_array;

int            playlist_initialized = 0;
int            playlist_changed = 0;
t_sys_thread_share  pl_cs;




/* code ---------------------------------------------------------------------*/


/*
 * initialize playlist engine.
 */
int audio_playlist_initialize(void)
{
	if(playlist_initialized)return 1;

	sys_thread_share_create(&pl_cs);

	srand(sys_timer_getms());
	
	playlist_count           = 0;
	playlist_currentindex    = 0;
	playlist_size  = internalout_playlist_memoryaddition;
	playlist_array = (playlist_item*) sys_mem_alloc(playlist_size * sizeof(playlist_item));
	
	playlist_initialized = 1;
	return 1;
}


/*
 * uninitialize playlist engine.
 */
int audio_playlist_uninitialize(void)
{
	unsigned long i = 0;

	if(!playlist_initialized)return 0;

	for(i=0; i<playlist_count; i++)
	{
		if(playlist_array[i].spath)
			sys_mem_free(playlist_array[i].spath);
	}

	sys_mem_free(playlist_array);
	playlist_initialized = 0;

	sys_thread_share_close(&pl_cs);
	return 1;
}


/*
 * clear playlist.
 */
int audio_playlist_clear(void)
{
	if(!playlist_initialized)return 0;

	sys_thread_share_request(&pl_cs);

	playlist_count        = 0;
	playlist_currentindex = 0;
	playlist_size         = internalout_playlist_memoryaddition;
	playlist_array        = (playlist_item*) sys_mem_realloc(playlist_array, playlist_size * sizeof(playlist_item));
	
	sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * add new entry to playlist.
 * to add a media library item, call this with fname = 0, gettags = id
 */
int audio_playlist_add(const string fname, int checkfile, int gettags)
{
	if(!playlist_initialized)return 0;

	sys_thread_share_request(&pl_cs);

    if(playlist_count >= playlist_size)
    {
		playlist_size += internalout_playlist_memoryaddition;
		playlist_array = (playlist_item*) sys_mem_realloc(playlist_array, playlist_size * sizeof(playlist_item));
    }

	/* update: june 6, 2010 */

	if(fname)
	{
		playlist_array[playlist_count].medialibid = (unsigned long)-1;
		playlist_array[playlist_count].spath = sys_mem_alloc(str_size(fname) + sizeof(letter));
		str_cpy(playlist_array[playlist_count].spath, fname);
	}else{
		playlist_array[playlist_count].medialibid = (unsigned long)gettags;
		playlist_array[playlist_count].spath = 0;
	}

    

    playlist_array[playlist_count].rindex = playlist_count;
    
    playlist_array[playlist_count].imarked = 0;

	sys_thread_share_release(&pl_cs);

    if(settings.player.playlist_shuffle)
    {
        audio_playlist_shuffle(0); /* shuffle items that haven't been played */
    }

	playlist_changed = 1;
	playlist_count++;

	return 1;
}


/*
 * exchange two entries.
 */
int audio_playlist_exchange(unsigned long idd, unsigned long ids)
{
	playlist_item itembackup;
	sys_thread_share_request(&pl_cs);

	memcpy(&itembackup, &playlist_array[idd], sizeof(playlist_item));
	memcpy(&playlist_array[idd], &playlist_array[ids], sizeof(playlist_item));
	memcpy(&playlist_array[ids], &itembackup, sizeof(playlist_item));

	sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * move single track.
 */
int audio_playlist_move(unsigned long idd, unsigned long ids, int mmode)
{
    unsigned long         i, srindex;
    playlist_item bpi;
	unsigned long         tmprindex;


	if(idd >= playlist_count || ids >= playlist_count)return 0;

    if(idd == ids)return 0;

	

	tmprindex = audio_playlist_getrealindex(playlist_currentindex);

	sys_thread_share_request(&pl_cs);

	srindex = playlist_array[tmprindex].rindex;
	

	if(mmode == 1) /* exchange */
	{
		playlist_item itembackup;

		memcpy(&itembackup, &playlist_array[idd], sizeof(playlist_item));
		memcpy(&playlist_array[idd], &playlist_array[ids], sizeof(playlist_item));
		memcpy(&playlist_array[ids], &itembackup, sizeof(playlist_item));

		sys_thread_share_release(&pl_cs);
		return 1;
	}

	if(!settings.player.playlist_shuffle)
	{
		if(ids == playlist_currentindex)
		{
			playlist_currentindex = idd;	
		}else{
			if(idd <= playlist_currentindex && ids > playlist_currentindex)playlist_currentindex++;
			if(idd >= playlist_currentindex && ids < playlist_currentindex)playlist_currentindex--;

		}
	}

    if(idd < ids)
    {
        memcpy(&bpi, &playlist_array[ids], sizeof(playlist_item));

        for(i=ids; i>idd; i--)
        {
            memcpy(&playlist_array[i], &playlist_array[i - 1], sizeof(playlist_item));
        }
        
        memcpy(&playlist_array[idd], &bpi, sizeof(playlist_item));

    }else{

        memcpy(&bpi, &playlist_array[ids], sizeof(playlist_item));

        for(i=ids; i<idd; i++)
        {
            memcpy(&playlist_array[i], &playlist_array[i + 1], sizeof(playlist_item));
        }
        
        memcpy(&playlist_array[idd], &bpi, sizeof(playlist_item));
    }
	
	

	if(settings.player.playlist_shuffle)
	{
		for(i=0; i<playlist_count; i++)
		{
			if(playlist_array[i].rindex == srindex)
			{
				sys_thread_share_release(&pl_cs);
				playlist_currentindex = audio_playlist_getlistindex(i);
				sys_thread_share_request(&pl_cs);
			}
		}
	}

	sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * (un)set shuffle.
 */
int audio_playlist_setshuffle(int shuffle, int doaction)
{
	if(shuffle == settings.player.playlist_shuffle)return 0;

	if(doaction)
	{
		if(shuffle)
		{
			audio_playlist_shuffle(1);
		}else{
			playlist_currentindex = audio_playlist_getcurrentindex();
			audio_playlist_unshuffle();
		}
	}
	
	settings.player.playlist_shuffle = (unsigned char)shuffle;
	return 1;
}


/*
 * shuffle the list.
 */
int audio_playlist_shuffle(int entirelist)
{
    unsigned long  i, j, id, rd;

	if(!playlist_initialized)return 0;

	//sys_thread_share_request(&pl_cs);


	/*
	if(entirelist)
	{
    */
		playlist_shufflefirst = playlist_currentindex;

		for(j= 0; j<=settings.player.playlist_shuffle_rate; j++)
		{
			for(i=0; i<playlist_count; i++)
			{
				rd = getrand(playlist_count);

				if(rd == playlist_currentindex)continue;
				if(i  == playlist_currentindex)continue;
				if(rd >= playlist_count)continue;

				id = playlist_array[i].rindex;

				playlist_array[i].rindex  = playlist_array[rd].rindex;
				playlist_array[rd].rindex = id;
			}
		}

		/* 'switch_list' will do the remaining */

	/*	
	}else{

		unsigned long  cads = 0;
		unsigned long  bindex;
		unsigned long  rval;
		unsigned long* dcads;

	
		dcads = (unsigned long*) sys_mem_alloc(sizeof(unsigned long) * (playlist_count + 10));

		for(i=0; i<playlist_count; i++)
		{
			if(!playlist_array[i].imarked)
			{
				dcads[cads] = i;
				cads++;
			}
		}

		for(i=0; i<cads; i++)
		{
			rval = getrand(cads - 1);
			bindex = playlist_array[dcads[rval]].rindex;

			playlist_array[dcads[rval]].rindex = playlist_array[dcads[i]].rindex;
			playlist_array[dcads[i]].rindex    = bindex;
		}

		sys_mem_free(dcads);

	}*/

	//sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * unshuffle the list.
 */
int audio_playlist_unshuffle(void)
{
	unsigned long i;

	if(!playlist_initialized)return 0;

	sys_thread_share_request(&pl_cs);

	for(i=0; i<playlist_count; i++)
	{
		playlist_array[i].rindex  = i;
	}

	sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * remove an entry.
 */
int audio_playlist_remove(unsigned long idx)
{
    unsigned long         i;

	if(idx >= playlist_count)return 0;

	sys_thread_share_request(&pl_cs);

	/* update: june 6, 2010 */

	if(playlist_array[idx].spath)
		sys_mem_free(playlist_array[idx].spath);

    for(i=idx; i<playlist_count; i++)
    {
        memcpy(&playlist_array[i], &playlist_array[i + 1], sizeof(playlist_item));
    }

	playlist_count--;

	sys_thread_share_release(&pl_cs);
	
	if(settings.player.playlist_shuffle)
	{
		audio_playlist_unshuffle();
		audio_playlist_shuffle(1);
	}
	return 1;
}


/*
 * get real index of an entry (shuffled index).
 */
unsigned long audio_playlist_getrealindex(unsigned long idx)
{
	unsigned long ret;
	if(!playlist_initialized)return (unsigned long)-1;
	if(playlist_count < idx)return (unsigned long)-1;

	if(!settings.player.playlist_shuffle)return idx;

	sys_thread_share_request(&pl_cs);

	ret = playlist_array[idx].rindex;
	sys_thread_share_release(&pl_cs);
	return ret;
}


/*
 * get playlist index (physical index) of an entry.
 */
unsigned long audio_playlist_getlistindex(unsigned long ridx)
{
	unsigned long i;

	if(!playlist_initialized)return (unsigned long)-1;
	if(playlist_count < ridx)return (unsigned long)-1;

	if(!settings.player.playlist_shuffle)return ridx;

	sys_thread_share_request(&pl_cs);

	for(i=0; i<=playlist_count; i++)
	{
		if(playlist_array[i].rindex == ridx)
		{
			sys_thread_share_release(&pl_cs);
			return i;
		}
	}

	sys_thread_share_release(&pl_cs);
	return (unsigned long)-1;
}


/*
 * set item as marked.
 */
int audio_playlist_mark(unsigned long idx)
{
	if(!playlist_initialized)return 0;
	if(playlist_count < idx)return 0;

	sys_thread_share_request(&pl_cs);

	playlist_array[idx].imarked = 1;

	sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * set item as unmarked.
 */
int audio_playlist_unmark(unsigned long idx)
{
	if(!playlist_initialized)return 0;
	if(playlist_count < idx)return 0;

	sys_thread_share_request(&pl_cs);
	
	playlist_array[idx].imarked = 0;

	sys_thread_share_release(&pl_cs);
	return 1;
}


/*
 * is item marked?
 */
int audio_playlist_ismarked(unsigned long idx)
{
	unsigned long im;

	if(!playlist_initialized)return 0;
	if(playlist_count < idx)return 0;

	sys_thread_share_request(&pl_cs);

	im = (playlist_array[idx].imarked == 1);

	sys_thread_share_release(&pl_cs);
	return im;
}


/*
 * get playlist items count.
 */
unsigned long audio_playlist_getcount(void)
{
	if(!playlist_initialized)return 0;
	return playlist_count;
}


/*
 * get source path.
 */
const string audio_playlist_getsource(unsigned long idx)
{
	string sp;

	if(!playlist_initialized)return 0;
	if(playlist_count <= idx)return 0;

	sys_thread_share_request(&pl_cs);

	if(playlist_array[idx].spath)
		sp = playlist_array[idx].spath;
	else
		sp = media_library_getsource(playlist_array[idx].medialibid);

	sys_thread_share_release(&pl_cs);
	return sp;
}


/*
 * get information (tags).
 */
int audio_playlist_getinfo(unsigned long idx, string sname, string sartist, string salbum, string sgenre, unsigned long* durationms, unsigned long* fsize)
{
	struct fennec_audiotag at;
	unsigned long          pid;
	string                 psource;

	sys_thread_share_request(&pl_cs);

	/* playlist item is a media library item or not (-1 = not) */
	if(sname == 0 && sartist == 0 && salbum == 0 && sgenre == 0 && durationms == 0 && fsize == 0)
	{
		int midr = (int)playlist_array[idx].medialibid;
		sys_thread_share_release(&pl_cs);
		return midr;
	}

	if(durationms)*durationms = 0;

	sys_thread_share_release(&pl_cs);

	psource = audio_playlist_getsource(idx);
	pid = audio_input_tagread(psource, &at);

	if(sname   && at.tag_title.tsize )str_cpy(sname,   at.tag_title.tdata );
	if(sartist && at.tag_artist.tsize)str_cpy(sartist, at.tag_artist.tdata);
	if(salbum  && at.tag_album.tsize )str_cpy(salbum,  at.tag_album.tdata );
	if(sgenre  && at.tag_genre.tsize )str_cpy(sgenre,  at.tag_genre.tdata );

	audio_input_tagread_known(pid, 0, &at); /* free */


	if(fsize)
	{
		t_sys_file_handle fhandle;

		fhandle = sys_file_openbuffering(audio_playlist_getsource(idx), v_sys_file_forread);

		if(fhandle != v_error_sys_file_open)
		{
			*fsize = sys_file_getsize(fhandle);
			sys_file_close(fhandle);
		}
	}

	
	return 0;
}


/*
 * switch to next track.
 */
int audio_playlist_next(void)
{
	/*
	unsigned long last_index = playlist_currentindex;
	*/

	if(!playlist_initialized)return 0;
		
	playlist_currentindex++;
	
	if(playlist_currentindex >= playlist_count)
	{
		if(settings.player.playlist_repeat_list)
		{
			playlist_currentindex = 0;
		}else{
			playlist_currentindex = playlist_count - 1;
			return 0;
		}
	}

	/*if(settings.player.playlist_shuffle)
	{
		if(playlist_currentindex == playlist_shufflefirst)
		{
			playlist_currentindex = last_index;
			return 0;
		}
	}*/

	audio_playlist_switch((unsigned long)-1);
	audio_play();
	return 1;
}


/*
 * switch to previous track.
 */
int audio_playlist_previous(void)
{
	unsigned long last_index = playlist_currentindex;

	if(!playlist_initialized)return 0;

	if(playlist_currentindex)
		playlist_currentindex--;
	
	if(last_index <= 0)
	{
		if(settings.player.playlist_shuffle)
		{
			//playlist_currentindex = playlist_count - 1;
		//}else{
			playlist_currentindex = 0;
			return 0;
		}
	}

	if(settings.player.playlist_shuffle)
	{
		if(playlist_currentindex < playlist_shufflefirst)
		{
			playlist_currentindex = last_index;
			return 0;
		}
	}

	audio_playlist_switch((unsigned long)-1);
	audio_play();
	return 1;
}


/*
 * get current index (shuffled).
 */
unsigned long audio_playlist_getcurrentindex(void)
{
	return audio_playlist_getrealindex(playlist_currentindex);
}


/*
 * directly switch into an item.
 */
int audio_playlist_switch(unsigned long idx)
{
	if(!playlist_initialized)return 0;
	if(!playlist_count)return 0;
	if(idx >= playlist_count && idx != (unsigned long)-1)return 0;

	if(idx == -1)
	{
		if(settings.player.playlist_shuffle)
		{
			playlist_array[audio_playlist_getcurrentindex()].imarked = 1;
			return audio_loadfile_soft(&playlist_changed, audio_playlist_getsource(audio_playlist_getcurrentindex()));
		}else{
			playlist_array[playlist_currentindex].imarked = 1;
			return audio_loadfile_soft(&playlist_changed, audio_playlist_getsource(playlist_currentindex));
		}
	}else{
		if(idx > playlist_count)return 0;
		if(settings.player.playlist_shuffle)
		{
			playlist_currentindex = playlist_array[idx].rindex;
			playlist_array[playlist_array[idx].rindex].imarked = 1;
			return audio_loadfile_soft(&playlist_changed, audio_playlist_getsource(playlist_array[idx].rindex));
		}else{
			playlist_currentindex = idx;
			playlist_array[idx].imarked = 1;
			return audio_loadfile_soft(&playlist_changed, audio_playlist_getsource(idx));
		}
	}
}


/*
 * switch into an item (by physical index).
 */
int audio_playlist_switch_list(unsigned long idx)
{
	if(!playlist_initialized)return 0;
	if(!playlist_count)return 0;
	if(idx >= playlist_count && idx != (unsigned long)-1)return 0;


	if(idx == -1)
	{
		return audio_loadfile_soft(&playlist_changed, audio_playlist_getsource(playlist_currentindex));
	}else{	
		if(settings.player.playlist_shuffle)
		{
			audio_playlist_shuffle(0);
		}

		playlist_currentindex = audio_playlist_getlistindex(idx);

		if(settings.player.playlist_shuffle)
		{
			unsigned long bkpi;

			/* this is a hack, that was just added this to make every item play.
			removing this can cause some (maybe, all!) files to be hidden. I'm not
			sure what I've done here, even I (chase) can't fully understand the playlist
			system. so... it says that this system should be rewritten! (maybe
			I'd chosen the confusing way to speed things up... or... I dunno ;-) ) */

			bkpi = playlist_array[0].rindex;
			playlist_array[0].rindex = idx;
			playlist_array[playlist_currentindex].rindex = bkpi;

			playlist_currentindex = 0;
		}


		return audio_loadfile_soft(&playlist_changed, audio_playlist_getsource(idx));
	}
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
